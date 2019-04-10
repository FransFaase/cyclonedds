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

#include <stdio.h>
#include "dds/ddsrt/retcode.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/string.h"
#include "dds/ddsrt/time.h"
#include "dds/ddsrt/misc.h"
#include "dds/ddsrt/strtol.h"
#include "dds/ddsts/typetree.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "templ_script.h"

typedef struct ostream_t ostream_t;
struct ostream_t {
  bool (*open)(ostream_t *ostream, const char *name);
  void (*close)(ostream_t *ostream);
  void (*put)(ostream_t *ostream, char ch);
};

static bool ostream_open(ostream_t *ostream, const char *name)
{
  return ostream->open != 0 && ostream->open(ostream, name);
}

static void ostream_close(ostream_t *ostream)
{
  if (ostream->close != 0) {
    ostream->close(ostream);
  }
}

static void ostream_put(ostream_t *ostream, char ch)
{
  if (ostream->put != 0) {
    ostream->put(ostream, ch);
  }
}

static void ostream_puts(ostream_t *ostream, const char *str)
{
  if (ostream->put != 0) {
    while (*str != '\0') {
      ostream->put(ostream, *str++);
    }
  }
}

/* file output stream */ 

typedef struct {
  ostream_t ostream;
  FILE *f;
} ostream_to_files_t;
 
static bool file_ostream_open(ostream_t *ostream, const char *name) 
{ 
DDSRT_WARNING_MSVC_OFF(4996); 
  return (((ostream_to_files_t*)ostream)->f = fopen(name, "wt")) != 0; 
DDSRT_WARNING_MSVC_ON(4996); 
} 
 
static void file_ostream_close(ostream_t *ostream) 
{ 
  fclose(((ostream_to_files_t*)ostream)->f);
  ((ostream_to_files_t*)ostream)->f = NULL; 
} 
 
static void file_ostream_put(ostream_t *ostream, char ch) 
{ 
  if (((ostream_to_files_t*)ostream)->f != NULL) {
    fputc(ch, ((ostream_to_files_t*)ostream)->f);
  }
} 
 
static void init_ostream_to_files(ostream_to_files_t *ostream) 
{ 
  ostream->ostream.open = file_ostream_open; 
  ostream->ostream.close = file_ostream_close; 
  ostream->ostream.put = file_ostream_put;
  ostream->f = NULL;
}


/* String appender */

#define STRING_APPENDER_BUF_TEXT_SIZE 100
typedef struct string_appender_buf_t string_appender_buf_t;
struct string_appender_buf_t
{
  char text[STRING_APPENDER_BUF_TEXT_SIZE];
  string_appender_buf_t *next;
};

typedef struct {
  string_appender_buf_t *buf;
  string_appender_buf_t **ref_buf;
  unsigned int pos;
  size_t size;
} string_appender_t;

static void string_appender_init(string_appender_t *appender)
{
  appender->buf = NULL;
  appender->ref_buf = &appender->buf;
  appender->pos = 0;
  appender->size = 0;
}

static void string_appender_start(string_appender_t *appender)
{
  appender->ref_buf = &appender->buf;
  appender->pos = 0;
  appender->size = 0;
}

static void string_appender_add(string_appender_t *appender, char ch)
{
  if (*appender->ref_buf == NULL) {
    *appender->ref_buf = (string_appender_buf_t*)ddsrt_malloc(sizeof(string_appender_buf_t));
    if (*appender->ref_buf == NULL) {
      return; /* malloc error */
    }
    (*appender->ref_buf)->text[0] = ch;
    (*appender->ref_buf)->next = 0;
    appender->pos = 1;
  }
  else {
    (*appender->ref_buf)->text[appender->pos++] = ch;
    if (appender->pos == STRING_APPENDER_BUF_TEXT_SIZE) {
      appender->ref_buf = &(*appender->ref_buf)->next;
      appender->pos = 0;
    }
  }
  appender->size++;
}

static char *string_appender_result(string_appender_t *appender)
{
  char *result = (char*)ddsrt_malloc(sizeof(char)*(appender->size + 1));
  string_appender_buf_t *buf = appender->buf;
  unsigned int pos = 0;
  for (size_t i = 0; i < appender->size; i++) {
    result[i] = buf->text[pos];
    if (++pos == STRING_APPENDER_BUF_TEXT_SIZE) {
      buf = buf->next;
      pos = 0;
    }
  }
  result[appender->size] = '\0';
  appender->ref_buf = &appender->buf;
  appender->pos = 0;
  appender->size = 0;
  return result;
}

static int string_appender_compare(string_appender_t *appender, const char *s)
{
  string_appender_buf_t *buf = appender->buf;
  unsigned int pos = 0;
  for (size_t i = 0; i < appender->size; i++) {
    int cmp = buf->text[pos] - s[i];
    if (cmp != 0) {
      return cmp;
    }
    if (++pos == STRING_APPENDER_BUF_TEXT_SIZE) {
      buf = buf->next;
      pos = 0;
    }
  }
  return '\0' - s[appender->size];
}

static void string_appender_free(string_appender_t *appender)
{
  for (string_appender_buf_t *buf = appender->buf; buf != NULL;) {
    string_appender_buf_t *next = buf->next;
    ddsrt_free((void*)buf);
    buf = next;
  }
}


/* Scanner */

typedef struct {
  unsigned long line;
  unsigned long column;
} file_pos_t;

typedef struct {
  FILE* file;
  char symbol;
  char *text;
  char cur_ch;
  file_pos_t start;
  file_pos_t end;
  file_pos_t cur;
  string_appender_t string_appender;
} scanner_t;

#define SCANNER_SYMBOL_EOF     '\0'
#define SCANNER_SYMBOL_ERROR   'A'
#define SCANNER_SYMBOL_IDENT   'B'
#define SCANNER_SYMBOL_NUMBER  'C'
#define SCANNER_SYMBOL_STRING  '\"'
#define SCANNER_SYMBOL_IF      'D'
#define SCANNER_SYMBOL_ELSE    'E'
#define SCANNER_SYMBOL_WITH    'F'
#define SCANNER_SYMBOL_FOR     'G'
#define SCANNER_SYMBOL_IN      'H'
#define SCANNER_SYMBOL_SWITCH  'I'
#define SCANNER_SYMBOL_CASE    'J'
#define SCANNER_SYMBOL_DEFAULT 'K'
#define SCANNER_SYMBOL_AND     'L'
#define SCANNER_SYMBOL_OR      'M'
#define SCANNER_SYMBOL_EQUAL   'N'
#define SCANNER_SYMBOL_TRUE    'O'
#define SCANNER_SYMBOL_FALSE   'P'

struct {
  const char *text;
  char symbol;
} keywords[] = {
  { "if",      SCANNER_SYMBOL_IF },
  { "else",    SCANNER_SYMBOL_ELSE },
  { "with",    SCANNER_SYMBOL_WITH },
  { "for",     SCANNER_SYMBOL_FOR },
  { "in",      SCANNER_SYMBOL_IN },
  { "switch",  SCANNER_SYMBOL_SWITCH },
  { "case",    SCANNER_SYMBOL_CASE },
  { "default", SCANNER_SYMBOL_DEFAULT },
  { "true",    SCANNER_SYMBOL_TRUE },
  { "false",   SCANNER_SYMBOL_FALSE },
};

static void scanner_next_char(scanner_t *scanner)
{
  scanner->cur_ch = (char)fgetc(scanner->file);
  if (feof(scanner->file)) {
    scanner->cur_ch = '\0';
  }
  scanner->cur.column++;
}

static void scanner_skip_white_space(scanner_t *scanner)
{
  for (;; scanner_next_char(scanner)) {
    if (scanner->cur_ch == '\t') {
      scanner->cur.column = ((scanner->cur.column + 3)/4)*4;
    }
    else if (scanner->cur_ch == '\n') {
      scanner->cur.column = 0;
      scanner->cur.line++;
    }
    else if (scanner->cur_ch != ' ' && scanner->cur_ch != '\r') {
      break;
    }
  }
}

static void scanner_print_state(scanner_t *scanner)
{
  (void)scanner;/*
  printf("%lu.%lu: ", scanner->cur.line, scanner->cur.column);
  if (scanner->symbol == SCANNER_SYMBOL_EOF)
    printf("eof\n");
  else {
    printf("%c", scanner->symbol);
    if (scanner->text != NULL)
      printf(" |%s|", scanner->text);
    printf("\n");
  }*/
}

static void scanner_next_token(scanner_t *scanner)
{
  if (scanner->text != NULL) {
    ddsrt_free((void*)scanner->text);
    scanner->text = NULL;
  }
  for (;;) {
    scanner_skip_white_space(scanner);
    if (scanner->cur_ch != '/') {
      break;
    }
    scanner_next_char(scanner);
    if (scanner->cur_ch != '*') {
      scanner->symbol = '/';
      return;
    }
    scanner_next_char(scanner);
    for (;;) {
      if (scanner->cur_ch == '\t') {
        scanner->cur.column = ((scanner->cur.column + 3)/4)*4;
        scanner_next_char(scanner);
      }
      else if (scanner->cur_ch == '\n') {
        scanner->cur.column = 0;
        scanner->cur.line++;
        scanner_next_char(scanner);
      }
      else if (scanner->cur_ch == '*') {
        scanner_next_char(scanner);
        if (scanner->cur_ch == '/') {
          scanner_next_char(scanner);
          break;
        }
      }
      else {
        scanner_next_char(scanner);
      }
    }
  }
  if (scanner->cur_ch == '\0') {
    scanner->symbol = SCANNER_SYMBOL_EOF;
    scanner_print_state(scanner);
    return;
  }
  scanner->start = scanner->cur;
  string_appender_start(&scanner->string_appender);
  if (   ('a' <= scanner->cur_ch && scanner->cur_ch <= 'z')
      || ('A' <= scanner->cur_ch && scanner->cur_ch <= 'Z')) {
    string_appender_add(&scanner->string_appender, scanner->cur_ch);
    scanner_next_char(scanner);
    while (   ('a' <= scanner->cur_ch && scanner->cur_ch <= 'z')
           || ('A' <= scanner->cur_ch && scanner->cur_ch <= 'Z')
           || ('0' <= scanner->cur_ch && scanner->cur_ch <= '9')
       || scanner->cur_ch == '_') {
      string_appender_add(&scanner->string_appender, scanner->cur_ch);
      scanner_next_char(scanner);
    }
    scanner->end = scanner->cur;
    /* compare with keywords */
    for (size_t i = 0; i < sizeof(keywords)/sizeof(*keywords); i++) {
      if (string_appender_compare(&scanner->string_appender, keywords[i].text) == 0) {
        scanner->symbol = keywords[i].symbol;
        scanner_print_state(scanner);
        return;
      }
    }
    scanner->text = string_appender_result(&scanner->string_appender);
    scanner->symbol = SCANNER_SYMBOL_IDENT;
  }
  else if ('0' <= scanner->cur_ch && scanner->cur_ch <= '9')
  {
    string_appender_add(&scanner->string_appender, scanner->cur_ch);
    scanner_next_char(scanner);
    while ('0' <= scanner->cur_ch && scanner->cur_ch <= '9') {
      string_appender_add(&scanner->string_appender, scanner->cur_ch);
      scanner_next_char(scanner);
    }
    scanner->end = scanner->cur;
    scanner->text = string_appender_result(&scanner->string_appender);
    scanner->symbol = SCANNER_SYMBOL_NUMBER;
  }
  else if (scanner->cur_ch == '\"') {
    while (scanner->cur_ch == '\"') {
      scanner_next_char(scanner);
      for (; scanner->cur_ch != '\0' && scanner->cur_ch != '\"'; scanner_next_char(scanner)) {
        if (scanner->cur_ch == '\\') {
          scanner_next_char(scanner);
          if (scanner->cur_ch == 'n') {
            string_appender_add(&scanner->string_appender, '\n');
          }
          else if (scanner->cur_ch == 't') {
            string_appender_add(&scanner->string_appender, '\t');
          }
          else if (scanner->cur_ch == '\"') {
            string_appender_add(&scanner->string_appender, '\"');
          }
          else {
	    string_appender_add(&scanner->string_appender, '\\');
            string_appender_add(&scanner->string_appender, scanner->cur_ch);
          }
        }
        else {
          string_appender_add(&scanner->string_appender, scanner->cur_ch);
        }
      }
      if (scanner->cur_ch == '\0') {
        scanner->symbol = SCANNER_SYMBOL_ERROR;
        scanner_print_state(scanner);
        return;
      }
      scanner_next_char(scanner);
      scanner->end = scanner->cur;
      scanner_skip_white_space(scanner);
    }
    scanner->text = string_appender_result(&scanner->string_appender);
    scanner->symbol = SCANNER_SYMBOL_STRING;
  }
  else if (scanner->cur_ch == '|') {
    scanner_next_char(scanner);
    if (scanner->cur_ch == '|') {
      scanner_next_char(scanner);
      scanner->symbol = SCANNER_SYMBOL_OR;
    }
    else {
      scanner->symbol = SCANNER_SYMBOL_ERROR;
    }
    scanner->end = scanner->cur;
  }
  else if (scanner->cur_ch == '&') {
    scanner_next_char(scanner);
    if (scanner->cur_ch == '&') {
      scanner_next_char(scanner);
      scanner->symbol = SCANNER_SYMBOL_AND;
    }
    else {
      scanner->symbol = SCANNER_SYMBOL_ERROR;
    }
    scanner->end = scanner->cur;
  }
  else if (scanner->cur_ch == '=') {
    scanner->symbol = scanner->cur_ch;
    scanner_next_char(scanner);
    if (scanner->cur_ch == '=') {
      scanner->symbol = SCANNER_SYMBOL_EQUAL;
      scanner_next_char(scanner);
    }
    scanner->end = scanner->cur;
  }
  else {
    scanner->symbol = scanner->cur_ch;
    scanner_next_char(scanner);
    scanner->end = scanner->cur;
  }
  scanner_print_state(scanner);
}

static void scanner_init(scanner_t *scanner, FILE *file)
{
  scanner->file = file;
  scanner->symbol = SCANNER_SYMBOL_ERROR;
  scanner->text = NULL;
  scanner->cur_ch = (char)fgetc(file);
  scanner->cur.line = 1;
  scanner->cur.column = 0;
  string_appender_init(&scanner->string_appender);
  scanner_next_token(scanner);
}

static void scanner_fini(scanner_t *scanner)
{
  string_appender_free(&scanner->string_appender);
}

/* Expressions */

typedef struct value_t value_t;
typedef struct expr_t expr_t;
typedef struct scope_t scope_t;
typedef struct proc_definition_t proc_definition_t;
typedef struct exec_context_t exec_context_t;

typedef bool (*eval_func_p)(expr_t *expr, scope_t *scope, exec_context_t *context, value_t *result);
#define EVAL_FUNC(X) static bool X(expr_t *expr, scope_t *scope, exec_context_t *context, value_t *result)

#define ENTER_EVAL_FUNC(N) \
  call_stack_entry_t call_stack_entry;\
  exec_context_enter(context, N, &call_stack_entry, scope, expr, result);
#define LEAVE_EVAL_FUNC exec_context_leave(context, &call_stack_entry, result);
#define EVAL_FUNC_ERROR(X) \
  exec_context_break(context); \
  fprintf(stdout, "Error: %s\n", X); \
  exec_context_debug(context, &call_stack_entry);
#define EVAL_FUNC_ERROR_VALUE(X,V) \
  exec_context_break(context); \
  fprintf(stdout, "Error: %s: ", X); value_print(V, stdout); fprintf(stdout, "\n"); \
  exec_context_debug(context, &call_stack_entry);

enum value_type_t { value_type_none, value_type_bool, value_type_int, value_type_text, value_type_node, value_type_expr };
struct value_t {
  enum value_type_t type;
  const char *text;
  unsigned long long number;
  ddsts_type_t *node;
  scope_t *scope;
  expr_t *expr;
};

struct expr_t {
  eval_func_p eval;
  const char *text;
  unsigned long long number;
  char ch;
  ddsts_type_t *node;
  proc_definition_t *proc_def;
  expr_t *children;
  expr_t *next;
  file_pos_t pos;
};

static void value_init(value_t *value)
{
  value->type = value_type_none;
  value->text = NULL;
  value->number = 0;
  value->node = NULL;
  value->scope = NULL;
  value->expr = NULL;
}

static bool value_valid(value_t *value)
{
  switch (value->type) {
    case value_type_none:
    case value_type_bool:
    case value_type_int:
    case value_type_text:
    case value_type_node:
    case value_type_expr:
      return true;
  }
  return false;
}

static int value_comp(value_t *lhs, value_t *rhs)
{
  if (lhs->type < rhs->type) return -1;
  if (lhs->type > rhs->type) return 1;
  switch (lhs->type) {
    case value_type_none: return 0;
    case value_type_bool: return (rhs->number != 0 ? 1 : 0) - (lhs->number != 0 ? 1 : 0);
    case value_type_int:
      if (lhs->number < rhs->number) return -1;
      if (lhs->number > rhs->number) return 1;
      break;
    case value_type_text: return strcmp(lhs->text, rhs->text);
    case value_type_node:
      if (lhs->text < rhs->text) return -1;
      if (lhs->text > rhs->text) return 1;
      break;
    case value_type_expr:
      break; // should not compare
    default:
      assert(0);
      break;
  }
  return 0;
}

bool value_is_true(value_t *value)
{
  switch(value->type) {
    case value_type_none: return false;
    case value_type_bool: return value->number != 0;
    case value_type_int: return value->number != 0;
    case value_type_text: return value->text != 0 && value->text[0] != '\0';
    case value_type_node: return value->node != 0;
    case value_type_expr:
      break; // should not occur
    default:
      assert(0);
      break;
  }
  return false;
}

const char *node_type(ddsts_type_t *node)
{
  switch (DDSTS_TYPE_OF(node)) {
    case DDSTS_SHORT:           return "short"; break;
    case DDSTS_LONG:            return "long"; break;
    case DDSTS_LONGLONG:        return "long_long"; break;
    case DDSTS_USHORT:          return "unsigned_short"; break;
    case DDSTS_ULONG:           return "unsigned_long"; break;
    case DDSTS_ULONGLONG:       return "unsigned_long_long"; break;
    case DDSTS_CHAR:            return "char"; break;
    case DDSTS_BOOLEAN:         return "boolean"; break;
    case DDSTS_OCTET:           return "octet"; break;
    case DDSTS_INT8:            return "int8"; break;
    case DDSTS_UINT8:           return "uint8"; break;
    case DDSTS_FLOAT:           return "float"; break;
    case DDSTS_DOUBLE:          return "double"; break;
    case DDSTS_LONGDOUBLE:      return "long_double"; break;
    case DDSTS_FIXED_PT_CONST:  return "fixed_pt_const"; break;
    case DDSTS_ANY:             return "any"; break;
    case DDSTS_SEQUENCE:        return "sequence"; break;
    case DDSTS_STRING:          return "string"; break;
    case DDSTS_WIDE_STRING:     return "wide_string"; break;
    case DDSTS_FIXED_PT:        return "fixed_pt"; break;
    case DDSTS_MAP:             return "map"; break;
    case DDSTS_ARRAY:           return "array"; break;
    case DDSTS_MODULE:          return "module"; break;
    case DDSTS_FORWARD_STRUCT:  return "forward_struct"; break;
    case DDSTS_STRUCT:          return "struct"; break;
    case DDSTS_DECLARATION:     return "declaration"; break;
    default: break;
  }
  return "<other>";
}

void value_print(value_t *value, FILE *f)
{
  switch(value->type) {
    case value_type_none: fprintf(f, "none"); break;
    case value_type_bool: fprintf(f, "bool %llu", value->number); break;
    case value_type_int: fprintf(f, "int %llu", value->number); break;
    case value_type_text: fprintf(f, "text '%s'", value->text); break;
    case value_type_node:
      fprintf(f, "%s node", node_type(value->node));
      if (DDSTS_IS_DEFINITION(value->node)) {
        fprintf(f, " '%s'", value->node->type.name);
      }
      break;
    case value_type_expr: fprintf(f, "expr at %lu.%lu", value->expr->pos.line, value->expr->pos.column); break;
    default: fprintf(f, "unknown"); break;
  }
}

struct scope_t {
  const char *var;
  value_t value;
  scope_t *prev;
};

EVAL_FUNC(eval_nop);

static void init_expr(expr_t *expr)
{
  expr->eval = eval_nop;
  expr->text = NULL;
  expr->number = 0ULL;
  expr->ch = '\0';
  expr->node = NULL;
  expr->proc_def = NULL;
  expr->children = NULL;
  expr->next = NULL;
  expr->pos.line = 0;
  expr->pos.column = 0;
}

static expr_t *create_expr()
{
  expr_t *expr = (expr_t*)ddsrt_malloc(sizeof(expr_t));
  if (expr == NULL) {
    return NULL;
  }
  init_expr(expr);
  return expr;
}

static void free_expr(expr_t *expr)
{
  while (expr != NULL) {
    expr_t *next = expr->next;
    free_expr(expr->children);
    ddsrt_free((void*)expr->text);
    ddsrt_free((void*)expr);
    expr = next;
  }
}

typedef struct proc_parameter_t proc_parameter_t;
struct proc_parameter_t {
  const char *name;
  proc_parameter_t *next;
};
struct proc_definition_t {
  const char *name;
  eval_func_p func;
  eval_func_p method;
  eval_func_p field;
  proc_parameter_t *proc_params;
  expr_t *proc_body;
  bool proc_called;
  proc_definition_t *next;
};

static void proc_definition_add_param(proc_definition_t *proc_def, const char *name)
{
  proc_parameter_t **ref_param = &proc_def->proc_params;
  while (*ref_param != NULL)
    ref_param = &(*ref_param)->next;
  *ref_param = (proc_parameter_t*)ddsrt_malloc(sizeof(proc_parameter_t));
  if (*ref_param == NULL) {
    return;
  }
  (*ref_param)->name = name;
  (*ref_param)->next = NULL;
}


/* Parser */

typedef struct {
  scanner_t scanner;
  proc_definition_t *proc_defs;
  bool error;
} parser_t;

static void parser_error(parser_t *parser, const char *message)
{
  printf("%lu.%lu: Error: %s\n", parser->scanner.start.line, parser->scanner.start.column, message);
  parser->error = true;
}

static void parser_error_ident(parser_t *parser, const char *message, const char *ident)
{
  printf("%lu.%lu: Error: %s '%s'\n", parser->scanner.start.line, parser->scanner.start.column, message, ident);
  parser->error = true;
}

static char *parser_claim_text(parser_t *parser)
{
  char *text = parser->scanner.text;
  parser->scanner.text = NULL;
  return text;
}

static bool parser_expect_sym(parser_t *parser, char symbol, char *descr)
{
  if (parser->scanner.symbol != symbol) {
    parser_error(parser, descr);
    return false;
  }
  scanner_next_token(&parser->scanner);
  return true;
}

static bool parser_accept_sym(parser_t *parser, char symbol)
{
  if (parser->scanner.symbol != symbol) {
    return false;
  }
  scanner_next_token(&parser->scanner);
  return true;
}

static proc_definition_t *parser_add_proc(parser_t *parser, const char *name)
{
  proc_definition_t **ref_proc = &parser->proc_defs;
  while (*ref_proc != NULL && strcmp((*ref_proc)->name, name) < 0)
    ref_proc = &(*ref_proc)->next;
  if (*ref_proc != NULL && strcmp((*ref_proc)->name, name) == 0) {
    return *ref_proc;
  }
  proc_definition_t *new_proc = (proc_definition_t*)ddsrt_malloc(sizeof(proc_definition_t));
  if (new_proc == NULL) {
    return NULL;
  }
  new_proc->name = ddsrt_strdup(name);
  if (new_proc->name == NULL) {
    ddsrt_free((void*)new_proc);
    return NULL;
  }
  new_proc->func = 0;
  new_proc->method = 0;
  new_proc->field = 0;
  new_proc->proc_params = NULL;
  new_proc->proc_body = NULL;
  new_proc->proc_called = false;
  new_proc->next = *ref_proc;
  *ref_proc = new_proc;
  return new_proc;
}

/* Parsing functions */

static bool parse_or_expr(parser_t *parser, expr_t **ref_expr);
static bool parse_statements(parser_t *parser, expr_t **ref_expr);

EVAL_FUNC(eval_proc_call);
EVAL_FUNC(eval_var);
EVAL_FUNC(eval_number_const);
EVAL_FUNC(eval_string_const);
EVAL_FUNC(eval_bool_const);

static bool parse_term(parser_t *parser, expr_t **ref_expr)
{
  if (parser_accept_sym(parser, '(')) {
    return    parse_or_expr(parser, ref_expr)
           && parser_expect_sym(parser, ')', "expect ')'");
  }
  else if (parser->scanner.symbol == SCANNER_SYMBOL_IDENT) {
    expr_t *expr = NULL;
    for (;;) {
      expr_t *new_expr = create_expr();
      if (new_expr == NULL) {
        free_expr(expr);
        return false;
      }
      new_expr->pos = parser->scanner.start;
      char *name = parser_claim_text(parser);
      scanner_next_token(&parser->scanner);
      if (parser_accept_sym(parser, '(')) {
        proc_definition_t *proc_def = parser_add_proc(parser, name);
        if (proc_def == NULL) {
          free_expr(new_expr);
          free_expr(expr);
          return false;
        }
        expr_t **ref_next_param;
        if (expr == NULL) {
          if (proc_def->func != 0) {
            new_expr->eval = proc_def->func;
          }
          else {
            new_expr->eval = eval_proc_call;
            new_expr->proc_def = proc_def;
            proc_def->proc_called = true;
          }
          ref_next_param = &new_expr->children;
        }
        else {
          if (proc_def->method != 0) {
            new_expr->eval = proc_def->method;
          }
          else {
            parser_error_ident(parser, "unknown method", name);
          }
          new_expr->children = expr;
          ref_next_param = &expr->next;
        }
        for (;;) {
          if (parser_accept_sym(parser, '{')) {
            parse_statements(parser, ref_next_param);
            if (!parser_expect_sym(parser, '}', "expect '}'")) {
              return false;
            }
          }
          else if (!parse_or_expr(parser, ref_next_param)) {
            break;
          }
          ref_next_param = &(*ref_next_param)->next;
          if (!parser_accept_sym(parser, ',')) {
            break;
          }
        }
        if (!parser_expect_sym(parser, ')', "expect ')'")) {
          return false;
        }
      }
      else {
        if (expr == NULL) {
          new_expr->eval = eval_var;
          new_expr->text = name;
          name = NULL;
        }
        else {
          proc_definition_t *proc_def = parser_add_proc(parser, name);
          if (proc_def == NULL) {
            free_expr(new_expr);
            free_expr(expr);
            return false;
          }
          if (proc_def->field != 0) {
            new_expr->eval = proc_def->field;
          }
          else {
            parser_error_ident(parser, "unknown field", name);
          }
          new_expr->children = expr;
        }
      }
      ddsrt_free((void*)name);
      expr = new_expr;
      if (!parser_accept_sym(parser, '.')) {
        break;
      }
    }
    *ref_expr = expr;
  }
  else if (parser->scanner.symbol == SCANNER_SYMBOL_NUMBER) {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_number_const;
    if (ddsrt_strtoull(parser->scanner.text, NULL, 0, &(*ref_expr)->number) != DDS_RETCODE_OK) {
      return false;
    }
    (*ref_expr)->pos = parser->scanner.start;
    scanner_next_token(&parser->scanner);
  }
  else if (parser->scanner.symbol == SCANNER_SYMBOL_STRING) {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_string_const;
    (*ref_expr)->text = parser_claim_text(parser);
    (*ref_expr)->pos = parser->scanner.start;
    scanner_next_token(&parser->scanner);
  }
  else if (   parser->scanner.symbol == SCANNER_SYMBOL_TRUE
           || parser->scanner.symbol == SCANNER_SYMBOL_FALSE) {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_bool_const;
    (*ref_expr)->number = parser->scanner.symbol == SCANNER_SYMBOL_TRUE ? 1 : 0;
    (*ref_expr)->pos = parser->scanner.start;
    scanner_next_token(&parser->scanner);
  }
  else {
    /*parser_error(parser, "Expect expression term");*/
    return false;
  }
  return true;
}

EVAL_FUNC(eval_not);

static bool parse_prefix_expr(parser_t *parser, expr_t **ref_expr)
{
  if (parser_accept_sym(parser, '!')) {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_not;
    (*ref_expr)->pos = parser->scanner.start;
    return parse_term(parser, &(*ref_expr)->children);
  }
  else {
    return parse_term(parser, ref_expr);
  }
}

EVAL_FUNC(eval_times);

static bool parse_mul_expr(parser_t *parser, expr_t **ref_expr)
{
  expr_t *expr = NULL;
  if (!parse_prefix_expr(parser, &expr)) {
    return false;
  }
  while (parser_accept_sym(parser, '*')) {
    expr_t *new_expr = create_expr();
    if (new_expr == NULL) {
      free_expr(expr);
      return false;
    }
    new_expr->eval = eval_times;
    new_expr->children = expr;
    new_expr->pos = parser->scanner.start;
    parse_prefix_expr(parser, &expr->next);
    expr = new_expr;
  }
  *ref_expr = expr;
  return true;
}

EVAL_FUNC(eval_add);

static bool parse_add_expr(parser_t *parser, expr_t **ref_expr)
{
  expr_t *expr = NULL;
  if (!parse_mul_expr(parser, &expr)) {
    return false;
  }
  while (parser_accept_sym(parser, '+')) {
    expr_t *new_expr = create_expr();
    if (new_expr == NULL) {
      free_expr(expr);
      return false;
    }
    new_expr->eval = eval_add;
    new_expr->children = expr;
    new_expr->pos = parser->scanner.start;
    parse_mul_expr(parser, &expr->next);
    expr = new_expr;
  }
  *ref_expr = expr;
  return true;
}

EVAL_FUNC(eval_equal);

static bool parse_compare_expr(parser_t *parser, expr_t **ref_expr)
{
  expr_t *expr = NULL;
  if (!parse_add_expr(parser, &expr)) {
    return false;
  }
  if (parser_accept_sym(parser, SCANNER_SYMBOL_EQUAL)) {
    expr_t *new_expr = create_expr();
    if (new_expr == NULL) {
      free_expr(expr);
      return false;
    }
    new_expr->eval = eval_equal;
    new_expr->children = expr;
    new_expr->pos = parser->scanner.start;
    parse_add_expr(parser, &expr->next);
    expr = new_expr;
  }
  *ref_expr = expr;
  return true;
}

EVAL_FUNC(eval_and);

static bool parse_and_expr(parser_t *parser, expr_t **ref_expr)
{
  expr_t *expr = NULL;
  if (!parse_compare_expr(parser, &expr)) {
    return false;
  }
  while (parser_accept_sym(parser, SCANNER_SYMBOL_AND)) {
    expr_t *new_expr = create_expr();
    if (new_expr == NULL) {
      free_expr(expr);
      return false;
    }
    new_expr->eval = eval_and;
    new_expr->children = expr;
    new_expr->pos = parser->scanner.start;
    parse_compare_expr(parser, &expr->next);
    expr = new_expr;
  }
  *ref_expr = expr;
  return true;
}

EVAL_FUNC(eval_or);

static bool parse_or_expr(parser_t *parser, expr_t **ref_expr)
{
  expr_t *expr = NULL;
  if (!parse_and_expr(parser, &expr)) {
    return false;
  }
  while (parser_accept_sym(parser, SCANNER_SYMBOL_OR)) {
    expr_t *new_expr = create_expr();
    if (new_expr == NULL) {
      free_expr(expr);
      return false;
    }
    new_expr->eval = eval_or;
    new_expr->children = expr;
    new_expr->pos = parser->scanner.start;
    parse_and_expr(parser, &expr->next);
  }
  *ref_expr = expr;
  return true;
}

EVAL_FUNC(eval_if);
EVAL_FUNC(eval_for);
EVAL_FUNC(eval_switch);
EVAL_FUNC(eval_emit);

static bool parse_statement(parser_t *parser, expr_t **ref_expr)
{
  if (parser_accept_sym(parser, '{')) {
    parse_statements(parser, ref_expr);
    if (!parser_expect_sym(parser, '}', "expect '}'")) {
      return false;
    }
  }
  else if (parser_accept_sym(parser, SCANNER_SYMBOL_IF)) {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_if;
    (*ref_expr)->pos = parser->scanner.start;
    if (   !parser_expect_sym(parser, '(', "expect '('")
        || !parse_or_expr(parser, &(*ref_expr)->children)
        || !parser_expect_sym(parser, ')', "expect ')'")
        || !parse_statement(parser, &(*ref_expr)->children->next)) {
      return false;
    }
    if (parser_accept_sym(parser, SCANNER_SYMBOL_ELSE)) {
      parse_statement(parser, &(*ref_expr)->children->next->next);
    }
  }
  else if (parser_accept_sym(parser, SCANNER_SYMBOL_FOR)) {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_for;
    (*ref_expr)->pos = parser->scanner.start;
    if (parser->scanner.symbol != SCANNER_SYMBOL_IDENT) {
      parser_error(parser, "expect variable");
      return false;
    }
    (*ref_expr)->text = parser_claim_text(parser);
    scanner_next_token(&parser->scanner);
    if (   !parser_expect_sym(parser, SCANNER_SYMBOL_IN, "expect 'in'")
        || !parse_or_expr(parser, &(*ref_expr)->children)) {
      return false;
    }
    parse_statement(parser, &(*ref_expr)->children->next);
  }
  else if (parser_accept_sym(parser, SCANNER_SYMBOL_SWITCH)) {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_switch;
    (*ref_expr)->pos = parser->scanner.start;
    if (   !parser_expect_sym(parser, '(', "expect '('")
        || !parse_or_expr(parser, &(*ref_expr)->children)
        || !parser_expect_sym(parser, ')', "expect ')'")
        || !parser_expect_sym(parser, '{', "expect '{'")) {
      return false;
    }
    expr_t **ref_next = &(*ref_expr)->children->next;
    while (parser_accept_sym(parser, SCANNER_SYMBOL_CASE)) {
      *ref_next = create_expr();
      if (*ref_next == NULL) {
        return false;
      }
      (*ref_next)->ch = 'c';
      (*ref_expr)->pos = parser->scanner.start;
      if (   !parse_or_expr(parser, &(*ref_next)->children)
          || !parser_expect_sym(parser, ':', "expect ':'")) {
        return false;
      }
      parse_statements(parser, &(*ref_next)->children->next);
      ref_next = &(*ref_next)->next;
    }
    if (parser_accept_sym(parser, SCANNER_SYMBOL_DEFAULT)) {
      *ref_next = create_expr();
      if (*ref_next == NULL) {
        return false;
      }
      (*ref_next)->ch = 'd';
      (*ref_expr)->pos = parser->scanner.start;
      if (!parser_expect_sym(parser, ':', "expect ':'")) {
        return false;
      }
      parse_statements(parser, &(*ref_next)->children);
      ref_next = &(*ref_next)->next;
    }
    if (!parser_expect_sym(parser, '}', "expect '}'")) {
      return false;
    }
  }
  else if (parser->scanner.symbol == SCANNER_SYMBOL_STRING)
  {
    *ref_expr = create_expr();
    if (*ref_expr == NULL) {
      return false;
    }
    (*ref_expr)->eval = eval_emit;
    (*ref_expr)->text = parser_claim_text(parser);
    (*ref_expr)->pos = parser->scanner.start;
    scanner_next_token(&parser->scanner);
    if (parser_accept_sym(parser, SCANNER_SYMBOL_WITH)) {
      expr_t **ref_next = &(*ref_expr)->children;
      for (; parser->scanner.symbol == SCANNER_SYMBOL_IDENT && strlen(parser->scanner.text) == 1;) {
        *ref_next = create_expr();
        if (*ref_expr == NULL) {
          return false;
        }
        (*ref_next)->ch = parser->scanner.text[0];
        (*ref_expr)->pos = parser->scanner.start;
        scanner_next_token(&parser->scanner);
        if (!parser_expect_sym(parser, '=', "expect '='")) {
          return false;
        }
        if (parser_accept_sym(parser, '{')) {
          parse_statements(parser, &(*ref_next)->children);
          if (!parser_expect_sym(parser, '}', "expect '}'")) {
            return false;
          }
        }
        else
          parse_or_expr(parser, &(*ref_next)->children);
        if (!parser_accept_sym(parser, ',')) {
          break;
        }
        ref_next = &(*ref_next)->next;
      }
    }
    if (!parser_expect_sym(parser, ';', "expect ';'")) {
      return false;
    }
  }
  else {
    return    parse_or_expr(parser, ref_expr)
           && parser_expect_sym(parser, ';', "expect ':'");
  }
  return true;
}

EVAL_FUNC(eval_statements);

static bool parse_statements(parser_t *parser, expr_t **ref_expr)
{
  *ref_expr = create_expr();
  if (*ref_expr == NULL) {
    return false;
  }
  (*ref_expr)->eval = eval_statements;
  (*ref_expr)->pos = parser->scanner.start;
  expr_t **ref_next = &(*ref_expr)->children;
  while (parse_statement(parser, ref_next)) {
    ref_next = &(*ref_next)->next;
  }
  return true;
}

static bool parse_definitions(parser_t *parser)
{
  while (parser->scanner.symbol == SCANNER_SYMBOL_IDENT) {
    fprintf(stderr, "parse_definition %s\n", parser->scanner.text);
    proc_definition_t *proc = parser_add_proc(parser, parser->scanner.text);
    if (proc == NULL) {
      return false;
    }
    if (proc->proc_body != NULL) {
      fprintf(stderr, "proc already defined\n");
      return false;
    }
    scanner_next_token(&parser->scanner);
    if (!parser_expect_sym(parser, '(', "expect '('")) {
      return false;
    }
    for (; parser->scanner.symbol == SCANNER_SYMBOL_IDENT;) {
      char *param = parser_claim_text(parser);
      proc_definition_add_param(proc, param);
      scanner_next_token(&parser->scanner);
      if (!parser_accept_sym(parser, ',')) {
        break;
      }
    }
    if (   !parser_expect_sym(parser, ')', "expect parameter or ')'")
        || !parser_expect_sym(parser, '{', "expect '{'")) {
      return false;
    }
    parse_statements(parser, &proc->proc_body);
    if (!parser_expect_sym(parser, '}', "expect '}'")) {
      return false;
    }
  }
  if (parser->scanner.symbol != SCANNER_SYMBOL_EOF) {
    parser_error(parser, "Unexpected input");
    printf("|%c|\n", parser->scanner.symbol);
  }
  return true;
}

static void parser_add_func_func(parser_t *parser, const char *name, eval_func_p func)
{
  proc_definition_t *proc = parser_add_proc(parser, name);
  if (proc != NULL) {
    proc->func = func;
  }
}

static void parser_add_method_func(parser_t *parser, const char *name, eval_func_p method)
{
  proc_definition_t *proc = parser_add_proc(parser, name);
  if (proc != NULL) {
    proc->method = method;
  }
}

static void parser_add_field_func(parser_t *parser, const char *name, eval_func_p field)
{
  proc_definition_t *proc = parser_add_proc(parser, name);
  if (proc != NULL) {
    proc->field = field;
  }
}

EVAL_FUNC(eval_field_name);
EVAL_FUNC(eval_field_type);
EVAL_FUNC(eval_field_tree_parent);
EVAL_FUNC(eval_field_decl_type);
EVAL_FUNC(eval_field_element_type);
EVAL_FUNC(eval_field_bounded);
EVAL_FUNC(eval_field_max);
EVAL_FUNC(eval_field_size);
EVAL_FUNC(eval_field_is_bounded);
EVAL_FUNC(eval_field_definition);
EVAL_FUNC(eval_method_hasProperty);
EVAL_FUNC(eval_method_getProperty);
EVAL_FUNC(eval_method_setProperty);
EVAL_FUNC(eval_method_removeProperties);
EVAL_FUNC(eval_func_file_name);
EVAL_FUNC(eval_func_toStr);
EVAL_FUNC(eval_func_toUpper);
EVAL_FUNC(eval_func_root);
EVAL_FUNC(eval_func_now);
EVAL_FUNC(eval_func_streamToFile);

static void parser_init(parser_t *parser, FILE *file)
{
  scanner_init(&parser->scanner, file);
  parser->proc_defs = NULL;
  parser->error = false;
  parser_add_field_func(parser, "name", eval_field_name);
  parser_add_field_func(parser, "type", eval_field_type);
  parser_add_field_func(parser, "tree_parent", eval_field_tree_parent);
  parser_add_field_func(parser, "decl_type", eval_field_decl_type);
  parser_add_field_func(parser, "element_type", eval_field_element_type);
  parser_add_field_func(parser, "bounded", eval_field_bounded);
  parser_add_field_func(parser, "max", eval_field_max);
  parser_add_field_func(parser, "size", eval_field_size);
  parser_add_field_func(parser, "is_bounded", eval_field_is_bounded);
  parser_add_field_func(parser, "definition", eval_field_definition);
  parser_add_method_func(parser, "hasProperty", eval_method_hasProperty);
  parser_add_method_func(parser, "getProperty", eval_method_getProperty);
  parser_add_method_func(parser, "setProperty", eval_method_setProperty);
  parser_add_method_func(parser, "removeProperties", eval_method_removeProperties);
  parser_add_func_func(parser, "file_name", eval_func_file_name);
  parser_add_func_func(parser, "toStr", eval_func_toStr);
  parser_add_func_func(parser, "toUpper", eval_func_toUpper);
  parser_add_func_func(parser, "root", eval_func_root);
  parser_add_func_func(parser, "now", eval_func_now);
  parser_add_func_func(parser, "streamToFile", eval_func_streamToFile);
}

static void parser_fini(parser_t *parser)
{
  scanner_fini(&parser->scanner);
}

/* Script execution */

typedef struct property_t property_t;
typedef struct call_stack_entry_t call_stack_entry_t;
typedef struct break_point_t break_point_t;
typedef struct debug_ostream_t debug_ostream_t;

struct call_stack_entry_t {
  const char *name;
  expr_t *expr;
  scope_t *scope;
  call_stack_entry_t *up;
  call_stack_entry_t *down;
};

struct exec_context_t {
  ostream_t *ostream;
  ddsts_type_t *root;
  const char *file_name;
  property_t *properties;
/* For debugging: */
  debug_ostream_t *debug_ostream;
  call_stack_entry_t *bottom;
  call_stack_entry_t top;
  break_point_t *break_points;
  bool one_step;
  expr_t *next_stop;
};

struct property_t {
  value_t key1;
  value_t key2;
  value_t value;
  property_t *next;
};

struct break_point_t {
  char *file_name;
  unsigned long line;
  unsigned long column;
  break_point_t *next;
};

break_point_t *create_break_point(char *file_name, unsigned long line, unsigned long column)
{
  break_point_t *break_point = (break_point_t*)ddsrt_malloc(sizeof(break_point));
  if (break_point == NULL) {
    return NULL;
  }
  break_point->file_name = file_name != NULL ? ddsrt_strdup(file_name) : NULL;
  break_point->line = line;
  break_point->column = column;
  return break_point;
}

void free_break_point(break_point_t *break_point)
{
  ddsrt_free((void*)break_point->file_name);
  ddsrt_free((void*)break_point);
}


static void exec_context_init(exec_context_t *context, ostream_t *ostream, ddsts_type_t *root, const char *file_name, bool debugging, debug_ostream_t *debug_ostream) {
  context->ostream = ostream;
  context->root = root;
  context->file_name = file_name;
  context->properties = NULL;
  context->debug_ostream = debug_ostream;
  context->bottom = &context->top;
  context->top.name = "<top>";
  context->top.expr = NULL;
  context->top.scope = NULL;
  context->top.up = NULL;
  context->top.down = NULL;
  context->break_points = NULL;
  context->one_step = debugging;
  context->next_stop = NULL;
}

static void exec_context_fini(exec_context_t *context)
{
  property_t *property = context->properties;
  while (property != NULL) {
    property_t *next = property->next;
    ddsrt_free((void*)property);
    property = next;
  }
}

static bool exec_context_get_property(exec_context_t *context, value_t *key1, value_t *key2, value_t *value)
{
  property_t *prop = context->properties;
  for (; prop != NULL; prop = prop->next) {
    int comp = value_comp(&prop->key1, key1);
    if (comp == 0) {
      comp = value_comp(&prop->key2, key2);
    }
    if (comp == 0) {
      *value = prop->value;
      return true;
    }
    if (comp > 0) {
      break;
    }
  }
  return false;
}

static void exec_context_set_property(exec_context_t *context, value_t *key1, value_t *key2, value_t *value)
{
  property_t **ref_prop = &context->properties;
  for (; *ref_prop != NULL; ref_prop = &(*ref_prop)->next) {
    int comp = value_comp(&(*ref_prop)->key1, key1);
    if (comp == 0) {
      comp = value_comp(&(*ref_prop)->key2, key2);
    }
    if (comp == 0) {
      (*ref_prop)->value = *value;
      return;
    }
    if (comp > 0) {
      break;
    }
  }
  property_t *new_prop = (property_t*)ddsrt_malloc(sizeof(property_t));
  if (new_prop == NULL) {
    return;
  }
  new_prop->key1 = *key1;
  new_prop->key2 = *key2;
  new_prop->value = *value;
  new_prop->next = *ref_prop;
  *ref_prop = new_prop;
}

static void exec_context_remove_properties(exec_context_t *context, value_t *key1, value_t *key2)
{
  property_t **ref_prop = &context->properties;
  while (*ref_prop != NULL) {
    bool match = false;
    if (value_comp(&(*ref_prop)->key2, key2) == 0) {
      if (key2->type != value_type_node || key1->type != value_type_node) {
        match = true;
      }
      else {
        for (ddsts_type_t *node = (*ref_prop)->key1.node; node != NULL; node = node->type.parent) {
          if (node == key1->node) {
            match = true;
            break;
          }
        }
      }
    }
    if (match) {
      property_t *prop = *ref_prop;
      *ref_prop = (*ref_prop)->next;
      ddsrt_free((void*)prop);
    }
    else {
      ref_prop = &(*ref_prop)->next;
    }
  }
}

struct debug_ostream_t {
  ostream_t ostream;
  bool debugging;
  bool emitted;
  char *output_file_name;
  file_pos_t output_pos;
  ostream_t *output;
};

bool debug_ostream_open(ostream_t *ostream, const char *name)
{
  debug_ostream_t* debug_ostream = (debug_ostream_t*)ostream;
  if (debug_ostream->debugging) {
    if (debug_ostream->emitted) {
      fprintf(stdout, "]\n");
      debug_ostream->emitted = false;
    }
    ddsrt_free((void*)debug_ostream->output_file_name);
    debug_ostream->output_file_name = ddsrt_strdup(name);
    debug_ostream->output_pos.line = 1UL;
    debug_ostream->output_pos.column = 1UL;
    fprintf(stdout, "Open file '%s'\n", name);
  }
  return ostream_open(debug_ostream->output, name);
}

void debug_ostream_close(ostream_t *ostream)
{
  debug_ostream_t* debug_ostream = (debug_ostream_t*)ostream;
  if (debug_ostream->debugging) {
    if (debug_ostream->emitted) {
      fprintf(stdout, "]\n");
      debug_ostream->emitted = false;
    }
    fprintf(stdout,"Close file '%s'\n", debug_ostream->output_file_name);
    ddsrt_free((void*)debug_ostream->output_file_name);
    debug_ostream->output_file_name = NULL;
    debug_ostream->output_pos.line = 0UL;
    debug_ostream->output_pos.column = 0UL;
  }
  ostream_close(((debug_ostream_t*)ostream)->output);
}

void debug_ostream_put(ostream_t *ostream, char ch)
{
  debug_ostream_t* debug_ostream = (debug_ostream_t*)ostream;
  if (debug_ostream->debugging) {
    if (!debug_ostream->emitted) {
      fprintf(stdout, "[");
      debug_ostream->emitted = true;
    }
    fprintf(stdout, "%c", ch);
    if (ch == '\n') {
      debug_ostream->output_pos.line++;
      debug_ostream->output_pos.column = 1UL;
    }
    else {
      debug_ostream->output_pos.column++;
    }
  }
  ostream_put(debug_ostream->output, ch);
}

void debug_ostream_init(debug_ostream_t *ostream, bool debugging, ostream_t *output)
{
  ostream->ostream.open = debug_ostream_open;
  ostream->ostream.close = debug_ostream_close;
  ostream->ostream.put = debug_ostream_put;
  ostream->emitted = false;
  ostream->debugging = debugging;
  ostream->output_file_name = NULL;
  ostream->output_pos.line = 0UL;
  ostream->output_pos.column = 0UL;
  ostream->output = output;
}

void debug_ostream_fini(debug_ostream_t *ostream)
{
  ddsrt_free((void*)ostream->output_file_name);
}


void exec_context_break(exec_context_t *exec_context)
{
  if (exec_context->debug_ostream->emitted) {
    fprintf(stdout, "]\n");
    exec_context->debug_ostream->emitted = false;
  }
}

void exec_context_debug(exec_context_t *exec_context, call_stack_entry_t *call_stack_entry)
{
  exec_context_break(exec_context);
  exec_context->one_step = false;
  exec_context->next_stop = NULL;
  call_stack_entry_t *here = call_stack_entry;
  for (;;) {
    fprintf(stdout, "At %lu.%lu at %s expression ", here->expr->pos.line, here->expr->pos.column, here->name);
    if (here->expr->text != NULL) {
      fprintf(stdout, " text='%s'", here->expr->text);
    }
    if (here->expr->ch != '\0') {
      fprintf(stdout, " ch='%c'", here->expr->ch);
    }
    if (here->expr->proc_def != NULL) {
      fprintf(stdout, " proc=%s", here->expr->proc_def->name);
    }
    if (here->expr->children != NULL) {
      fprintf(stdout, " children");
    }
    fprintf(stdout, "\n> ");
    char command[100];
    fgets(command, 99, stdin);
    for (char *s = command; *s != '\0'; s++) {
      if (*s == '\n') {
        *s = '\0';
      }
    }
    if (strcmp(command, "") == 0) {
      exec_context->one_step = true;
      break;
    }
    else if (strcmp(command, "skip") == 0) {
      for (call_stack_entry_t *entry = call_stack_entry; entry != NULL; entry = entry->up) {
        if (entry->expr != NULL && entry->expr->next != NULL) {
          exec_context->next_stop = entry->expr->next;
          break;
        }
      }
      break;
    }
    else if (strcmp(command, "cont") == 0) {
      break;
    }
    else if (strcmp(command, "up") == 0) {
      if (here->up == NULL || here->up->expr == NULL) {
        fprintf(stdout, "At top\n");
      }
      else {
        here = here->up;
      }
    }
    else if (strcmp(command, "down") == 0) {
      if (here->down == NULL) {
        fprintf(stdout, "At bottom\n");
      }
      else {
        here = here->down;
      }
    }
    else if (strcmp(command, "p") == 0) {
      for (scope_t *scope = here->scope; scope != NULL; scope = scope->prev) {
        fprintf(stdout, " %s = ", scope->var);
        value_print(&scope->value, stdout);
        fprintf(stdout, "\n");
      }
    }
    else if (   strncmp(command, "break ", 6) == 0
             || strncmp(command, "clear ", 6) == 0)
    {
      char *s = command + 6;
      while (*s == ' ') {
        s++;
      }
      char *file_name = NULL;
      if (!('0' <= *s && *s <= '9')) {
        file_name = s;
        while (*s != '\0' && *s != ':') {
          s++;
        }
        if (*s == ':') {
          *s = '\0';
          s++;
        }
        fprintf(stdout, "File '%s'\n", file_name);
      }
      fprintf(stdout, " at '%s' \n", s);
      unsigned long line = 0UL;
      for (; '0' <= *s && *s <= '9'; s++) {
        line = 10UL * line + (unsigned long)(*s - '0');
      }
      unsigned long column = 0UL;
      if (*s == '.') {
        for (s++ ; '0' <= *s && *s <= '9'; s++) {
          column = 10UL * column + (unsigned long)(*s - '0');
        }
      }
      if (line == 0UL) {
        fprintf(stdout, "Do not understand '%s'\n", command);
      }
      else if (*command == 'b') {
        break_point_t *new_break_point = create_break_point(file_name, line, column);
        if (new_break_point != NULL) {
          new_break_point->next = exec_context->break_points;
          exec_context->break_points = new_break_point;
        }
      }
      else {
        break_point_t **ref_break_point = &exec_context->break_points;
        while ((*ref_break_point) != NULL) {
          if (   (*ref_break_point)->line == line
              && (   (*ref_break_point)->column == 0
                  || column == 0
                  || (*ref_break_point)->column == column)
              && (   ((*ref_break_point)->file_name == NULL && file_name == NULL)
                  || (   (*ref_break_point)->file_name != NULL && file_name != NULL
                      && strcmp((*ref_break_point)->file_name, file_name) == 0))) {
             break_point_t *old_break_point = *ref_break_point;
             *ref_break_point = (*ref_break_point)->next;
             free_break_point(old_break_point);
          }
          else {
            ref_break_point = &(*ref_break_point)->next;
          }
        }
      }
    }
    else {
      fprintf(stdout, "Do not understand '%s'\n", command);
    }
  }
}

void exec_context_enter(exec_context_t *exec_context, const char* name, call_stack_entry_t *call_stack_entry, scope_t *scope, expr_t *expr, value_t *result)
{
  call_stack_entry->name = name;
  call_stack_entry->expr = expr;
  call_stack_entry->scope = scope;
  call_stack_entry->up = exec_context->bottom;
  exec_context->bottom->down = call_stack_entry;
  exec_context->bottom = call_stack_entry;
  bool stop = exec_context->one_step || exec_context->next_stop == expr;
  if (!stop) {
    for (break_point_t *break_point = exec_context->break_points; break_point != NULL; break_point = break_point->next) {
      if (  break_point->file_name != NULL
          ?    break_point->line == exec_context->debug_ostream->output_pos.line
            && (break_point->column == 0 || break_point->column == exec_context->debug_ostream->output_pos.column)
            && strcmp(break_point->file_name, exec_context->debug_ostream->output_file_name) == 0
          :    break_point->line == expr->pos.line
            && (break_point->column == 0 || break_point->column == expr->pos.column)) {
        stop = true;
        break;
      }
    }
  }
  if (!value_valid(result)) {
    fprintf(stdout, "Error: result invalid\n");
    stop = true;
  }
  if (stop) {
    exec_context_debug(exec_context, call_stack_entry);
  }
}

void exec_context_leave(exec_context_t *exec_context, call_stack_entry_t *call_stack_entry, value_t *result)
{
  if (value_valid(result)) {
    if (exec_context->one_step) {
      fprintf(stdout, "Result: ");
      value_print(result, stdout);
      fprintf(stdout, "\n");
    }
  }
  else {
    exec_context_break(exec_context);
    fprintf(stdout, "Result invalid on exit\n");
    exec_context_debug(exec_context, call_stack_entry);
  }
  exec_context->bottom = exec_context->bottom->up;
  exec_context->bottom->down = NULL;
}

static bool check_script(parser_t *parser);

extern void dds_exec_templ_script(const char* file_name, ddsts_type_t *root_node, FILE* tsf, bool debugging)
{
  printf("dds_exec_templ_script\n");

  parser_t parser;
  parser_init(&parser, tsf);

  if (!parse_definitions(&parser) || parser.error) {
    fprintf(stderr, "Parse errors\n");
    return;
  }

  if (!check_script(&parser)) {
    return;
  }

  ostream_to_files_t ostream_to_files;
  init_ostream_to_files(&ostream_to_files);
  debug_ostream_t debug_ostream;
  debug_ostream_init(&debug_ostream, debugging, (ostream_t*)&ostream_to_files);
  exec_context_t context;
  exec_context_init(&context, (ostream_t*)&debug_ostream, root_node, file_name, debugging, &debug_ostream);

  proc_definition_t *main_proc = parser_add_proc(&parser, "main");
  if (main_proc == NULL || main_proc->proc_body == NULL) {
    fprintf(stderr, "Missing 'main' proc\n");
  }
  else {
    scope_t scope;
    scope.var = main_proc->proc_params != NULL ? main_proc->proc_params->name : "root";
    scope.value.type = value_type_node;
    scope.value.node = root_node;
    scope.prev = NULL;
    value_t result;
    value_init(&result);
    main_proc->proc_body->eval(main_proc->proc_body, &scope, &context, &result);
  }
  exec_context_fini(&context);
  debug_ostream_fini(&debug_ostream);
  parser_fini(&parser);
}

/* ostream to string */

typedef struct {
  ostream_t ostream;
  string_appender_t string_appender;
} ostream_to_string_t;

void ostream_to_string_put(ostream_t *ostream, char ch)
{
  string_appender_add(&((ostream_to_string_t*)ostream)->string_appender, ch);
}

void ostream_to_string_init(ostream_to_string_t *ostream)
{
  ostream->ostream.open = NULL;
  ostream->ostream.close = NULL;
  ostream->ostream.put = ostream_to_string_put;
  string_appender_init(&ostream->string_appender);
}

char *ostream_to_string_take_string(ostream_to_string_t *ostream)
{
  char *result = string_appender_result(&ostream->string_appender);
  string_appender_free(&ostream->string_appender);
  return result;
}

/* Eval functions */

EVAL_FUNC(eval_nop)
{
  ENTER_EVAL_FUNC("nop")
  DDSRT_UNUSED_ARG(expr);
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  DDSRT_UNUSED_ARG(result);
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_field_name)
{
  ENTER_EVAL_FUNC("field 'name'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node == NULL || !DDSTS_IS_DEFINITION(value.node)) {
    EVAL_FUNC_ERROR_VALUE("does not have a name", &value);
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node->type.name == NULL) {
    //EVAL_FUNC_ERROR("NULL string")
    result->type = value_type_text;
    result->text = "(null)";
    LEAVE_EVAL_FUNC
    return false;
  }
  result->type = value_type_text;
  result->text = value.node->type.name;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_field_type)
{
  ENTER_EVAL_FUNC("field 'type'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node == NULL) {
    EVAL_FUNC_ERROR_VALUE("field 'type': is not a node, but ", &value);
    LEAVE_EVAL_FUNC
    return false;
  }
  result->type = value_type_text;
  result->text = node_type(value.node);
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_field_tree_parent)
{
  ENTER_EVAL_FUNC("field 'tree_parent'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node == NULL) {
    EVAL_FUNC_ERROR_VALUE("field 'tree_parent': is not a node, but ", &value);
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node->type.parent != NULL && value.node->type.parent->type.parent != NULL) {
    result->type = value_type_node;
    result->node = value.node->type.parent;
  }
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_field_decl_type)
{
  ENTER_EVAL_FUNC("field 'decl_type'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node == NULL) {
    EVAL_FUNC_ERROR_VALUE("value is not a type", &value);
    LEAVE_EVAL_FUNC
    return false;
  }
  if (DDSTS_IS_TYPE(value.node, DDSTS_DECLARATION)) {
    result->type = value_type_node;
    result->node = value.node->declaration.decl_type;
    LEAVE_EVAL_FUNC
    return true;
  }
  EVAL_FUNC_ERROR_VALUE("has no field 'decl_type'", &value);
  LEAVE_EVAL_FUNC
  return false;
}

EVAL_FUNC(eval_field_element_type)
{
  ENTER_EVAL_FUNC("field 'element_type'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node == NULL) {
    EVAL_FUNC_ERROR_VALUE("value is not a type", &value);
    LEAVE_EVAL_FUNC
    return false;
  }
  if (DDSTS_IS_TYPE(value.node, DDSTS_SEQUENCE)) {
    result->type = value_type_node;
    result->node = value.node->sequence.element_type;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (DDSTS_IS_TYPE(value.node, DDSTS_ARRAY)) {
    result->type = value_type_node;
    result->node = value.node->array.element_type;
    LEAVE_EVAL_FUNC
    return true;
  }
  EVAL_FUNC_ERROR_VALUE("has no field 'element_type'", &value);
  LEAVE_EVAL_FUNC
  return false;
}

EVAL_FUNC(eval_field_bounded)
{
  ENTER_EVAL_FUNC("field 'bounded'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_SEQUENCE)) {
    result->type = value_type_int;
    result->number = value.node->sequence.max > 0;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_STRING)) {
    result->type = value_type_int;
    result->number = value.node->string.max > 0;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_MAP)) {
    result->type = value_type_int;
    result->number = value.node->map.max > 0;
    LEAVE_EVAL_FUNC
    return true;
  }
  EVAL_FUNC_ERROR_VALUE("does not have field 'bounded'", &value);
  LEAVE_EVAL_FUNC
  return false;
}

EVAL_FUNC(eval_field_max)
{
  ENTER_EVAL_FUNC("field 'max'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_SEQUENCE)) {
    result->type = value_type_int;
    result->number = value.node->sequence.max;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_STRING)) {
    result->type = value_type_int;
    result->number = value.node->string.max;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_MAP)) {
    result->type = value_type_int;
    result->number = value.node->map.max;
    LEAVE_EVAL_FUNC
    return true;
  }
  EVAL_FUNC_ERROR_VALUE("does not have field 'max'", &value);
  LEAVE_EVAL_FUNC
  return false;
}

EVAL_FUNC(eval_field_size)
{
  ENTER_EVAL_FUNC("field 'size'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node == NULL || !DDSTS_IS_TYPE(value.node, DDSTS_ARRAY)) {
    EVAL_FUNC_ERROR_VALUE("does not have field 'size'", &value);
    LEAVE_EVAL_FUNC
    return false;
  }
  result->type = value_type_int;
  result->number = value.node->array.size;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_field_is_bounded)
{
  ENTER_EVAL_FUNC("field 'is_bounded'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_SEQUENCE)) {
    result->type = value_type_bool;
    result->number = value.node->sequence.max > 0;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_STRING)) {
    result->type = value_type_bool;
    result->number = value.node->string.max > 0;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_MAP)) {
    result->type = value_type_bool;
    result->number = value.node->map.max > 0;
    LEAVE_EVAL_FUNC
    return true;
  }
  EVAL_FUNC_ERROR_VALUE("does not have field 'is_bounded'", &value);
  LEAVE_EVAL_FUNC
  return false;
}

EVAL_FUNC(eval_field_definition)
{
  ENTER_EVAL_FUNC("field 'definition'")
  value_t value;
  value_init(&value);
  if (!expr->children->eval(expr->children, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value.node != NULL && DDSTS_IS_TYPE(value.node, DDSTS_FORWARD_STRUCT)) {
    result->node = value.node->forward.definition;
    if (result->node != NULL) {
      result->type = value_type_node;
    }
    LEAVE_EVAL_FUNC
    return true;
  }
  EVAL_FUNC_ERROR_VALUE("does not have field 'definition'", &value);
  LEAVE_EVAL_FUNC
  return false;
}

EVAL_FUNC(eval_func_file_name)
{
  ENTER_EVAL_FUNC("function 'file_name'")
  DDSRT_UNUSED_ARG(expr);
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  DDSRT_UNUSED_ARG(result);
  const char *s = context->file_name;
  const char *e = NULL;
  for (; *s != '\0'; s++) {
    if (*s == '.') {
      e = s;
    }
  }
  if (e == NULL) {
    e = s;
  }
  for (s = context->file_name; s < e; s++) {
    ostream_put(context->ostream, *s);
  }
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_method_hasProperty)
{
  ENTER_EVAL_FUNC("method 'hasProperty'")
  if (expr->children->next == NULL || expr->children->next->next != NULL) {
    EVAL_FUNC_ERROR("wrong number of arguments");
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key1;
  value_init(&key1);
  if (!expr->children->eval(expr->children, scope, context, &key1)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key2;
  value_init(&key2);
  if (!expr->children->next->eval(expr->children->next, scope, context, &key2)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t dummy_value;
  result->type = value_type_bool;
  result->number = exec_context_get_property(context, &key1, &key2, &dummy_value);
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_method_getProperty)
{
  ENTER_EVAL_FUNC("method 'getProperty'")
  if (expr->children->next == NULL || expr->children->next->next != NULL) {
    EVAL_FUNC_ERROR("wrong number of arguments");
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key1;
  value_init(&key1);
  if (!expr->children->eval(expr->children, scope, context, &key1)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key2;
  value_init(&key2);
  if (!expr->children->next->eval(expr->children->next, scope, context, &key2)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  exec_context_get_property(context, &key1, &key2, result);
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_method_setProperty)
{
  ENTER_EVAL_FUNC("method 'setProperty'")
  if (expr->children->next == NULL || expr->children->next->next == NULL || expr->children->next->next->next != NULL) {
    EVAL_FUNC_ERROR("wrong number of arguments");
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key1;
  value_init(&key1);
  if (!expr->children->eval(expr->children, scope, context, &key1)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key2;
  value_init(&key2);
  if (!expr->children->next->eval(expr->children->next, scope, context, &key2)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t value;
  value_init(&value);
  if (!expr->children->next->next->eval(expr->children->next->next, scope, context, &value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  result->type = value_type_bool;
  exec_context_set_property(context, &key1, &key2, &value);
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_method_removeProperties)
{
  ENTER_EVAL_FUNC("method 'setProperty'")
  if (expr->children->next == NULL || expr->children->next->next != NULL) {
    EVAL_FUNC_ERROR("wrong number of arguments");
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key1;
  value_init(&key1);
  if (!expr->children->eval(expr->children, scope, context, &key1)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t key2;
  value_init(&key2);
  if (!expr->children->next->eval(expr->children->next, scope, context, &key2)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  result->type = value_type_bool;
  exec_context_remove_properties(context, &key1, &key2);
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_func_toStr)
{
  ENTER_EVAL_FUNC("function 'toStr'")
  DDSRT_UNUSED_ARG(result);
  if (expr->children == NULL || expr->children->next != NULL) {
    EVAL_FUNC_ERROR("Expect one argument");
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t number;
  value_init(&number);
  if (!expr->children->eval(expr->children, scope, context, &number)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (number.type != value_type_int) {
    EVAL_FUNC_ERROR_VALUE("expect int value", &number);
    LEAVE_EVAL_FUNC
    return false;
  }
  char buffer[30];
  ddsrt_ulltostr(number.number, buffer, 30, NULL);
  ostream_puts(context->ostream, buffer);
  LEAVE_EVAL_FUNC
  return true;
}

typedef struct {
  ostream_t ostream;
  ostream_t *output;
} ostream_to_upper_t;

void ostream_to_upper_put(ostream_t *ostream, char ch)
{
  if ('a' <= ch && ch <= 'z') {
    ch = (char)(ch - 'a' + 'A');
  }
  ostream_put(((ostream_to_upper_t*)ostream)->output, ch);
}

void ostream_to_upper_init(ostream_to_upper_t *ostream, ostream_t *output)
{
  ostream->ostream.open = NULL;
  ostream->ostream.close = NULL;
  ostream->ostream.put = ostream_to_upper_put;
  ostream->output = output;
}

EVAL_FUNC(eval_func_toUpper)
{
  ENTER_EVAL_FUNC("function 'toUpper'")
  DDSRT_UNUSED_ARG(result);
  if (expr->children == NULL || expr->children->next != NULL) {
    EVAL_FUNC_ERROR("Expect one argument");
    LEAVE_EVAL_FUNC
    return false;
  }
  ostream_to_upper_t ostream_to_upper;
  ostream_to_upper_init(&ostream_to_upper, context->ostream);
  ostream_t *cur_ostream = context->ostream;
  context->ostream = &ostream_to_upper.ostream;
  value_t text;
  value_init(&text);
  if (!expr->children->eval(expr->children, scope, context, &text)) {
    context->ostream = cur_ostream;
    LEAVE_EVAL_FUNC
    return false;
  }
  context->ostream = cur_ostream;
  if (text.type == value_type_text) {
    for (const char *s = text.text; *s != '\0'; s++) {
      if ('a' <= *s && *s <= 'z') {
        ostream_put(context->ostream, (char)(*s - 'a' + 'A'));
      }
      else {
        ostream_put(context->ostream, *s);
      }
    }
  }
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_func_root)
{
  ENTER_EVAL_FUNC("function 'root'")
  DDSRT_UNUSED_ARG(expr);
  DDSRT_UNUSED_ARG(scope);
  result->type = value_type_node;
  result->node = context->root;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_func_now)
{
  ENTER_EVAL_FUNC("function 'now'")
  DDSRT_UNUSED_ARG(expr);
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(result);
  dds_time_t time_now = dds_time();
  char time_descr[DDSRT_RFC3339STRLEN+1];
  ddsrt_ctime(time_now, time_descr, DDSRT_RFC3339STRLEN);
  ostream_puts(context->ostream, time_descr);
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_func_streamToFile)
{
  ENTER_EVAL_FUNC("function 'streamToFile'")
  if (expr->children == NULL || expr->children->next == NULL || expr->children->next->next != NULL) {
    EVAL_FUNC_ERROR("Expect two arguments");
    LEAVE_EVAL_FUNC
    return false;
  }
  ostream_to_string_t ostream_to_string;
  ostream_to_string_init(&ostream_to_string);
  ostream_t *cur_ostream = context->ostream;
  context->ostream = &ostream_to_string.ostream;
  value_t dummy;
  value_init(&dummy);
  if (!expr->children->eval(expr->children, scope, context, &dummy)) {
    context->ostream = cur_ostream;
    LEAVE_EVAL_FUNC
    return false;
  }
  /* Check on dummy being not used? */
  context->ostream = cur_ostream;
  char *file_name = ostream_to_string_take_string(&ostream_to_string);
  if (file_name == NULL) {
    LEAVE_EVAL_FUNC
    return false;
  }
  ostream_open(context->ostream, file_name);
  ddsrt_free((void*)file_name);
  bool eval_result = expr->children->next->eval(expr->children->next, scope, context, result);
  ostream_close(context->ostream);
  LEAVE_EVAL_FUNC
  return eval_result;
}

EVAL_FUNC(eval_proc_call)
{
  ENTER_EVAL_FUNC("proc_call")
  if (expr->proc_def == NULL || expr->proc_def->proc_body == NULL) {
    EVAL_FUNC_ERROR("proc not defined");
    LEAVE_EVAL_FUNC
    return false;
  }
  scope_t *new_scope = NULL;
  expr_t *child_expr = expr->children;
  proc_parameter_t *param = expr->proc_def->proc_params;
  for (; child_expr != NULL && param != NULL; child_expr = child_expr->next, param = param->next) {
    scope_t *n_scope = (scope_t*)ddsrt_malloc(sizeof(scope_t));
    if (n_scope == NULL) {
    LEAVE_EVAL_FUNC
      return false;
    }
    n_scope->var = param->name;
    value_init(&n_scope->value);
    n_scope->value.type = value_type_expr;
    n_scope->value.scope = scope;
    n_scope->value.expr = child_expr;
    n_scope->prev = new_scope;
    new_scope = n_scope;
  }
  if (child_expr != NULL || param != NULL) {
    EVAL_FUNC_ERROR("number parameters does not match");
  }
  else {
    expr->proc_def->proc_body->eval(expr->proc_def->proc_body, new_scope, context, result);
  }

  while (new_scope != NULL) {
    scope_t *prev = new_scope->prev;
    ddsrt_free((void*)new_scope);
    new_scope = prev;
  }

  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_var)
{
  ENTER_EVAL_FUNC("var")
  DDSRT_UNUSED_ARG(expr);
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  DDSRT_UNUSED_ARG(result);
  for (; scope != NULL; scope = scope->prev) {
    /*
    fprintf(stderr, "scope->var = '%s'\n", scope->var == NULL ? "(null)" : scope->var);
    fprintf(stderr, "expr->text = '%s'\n", expr->text == NULL ? "(null)" : expr->text);
    */
    if (strcmp(scope->var, expr->text) == 0) {
      if (scope->value.type == value_type_expr) {
    LEAVE_EVAL_FUNC
        return scope->value.expr->eval(scope->value.expr, scope->value.scope, context, result);
      }
      *result = scope->value;
      LEAVE_EVAL_FUNC
      return true;
    }
  }
  EVAL_FUNC_ERROR("Undefined variable");
  LEAVE_EVAL_FUNC
  return false;
}

EVAL_FUNC(eval_number_const)
{
  ENTER_EVAL_FUNC("number_const")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  result->type = value_type_int;
  result->number = expr->number;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_string_const)
{
  ENTER_EVAL_FUNC("string_const")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  result->type = value_type_text;
  result->text = expr->text;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_bool_const)
{
  ENTER_EVAL_FUNC("bool_const")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  result->type = value_type_bool;
  result->number = expr->number;
  LEAVE_EVAL_FUNC
  return true;
}


EVAL_FUNC(eval_not)
{
  ENTER_EVAL_FUNC("not")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  if (!expr->children->eval(expr->children, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  result->type = value_type_bool;
  result->number = result->number != 0 ? 0 : 1;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_times)
{
  ENTER_EVAL_FUNC("times")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  if (!expr->children->eval(expr->children, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (result->type != value_type_int) {
    EVAL_FUNC_ERROR_VALUE("expect integer for lhs", result);
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t rhs;
  value_init(&rhs);
  if (!expr->children->next->eval(expr->children->next, scope, context, &rhs)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (rhs.type != value_type_int) {
    EVAL_FUNC_ERROR_VALUE("expect integer for rhs", &rhs);
    LEAVE_EVAL_FUNC
    return false;
  }
  result->number *= rhs.number;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_add)
{
  ENTER_EVAL_FUNC("add")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  if (!expr->children->eval(expr->children, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (result->type != value_type_int) {
    EVAL_FUNC_ERROR_VALUE("expect integer for lhs", result);
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t rhs;
  value_init(&rhs);
  if (!expr->children->next->eval(expr->children->next, scope, context, &rhs)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (rhs.type != value_type_int) {
    EVAL_FUNC_ERROR_VALUE("expect integer for rhs", &rhs);
    LEAVE_EVAL_FUNC
    return false;
  }
  result->number += rhs.number;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_equal)
{
  ENTER_EVAL_FUNC("equal")
  value_t lhs;
  value_init(&lhs);
  if (!expr->children->eval(expr->children, scope, context, &lhs)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  value_t rhs;
  value_init(&rhs);
  if (!expr->children->next->eval(expr->children->next, scope, context, &rhs)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  result->type = value_type_bool;
  result->number = value_comp(&lhs, &rhs) == 0;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_and)
{
  ENTER_EVAL_FUNC("and")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  if (!expr->children->eval(expr->children, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (!value_is_true(result)) {
    result->type = value_type_bool;
    result->number = 0;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (!expr->children->next->eval(expr->children->next, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  result->number = value_is_true(result) ? 1 : 0;
  result->type = value_type_bool;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_or)
{
  ENTER_EVAL_FUNC("or")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  if (!expr->children->eval(expr->children, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value_is_true(result)) {
    result->type = value_type_bool;
    result->number = 1;
    LEAVE_EVAL_FUNC
    return true;
  }
  if (!expr->children->next->eval(expr->children->next, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  result->number = value_is_true(result) ? 1 : 0;
  result->type = value_type_bool;
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_if)
{
  ENTER_EVAL_FUNC("if")
  DDSRT_UNUSED_ARG(scope);
  DDSRT_UNUSED_ARG(context);
  if (!expr->children->eval(expr->children, scope, context, result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (value_is_true(result)) {
    if (!expr->children->next->eval(expr->children->next, scope, context, result)) {
      LEAVE_EVAL_FUNC
      return false;
    }
  }
  else if (expr->children->next->next != NULL) {
    if (!expr->children->next->next->eval(expr->children->next->next, scope, context, result)) {
      LEAVE_EVAL_FUNC
      return false;
    }
  }

  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_for)
{
  ENTER_EVAL_FUNC("for")
  /* fprintf(stderr, "Execute for for '%s'\n", expr->text == NULL ? "(null)" : expr->text); */
  DDSRT_UNUSED_ARG(result);
  value_t node_result;
  value_init(&node_result);
  if (!expr->children->eval(expr->children, scope, context, &node_result)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  if (node_result.type != value_type_node) {
    EVAL_FUNC_ERROR_VALUE("Expect node, got: ", &node_result);
    LEAVE_EVAL_FUNC
    return false;
  }
  ddsts_type_t *child = NULL;
  if (DDSTS_IS_TYPE(node_result.node, DDSTS_MODULE)) {
    child = node_result.node->module.members;
  }
  else if (DDSTS_IS_TYPE(node_result.node, DDSTS_STRUCT)) {
    child = node_result.node->struct_def.members;
  }
  else {
    EVAL_FUNC_ERROR_VALUE("Expect module or struct, got: ", &node_result);
    LEAVE_EVAL_FUNC
    return false;
  }
  scope_t for_scope;
  for_scope.var = expr->text;
  for_scope.prev = scope;
  value_init(&for_scope.value);
  for_scope.value.type = value_type_node;
  for (; child != NULL; child = child->type.next) {
    for_scope.value.node = child;
    value_t loop_result;
    value_init(&loop_result);
    if (!expr->children->next->eval(expr->children->next, &for_scope, context, &loop_result)) {
      LEAVE_EVAL_FUNC
      return false;
    }
  }
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_switch)
{
  ENTER_EVAL_FUNC("switch")
  value_t switch_value;
  value_init(&switch_value);
  if (!expr->children->eval(expr->children, scope, context, &switch_value)) {
    LEAVE_EVAL_FUNC
    return false;
  }
  /* fprintf(stderr, "switch value: "); value_print(&switch_value, stderr); fprintf(stderr, "\n"); */
  expr_t *switch_case = expr->children->next;
  for (; switch_case != NULL; switch_case = switch_case->next) {
    if (switch_case->ch == 'c') {
      value_t case_value;
      value_init(&case_value);
      if (!switch_case->children->eval(switch_case->children, scope, context, &case_value)) {
        LEAVE_EVAL_FUNC
        return false;
      }
      /* fprintf(stderr, "case value: "); value_print(&case_value, stderr); fprintf(stderr, "\n"); */
      if (value_comp(&switch_value, &case_value) == 0) {
        LEAVE_EVAL_FUNC
        return switch_case->children->next->eval(switch_case->children->next, scope, context, result);
      }
    }
    else {
      LEAVE_EVAL_FUNC
      return switch_case->children->eval(switch_case->children, scope, context, result);
    }
  }
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_emit)
{
  ENTER_EVAL_FUNC("emit")
  for (const char *s = expr->text; *s != '\0'; s++) {
    if (*s == '\\') {
      s++;
      if (*s == '\0') {
        break;
      }
      else if (*s == 'n') {
        ostream_put(context->ostream, '\n');
      }
      else if (*s == 't') {
        ostream_put(context->ostream, '\t');
      }
      else {
        ostream_put(context->ostream, *s);
      }
    }
    else if (*s == '$') {
      s++;
      if (*s == '\0') {
        break;
      }
      else if (*s == '$') {
        ostream_put(context->ostream, '$');
      }
      else {
        expr_t *letter = expr->children;
        for (; letter != NULL; letter = letter->next) {
          if (letter->ch == *s) {
            break;
          }
        }
        if (letter != NULL) {
          value_init(result);
          if (!letter->children->eval(letter->children, scope, context, result)) {
            LEAVE_EVAL_FUNC
            return false;
          }
          if (result->type == value_type_text) {
            ostream_puts(context->ostream, result->text);
            value_init(result);
          }
        }
        else {
          ostream_put(context->ostream, *s);
        }
      }
    }
    else {
      ostream_put(context->ostream, *s);
    }
  }
  LEAVE_EVAL_FUNC
  return true;
}

EVAL_FUNC(eval_statements)
{
  ENTER_EVAL_FUNC("statements")
  for (expr_t *child = expr->children; child != NULL; child = child->next) {
    value_init(result);
    if (!child->eval(child, scope, context, result)) {
      LEAVE_EVAL_FUNC
      return false;
    }
    if (result->type == value_type_text) {
      ostream_puts(context->ostream, result->text);
      value_init(result);
    }
  }
  LEAVE_EVAL_FUNC
  return true;
}

static void check_expr(expr_t *expr, scope_t *scope, bool *errors)
{
  if (expr->eval == eval_proc_call) {
    if (expr->proc_def == NULL) {
      fprintf(stdout, "Error (%lu.%lu): proc not defined.\n", expr->pos.line, expr->pos.column);
      *errors = true;
    }
    else if (expr->proc_def->proc_body == NULL) {
      fprintf(stdout, "Error (%lu.%lu): proc '%s' not defined.", expr->pos.line, expr->pos.column, expr->proc_def->name);
      if (expr->proc_def->method != NULL) {
        fprintf(stdout, " Did you mean method?");
      }
      fprintf(stdout, "\n");
      *errors = true;
    }
    for (expr_t *child = expr->children; child != NULL; child = child->next) {
      check_expr(child, scope, errors);
    }
  }
  else if (expr->eval == eval_for) {
    check_expr(expr->children, scope, errors);
    scope_t for_var;
    for_var.var = expr->text;
    for_var.prev = scope;
    check_expr(expr->children->next, &for_var, errors);
  }
  else if (expr->eval == eval_var) {
    bool found = false;
    for (scope_t *s = scope; s != NULL; s = s->prev) {
      if (strcmp(s->var, expr->text) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      fprintf(stdout, "Error (%lu.%lu): var '%s' not defined.\n", expr->pos.line, expr->pos.column, expr->text);
      *errors = true;
    }
  }
  else if (expr->eval == eval_switch) {
    for (expr_t *switch_case = expr->children; switch_case != NULL; switch_case = switch_case->next) {
      if (switch_case->ch == 'c') {
        check_expr(switch_case->children, scope, errors);
        check_expr(switch_case->children->next, scope, errors);
      }
      else {
        check_expr(switch_case->children, scope, errors);
      }
    }
  }
  else {
    for (expr_t *child = expr->children; child != NULL; child = child->next) {
      check_expr(child, scope, errors);
    }
  }
}

static bool check_script(parser_t *parser)
{
  bool errors = false;
  for (proc_definition_t *proc_def = parser->proc_defs; proc_def != NULL; proc_def = proc_def->next) {
    if (proc_def->func != 0) {
      if (proc_def->proc_body != NULL) {
        fprintf(stdout, "Error: proc '%s' also defined as build-in func.\n", proc_def->name);
      }
    }
    else if (proc_def->method != 0 || proc_def->field != 0) {
    }
    else if (proc_def->proc_body == NULL) {
      fprintf(stdout, "Error: proc '%s' not defined.\n", proc_def->name);
    }
    else {
      if (!proc_def->proc_called && strcmp(proc_def->name, "main") != 0) {
        fprintf(stdout, "Warning: proc '%s' never called.\n", proc_def->name);
      }
      scope_t *scope = NULL;
      for (proc_parameter_t *param = proc_def->proc_params; param != NULL; param = param->next) {
        scope_t *scope_param = (scope_t*)ddsrt_malloc(sizeof(scope_t));
        if (scope_param == NULL) {
          return false;
        }
        scope_param->var = param->name;
        scope_param->prev = scope;
        scope = scope_param;
      }
      check_expr(proc_def->proc_body, scope, &errors);

      while (scope != NULL) {
        scope_t *prev = scope->prev;
        ddsrt_free((void*)scope);
        scope = prev;
      }
    }
  }
  return !errors;
}

