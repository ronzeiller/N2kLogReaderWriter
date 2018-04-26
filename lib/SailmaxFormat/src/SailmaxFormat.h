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
      * wath is really going on the N2k bus.

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
#ifndef _SailmaxFormat_h_
#define _SailmaxFormat_h_

#include <N2kMsg.h>

/**
 *  Converts a tN2kMsg into a proprietary Sailmax-Sentence used for logging
 *
 *  If the buffer is not long enough, this function returns 0 and does not do
 *  anything.
 *
 *  If the buffer is long enough, this function returns the number of bytes
 *  written including the terminating \0 (but this function does not add the
 *  Return/LineFeed \r\n).
 */
size_t N2kToSailmax(const tN2kMsg &msg, uint32_t timestamp, char *buffer, size_t size);

/**
 *    Converts Sailmax sentence to N2k message
 */
bool SailmaxToN2k(const char *buffer, uint32_t &timestamp, tN2kMsg &msg);

#endif
