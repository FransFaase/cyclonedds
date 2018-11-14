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

#define YYSTYPE IDL_PARSER_STYPE
#define YYLTYPE IDL_PARSER_LTYPE

#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;

#include "parser.h"
#include "idl.parser.h"
#include "yy_decl.h"
#include "idl.lexer.h"

extern void idl_stringify(idl_context_t *context, char *buffer, size_t len);
extern void idl_stringify2(idl_context_t *context, char *buffer, size_t len);

extern int
idl_parse_file(const char *file)
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

    idl_parser_lex_init(&scanner);
    idl_parser_set_in(fh, scanner);
    idl_context_t *context = idl_create_context();
    err = idl_parser_parse(scanner, context);
    if (err == 0) {
       char buffer[1000];
       idl_stringify(context, buffer, 1000);
       printf("Result: '%s'\n", buffer);
    }
    idl_free_context(context);
    idl_parser_lex_destroy(scanner);
    (void)fclose(fh);
  }

  return err;
}

extern int
idl_parse_string(const char *str, bool ignore_yyerror)
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

    idl_parser_lex_init(&scanner);
    idl_parser__scan_string(str, scanner);
    idl_context_t *context = idl_create_context();
    idl_context_set_ignore_yyerror(context, ignore_yyerror);
    err = idl_parser_parse(scanner, context);
    idl_free_context(context);
    idl_parser_lex_destroy(scanner);
  }

  return err;
}

/* For testing: */

int idl_parse_string_stringify(const char *str, char *buffer, size_t len)
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

    idl_parser_lex_init(&scanner);
    idl_parser__scan_string(str, scanner);
    idl_context_t *context = idl_create_context();
    idl_context_set_ignore_yyerror(context, false);
    err = idl_parser_parse(scanner, context);
    if (err != 0) {
      strncpy(buffer, "PARSING ERROR", len);
      buffer[len-1] = '\0';
    } else {
      idl_stringify(context, buffer, len);
      idl_stringify2(context, buffer, len);
    }

    idl_free_context(context);
    idl_parser_lex_destroy(scanner);
  }

  return err;
}
