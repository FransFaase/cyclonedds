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
#include <stdbool.h>

#include "type_api.h"

typedef struct
{
  char *s;
  const char *e;
} idl_ostream_t;

static void idl_ostream_init(idl_ostream_t *stream, char *buffer, size_t len)
{
  stream->s = buffer;
  stream->e = buffer + len - 1;
}

/* Not used yet:
static void idl_ostream_emit_char(idl_ostream_t *stream, char ch)
{
  if (stream->s < stream->e) {
    *stream->s++ = ch;
    *stream->s = '\0';
  }
}
*/

static void idl_ostream_emit(idl_ostream_t *stream, const char *s)
{
  while(*s != '\0' && stream->s < stream->e) {
    *stream->s++ = *s++;
  }
  *stream->s = '\0';
}

static void idl_stringify_module_content(idl_module_it_t *module_it, idl_ostream_t *stream)
{
  idl_module_it_t *child_module_it = idl_create_module_it(module_it);
  for (; idl_module_it_more(child_module_it); idl_module_it_next(child_module_it)) {
    idl_ostream_emit(stream, "module ");
    idl_ostream_emit(stream, idl_module_it_name(child_module_it));
    idl_ostream_emit(stream, "{");
    idl_stringify_module_content(child_module_it, stream);
    idl_ostream_emit(stream, "}");
  }
  idl_free_module_it(&child_module_it);

  idl_enum_it_t *enum_it = idl_create_enum_it(module_it);
  for (; idl_enum_it_more(enum_it); idl_enum_it_next(enum_it)) {
    idl_ostream_emit(stream, "enum ");
    idl_ostream_emit(stream, idl_enum_it_name(enum_it));
    idl_ostream_emit(stream, "{");

    idl_enum_value_it_t *enum_value_it = idl_create_enum_value_it(enum_it);
    for (; idl_enum_value_it_more(enum_value_it); idl_enum_value_it_next(enum_value_it)) {
      idl_ostream_emit(stream, idl_enum_value_it_name(enum_value_it));
      idl_ostream_emit(stream, ",");
    }
    idl_free_enum_value_it(&enum_value_it);
    idl_ostream_emit(stream, "}");
  }
  idl_free_enum_it(&enum_it);
}

extern void idl_stringify(idl_context_t *context, char *buffer, size_t len)
{
  idl_ostream_t stream;
  idl_ostream_init(&stream, buffer, len);

  idl_module_it_t *module_it = idl_create_module_it_from_context(context);
  idl_stringify_module_content(module_it, &stream);
  idl_free_module_it(&module_it);

}

