/*

       SSS       A     I L      M    M      A       X   X
      S         A A    I L      MM  MM     A A       X X
        S      A   A   I L      M MM M    A   A       X
          S   AAAAAAA  I L      M    M   AAAAAAA     X X
      SSS    A       A I LLLLL  M    M  A       A   X   X

      * Project:  Sailmax-CU, N2k Log Reader-Writer
      * Purpose:  reading propriatary Sailmax log format from SD-Card,
      *           writing to NMEA2000 bus for Teensy 3.6
      * Author:   © Ronnie Zeiller, 2018

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

#define N2k_CAN_INT_PIN 21

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <NMEA2000_CAN.h>
#include <N2kMessages.h>
#include <SailmaxFormat.h>

typedef uint16_t pin_t;
static const pin_t sdcard_cs = 15;
static const char *logFilename = "RPC2018.log";

bool sdCardInit();
void errorHalt(const char* msg);

void readAndSendMsg();
void testReadFile();

static SdFatSdio sd;
SdFile logFile;

char line[223];
uint32_t pos;
tN2kMsg msg;
bool sendNext = true;

int er;
uint32_t n = 0;
uint32_t timeSent = 0;
int32_t delayTime = 0;
uint32_t timeNext = 0;
int64_t delta     = 0;

uint64_t lineNumber = 0;
int8_t mode = 1;

void setup() {
  Serial.begin(115200);
  Serial.printf("Starting with LogFile: %s\n", logFilename);
  pinMode(LED_BUILTIN, OUTPUT);

  // Start Header
  Serial.println();
  Serial.println(" ******************************************************************");
  Serial.println(" *    Sailmax-CU, N2k Log Reader-Writer, © Ronnie Zeiller, 2018   *");
  Serial.println(" ******************************************************************");
  Serial.println();
  delay(1000);

  SdFatSdio getSdFat();
  sdCardInit();


  if (sd.exists(logFilename)) {
    if (!logFile.open(logFilename, O_READ)) {
      errorHalt("open failed");
    }
  } else {
    errorHalt("Logfile does not exist");
  }

  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly);
  //NMEA2000.SetMode(tNMEA2000::N2km_ListenAndSend);
  NMEA2000.SetN2kCANMsgBufSize(8);
  NMEA2000.SetN2kCANSendFrameBufSize(150);
  NMEA2000.SetForwardStream(&Serial);  // PC output on due native port
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text
  // NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)

  //NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.Open();

  while ((er = logFile.fgets(line, sizeof(line))) > 0) {
    n++;
    if (SailmaxToN2k(line, timeNext, msg)){
      if (n == 1) {
        delayTime = 0;
        delta     = millis() - timeNext;  // difference millis to timestamp from 1st sentence in logFile
      } else {
        delayTime = timeNext - timeSent + (delta - millis() + timeNext) - 40;
        //Serial.printf("Line: %d Differenz: %d, Delay: %d\n", n, millis() - timeNext, delayTime);
        if (delayTime > 0) delay(delayTime);
      }
      Serial.printf("%d : ", n);
      NMEA2000.SendMsg(msg, -1);

      timeSent = timeNext;
    } else {
      Serial.printf("Could not convert line %d\n",n);
    }
  }
  if (n <= 1) {
    Serial.printf("Could not read LogFile\n");
  } else {
    Serial.printf("End of LogFile\n");
  }

}

void loop() {}


bool sdCardInit() {
  if (!sd.begin()){
    if (sd.card()->errorCode()) {
      Serial.printf("Something went wrong ... SD card errorCode: %i errorData: %i\n", sd.card()->errorCode(), sd.card()->errorData());
      return false;
    }

    if (!sd.vwd()->isOpen()) {
      Serial.printf("Can't open root directory. Try reformatting the card.\n");
      return false;
    }

    Serial.printf("Unknown error while initializing card.\n");
    return false;
  }
  else {
    Serial.printf("SdCard successfully initialized.\n");
  }
  return true;
}

//-----------------------------------------------------------------------------
void errorHalt(const char* msg) {
  sd.errorHalt(msg);
}
