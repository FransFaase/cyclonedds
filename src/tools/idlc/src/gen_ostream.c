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
#include <stdio.h>
#include <stdbool.h>

#include "gen_ostream.h"

#include "os/os.h"

extern bool ddsts_ostream_open(ddsts_ostream_t *ostream, const char *name)
{
  return ostream->open(ostream, name);
}

extern void ddsts_ostream_close(ddsts_ostream_t *ostream)
{
  ostream->close(ostream);
}

extern void ddsts_ostream_put(ddsts_ostream_t *ostream, char ch)
{
  ostream->put(ostream, ch);
}

extern void ddsts_ostream_puts(ddsts_ostream_t *ostream, const char *str)
{
  for (; *str != '\0'; str++) {
    ostream->put(ostream, *str);
  }
}

/* output stream to files */

typedef struct {
  ddsts_ostream_t ostream;
  FILE *f;
} ostream_to_files_t;
 
bool files_ostream_open(ddsts_ostream_t *ostream, const char *name) 
{ 
OS_WARNING_MSVC_OFF(4996); 
  return (((ostream_to_files_t*)ostream)->f = fopen(name, "wt")) != 0; 
OS_WARNING_MSVC_ON(4996); 
} 
 
void files_ostream_close(ddsts_ostream_t *ostream) 
{ 
  fclose(((ostream_to_files_t*)ostream)->f); 
} 
 
void files_ostream_put(ddsts_ostream_t *ostream, char ch) 
{ 
  fputc(ch, ((ostream_to_files_t*)ostream)->f); 
} 
 
extern void ddsts_create_ostream_to_files(ddsts_ostream_t **ref_ostream)
{
  ostream_to_files_t *ostream_to_files = (ostream_to_files_t*)os_malloc(sizeof(ostream_to_files_t));
  if (ostream_to_files == NULL) {
    *ref_ostream = NULL;
    return;
  }
  ostream_to_files->ostream.open = files_ostream_open;
  ostream_to_files->ostream.close = files_ostream_close;
  ostream_to_files->ostream.put = files_ostream_put;
  ostream_to_files->f = NULL;
  *ref_ostream = &ostream_to_files->ostream;
}

/* output stream to buffer */
 
typedef struct {
  ddsts_ostream_t ostream;
  char *s;
  const char *e;
} ostream_to_buffer_t;

static void buffer_ostream_put(ddsts_ostream_t *ostream, char ch)
{
  if (((ostream_to_buffer_t*)ostream)->s < ((ostream_to_buffer_t*)ostream)->e) {
    *((ostream_to_buffer_t*)ostream)->s++ = ch;
    *((ostream_to_buffer_t*)ostream)->s = '\0';
  }
}

static bool buffer_ostream_open(ddsts_ostream_t *ostream, const char* name)
{
  OS_UNUSED_ARG(ostream);
  OS_UNUSED_ARG(name);
  return true;
}

static void buffer_ostream_close(ddsts_ostream_t *ostream)
{
  OS_UNUSED_ARG(ostream);
}


extern void ddsts_create_ostream_to_buffer(char *buffer, size_t len, ddsts_ostream_t **ref_ostream)
{
  ostream_to_buffer_t *ostream_to_buffer = (ostream_to_buffer_t*)os_malloc(sizeof(ostream_to_buffer_t));
  if (ostream_to_buffer == NULL) {
    *ref_ostream = NULL;
    return;
  }
  ostream_to_buffer->ostream.open = buffer_ostream_open;
  ostream_to_buffer->ostream.close = buffer_ostream_close;
  ostream_to_buffer->ostream.put = buffer_ostream_put;
  ostream_to_buffer->s = buffer;
  ostream_to_buffer->e = buffer + len - 1;
  *ref_ostream = &ostream_to_buffer->ostream;
}


