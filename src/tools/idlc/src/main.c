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

#include "parser.h"

static void
usage(const char *prog)
{
  fprintf(stderr, "Usage: %s FILE\n", prog);
}

void run_test();

int
main(int argc, char *argv[])
{
  int ret = EXIT_FAILURE;

  if (argc == 1) {
    run_test();
  }
  if (argc != 2) {
    usage(argv[0]);
  } else if (dds_tt_parse_file(argv[1]) == 0) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

#include <string.h>

bool test_parse_stringify(const char *input, const char *output)
{
  char buffer[1000];
  buffer[0] = '\0';
  dds_tt_parse_string_stringify(input, buffer, 1000);
  if (strcmp(buffer, output) == 0) {
    return true;
  }
  fprintf(stderr, "Parsing: '%s'\n", input);
  fprintf(stderr, "Expect:  '%s'\n", output);
  fprintf(stderr, "Result:  '");
  for (char *s = buffer; *s != '\0'; s++)
    if (*s < ' ')
      fprintf(stderr, "<%02x>", *s);
    else
      fprintf(stderr, "%c", *s);
  fprintf(stderr, "'\n");
  fprintf(stderr, "%s\n", strcmp(buffer, output) == 0 ? "OK" : "ERROR"); 
  return false;
}

#define CU_ASSERT(X) X
void run_test()
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
  CU_ASSERT(test_parse_stringify("struct s {map<short,map<char,short>,5> c;};","struct s{map<short,map<char,short>,5> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {map<map<char,short>,short,5> c;};","struct s{map<map<char,short>,short,5> c,;}"));
  CU_ASSERT(test_parse_stringify("struct s {char c,b;};","struct s{char c,b,;}"));
  CU_ASSERT(test_parse_stringify("struct s {char c;wchar d,e;};","struct s{char c,;wchar d,e,;}"));
  //CU_ASSERT(test_parse_stringify("struct s {struct x{char c;};};", "struct s{struct x{char c,}}"));
  CU_ASSERT(test_parse_stringify("union u switch (char) {case 'c': char d; default: short e;};","union u switch(char){c:char d;d:short e;}"));
}

