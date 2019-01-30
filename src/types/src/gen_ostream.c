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

extern bool dds_ts_ostream_open(dds_ts_ostream_t *ostream, const char *name)
{
  return ostream->open(ostream, name);
}

extern void dds_ts_ostream_close(dds_ts_ostream_t *ostream)
{
  ostream->close(ostream);
}

extern void dds_ts_ostream_put(dds_ts_ostream_t *ostream, char ch)
{
  ostream->put(ostream, ch);
}

extern void dds_ts_ostream_puts(dds_ts_ostream_t *ostream, const char *str)
{
  for (; *str != '\0'; str++) {
    ostream->put(ostream, *str);
  }
}


typedef struct {
  dds_ts_ostream_t ostream;
  char *s;
  const char *e;
} ostream_to_buffer_t;


static void buffer_ostream_put(dds_ts_ostream_t *ostream, char ch)
{
  if (((ostream_to_buffer_t*)ostream)->s < ((ostream_to_buffer_t*)ostream)->e) {
    *((ostream_to_buffer_t*)ostream)->s++ = ch;
    *((ostream_to_buffer_t*)ostream)->s = '\0';
  }
}

static bool buffer_ostream_open(dds_ts_ostream_t *ostream, const char* name)
{
  OS_UNUSED_ARG(ostream);
  OS_UNUSED_ARG(name);
  return true;
}

static void buffer_ostream_close(dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(ostream);
}


extern void dds_ts_create_ostream_to_buffer(char *buffer, size_t len, dds_ts_ostream_t **ref_ostream)
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


