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
#include "CUnit/Test.h"

#include "parser.h"


bool test_parse(const char *str, bool okay)
{
  int result = dds_tt_parse_string(str, /*ignore_yyerror=*/!okay);
  if (okay && result != 0) {
    return false;
  } else if (!okay && result == 0) {
    return false;
  }
  return true;
}

CU_Test(parser, basic)
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
  dds_tt_parse_string_stringify(input, buffer, 500);
  return strcmp(buffer, output) == 0;
}

CU_Test(parser, module)
{

  CU_ASSERT(test_parse_stringify("enum e{x, z};", "enum e{x,z,}"));
  CU_ASSERT(test_parse_stringify("module a { enum e{x};};", "module a{enum e{x,}}"));
  CU_ASSERT(test_parse_stringify("module a{enum f{y};}; module a { enum e{x};};", "module a{enum f{y,}}module a{enum e{x,}}"));
  CU_ASSERT(test_parse_stringify("module x{module a{enum f{y};};};", "module x{module a{enum f{y,}}}"));
  CU_ASSERT(test_parse_stringify("struct s {boolean c;};","struct s{bool c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {char c;};","struct s{char c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {wchar c;};","struct s{wchar c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {short c;};","struct s{short c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {int16 c;};","struct s{short c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {long c;};","struct s{long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {int32 c;};","struct s{long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {long long c;};","struct s{long long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {int64 c;};","struct s{long long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {unsigned short c;};","struct s{unsigned short c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {uint16 c;};","struct s{unsigned short c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {unsigned long c;};","struct s{unsigned long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {uint32 c;};","struct s{unsigned long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {unsigned long long c;};","struct s{unsigned long long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {uint64 c;};","struct s{unsigned long long c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {octet c;};","struct s{octet c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {int8 c;};","struct s{int8 c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {uint8 c;};","struct s{uint8 c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {float c;};","struct s{float c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {double c;};","struct s{double c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {long double c;};","struct s{long double c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {sequence<short> c;};","struct s{sequence<short> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {sequence<short,7> c;};","struct s{sequence<short,7> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {string c;};","struct s{string c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {string<9> c;};","struct s{string<9> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {wstring c;};","struct s{wstring c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {wstring<9> c;};","struct s{wstring<9> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {fixed<5,3> c;};","struct s{fixed<5,3> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {map<short,char> c;};","struct s{map<short,char> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {map<short,char,5> c;};","struct s{map<short,char,5> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {char c,b;};","struct s{char c,b,;}"));
  CU_ASSERT(test_parse_stringify("struct s {char c;wchar d,e;};","struct s{char c,;wchar d,e,;}"));
  CU_ASSERT(test_parse_stringify("union u switch (char) {case 'c': char d; default: short e;};","union u switch(char){c:char d;d:short e;}"));
}

