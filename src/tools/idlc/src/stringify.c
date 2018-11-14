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
#include "type_walker.h"

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

  idl_struct_it_t *struct_it = idl_create_struct_it(module_it);
  for (; idl_struct_it_more(struct_it); idl_struct_it_next(struct_it)) {
    idl_ostream_emit(stream, "struct ");
    idl_ostream_emit(stream, idl_struct_it_name(struct_it));
    idl_ostream_emit(stream, "{");

    idl_struct_member_it_t *struct_member_it = idl_create_struct_member_it(struct_it);
    for (; idl_struct_member_it_more(struct_member_it); idl_struct_member_it_next(struct_member_it)) {
      idl_declarator_it_t *declarator_it = idl_create_declarator_it_for_struct(struct_member_it);
      for (; idl_declarator_it_more(declarator_it); idl_declarator_it_next(declarator_it)) {
	idl_ostream_emit(stream, idl_declarator_it_name(declarator_it));
	idl_ostream_emit(stream, ",");
      }
      idl_free_declarator_it(&declarator_it);
      idl_ostream_emit(stream, ";");
    }
    idl_free_struct_member_it(&struct_member_it);
    idl_ostream_emit(stream, "}");
  }
  idl_free_struct_it(&struct_it);

  idl_union_it_t *union_it = idl_create_union_it(module_it);
  for (; idl_union_it_more(union_it); idl_union_it_next(union_it)) {
    idl_ostream_emit(stream, "union ");
    idl_ostream_emit(stream, idl_union_it_name(union_it));
    idl_ostream_emit(stream, "{");

    idl_union_case_it_t *union_case_it = idl_create_union_case_it(union_it);
    for (; idl_union_case_it_more(union_case_it); idl_union_case_it_next(union_case_it)) {
      idl_union_case_label_it_t *label_it = idl_create_union_case_label_it(union_case_it);
      for (; idl_union_case_label_it_more(label_it); idl_union_case_label_it_next(label_it)) {
	idl_ostream_emit(stream, idl_union_case_label_it_is_default(label_it) ? "d:" : "c:");
      }
      idl_free_union_case_label_it(&label_it);
      idl_declarator_it_t *declarator_it = idl_create_declarator_it_for_union(union_case_it);
      for (; idl_declarator_it_more(declarator_it); idl_declarator_it_next(declarator_it)) {
	idl_ostream_emit(stream, idl_declarator_it_name(declarator_it));
	idl_ostream_emit(stream, ",");
      }
      idl_free_declarator_it(&declarator_it);
      idl_ostream_emit(stream, ";");
    }
    idl_free_union_case_it(&union_case_it);
    idl_ostream_emit(stream, "}");
  }
  idl_free_union_it(&union_it);

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

extern void idl_stringify2(idl_context_t *context, char *buffer, size_t len)
{
  idl_walker_t *walker = idl_create_walker(context);

  idl_walker_def_proc(walker, "module");
    idl_walker_for_all_modules(walker);
      idl_walker_emit(walker, "module ");
      idl_walker_emit_name(walker);
      idl_walker_emit(walker, "{");
      idl_walker_call_proc(walker, "module");
      idl_walker_emit(walker, "}");
    idl_walker_end_for(walker);

    idl_walker_for_all_structs(walker);
      idl_walker_emit(walker, "struct ");
      idl_walker_emit_name(walker);
      idl_walker_emit(walker, "{");
      idl_walker_for_all_members(walker);
        idl_walker_for_all_declarators(walker);
	  idl_walker_emit_name(walker);
	  idl_walker_emit(walker, ",");
        idl_walker_end_for(walker);
        idl_walker_emit(walker, ";");
      idl_walker_end_for(walker);
      idl_walker_emit(walker, "}");
    idl_walker_end_for(walker);

    idl_walker_for_all_unions(walker);
      idl_walker_emit(walker, "union ");
      idl_walker_emit_name(walker);
      idl_walker_emit(walker, "{");
      idl_walker_for_all_cases(walker);
        idl_walker_for_all_case_labels(walker);
	  idl_walker_if_default_case_label(walker);
	    idl_walker_emit(walker, "d:");
	  idl_walker_else(walker);
	    idl_walker_emit(walker, "c:");
	  idl_walker_end_if(walker);
	idl_walker_end_for(walker);
        idl_walker_for_all_declarators(walker);
	  idl_walker_emit_name(walker);
	  idl_walker_emit(walker, ",");
        idl_walker_end_for(walker);
        idl_walker_emit(walker, ";");
      idl_walker_end_for(walker);
      idl_walker_emit(walker, "}");
    idl_walker_end_for(walker);

    idl_walker_for_all_enums(walker);
      idl_walker_emit(walker, "enum ");
      idl_walker_emit_name(walker);
      idl_walker_emit(walker, "{");
      idl_walker_for_all_enum_value(walker);
        idl_walker_emit_name(walker);
	idl_walker_emit(walker, ",");
      idl_walker_end_for(walker);
      idl_walker_emit(walker, "}");
    idl_walker_end_for(walker);
  idl_walker_end_def(walker);

  idl_walker_main(walker);
    idl_walker_call_proc(walker, "module");
  idl_walker_end(walker);

  idl_walker_execute(walker, buffer, len);
}

