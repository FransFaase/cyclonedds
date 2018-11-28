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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define YYSTYPE DDS_TS_PARSER_STYPE
#define YYLTYPE DDS_TS_PARSER_LTYPE

#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;

#include "parser.h"
#include "idl.parser.h"
#include "yy_decl.h"
#include "idl.lexer.h"

extern void dds_ts_stringify(dds_ts_node_t *root_node, char *buffer, size_t len);

extern int
dds_ts_parse_file(const char *file)
{
  int err = 0;
  FILE *fh;

  assert(file != NULL);

  if ((fh = fopen(file, "rb")) == NULL) {
    err = errno;
    fprintf(stderr, "Cannot open %s: %s", file, strerror(err));
  } else {
    /* FIXME: YYSTYPE yystype; */
    /* FIXME: YYLTYPE yyltype; */
    yyscan_t scanner;

    dds_ts_parser_lex_init(&scanner);
    dds_ts_parser_set_in(fh, scanner);
    dds_ts_context_t *context = dds_ts_create_context();
    err = dds_ts_parser_parse(scanner, context);
    if (err == 0) {
       char buffer[1000];
       dds_ts_stringify(dds_ts_context_get_root_node(context), buffer, 1000);
       printf("Result: '%s'\n", buffer);
    }
    dds_ts_free_context(context);
    dds_ts_parser_lex_destroy(scanner);
    (void)fclose(fh);
  }

  return err;
}

extern int
dds_ts_parse_string(const char *str, bool ignore_yyerror)
{
  int err = 0;

  assert(str != NULL);
  if (str == NULL) {
    fprintf(stderr, "String argument is NULL\n");
    err = -1;
  } else {
    /* FIXME: YYSTYPE yystype; */
    /* FIXME: YYLTYPE yyltype; */
    yyscan_t scanner;

    dds_ts_parser_lex_init(&scanner);
    dds_ts_parser__scan_string(str, scanner);
    dds_ts_context_t *context = dds_ts_create_context();
    dds_ts_context_set_ignore_yyerror(context, ignore_yyerror);
    err = dds_ts_parser_parse(scanner, context);
    dds_ts_free_context(context);
    dds_ts_parser_lex_destroy(scanner);
  }

  return err;
}

/* For testing: */

int dds_ts_parse_string_stringify(const char *str, char *buffer, size_t len)
{
  int err = 0;

  assert(str != NULL);
  if (str == NULL) {
    fprintf(stderr, "String argument is NULL\n");
    err = -1;
  } else {
    /* FIXME: YYSTYPE yystype; */
    /* FIXME: YYLTYPE yyltype; */
    yyscan_t scanner;

    dds_ts_parser_lex_init(&scanner);
    dds_ts_parser__scan_string(str, scanner);
    dds_ts_context_t *context = dds_ts_create_context();
    dds_ts_context_set_ignore_yyerror(context, false);
    err = dds_ts_parser_parse(scanner, context);
    if (err != 0) {
      strncpy(buffer, "PARSING ERROR", len);
      buffer[len-1] = '\0';
    } else {
      dds_ts_stringify(dds_ts_context_get_root_node(context), buffer, len);
    }

    dds_ts_free_context(context);
    dds_ts_parser_lex_destroy(scanner);
  }

  return err;
}
