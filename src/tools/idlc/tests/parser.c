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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "CUnit/Runner.h"

#include "parser.h"


bool test_parse(const char *str, bool okay)
{
  int result = idl_parse_string(str, /*ignore_yyerror=*/!okay);
  if (okay && result != 0) {
    return false;
  } else if (!okay && result == 0) {
    return false;
  }
  return true;
}

CUnit_Test(parser, basic)
{

  CU_ASSERT(test_parse("", false));
  CU_ASSERT(test_parse("??", false));
  CU_ASSERT(test_parse("const short een = 1;", true));
  CU_ASSERT(test_parse("const short een = 1 + 1;", true));
  CU_ASSERT(test_parse("const short een = 3 - 1;", true));
  CU_ASSERT(test_parse("const short een = 2 * 3;", true));
  CU_ASSERT(test_parse("const short een = 12 / 3;", true));
  CU_ASSERT(test_parse("const short een = 13 % 3;", true));
  CU_ASSERT(test_parse("const short een = 2 << 3;", true));
  CU_ASSERT(test_parse("const short een = 16 >> 3;", true));
  CU_ASSERT(test_parse("const short een = 6 & 3;", true));
  CU_ASSERT(test_parse("const short een = 6 | 3;", true));
  CU_ASSERT(test_parse("const short een = 6 ^ 3;", true));
  CU_ASSERT(test_parse("const short een = 6 + +3;", true));
  CU_ASSERT(test_parse("const short een = 6 - - 3;", true));
  CU_ASSERT(test_parse("const short een = 6 + ~(-3);", true));
}

bool test_parse_stringify(const char *input, const char *output)
{
  char buffer[1000];
  idl_parse_string_stringify(input, buffer, 500);
  return strcmp(buffer, output) == 0;
}

CUnit_Test(parser, module)
{

  CU_ASSERT(test_parse_stringify("enum e{x, y};", "enum e{x,y,}"));
  CU_ASSERT(test_parse_stringify("enum e{x, z};", "enum e{x,z,}"));
  CU_ASSERT(test_parse_stringify("const short een = 1;", ""));
  //CU_ASSERT(test_parse_stringify("module a { enum e{x};};", "module a{enum e{x,}}"));
}

