/*
 * Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include <assert.h>
#include <stdlib.h>

#include "tools.h"


static int
toxdigit_c(int c)
{
  if (c >= '0' || c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'z') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'Z') {
    return c - 'A' + 10;
  }
  return -1;
}

static int
isxdigit_c(int c)
{
  if ((c >= 'a' && c <= 'f') ||
      (c >= 'A' && c <= 'F') ||
      (c >= '0' && c <= '9'))
  {
    return 1;
  }

  return 0;
}

unsigned long
dds_ts_unescape_wchar(const char *str, const char **endptr)
{
  int i;
  unsigned long chr = 0;
  const char *end;

  assert(str != NULL);

  if (str[0] == '\\') {
    if (str[1] >= '0' && str[1] <= '7') {
      for (i = 1; i <= 3 && (str[i] >= '0' && str[i] <= '7'); i++) {
        chr = (chr * 8) + (unsigned long)(str[i] - '0');
      }
      end = str + i;
    } else if ((str[1] == 'x' || str[1] == 'X') && isxdigit_c(str[2])) {
      for (i = 2; i <= 3 && isxdigit_c(str[i]); i++) {
        chr = (chr * 16) + (unsigned long)toxdigit_c(str[i]);
      }
      end = str + i;
    } else if (str[1] == 'u' && isxdigit_c(str[2])) {
      for (i = 2; i <= 6 && isxdigit_c(str[i]); i++) {
        chr = (chr * 16) + (unsigned long)toxdigit_c(str[i]);
      }
      end = str + i;
    } else {
      switch (str[1]) {
        case 'n':  chr = '\n'; break;
        case 't':  chr = '\t'; break;
        case 'v':  chr = '\v'; break;
        case 'b':  chr = '\b'; break;
        case 'r':  chr = '\r'; break;
        case 'f':  chr = '\f'; break;
        case 'a':  chr = '\a'; break;
        case '\\': chr = '\\'; break;
        case '?':  chr = '\?'; break;
        case '\'': chr = '\''; break;
        case '"':  chr = '"';  break;
        default:
          chr = (unsigned long)str[1];
          break;
      }
      end = str + 2;
    }
  } else {
    chr = (unsigned long)str[0];
    end = str + 1;
  }

  if (endptr != NULL) {
    *endptr = end;
  }

  return chr;
}

char
dds_ts_unescape_char(const char *str, const char **endptr)
{
  unsigned long wchar = dds_ts_unescape_wchar(str, endptr);
  return (char)(unsigned char)(wchar & 0xff);
}
