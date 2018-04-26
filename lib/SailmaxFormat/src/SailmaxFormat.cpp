/*

       SSS       A     I L      M    M      A       X   X
      S         A A    I L      MM  MM     A A       X X
        S      A   A   I L      M MM M    A   A       X
          S   AAAAAAA  I L      M    M   AAAAAAA     X X
      SSS    A       A I LLLLL  M    M  A       A   X   X

      * Project:  Sailmax-CU, Sailmax Format Conversions
      * Purpose:  reading and writing propriatary Sailmax format
      * Author:   © Ronnie Zeiller, 2018
      * Some code taken from the Seasmart conversion by Thomas Sarlandie
      * Format is similar to Seasmart, but easier readable by humans to analyze
      * what is really going on the N2k bus.
      * @timestamp,PGN,Source,Data*checksum
      * @22643312,128267,23,DB28010000A0F6FF*28

The MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "SailmaxFormat.h"

/* Some private helper functions to generate hex-serialized NMEA messages */
static const char *hex = "0123456789ABCDEF";

/*============================================================================*/
/* Convert unsigned long value to d-digit decimal string in local buffer      */
/*============================================================================*/
char *u2s(unsigned long x,unsigned d){
  static char b[16];
  char *p;
  unsigned digits = 0;
  unsigned long t = x;

  do ++digits; while (t /= 10);
  // if (digits > d) d = digits; // uncomment to allow more digits than spec'd
  *(p = b + d) = '\0';
  do *--p = x % 10 + '0'; while (x /= 10);
  while (p != b) *--p = ' ';
  return b;
}

static int appendByte(char *s, uint8_t byte) {
  s[0] = hex[byte >> 4];
  s[1] = hex[byte & 0xf];
  return 2;
}

static uint8_t nmea_compute_checksum(const char *sentence) {
  if (sentence == 0) {
    return 0;
  }

  // Skip the $ at the beginning
  int i = 1;

  int checksum = 0;
  while (sentence[i] != '*') {
    checksum ^= sentence[i];
    i++;
  }
  return checksum;
}

/*
 * Attemts to read n bytes in hexadecimal from input string to value.
 * Returns true if successful, false otherwise.
 */
static bool readNHexByte(const char *s, unsigned int n, uint32_t &value) {
  if (strlen(s) < 2*n) {
    return -1;
  }
  for (unsigned int i = 0; i < 2*n; i++) {
    if (!isxdigit(s[i])) {
      return -1;
    }
  }

  char sNumber[2*n + 1];
  strncpy(sNumber, s, sizeof(sNumber));
  sNumber[sizeof(sNumber) - 1] = 0;

  value = strtol(sNumber, 0, 16);
  return true;
}

size_t N2kToSailmax(const tN2kMsg &msg, uint32_t timestamp, char *buffer, size_t size) {
  //size_t pcdin_sentence_length = 6+1+6+1+8+1+2+1+msg.DataLen*2+1+2 + 1;
  uint32_t number = timestamp;
  int digits = 0; do { number /= 10; digits++; } while (number != 0);
  // @millis,PGN,Source,Data*checksum
  // @22643312,128267,23,DB28010000A0F6FF*28
  size_t pcdin_sentence_length = 1+digits+1+6+1+2+1+msg.DataLen*2+1+2 + 1;
  if (size < pcdin_sentence_length) {
    return 0;
  }

  char *s = buffer;
  sprintf(s, "@%lu,",timestamp );

  s += digits + 2;
  sprintf(s,"%06lu,",msg.PGN );

  s += 7;
  s += appendByte(s, msg.Source);
  *s++ = ',';

  for (int i = 0; i < msg.DataLen; i++) {
    s += appendByte(s, msg.Data[i]);
  }

  *s++ = '*';
  s += appendByte(s, nmea_compute_checksum(buffer));
  *s = 0;
  return (size_t)(s - buffer);
}

bool SailmaxToN2k(const char *buffer, uint32_t &timestamp, tN2kMsg &msg) {

  msg.Clear();
  msg.Destination = 0xFF;

  const char *s = buffer;
  int i = 0;

  // check if line starts with @
  if (*s == '@') {
    i = 1;

    timestamp = 0;
    while (s[i] != ',') {
      timestamp = timestamp * 10 + ((int)s[i] - 48);
      i++;
    }
    msg.MsgTime = timestamp;

    i ++;
    // the next 6 chars are the PGN (N2k page number)
    msg.PGN = 0;
    while (s[i] != ',') {
      msg.PGN = msg.PGN * 10 + ((int)s[i] - 48);
      i++;
    }

    i ++; // skip the comma
    s += i;
    uint32_t source;
    if (!readNHexByte(s, 1, source)) {
      return false;
    }
    msg.Source = source;

    s += 3;
    int dataLen = 0;
    while (s[dataLen] != 0 && s[dataLen] != '*') {
      dataLen++;
    }
    if (dataLen % 2 != 0) {
      return false;
    }
    dataLen /= 2;

    msg.DataLen = dataLen;
    if (msg.DataLen > msg.MaxDataLen) {
      return false;
    }

    for (int j = 0; j < dataLen; j++) {
      uint32_t byte;
      if (!readNHexByte(s, 1, byte)) {
        return false;
      }
      msg.Data[j] = byte;
      s += 2;
    }

    s += 1;
    uint32_t checksum;
    //Serial.printf("checksum s: %s\n", s);
    if (!readNHexByte(s, 1, checksum)) {
      Serial.printf("readNHexByte nicht ok \n");
      return false;
    }

    if (checksum != nmea_compute_checksum(buffer)) {
      Serial.printf("nmea_compute_checksum nicht ok \n");
      return false;
    }

    return true;
  } else {
    return false;
  }
}
