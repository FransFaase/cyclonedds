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

#define YYSTYPE IDL_PARSER_STYPE
#define YYLTYPE IDL_PARSER_LTYPE

#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;

#include "idl.parser.h"
#include "yy_decl.h"
#include "idl.lexer.h"

int
idl_parse_file(const char *file)
{
  int err = 0;
  FILE *fh;

  assert(file != NULL);

  if ((fh = fopen(file, "rb")) == NULL) {
    err = errno;
    fprintf(stderr, "Cannot open %s: %s", file, strerror(err));
  } else {
    YYSTYPE yystype;
    YYLTYPE yyltype;
    yyscan_t scanner;

    idl_parser_lex_init(&scanner);
    idl_parser_set_in(fh, scanner);
    idl_parser_parse(scanner, NULL);
    idl_parser_lex_destroy(scanner);
    (void)fclose(fh);
  }

  return err;
}
