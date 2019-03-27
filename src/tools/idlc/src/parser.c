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
#include "dds/ddsrt/misc.h"
#include "dds/ddsts/typetree.h"

#define YYSTYPE DDS_PARSER_STYPE
#define YYLTYPE DDS_PARSER_LTYPE

#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;

#define YY_NO_UNISTD_H 1 /* to surpress #include <unistd.h> */

#include "parser.h"
#include "idl.parser.h"
#include "yy_decl.h"
#include "idl.lexer.h"

dds_retcode_t dds_idl_parse_file(const char *file, void (*error_func)(int line, int column, const char *msg), ddsts_type_t **ref_root_type)
{
  if (file == NULL || ref_root_type == NULL) {
    return DDS_RETCODE_BAD_PARAMETER;
  }
  *ref_root_type = NULL;

DDSRT_WARNING_MSVC_OFF(4996);
  FILE *fh = fopen(file, "rb");
DDSRT_WARNING_MSVC_ON(4996);

  if (fh == NULL) {
    if (error_func != 0) {
      error_func(0, 0, "Cannot open file");
    }
    return DDS_RETCODE_ERROR;
  }

  dds_context_t *context = dds_create_context();
  if (context == NULL) {
    return DDS_RETCODE_OUT_OF_RESOURCES;
  }
  dds_context_set_error_func(context, error_func);
  yyscan_t scanner;
  dds_parser_lex_init(&scanner);
  dds_parser_set_in(fh, scanner);
  if (dds_parser_parse(scanner, context) == 0) {
    *ref_root_type = dds_context_take_root_type(context);
  }
  dds_retcode_t rc = dds_context_get_retcode(context);
  dds_free_context(context);
  dds_parser_lex_destroy(scanner);
  (void)fclose(fh);

  return rc;
}

dds_retcode_t dds_idl_parse_string(const char *str, void (*error_func)(int line, int column, const char *text), ddsts_type_t **ref_root_type)
{
  if (str == NULL || ref_root_type == NULL) {
    return DDS_RETCODE_BAD_PARAMETER;
  }
  *ref_root_type = NULL;

  dds_context_t *context = dds_create_context();
  if (context == NULL) {
    return DDS_RETCODE_OUT_OF_RESOURCES;
  }
  dds_context_set_error_func(context, error_func);
  yyscan_t scanner;
  dds_parser_lex_init(&scanner);
  dds_parser__scan_string(str, scanner);
  if (dds_parser_parse(scanner, context) == 0) {
    *ref_root_type = dds_context_take_root_type(context);
  }
  dds_retcode_t rc = dds_context_get_retcode(context);
  dds_free_context(context);
  dds_parser_lex_destroy(scanner);

  return rc;
}

