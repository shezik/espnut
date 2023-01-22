/*
 $Id$
 Copyright 1995, 2003, 2004, 2005 Eric L. Smith <eric@brouhaha.com>

 Nonpareil is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License version 2 as
 published by the Free Software Foundation.  Note that I am not
 granting permission to redistribute or modify Nonpareil under the
 terms of any later version of the General Public License.

 Nonpareil is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program (in the file "COPYING"); if not, write to the
 Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 MA 02111, USA.
 */

//
// changes for mac os x by Maciej Bartosiak
// changes for esp32 by shezik
//

#pragma once

#include <Arduino.h>
#include <Esp.h>
#include "KeyboardMan.h"

void exit(int ret);
void *alloc(size_t size);
void trim_trailing_whitespace(char *s);

#define PRINTF_BUF_LEN 256

// An alias of printf_log
#define fprintf(stream, format, ...)       \
    do                                     \
    {                                      \
        printf_log(#stream ": " format, ##__VA_ARGS__); \
    } while (0)

#define printf_log(format, ...)                            \
    do                                                     \
    {                                                      \
        extern void appendLog(char *);                     \
        char buf[PRINTF_BUF_LEN];                          \
        snprintf(buf, sizeof(buf), format, ##__VA_ARGS__); \
        printf("%s", buf);                                 \
        appendLog(buf);                                    \
    } while (0)

// Uses 'do while (0)' to wrap up multiple lines nicely, and limit scope of variables!
// Print, log and show crash message, then halt
#define fatal(ret, format, ...)                                                                                                                \
    do                                                                                                                                         \
    {                                                                                                                                          \
        extern void appendLog(char *);                                                                                                         \
        extern void U8g2DrawAndSendDialog(char *);                                                                                             \
        char buf[PRINTF_BUF_LEN];                                                                                                              \
        snprintf(buf, sizeof(buf), "\nFatal error at file %s, func %s, line %s: \n" format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        printf("%s", buf);                                                                                                                     \
        appendLog(buf);                                                                                                                        \
        U8g2DrawAndSendDialog(buf);                                                                                                            \
        exit(ret);                                                                                                                             \
    } while (0)

/*
#define warning(format, ...) \
    printf("\nWarning: \n" format "\n", ##__VA_ARGS__)
*/
