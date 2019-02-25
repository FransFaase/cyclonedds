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
#include "gen_c99.h"

#define YYSTYPE DDS_TS_PARSER_STYPE
#define YYLTYPE DDS_TS_PARSER_LTYPE

#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;

#define YY_NO_UNISTD_H 1 /* to surpress #include <unistd.h> */

#include "parser.h"
#include "idl.parser.h"
#include "yy_decl.h"
#include "idl.lexer.h"
#include "stringify.h"
#include "gen_ostream.h"

/* file output stream */ 

typedef struct {
  dds_ts_ostream_t ostream;
  FILE *f;
} ostream_to_files_t;
 
bool file_ostream_open(dds_ts_ostream_t *ostream, const char *name) 
{ 
OS_WARNING_MSVC_OFF(4996); 
  return (((ostream_to_files_t*)ostream)->f = fopen(name, "wt")) != 0; 
OS_WARNING_MSVC_ON(4996); 
} 
 
void file_ostream_close(dds_ts_ostream_t *ostream) 
{ 
  fclose(((ostream_to_files_t*)ostream)->f); 
} 
 
void file_ostream_put(dds_ts_ostream_t *ostream, char ch) 
{ 
  fputc(ch, ((ostream_to_files_t*)ostream)->f); 
} 
 
void init_ostream_to_files(ostream_to_files_t *ostream) 
{ 
  ostream->ostream.open = file_ostream_open; 
  ostream->ostream.close = file_ostream_close; 
  ostream->ostream.put = file_ostream_put; 
} 


extern int
dds_ts_parse_file(const char *file, void (*error_func)(int line, int column, const char *msg))
{
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
    dds_ts_context_t *context = dds_ts_create_context();
    if (context == NULL) {
      if (error_func != 0) {
        error_func(0, 0, "Error: out of memory\n");
      }
      return 2;
    }
    dds_ts_context_set_error_func(context, error_func);
    yyscan_t scanner;
    dds_ts_parser_lex_init(&scanner);
    dds_ts_parser_set_in(fh, scanner);
    err = dds_ts_parser_parse(scanner, context);
    if (err == 0) {
      ostream_to_files_t ostream_to_files;
      init_ostream_to_files(&ostream_to_files);
      dds_ts_generate_C99(file, dds_ts_context_get_root_node(context), (dds_ts_ostream_t*)&ostream_to_files);
    }
    else if (dds_ts_context_get_out_of_memory_error(context)) {
      if (error_func != 0) {
        error_func(0, 0, "Error: out of memory\n");
      }
    }
    dds_ts_free_context(context);
    dds_ts_parser_lex_destroy(scanner);
    (void)fclose(fh);
  }

  return err;
}

extern int
dds_ts_parse_string(const char *str, void (*error_func)(int line, int column, const char *text))
{
  int err = 0;

  assert(str != NULL);
  if (str == NULL) {
    if (error_func != NULL) {
      error_func(0, 0, "String argument is NULL");
    }
    err = -1;
  }
  else {
    dds_ts_context_t *context = dds_ts_create_context();
    if (context == NULL) {
      if (error_func != NULL) {
        error_func(0, 0, "Out of memory");
      }
      return 2;
    }
    dds_ts_context_set_error_func(context, error_func);
    yyscan_t scanner;
    dds_ts_parser_lex_init(&scanner);
    dds_ts_parser__scan_string(str, scanner);
    err = dds_ts_parser_parse(scanner, context);
    if (err != 0) {
      if (dds_ts_context_get_out_of_memory_error(context)) {
        if (error_func != NULL) {
          error_func(0, 0, "Out of memory");
        }
      }
    }
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
  assert(buffer != NULL);

  if (str == NULL || buffer == NULL) {
    err = -1;
  }
  else {
    dds_ts_context_t *context = dds_ts_create_context();
    if (context == NULL) {
      os_strlcpy(buffer, "OUT_OF_MEMORY", len);
      return 2;
    }
    yyscan_t scanner;
    dds_ts_parser_lex_init(&scanner);
    dds_ts_parser__scan_string(str, scanner);
    err = dds_ts_parser_parse(scanner, context);
    if (err != 0) {
      if (dds_ts_context_get_out_of_memory_error(context)) {
        os_strlcpy(buffer, "OUT_OF_MEMORY", len);
      }
      else {
        os_strlcpy(buffer, "PARSING ERROR", len);
      }
      buffer[len-1] = '\0';
    }
    else {
      dds_ts_ostream_t *ostream = NULL;
      dds_ts_create_ostream_to_buffer(buffer, len, &ostream);
      if (ostream == NULL) {
        os_strlcpy(buffer, "OUT_OF_MEMORY", len);
        return 2;
      }
      dds_ts_stringify(dds_ts_context_get_root_node(context), ostream);
      os_free((void*)ostream);
    }

    dds_ts_free_context(context);
    dds_ts_parser_lex_destroy(scanner);
  }

  return err;
}

int dds_ts_parse_string_gen_C99(const char *str, char *buffer, size_t len)
{
  int err = 0;

  assert(str != NULL);
  if (str == NULL) {
    err = -1;
  }
  else {
    dds_ts_context_t *context = dds_ts_create_context();
    if (context == NULL) {
      os_strlcpy(buffer, "OUT_OF_MEMORY", len);
      buffer[len-1] = '\0';
      return 2;
    }
    yyscan_t scanner;
    dds_ts_parser_lex_init(&scanner);
    dds_ts_parser__scan_string(str, scanner);
    err = dds_ts_parser_parse(scanner, context);
    if (err != 0) {
      if (dds_ts_context_get_out_of_memory_error(context)) {
        os_strlcpy(buffer, "OUT_OF_MEMORY", len);
      }
      else {
        os_strlcpy(buffer, "PARSING ERROR", len);
      }
      buffer[len-1] = '\0';
    }
    else {
      dds_ts_ostream_t *ostream = NULL;
      dds_ts_create_ostream_to_buffer(buffer, len, &ostream);
      if (ostream == NULL) {
        os_strlcpy(buffer, "OUT_OF_MEMORY", len);
        return 2;
      }
      dds_ts_generate_C99("test.idl", dds_ts_context_get_root_node(context), ostream);
      os_free((void*)ostream);
    }

    dds_ts_free_context(context);
    dds_ts_parser_lex_destroy(scanner);
  }

  return err;
}

