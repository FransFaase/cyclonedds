/*
 * Copyright(c) 2006 to 2019 ADLINK Technology Limited and others
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
#include "os/os.h"
#include "CUnit/Test.h"

#include "dds/ddsts/typetree.h"
#include "parser.h"
#include "stringify.h"

static void report_error(int line, int column, const char *msg)
{
  fprintf(stderr, "ERROR %d.%d: %s\n", line, column, msg);
}

bool test_parse_stringify(const char *input, const char *output)
{
  ddsts_type_t *root_type = NULL;
  if (ddsts_parse_string(input, report_error, &root_type) != 0) {
    return false;
  }

  char buffer[1000];
  buffer[0] = '\0';

  ddsts_stringify(root_type, buffer, 1000);

  ddsts_free_type(root_type);

  if (strcmp(buffer, output) == 0)
  {
    return true;
  }
  /* In case of a difference, print some information (for debugging) */
  printf("Expect:   |%s|\n"
         "Returned: |%s|\n", output, buffer);
  return false;
}

static bool test_type(ddsts_type_t *type, ddsts_flags_t flags, const char *name, ddsts_type_t *parent, bool next_is_null)
{
  return    type != NULL && type->type.flags == flags && (name == NULL ? type->type.name == NULL : type->type.name != NULL && strcmp(type->type.name, name) == 0)
         && type->type.parent == parent
         && (!next_is_null || type->type.next == NULL);
}

static void test_basic_type(const char *idl, ddsts_flags_t flags)
{
  ddsts_type_t *root_type = NULL;
  CU_ASSERT(ddsts_parse_string(idl, report_error, &root_type) == 0);
  CU_ASSERT(test_type(root_type, DDSTS_MODULE, NULL, NULL, true));
  CU_ASSERT(root_type->module.previous == NULL);
    ddsts_type_t *struct_s = root_type->module.members;
    CU_ASSERT(test_type(struct_s, DDSTS_STRUCT, "s", root_type, true));
      ddsts_type_t *decl_c = struct_s->struct_def.members;
      CU_ASSERT(test_type(decl_c, DDSTS_DECLARATION, "c", struct_s, true));
        ddsts_type_t *char_type = decl_c->declaration.decl_type;
        CU_ASSERT(test_type(char_type, flags, NULL, decl_c, true));
  ddsts_free_type(root_type);
}

CU_Test(parser, basic_types)
{
  test_basic_type("struct s{boolean c;};", DDSTS_BOOLEAN);
  test_basic_type("struct s{char c;};", DDSTS_CHAR);
  test_basic_type("struct s{wchar c;};", DDSTS_WIDE_CHAR);
  test_basic_type("struct s{short c;};", DDSTS_SHORT);
  test_basic_type("struct s{int16 c;};", DDSTS_SHORT);
  test_basic_type("struct s{long c;};", DDSTS_LONG);
  test_basic_type("struct s{int32 c;};", DDSTS_LONG);
  test_basic_type("struct s{long long c;};", DDSTS_LONGLONG);
  test_basic_type("struct s{int64 c;};", DDSTS_LONGLONG);
  test_basic_type("struct s{unsigned short c;};", DDSTS_USHORT);
  test_basic_type("struct s{uint16 c;};", DDSTS_USHORT);
  test_basic_type("struct s{unsigned long c;};", DDSTS_ULONG);
  test_basic_type("struct s{uint32 c;};", DDSTS_ULONG);
  test_basic_type("struct s{unsigned long long c;};", DDSTS_ULONGLONG);
  test_basic_type("struct s{uint64 c;};", DDSTS_ULONGLONG);
  test_basic_type("struct s{octet c;};", DDSTS_OCTET);
  test_basic_type("struct s{int8 c;};", DDSTS_INT8);
  test_basic_type("struct s{uint8 c;};", DDSTS_UINT8);
  test_basic_type("struct s{float c;};", DDSTS_FLOAT);
  test_basic_type("struct s{double c;};", DDSTS_DOUBLE);
  test_basic_type("struct s{long double c;};", DDSTS_LONGDOUBLE);
}

CU_Test(parser, one_module1)
{
  ddsts_type_t *root_type = NULL;
  CU_ASSERT(ddsts_parse_string("module a{ struct s{char c;};};", report_error, &root_type) == 0);
  CU_ASSERT(test_type(root_type, DDSTS_MODULE, NULL, NULL, true));
  CU_ASSERT(root_type->module.previous == NULL);
    ddsts_type_t *module_a = root_type->module.members;
    CU_ASSERT(test_type(module_a, DDSTS_MODULE, "a", root_type, true));
    CU_ASSERT(module_a->module.previous == NULL);
      ddsts_type_t *struct_s = module_a->module.members;
      CU_ASSERT(test_type(struct_s, DDSTS_STRUCT, "s", module_a, true));
        ddsts_type_t *decl_c = struct_s->struct_def.members;
        CU_ASSERT(test_type(decl_c, DDSTS_DECLARATION, "c", struct_s, true));
          ddsts_type_t *char_type = decl_c->declaration.decl_type;
          CU_ASSERT(test_type(char_type, DDSTS_CHAR, NULL, decl_c, true));
  ddsts_free_type(root_type);
}

CU_Test(parser, reopen_module)
{
  ddsts_type_t *root_type = NULL;
  CU_ASSERT(ddsts_parse_string("module a{ struct s{char c;};}; module a { struct t{char x;};};", report_error, &root_type) == 0);
  CU_ASSERT(test_type(root_type, DDSTS_MODULE, NULL, NULL, true));
  CU_ASSERT(root_type->module.previous == NULL);
    ddsts_type_t *module_a = root_type->module.members;
    CU_ASSERT(test_type(module_a, DDSTS_MODULE, "a", root_type, false));
    CU_ASSERT(module_a->module.previous == NULL);
    {
      ddsts_type_t *struct_s = module_a->module.members;
      CU_ASSERT(test_type(struct_s, DDSTS_STRUCT, "s", module_a, true));
        ddsts_type_t *decl_c = struct_s->struct_def.members;
        CU_ASSERT(test_type(decl_c, DDSTS_DECLARATION, "c", struct_s, true));
          ddsts_type_t *char_type = decl_c->declaration.decl_type;
          CU_ASSERT(test_type(char_type, DDSTS_CHAR, NULL, decl_c, true));
    }
    ddsts_type_t *module_a2 = module_a->type.next;
    CU_ASSERT(test_type(module_a2, DDSTS_MODULE, "a", root_type, true));
    CU_ASSERT(module_a2->module.previous == &module_a->module);
    {
      ddsts_type_t *struct_t = module_a2->module.members;
      CU_ASSERT(test_type(struct_t, DDSTS_STRUCT, "t", module_a2, true));
        ddsts_type_t *decl_x = struct_t->struct_def.members;
        CU_ASSERT(test_type(decl_x, DDSTS_DECLARATION, "x", struct_t, true));
          ddsts_type_t *char_type = decl_x->declaration.decl_type;
          CU_ASSERT(test_type(char_type, DDSTS_CHAR, NULL, decl_x, true));
    }
  ddsts_free_type(root_type);
}

CU_Test(parser, comma)
{
  ddsts_type_t *root_type = NULL;
  CU_ASSERT(ddsts_parse_string("struct s{char a, b;};", report_error, &root_type) == 0);
  CU_ASSERT(test_type(root_type, DDSTS_MODULE, NULL, NULL, true));
  CU_ASSERT(root_type->module.previous == NULL);
    ddsts_type_t *struct_s = root_type->module.members;
    CU_ASSERT(test_type(struct_s, DDSTS_STRUCT, "s", root_type, true));
      ddsts_type_t *decl_a = struct_s->struct_def.members;
      CU_ASSERT(test_type(decl_a, DDSTS_DECLARATION, "a", struct_s, false));
        ddsts_type_t *char_type = decl_a->declaration.decl_type;
        CU_ASSERT(test_type(char_type, DDSTS_CHAR, NULL, decl_a, true));
      ddsts_type_t *decl_b = decl_a->type.next;
      CU_ASSERT(test_type(decl_b, DDSTS_DECLARATION, "b", struct_s, true));
        CU_ASSERT(decl_b->declaration.decl_type == char_type);
  ddsts_free_type(root_type);
}

UC_Test(parser, sequences)
{
  ddsts_type_t *root_type = NULL;
  CU_ASSERT(ddsts_parse_string("struct s{sequence<char> us; sequence<char,8> bs; string ust; string<7> bst; wstring uwst; wstring<7> bwst;};", report_error, &root_type) == 0);
  CU_ASSERT(test_type(root_type, DDSTS_MODULE, NULL, NULL, true));
  CU_ASSERT(root_type->module.previous == NULL);
    ddsts_type_t *struct_s = root_type->module.members;
    CU_ASSERT(test_type(struct_s, DDSTS_STRUCT, "s", root_type, true));
      ddsts_type_t *decl = struct_s->struct_def.members;
      CU_ASSERT(test_type(decl, DDSTS_DECLARATION, "us", struct_s, false));
        ddsts_type_t *s_type = decl->declaration.decl_type;
        CU_ASSERT(test_type(s_type, DDSTS_, NULL, decl, true));
      decl = decl->type.next;
      CU_ASSERT(test_type(decl_b, DDSTS_DECLARATION, "bs", struct_s, false));
        s_type = decl->declaration.decl_type;
        CU_ASSERT(test_type(s_type, DDSTS_, NULL, decl, true));
      decl = decl->type.next;
      CU_ASSERT(test_type(decl_b, DDSTS_DECLARATION, "ust", struct_s, false));
        s_type = decl->declaration.decl_type;
        CU_ASSERT(test_type(s_type, DDSTS_, NULL, decl, true));
      decl = decl->type.next;
      CU_ASSERT(test_type(decl_b, DDSTS_DECLARATION, "bst", struct_s, false));
        s_type = decl->declaration.decl_type;
        CU_ASSERT(test_type(s_type, DDSTS_, NULL, decl, true));
      decl = decl->type.next;
      CU_ASSERT(test_type(decl_b, DDSTS_DECLARATION, "uwst", struct_s, false));
        s_type = decl->declaration.decl_type;
        CU_ASSERT(test_type(s_type, DDSTS_, NULL, decl, true));
      decl = decl->type.next;
      CU_ASSERT(test_type(decl_b, DDSTS_DECLARATION, "bwst", struct_s, true));
        s_type = decl->declaration.decl_type;
        CU_ASSERT(test_type(s_type, DDSTS_, NULL, decl, true));
  ddsts_free_type(root_type);
}

/*
UC_Test(parser, module26)
{
  CU_ASSERT(test_parse_stringify("struct s {sequence<short> us; sequence<short> bs};",""));
}

UC_Test(parser, module27)
{
  CU_ASSERT(test_parse_stringify("struct s {sequence<short,7> c;};",""));
}

UC_Test(parser, module28)
{
  CU_ASSERT(test_parse_stringify("struct s {string c;};",""));
}

UC_Test(parser, module29)
{
  CU_ASSERT(test_parse_stringify("struct s {string<9> c;};",""));
}

UC_Test(parser, module30)
{
  CU_ASSERT(test_parse_stringify("struct s {wstring c;};",""));
}

UC_Test(parser, module31)
{
  CU_ASSERT(test_parse_stringify("struct s {wstring<9> c;};",""));
}

UC_Test(parser, module32)
{
  CU_ASSERT(test_parse_stringify("struct s {fixed<5,3> c;};",""));
}

UC_Test(parser, module33)
{
  CU_ASSERT(test_parse_stringify("struct s {map<short,char> c;};",""));
}

UC_Test(parser, module34)
{
  CU_ASSERT(test_parse_stringify("struct s {map<short,char,5> c;};",""));
}

UC_Test(parser, module35)
{
  CU_ASSERT(test_parse_stringify("struct s {char c,b;};",""));
}

UC_Test(parser, module36)
{
  CU_ASSERT(test_parse_stringify("struct s {char c;wchar d,e;};",""));
}

UC_Test(parser, module37)
{
  CU_ASSERT(test_parse_stringify("struct a{char c;};struct b{sequence<a> s;};",
                                 ""));
}
*/
