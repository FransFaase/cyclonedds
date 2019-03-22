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
#include "os/os.h"

#include "stringify.h"

typedef struct {
  char *c;
  char *e;
} ostream_t;

static void ostream_puts(ostream_t *ostream, char *s)
{
  for (; *s != '\0'; s++) {
    if (ostream->c < ostream->e) {
      *ostream->c++ = *s;
    }
  }
}

static void ostream_puts_ull(ostream_t *ostream, unsigned long long ull)
{
  char buffer[100];
  os_ulltostr(ull, buffer, 99, NULL);
  ostream_puts(ostream, buffer);
}

extern void stringify_type(ddsts_type_t *type, ddsts_type_t *parent, ostream_t *ostream)
{
  if (type == NULL) {
    ostream_puts(ostream, "<NULL>");
    return;
  }
  ostream_puts(ostream, "(");
  bool is_reference = type->type.parent != parent;
  if (type->type.parent == NULL) {
    ostream_puts(ostream, "0!");
  }
  else if (is_reference) {
    ostream_puts(ostream, "*");
  }
  if (type->type.name != NULL) {
    ostream_puts(ostream, "'");
    ostream_puts(ostream, type->type.name);
    ostream_puts(ostream, "' ");
  }
  switch (type->type.flags)
  {
    case DDSTS_SHORT: ostream_puts(ostream, "short"); break;
    case DDSTS_LONG: ostream_puts(ostream, "long"); break;
    case DDSTS_LONGLONG: ostream_puts(ostream, "long long"); break;
    case DDSTS_USHORT: ostream_puts(ostream, "unsigned short"); break;
    case DDSTS_ULONG: ostream_puts(ostream, "unsigned long"); break;
    case DDSTS_ULONGLONG: ostream_puts(ostream, "unsigned long long"); break;
    case DDSTS_CHAR: ostream_puts(ostream, "char"); break;
    case DDSTS_WIDE_CHAR: ostream_puts(ostream, "wchar"); break;
    case DDSTS_OCTET: ostream_puts(ostream, "octet"); break;
    case DDSTS_INT8: ostream_puts(ostream, "int8"); break;
    case DDSTS_UINT8: ostream_puts(ostream, "uint8"); break;
    case DDSTS_BOOLEAN: ostream_puts(ostream, "bool"); break;
    case DDSTS_FLOAT: ostream_puts(ostream, "float"); break;
    case DDSTS_DOUBLE: ostream_puts(ostream, "double"); break;
    case DDSTS_LONGDOUBLE: ostream_puts(ostream, "long double"); break;
    case DDSTS_FIXED_PT_CONST: ostream_puts(ostream, "fixed"); break;
    case DDSTS_ANY: ostream_puts(ostream, "any"); break;
    case DDSTS_SEQUENCE:
      {
        ostream_puts(ostream, "sequence<");
        stringify_type(type->sequence.element_type, type, ostream);
        if (type->sequence.max > 0ULL) {
          ostream_puts(ostream, ",");
          ostream_puts_ull(ostream, type->sequence.max);
        }
        ostream_puts(ostream, ">");
      }
      break;
    case DDSTS_ARRAY:
      {
        ostream_puts(ostream, "array<");
        stringify_type(type->array.element_type, type, ostream);
        ostream_puts(ostream, ",");
        ostream_puts_ull(ostream, type->array.size);
        ostream_puts(ostream, ">");
      }
      break;
    case DDSTS_STRING:
      {
        ostream_puts(ostream, "string");
        if (type->string.max > 0ULL) {
          ostream_puts(ostream, "<");
          ostream_puts_ull(ostream, type->string.max);
          ostream_puts(ostream, ">");
        }
      }
      break;
    case DDSTS_WIDE_STRING:
      {
        ostream_puts(ostream, "wstring");
        if (type->string.max > 0ULL) {
          ostream_puts(ostream, "<");
          ostream_puts_ull(ostream, type->string.max);
          ostream_puts(ostream, ">");
        }
      }
      break;
    case DDSTS_FIXED_PT:
      {
        ostream_puts(ostream, "fixed<");
        ostream_puts_ull(ostream, type->fixed_pt.digits);
        ostream_puts(ostream, ",");
        ostream_puts_ull(ostream, type->fixed_pt.fraction_digits);
        ostream_puts(ostream, ">");
      }
      break;
    case DDSTS_MAP:
      {
        ostream_puts(ostream, "map<");
        stringify_type(type->map.key_type, type, ostream);
        ostream_puts(ostream, ",");
        stringify_type(type->map.value_type, type, ostream);
        if (type->map.max > 0ULL) {
          ostream_puts(ostream, ",");
          ostream_puts_ull(ostream, type->map.max);
        }
        ostream_puts(ostream, ">");
      }
      break;
    case DDSTS_MODULE:
      {
        ostream_puts(ostream, "module");
        for (ddsts_type_t *member = type->module.members; member != NULL; member = member->type.next) {
          stringify_type(member, type, ostream);
        }
      }
      break;
    case DDSTS_STRUCT:
      {
        if (!is_reference) {
          ostream_puts(ostream, "struct");
          for (ddsts_type_t *member = type->struct_def.members; member != NULL; member = member->type.next) {
            stringify_type(member, type, ostream);
          }
        }
      }
      break;
    case DDSTS_DECLARATION:
      {
        stringify_type(type->declaration.decl_type, type, ostream);
      }
      break;
    default:
      {
        ostream_puts(ostream, "?");
        ostream_puts_ull(ostream, type->type.flags);
        ostream_puts(ostream, "?");
      }
      break;
  }
  if (type->type.next != 0) {
    ostream_puts(ostream, "$");
  }
  ostream_puts(ostream, ")");
}

extern void ddsts_stringify(ddsts_type_t *root_type, char *buffer, size_t size)
{
  ostream_t ostream;
  ostream.c = buffer;
  ostream.e = buffer + size - 1;
 
  if (root_type->type.flags != DDSTS_MODULE) {
    ostream_puts(&ostream, "Error: root type is not module");
    return;
  } 
  for (ddsts_type_t *member = root_type->module.members; member != NULL; member = member->type.next) {
    stringify_type(member, root_type, &ostream);
  }
  *ostream.c = '\0';
}

