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
#define YYSTYPE DDS_TS_PARSER_STYPE
#define YYLTYPE DDS_TS_PARSER_LTYPE

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
%x INSTRING
%x INWSTRING

%option noyywrap
%option nounistd
%option never-interactive
%option prefix="dds_ts_parser_"
%option reentrant
%option bison-bridge
%option bison-locations
%option nounistd

number                    [1-9][0-9]*
octal_number              0[0-7]*
hexadecimal_number        0[xX][a-fA-F0-9]*
integer_literal           ({number}|{octal_number}|{hexadecimal_number})
base_e                    ([eE]("+"|"-")?[0-9]+)
floating_pt_number        ([0-9]+"."[0-9]*|"."[0-9]+)
floating_pt_literal       ({floating_pt_number}{base_e}?|[0-9]+{base_e})
fixed_pt_literal          ({floating_pt_number}[dD]|[0-9]+[dD])
esc_sequence              ("\\"([ntvbrfa\\\?\'\"]|[0-7]{1,3}|[xX][a-fA-F0-9]{1,2}|"u"[a-fA-F0-9]{1,4}))
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
                            yylval->literal.flags = DDS_TS_LONG_LONG_TYPE;
                            yylval->literal.llng = strtoll(yytext, NULL, 0); /* FIXME: use strtoll_l */
                            if (errno == ERANGE && (yylval->literal.llng == LLONG_MIN ||
                                                    yylval->literal.llng == LLONG_MAX))
                            {
                              perror("Integer value invalid"); /* FIXME: use yyerror */
                            }
                            return INTEGER_LITERAL;
                          }

{floating_pt_literal}     {
                            errno = 0;
                            yylval->literal.flags = DDS_TS_LONG_DOUBLE_TYPE;
                            yylval->literal.ldbl = strtold(yytext, NULL); /* FIXME: use strtold_l */
                            if (errno == ERANGE && (yylval->literal.ldbl == HUGE_VALL ||
                                                    yylval->literal.ldbl == 0))
                            {
                              perror("Invalid floating_pt_literal"); /* FIXME: use yyerror */
                            }
                            return FLOATING_PT_LITERAL;
                          }

{fixed_pt_literal}        {
                            errno = 0;
                            yylval->literal.flags = DDS_TS_FIXED_PT_TYPE;
                            yylval->literal.ldbl = strtold(yytext, NULL); /* FIXME: use strtold_l */
                            if (errno == ERANGE && (yylval->literal.ldbl == HUGE_VALL ||
                                                    yylval->literal.ldbl == 0))
                            {
                              perror("Invalid fixed_pt_literal"); /* FIXME: use yyerror */
                            }
                            return FIXED_PT_LITERAL;
                          }

"'"{char_literal}"'"      {
                            yylval->literal.flags = DDS_TS_CHAR_TYPE;
                            yylval->literal.chr = dds_ts_unescape_char(yytext + 1, NULL);
                            return CHARACTER_LITERAL;
                          }

"L'"{char_literal}"'"      {
                            yylval->literal.flags = DDS_TS_WIDE_CHAR_TYPE;
                            yylval->literal.wchr = dds_ts_unescape_wchar(yytext + 1, NULL);
                            return WIDE_CHARACTER_LITERAL;
                          }

"\""                      { BEGIN(INSTRING); }
<INSTRING>{string_literal}  {
                            yylval->literal.flags = DDS_TS_STRING_TYPE;
                            yylval->literal.str = strdup(yytext);
                            if (yylval->literal.str == NULL) {
                              perror("Could not copy string_literal"); /* FIXME: use yyerror */
                            }
                            return STRING_LITERAL;
                          }
<INSTRING>"\""            { BEGIN(INITIAL); }

"L\""                     { BEGIN(INWSTRING); }
<INWSTRING>{string_literal} {
                            yylval->literal.flags = DDS_TS_WIDE_STRING_TYPE;
                            yylval->literal.str = strdup(yytext);
                            if (yylval->literal.str == NULL) {
                              perror("Could not copy string_literal"); /* FIXME: use yyerror */
                            }
                            return WIDE_STRING_LITERAL;
                          }
<INWSTRING>"\""           { BEGIN(INITIAL); }

{identifier}              {
                            int token_number;
                            if (parser_token_matches_keyword(yytext, &token_number)) {
                              return token_number;
                            }
                            yylval->identifier = strdup(yytext);
                            if (yylval->identifier == NULL) {
                              perror("Could not copy identifier"); /* FIXME: use yyerror */
                            }
                            return IDENTIFIER;
                          }

.                         { return yytext[0]; /* FIXME: is this correct?!?! */ }

%%

