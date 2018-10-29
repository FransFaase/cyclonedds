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
tolower_c(int c)
{
  if (c >= 'A' && c <= 'Z') {
    return c - 'A';
  }

  return c;
}

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

#define lc(c) tolower_c(c)

int
idl_strcasecmp_c(const char *s1, const char *s2)
{
  int eq;

  assert(s1 != NULL);
  assert(s2 != NULL);

  eq = tolower_c(*s1) - tolower_c(*s2);
  while (*s1 && *s2 && eq == 0) {
    s1++;
    s2++;
    eq = tolower_c(*s1) - tolower_c(*s2);
  }

  return eq;
}

int
idl_strncasecmp_c(const char *s1, const char *s2, size_t n)
{
  size_t i;
  int eq = 0;

  assert(s1 != NULL);
  assert(s2 != NULL);

  for (i = 0;
       i < n && (eq = lc(s1[i]) - lc(s2[i])) == 0 && s1[i] && s2[i];
       i++)
  {
    /* do nothing */
  }

  return eq;
}

#undef lc

int
idl_unescape_char(const char *str, const char **endptr)
{
  int i, chr = 0;
  const char *end;

  assert(str != NULL);

  if (str[0] == '\\') {
    if (str[1] >= '0' && str[1] <= '7') {
      for (i = 1; i <= 3 && (str[i] >= '0' && str[i] <= '7'); i++) {
        chr = (chr * 8) + (str[i] - '0');
      }
      end = str + i;
    } else if ((str[1] == 'x' || str[1] == 'X') && isxdigit_c(str[2])) {
      for (i = 2; i <= 3 && isxdigit_c(str[i]); i++) {
        chr = (chr * 16) + toxdigit_c(str[i]);
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
          chr = str[1];
          break;
      }
      end = str + 2;
    }
  } else {
    chr = str[0];
    end = str + 1;
  }

  if (endptr != NULL) {
    *endptr = end;
  }

  return chr;
}
