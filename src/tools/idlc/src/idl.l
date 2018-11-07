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
%{
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* YYSTYPE and YYLTYPE defines are required because idl.y defines api.prefix */
#define YYSTYPE IDL_PARSER_STYPE
#define YYLTYPE IDL_PARSER_LTYPE

#include "idl.parser.h"
/* YY_DECL must be defined here because yyparse passes an additional argument */
#include "yy_decl.h"

#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 0; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }

#include "tools.h"
%}

%x MULTILINE_COMMENT
%x COMMENT
%x STRING

%option noyywrap
%option nounistd
%option never-interactive
%option prefix="idl_parser_"
%option reentrant
%option bison-bridge
%option bison-locations

number                    [1-9][0-9]*
octal_number              0[0-7]*
hexadecimal_number        0[xX][a-fA-F0-9]*
integer_literal           ({number}|{octal_number}|{hexadecimal_number})
base_e                    ([eE]("+"|"-")?[0-9]+)
floating_pt_number        ([0-9]+"."[0-9]*|"."[0-9]+)
floating_pt_literal       ({floating_pt_number}{base_e}?|[0-9]+{base_e})
fixed_pt_literal          ({floating_pt_number}[dD]|[0-9]+[dD])
esc_sequence              ("\\"([ntvbrfa\\\?\'\"]|[0-7]{1,3}|[xX][a-fA-F0-9]{1,2}))
char_literal              ([^\n\t\'\\]|{esc_sequence})
string_literal            ([^\n\t\"\\]|{esc_sequence})*
identifier                [a-zA-Z_][a-zA-Z0-9_]*

%%

[ \t\r]                   { }
[\n]                      { }

"<<"                      { return(LSHIFT); }
">>"                      { return(RSHIFT); }

"/*"                      { BEGIN(MULTILINE_COMMENT); }
<MULTILINE_COMMENT>.      { }
<MULTILINE_COMMENT>"\n"   { }
<MULTILINE_COMMENT>"*/"   { BEGIN(INITIAL); }

"//"                      { BEGIN(COMMENT); }
<COMMENT>"\n"             { BEGIN(INITIAL); }
<COMMENT>.                { }

const                     { return(CONST); }
unsigned                  { return(UNSIGNED); }

float                     { return(FLOAT); }
double                    { return(DOUBLE); }
short                     { return(SHORT); }
long                      { return(LONG); }
char                      { return(CHAR); }
wchar                     { return(WCHAR); }
boolean                   { return(BOOLEAN); }
octet                     { return(OCTET); }
any                       { return(ANY); }

module                    { return(MODULE); }
enum                      { return(ENUM); }

TRUE                      {
                            yylval->boolean = true;
                            return(BOOLEAN_LITERAL);
                          }

FALSE                     {
                            yylval->boolean = false;
                            return(BOOLEAN_LITERAL);
                          }

{integer_literal}         {
                            errno = 0;
                            /* strtoll recognizes if the value is dec, oct or hex if base is zero */
                            yylval->literal.type = idl_integer_literal;
                            yylval->literal.value.llng = strtoll(yytext, NULL, 0); /* FIXME: use strtoll_l */
                            if (errno == ERANGE && (yylval->literal.value.llng == LLONG_MIN ||
                                                    yylval->literal.value.llng == LLONG_MAX))
                            {
                              perror("Integer value invalid"); /* FIXME: use yyerror */
                            }
                            return INTEGER_LITERAL;
                          }

{floating_pt_literal}     {
                            errno = 0;
                            yylval->literal.type = idl_floating_pt_literal;
                            yylval->literal.value.ldbl = strtold(yytext, NULL); /* FIXME: use strtold_l */
                            if (errno == ERANGE && (yylval->literal.value.ldbl == HUGE_VALL ||
                                                    yylval->literal.value.ldbl == 0))
                            {
                              perror("Invalid floating_pt_literal"); /* FIXME: use yyerror */
                            }
                            return FLOATING_PT_LITERAL;
                          }

{fixed_pt_literal}        {
                            errno = 0;
                            yylval->literal.type = idl_fixed_pt_literal;
                            yylval->literal.value.ldbl = strtold(yytext, NULL); /* FIXME: use strtold_l */
                            if (errno == ERANGE && (yylval->literal.value.ldbl == HUGE_VALL ||
                                                    yylval->literal.value.ldbl == 0))
                            {
                              perror("Invalid fixed_pt_literal"); /* FIXME: use yyerror */
                            }
                            return FIXED_PT_LITERAL;
                          }

"'"{char_literal}"'"      {
                            yylval->literal.type = idl_character_literal;
                            yylval->literal.value.chr = idl_unescape_char(yytext + 1, NULL);
                            return CHARACTER_LITERAL;
                          }

"\""                      { BEGIN(STRING); }
<STRING>{string_literal}  {
                            yylval->literal.type = idl_string_literal;
                            yylval->literal.value.str = strdup(yytext);
                            if (yylval->literal.value.str == NULL) {
                              perror("Could not copy string_literal"); /* FIXME: use yyerror */
                            }
                            return STRING_LITERAL;
                          }
<STRING>"\""              { BEGIN(INITIAL); }

{identifier}              {
                            yylval->identifier = strdup(yytext);
                            if (yylval->identifier == NULL) {
                              perror("Could not copy identifier"); /* FIXME: use yyerror */
                            }
                            return IDENTIFIER;
                          }

.                         { return yytext[0]; /* FIXME: is this correct?!?! */ }

%%

