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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "os/os.h"
#include "dds/ddsts/typetree.h"

#define YYSTYPE DDSTS_PARSER_STYPE
#define YYLTYPE DDSTS_PARSER_LTYPE

#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;

#define YY_NO_UNISTD_H 1 /* to surpress #include <unistd.h> */

#include "parser.h"
#include "idl.parser.h"
#include "yy_decl.h"
#include "idl.lexer.h"

extern int
ddsts_parse_file(const char *file, void (*error_func)(int line, int column, const char *msg), ddsts_type_t **ref_root_type)
{
  *ref_root_type = NULL;

  int err = 0;
  FILE *fh;

  assert(file != NULL);

OS_WARNING_MSVC_OFF(4996);
  fh = fopen(file, "rb");
OS_WARNING_MSVC_ON(4996);

  if (fh == NULL) {
    err = errno;
    if (error_func != 0) {
      error_func(0, 0, "Cannot open file");
    }
  }
  else {
    ddsts_context_t *context = ddsts_create_context();
    if (context == NULL) {
      if (error_func != 0) {
        error_func(0, 0, "Error: out of memory\n");
      }
      return 2;
    }
    ddsts_context_set_error_func(context, error_func);
    yyscan_t scanner;
    ddsts_parser_lex_init(&scanner);
    ddsts_parser_set_in(fh, scanner);
    err = ddsts_parser_parse(scanner, context);
    if (err == 0) {
      *ref_root_type = ddsts_context_take_root_type(context);
    }
    else if (ddsts_context_get_out_of_memory_error(context)) {
      if (error_func != 0) {
        error_func(0, 0, "Error: out of memory\n");
      }
    }
    ddsts_free_context(context);
    ddsts_parser_lex_destroy(scanner);
    (void)fclose(fh);
  }

  return err;
}

extern int
ddsts_parse_string(const char *str, void (*error_func)(int line, int column, const char *text), ddsts_type_t **ref_root_type)
{
  *ref_root_type = NULL;

  int err = 0;

  assert(str != NULL);
  if (str == NULL) {
    if (error_func != NULL) {
      error_func(0, 0, "String argument is NULL");
    }
    err = -1;
  }
  else {
    ddsts_context_t *context = ddsts_create_context();
    if (context == NULL) {
      if (error_func != NULL) {
        error_func(0, 0, "Out of memory");
      }
      return 2;
    }
    ddsts_context_set_error_func(context, error_func);
    yyscan_t scanner;
    ddsts_parser_lex_init(&scanner);
    ddsts_parser__scan_string(str, scanner);
    err = ddsts_parser_parse(scanner, context);
    if (err == 0) {
      *ref_root_type = ddsts_context_take_root_type(context);
    }
    else if (ddsts_context_get_out_of_memory_error(context)) {
      if (error_func != 0) {
        error_func(0, 0, "Error: out of memory\n");
      }
    }
    ddsts_free_context(context);
    ddsts_parser_lex_destroy(scanner);
  }

  return err;
}

