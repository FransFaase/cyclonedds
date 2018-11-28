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
%{
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* yyscan_t is an opaque pointer, a typedef is required here to break a
   circular dependency introduced with bison 2.6 (normally a typedef is
   generated by flex). the define is required to disable the typedef in flex
   generated code */
#define YY_TYPEDEF_YY_SCANNER_T

typedef void *yyscan_t;

#include "idl.parser.h"
#include "yy_decl.h" /* prevent implicit declaration of yylex */

#define YYFPRINTF (unsigned int)fprintf

#define YYPRINT(A,B,C) YYUSE(A) /* to make yytoknum available */

int
yyerror(
  YYLTYPE *yylloc, yyscan_t yyscanner, dds_ts_context_t *context, char *text);

int illegal_identifier(const char *token);

%}

%code requires {

#include "typetree.h"
#include "tt_create.h"
#include "os/os.h"

}


%union {
  dds_ts_node_flags_t base_type_flags;
  dds_ts_type_spec_t *type_spec;
  dds_ts_literal_t literal;
  dds_ts_identifier_t identifier;
  dds_ts_scoped_name_t* scoped_name;
}

%define api.pure full
%define api.prefix {dds_ts_parser_}
%define parse.trace

%locations
%param {yyscan_t scanner}
%param {dds_ts_context_t *context}

%token-table

%start specification

%token <identifier>
  IDENTIFIER

%token <literal>
  INTEGER_LITERAL

%type <base_type_flags>
  base_type_spec
  floating_pt_type
  integer_type
  signed_int
  signed_tiny_int
  signed_short_int
  signed_long_int
  signed_longlong_int
  unsigned_int
  unsigned_tiny_int
  unsigned_short_int
  unsigned_long_int
  unsigned_longlong_int
  char_type
  wide_char_type
  boolean_type
  octet_type

%type <type_spec>
  type_spec
  simple_type_spec
  template_type_spec
  sequence_type
  string_type
  wide_string_type
  fixed_pt_type
  map_type

%type <scoped_name>
  scoped_name

%type <literal>
  positive_int_const
  literal
  const_expr

%type <identifier>
  simple_declarator
  identifier

/* keywords */
%token DDS_MODULE "module"
%token DDS_CONST "const"
%token DDS_NATIVE "native"
%token DDS_STRUCT "struct"
%token DDS_TYPEDEF "typedef"
%token DDS_UNION "union"
%token DDS_SWITCH "switch"
%token DDS_CASE "case"
%token DDS_DEFAULT "default"
%token DDS_ENUM "enum"
%token DDS_UNSIGNED "unsigned"
%token DDS_FIXED "fixed"
%token DDS_SEQUENCE "sequence"
%token DDS_STRING "string"
%token DDS_WSTRING "wstring"

%token DDS_FLOAT "float"
%token DDS_DOUBLE "double"
%token DDS_SHORT "short"
%token DDS_LONG "long"
%token DDS_CHAR "char"
%token DDS_WCHAR "wchar"
%token DDS_BOOLEAN "boolean"
%token DDS_OCTET "octet"
%token DDS_ANY "any"

%token DDS_MAP "map"
%token DDS_BITSET "bitset"
%token DDS_BITFIELD "bitfield"
%token DDS_BITMASK "bitmask"

%token DDS_INT8 "int8"
%token DDS_UINT8 "uint8"
%token DDS_INT16 "int16"
%token DDS_INT32 "int32"
%token DDS_INT64 "int64"
%token DDS_UINT16 "uint16"
%token DDS_UINT32 "uint32"
%token DDS_UINT64 "uint64"



%%


/* Constant Declaration */

specification:
    definitions
  ;

definitions:
    definition definitions
  | definition
  ;

definition:
    module_dcl ';'
  | type_dcl ';'
  ;

module_dcl:
    "module" identifier
      {
        if (!dds_ts_module_open(context, $2)) {
          YYABORT;
        }
      }
    '{' definitions '}'
      { dds_ts_module_close(context); };

scoped_name:
    identifier
      {
        if (!dds_ts_new_scoped_name(context, 0, false, $1, &($$))) {
          YYABORT;
        }
      }
  | "::" identifier
      {
        if (!dds_ts_new_scoped_name(context, 0, true, $2, &($$))) {
          YYABORT;
        }
      }
  | scoped_name "::" identifier
      {
        if (!dds_ts_new_scoped_name(context, $1, false, $3, &($$))) {
          YYABORT;
        }
      }
  ;

const_expr:
    literal
  | '(' const_expr ')'
      { $$ = $2; };

literal:
    INTEGER_LITERAL
  ;

positive_int_const:
    const_expr;

type_dcl:
    constr_type_dcl
  ;

type_spec:
    simple_type_spec
  ;

simple_type_spec:
    base_type_spec
      {
        if (!dds_ts_new_base_type(context, $1, &($$))) {
          YYABORT;
        }
      }
  | scoped_name
      {
        if (!dds_ts_get_type_spec_from_scoped_name(context, $1, &($$))) {
          YYABORT;
        }
      }
  ;

base_type_spec:
    integer_type
  | floating_pt_type
  | char_type
  | wide_char_type
  | boolean_type
  | octet_type
  ;

/* Basic Types */
floating_pt_type:
    "float" { $$ = DDS_TS_FLOAT_TYPE; }
  | "double" { $$ = DDS_TS_DOUBLE_TYPE; }
  | "long" "double" { $$ = DDS_TS_LONG_DOUBLE_TYPE; };

integer_type:
    signed_int
  | unsigned_int
  ;

signed_int:
    "short" { $$ = DDS_TS_SHORT_TYPE; }
  | "long" { $$ = DDS_TS_LONG_TYPE; }
  | "long" "long" { $$ = DDS_TS_LONG_LONG_TYPE; }
  ;

unsigned_int:
    "unsigned" "short" { $$ = DDS_TS_UNSIGNED_SHORT_TYPE; }
  | "unsigned" "long" { $$ = DDS_TS_UNSIGNED_LONG_TYPE; }
  | "unsigned" "long" "long" { $$ = DDS_TS_UNSIGNED_LONG_LONG_TYPE; }
  ;

char_type:
    "char" { $$ = DDS_TS_CHAR_TYPE; };

wide_char_type:
    "wchar" { $$ = DDS_TS_WIDE_CHAR_TYPE; };

boolean_type:
    "boolean" { $$ = DDS_TS_BOOLEAN_TYPE; };

octet_type:
    "octet" { $$ = DDS_TS_OCTET_TYPE; };

template_type_spec:
    sequence_type
  | string_type
  | wide_string_type
  | fixed_pt_type
  ;

sequence_type:
    "sequence" '<' type_spec ',' positive_int_const '>'
      {
        if (!dds_ts_new_sequence(context, $3, &($5), &($$))) {
          YYABORT;
        }
      }
  | "sequence" '<' type_spec '>'
      {
        if (!dds_ts_new_sequence_unbound(context, $3, &($$))) {
          YYABORT;
        }
      }
  ;

string_type:
    "string" '<' positive_int_const '>'
      {
        if (!dds_ts_new_string(context, &($3), &($$))) {
          YYABORT;
        }
      }
  | "string"
      {
        if (!dds_ts_new_string_unbound(context, &($$))) {
          YYABORT;
        }
      }
  ;

wide_string_type:
    "wstring" '<' positive_int_const '>'
      {
        if (!dds_ts_new_wide_string(context, &($3), &($$))) {
          YYABORT;
        }
      }
  | "wstring"
      {
        if (!dds_ts_new_wide_string_unbound(context, &($$))) {
          YYABORT;
        }
      }
  ;

fixed_pt_type:
    "fixed" '<' positive_int_const ',' positive_int_const '>'
      {
        if (!dds_ts_new_fixed_pt(context, &($3), &($5), &($$))) {
          YYABORT;
        }
      }
  ;

constr_type_dcl:
    struct_dcl
  ;

struct_dcl:
    struct_def
  | struct_forward_dcl
  ;

struct_def:
    "struct" identifier '{'
      {
        if (!dds_ts_add_struct_open(context, $2)) {
          YYABORT;
        }
      }
    members '}'
      { dds_ts_struct_close(context); }
  ;
members:
    member members
  | member
  ;

member:
    type_spec
      {
        if (!dds_ts_add_struct_member(context, $1)) {
          YYABORT;
        }
      }
    declarators ';'
  ;

struct_forward_dcl:
    "struct" identifier
      {
        if (!dds_ts_add_struct_forward(context, $2)) {
          YYABORT;
        }
      };

array_declarator:
    identifier
    fixed_array_sizes
      {
        if (!dds_ts_add_declarator(context, $1)) {
          YYABORT;
        }
      }
  ;

fixed_array_sizes:
    fixed_array_size fixed_array_sizes
  | fixed_array_size
  ;

fixed_array_size:
    '[' positive_int_const ']'
      {
        if (!dds_ts_add_array_size(context, &($2))) {
          YYABORT;
        }
      }
  ;

simple_declarator: identifier ;

declarators:
    declarator ',' declarators
  | declarator
  ;

declarator: simple_declarator
      {
        if (!dds_ts_add_declarator(context, $1)) {
          YYABORT;
        }
      };


/* From Building Block Extended Data-Types: */
struct_def:
    "struct" identifier ':' scoped_name '{'
      {
        if (!dds_ts_add_struct_extension_open(context, $2, $4)) {
          YYABORT;
        }
      }
    members '}'
      { dds_ts_struct_close(context); }
  | "struct" identifier '{'
      {
        if (!dds_ts_add_struct_open(context, $2)) {
          YYABORT;
        }
      }
    '}'
      { dds_ts_struct_empty_close(context); }
  ;

template_type_spec:
     map_type
  ;

map_type:
    "map" '<' type_spec ',' type_spec ',' positive_int_const '>'
      { 
        if (!dds_ts_new_map(context, $3, $5, &($7), &($$))) {
          YYABORT;
        }
      }
  | "map" '<' type_spec ',' type_spec '>'
      {
        if (!dds_ts_new_map_unbound(context, $3, $5, &($$))) {
          YYABORT;
        }
      }
  ;

signed_int:
    signed_tiny_int
  | signed_short_int
  | signed_long_int
  | signed_longlong_int
  ;

unsigned_int:
    unsigned_tiny_int
  | unsigned_short_int
  | unsigned_long_int
  | unsigned_longlong_int
  ;

signed_tiny_int: "int8" { $$ = DDS_TS_INT8_TYPE; };
unsigned_tiny_int: "uint8" { $$ = DDS_TS_UINT8_TYPE; };
signed_short_int: "int16" { $$ = DDS_TS_SHORT_TYPE; };
signed_long_int: "int32" { $$ = DDS_TS_LONG_TYPE; };
signed_longlong_int: "int64" { $$ = DDS_TS_LONG_LONG_TYPE; };
unsigned_short_int: "uint16" { $$ = DDS_TS_UNSIGNED_SHORT_TYPE; };
unsigned_long_int: "uint32" { $$ = DDS_TS_UNSIGNED_LONG_TYPE; };
unsigned_longlong_int: "uint64" { $$ = DDS_TS_UNSIGNED_LONG_LONG_TYPE; };

/* From Building Block Anonymous Types: */
type_spec: template_type_spec ;
declarator: array_declarator ;


identifier:
    IDENTIFIER
      {
        size_t offset = 0;
        if ($1[0] == '_') {
          offset = 1;
        } else if (illegal_identifier($1) != 0) {
          yyerror(&yylloc, scanner, context, "Identifier collides with a keyword");
        }
        if (($$ = os_strdup($1 + offset)) == NULL) {
          dds_ts_context_set_out_of_memory_error(context);
          YYABORT;
        }
      };


%%

int
yyerror(
  YYLTYPE *yylloc, yyscan_t yyscanner, dds_ts_context_t *context, char *text)
{
  (void)yyscanner;
  dds_ts_context_error(context, yylloc->first_line, yylloc->first_column, text);
  return 0;
}

int
illegal_identifier(const char *token)
{
  size_t i, n;

  assert(token != NULL);

  for (i = 0, n = strlen(token); i < YYNTOKENS; i++) {
    if (yytname[i] != 0
        && yytname[i][    0] == '"'
        && os_strncasecmp(yytname[i] + 1, token, n) == 0
        && yytname[i][n + 1] == '"'
        && yytname[i][n + 2] == '\0')
    {
      return 1;
    }
  }

  return 0;
}

int
parser_token_matches_keyword(const char *token, int *token_number)
{
  size_t i, n;

  assert(token != NULL);
  assert(token_number != NULL);

  for (i = 0, n = strlen(token); i < YYNTOKENS; i++) {
    if (yytname[i] != 0
        && yytname[i][    0] == '"'
        && strncmp(yytname[i] + 1, token, n) == 0
        && yytname[i][n + 1] == '"'
        && yytname[i][n + 2] == 0)
    {
      *token_number = yytoknum[i];
      return 1;
    }
  }

  return 0;
}

