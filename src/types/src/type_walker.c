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
#include <string.h>

#include "dds_tt.h"
#include "tt_create.h"
#include "type_walker.h"
#include "os/os.h"

typedef struct dds_ts_walker_expr dds_ts_walker_expr_t;
typedef struct dds_ts_walker_proc_def dds_ts_walker_proc_def_t;

struct dds_ts_walker_proc_def {
  const char* name;
  dds_ts_walker_expr_t *body;
  dds_ts_walker_proc_def_t *next;
};

typedef enum {
  dds_ts_walker_expr_for_all_modules,
  dds_ts_walker_expr_for_all_structs,
  dds_ts_walker_expr_for_all_members,
  dds_ts_walker_expr_for_all_unions,
  dds_ts_walker_expr_for_all_cases,
  dds_ts_walker_expr_for_all_case_labels,
  dds_ts_walker_expr_if_default_case_label,
  dds_ts_walker_expr_for_all_declarators,
  dds_ts_walker_expr_for_all_enums,
  dds_ts_walker_expr_for_all_enum_values,
  dds_ts_walker_expr_else,
  dds_ts_walker_expr_end_if,
  dds_ts_walker_expr_end_for,
  dds_ts_walker_expr_emit_type,
  dds_ts_walker_expr_emit_name,
  dds_ts_walker_expr_emit,
  dds_ts_walker_expr_end_def,
  dds_ts_walker_expr_call_proc,
} dds_ts_walker_expr_type_t;

struct dds_ts_walker_expr {
  dds_ts_walker_expr_t *parent;
  dds_ts_walker_expr_type_t type;
  dds_ts_walker_expr_t *sub1;
  dds_ts_walker_expr_t *sub2;
  const char *text;
  dds_ts_walker_expr_t *next;
};

struct dds_ts_walker {
  dds_ts_node_t *root_node;
  dds_ts_walker_proc_def_t *proc_defs;
  dds_ts_walker_expr_t *main;
  dds_ts_walker_expr_t *cur_parent_expr;
  dds_ts_walker_expr_t **ref_next_expr;
};

dds_ts_walker_t *dds_ts_create_walker(dds_ts_node_t *root_node)
{
  dds_ts_walker_t *walker = (dds_ts_walker_t*)os_malloc(sizeof(dds_ts_walker_t));
  walker->root_node = root_node;
  walker->proc_defs = NULL;
  walker->main = NULL;
  walker->cur_parent_expr = NULL;
  walker->ref_next_expr = NULL;
  return walker;
}

void dds_ts_walker_def_proc(dds_ts_walker_t *walker, const char *name)
{
  dds_ts_walker_proc_def_t *proc_def = (dds_ts_walker_proc_def_t*)os_malloc(sizeof(dds_ts_walker_proc_def_t));
  proc_def->name = name;
  proc_def->body = NULL;
  proc_def->next = walker->proc_defs;
  walker->proc_defs = proc_def;
  walker->ref_next_expr = &proc_def->body;
}

static dds_ts_walker_expr_t *dds_ts_create_expr(dds_ts_walker_expr_type_t type, dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = (dds_ts_walker_expr_t*)os_malloc(sizeof(dds_ts_walker_expr_t));
  expr->parent = walker->cur_parent_expr;
  expr->type = type;
  expr->sub1 = NULL;
  expr->sub2 = NULL;
  expr->text = NULL;
  expr->next = NULL;
  *walker->ref_next_expr = expr;
  walker->ref_next_expr = &expr->next;
  return expr;
}

void dds_ts_walker_for_all_modules(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_modules, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_for_all_structs(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_structs, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
  (void)walker;
}

void dds_ts_walker_for_all_members(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_members, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_for_all_unions(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_unions, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_for_all_cases(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_cases, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_for_all_case_labels(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_case_labels, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_if_default_case_label(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_if_default_case_label, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_for_all_declarators(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_declarators, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_for_all_enums(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_enums, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_for_all_enum_value(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_for_all_enum_values, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

void dds_ts_walker_else(dds_ts_walker_t *walker)
{
  /*if (walker->cur_parent_expr->type != dds_ts_walker_expr_if_default_case_label) {
    fprintf(stderr, "ERROR: else does not match if\n");
  }*/
  walker->ref_next_expr = &walker->cur_parent_expr->sub2;
}

void dds_ts_walker_end_if(dds_ts_walker_t *walker)
{
  /*if (walker->cur_parent_expr->type != dds_ts_walker_expr_if_default_case_label) {
    fprintf(stderr, "ERROR: end if does not match if\n");
  }*/
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
}

void dds_ts_walker_end_for(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
}

void dds_ts_walker_emit_type(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_emit_type, walker);
  (void)expr;
}

void dds_ts_walker_emit_name(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_emit_name, walker);
  (void)expr;
}

void dds_ts_walker_emit(dds_ts_walker_t *walker, const char *text)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_emit, walker);
  expr->text = text;
}

void dds_ts_walker_end_def(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = NULL;
}

void dds_ts_walker_call_proc(dds_ts_walker_t *walker, const char *name)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_call_proc, walker);
  expr->text = name;
}

void dds_ts_walker_main(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = &walker->main;
}

void dds_ts_walker_end(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = NULL;
}

typedef struct
{
  char *s;
  const char *e;
} dds_ts_ostream_t;

static void dds_ts_ostream_init(dds_ts_ostream_t *stream, char *buffer, size_t len)
{
  stream->s = buffer;
  stream->e = buffer + len - 1;
}

static void dds_ts_ostream_emit(dds_ts_ostream_t *stream, const char *s)
{
  while(*s != '\0' && stream->s < stream->e) {
    *stream->s++ = *s++;
  }
  *stream->s = '\0';
}

static void dds_ts_ostream_emit_ull(dds_ts_ostream_t *stream, unsigned long long ull)
{
  char buffer[100];
  sprintf(buffer, "%llu", ull);
  dds_ts_ostream_emit(stream, buffer);
}

typedef struct {
  dds_ts_node_t *cur_node;
} dds_ts_exec_state_t;

void emit_type_spec(dds_ts_type_spec_t *type_spec, dds_ts_ostream_t *stream)
{
  switch (type_spec->flags) {
    case DDS_TS_SHORT_TYPE: dds_ts_ostream_emit(stream, "short"); break;
    case DDS_TS_LONG_TYPE: dds_ts_ostream_emit(stream, "long"); break;
    case DDS_TS_LONG_LONG_TYPE: dds_ts_ostream_emit(stream, "long long"); break;
    case DDS_TS_UNSIGNED_SHORT_TYPE: dds_ts_ostream_emit(stream, "unsigned short"); break;
    case DDS_TS_UNSIGNED_LONG_TYPE: dds_ts_ostream_emit(stream, "unsigned long"); break;
    case DDS_TS_UNSIGNED_LONG_LONG_TYPE: dds_ts_ostream_emit(stream, "unsigned long long"); break;
    case DDS_TS_CHAR_TYPE: dds_ts_ostream_emit(stream, "char"); break;
    case DDS_TS_WIDE_CHAR_TYPE: dds_ts_ostream_emit(stream, "wchar"); break;
    case DDS_TS_OCTET_TYPE: dds_ts_ostream_emit(stream, "octet"); break;
    case DDS_TS_INT8_TYPE: dds_ts_ostream_emit(stream, "int8"); break;
    case DDS_TS_UINT8_TYPE: dds_ts_ostream_emit(stream, "uint8"); break;
    case DDS_TS_BOOLEAN_TYPE: dds_ts_ostream_emit(stream, "bool"); break;
    case DDS_TS_FLOAT_TYPE: dds_ts_ostream_emit(stream, "float"); break;
    case DDS_TS_DOUBLE_TYPE: dds_ts_ostream_emit(stream, "double"); break;
    case DDS_TS_LONG_DOUBLE_TYPE: dds_ts_ostream_emit(stream, "long double"); break;
    case DDS_TS_FIXED_PT_CONST_TYPE: dds_ts_ostream_emit(stream, "fixed"); break;
    case DDS_TS_ANY_TYPE: dds_ts_ostream_emit(stream, "any"); break;
    case DDS_TS_SEQUENCE_TYPE:
      {
        dds_ts_ostream_emit(stream, "sequence<");
        dds_ts_sequence_type_t *sequence = (dds_ts_sequence_type_t*)type_spec;
        emit_type_spec(sequence->element_type, stream);
        if (sequence->bounded) {
          dds_ts_ostream_emit(stream, ",");
          dds_ts_ostream_emit_ull(stream, sequence->max);
        }
        dds_ts_ostream_emit(stream, ">");
      }
      break;
    case DDS_TS_STRING_TYPE:
      {
        dds_ts_ostream_emit(stream, "string");
        dds_ts_string_type_t *string = (dds_ts_string_type_t*)type_spec;
        if (string->bounded) {
          dds_ts_ostream_emit(stream, "<");
          dds_ts_ostream_emit_ull(stream, string->max);
          dds_ts_ostream_emit(stream, ">");
        }
      }
      break;
    case DDS_TS_WIDE_STRING_TYPE:
      {
        dds_ts_ostream_emit(stream, "wstring");
        dds_ts_string_type_t *string = (dds_ts_string_type_t*)type_spec;
        if (string->bounded) {
          dds_ts_ostream_emit(stream, "<");
          dds_ts_ostream_emit_ull(stream, string->max);
          dds_ts_ostream_emit(stream, ">");
        }
      }
      break;
    case DDS_TS_FIXED_PT_TYPE:
      {
        dds_ts_ostream_emit(stream, "fixed<");
        dds_ts_fixed_pt_type_t *fixedpt = (dds_ts_fixed_pt_type_t*)type_spec;
        dds_ts_ostream_emit_ull(stream, fixedpt->digits);
        dds_ts_ostream_emit(stream, ",");
        dds_ts_ostream_emit_ull(stream, fixedpt->fraction_digits);
        dds_ts_ostream_emit(stream, ">");
      }
      break;
    case DDS_TS_MAP_TYPE:
      {
        dds_ts_ostream_emit(stream, "map<");
        dds_ts_map_type_t *map = (dds_ts_map_type_t*)type_spec;
        emit_type_spec(map->key_type, stream);
        dds_ts_ostream_emit(stream, ",");
        emit_type_spec(map->value_type, stream);
        if (map->bounded) {
          dds_ts_ostream_emit(stream, ",");
          dds_ts_ostream_emit_ull(stream, map->max);
        }
        dds_ts_ostream_emit(stream, ">");
      }
      break;
    default:
      {
        char buffer[40];
        sprintf(buffer, "?%x?", type_spec->flags);
        dds_ts_ostream_emit(stream, buffer);
      }
      break;
  }
}

void dds_ts_walker_execute_expr(dds_ts_walker_t *walker, dds_ts_walker_expr_t *expr, dds_ts_exec_state_t *state, dds_ts_ostream_t *stream)
{
  for (; expr != NULL; expr = expr->next) {
    switch(expr->type) {
      case dds_ts_walker_expr_for_all_modules:
        if (state->cur_node->flags == DDS_TS_MODULE) {
          for (dds_ts_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TS_MODULE) {
              dds_ts_exec_state_t new_state = *state;
              new_state.cur_node = node;
              dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
            }
          }
        }
        break;
      case dds_ts_walker_expr_for_all_structs:
        if (state->cur_node->flags == DDS_TS_MODULE) {
          for (dds_ts_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TS_STRUCT) {
              dds_ts_exec_state_t new_state = *state;
              new_state.cur_node = node;
              dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	     }
	   }
	 }
	 break;
      case dds_ts_walker_expr_for_all_members:
        if (state->cur_node->flags == DDS_TS_STRUCT) {
          dds_ts_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	     dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_ts_walker_expr_for_all_unions:
	if (state->cur_node->flags == DDS_TS_MODULE) {
          for (dds_ts_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TS_UNION) {
	      dds_ts_exec_state_t new_state = *state;
	      new_state.cur_node = node;
	      dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	    }
	  }
	}
	break;
      case dds_ts_walker_expr_for_all_cases:
	if (state->cur_node->flags == DDS_TS_UNION) {
          dds_ts_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	    dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_ts_walker_expr_for_all_case_labels:
	if (state->cur_node->flags == DDS_TS_UNION_CASE) {
	  dds_ts_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	     dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_ts_walker_expr_if_default_case_label:
	if (state->cur_node->flags == DDS_TS_UNION_CASE_DEFAULT || state->cur_node->flags == DDS_TS_UNION_CASE_LABEL) {
          dds_ts_walker_execute_expr(walker, state->cur_node->flags == DDS_TS_UNION_CASE_DEFAULT ? expr->sub1 : expr->sub2, state, stream);
	}
	break;
      case dds_ts_walker_expr_for_all_declarators:
	if (state->cur_node->flags == DDS_TS_STRUCT_MEMBER || state->cur_node->flags == DDS_TS_UNION_CASE) {
          dds_ts_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	    dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_ts_walker_expr_for_all_enums:
	if (state->cur_node->flags == DDS_TS_MODULE) {
          for (dds_ts_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TS_ENUM) {
	      dds_ts_exec_state_t new_state = *state;
	      new_state.cur_node = node;
	      dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	    }
	  }
	}
	break;
      case dds_ts_walker_expr_for_all_enum_values:
	if (state->cur_node->flags == DDS_TS_ENUM) {
	  dds_ts_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
            dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_ts_walker_expr_else:
	break;
      case dds_ts_walker_expr_end_if:
	break;
      case dds_ts_walker_expr_end_for:
        break;
      case dds_ts_walker_expr_emit_type:
        {
          dds_ts_type_spec_t base_type;
          dds_ts_type_spec_t *type_spec = 0;
          if (state->cur_node->flags == DDS_TS_STRUCT_MEMBER) {
            type_spec = ((dds_ts_struct_member_t*)state->cur_node)->member_type;
          }
          else if (state->cur_node->flags == DDS_TS_UNION) {
            base_type.flags = ((dds_ts_union_t*)state->cur_node)->switch_type;
            type_spec = &base_type;
          }
          else if (state->cur_node->flags == DDS_TS_UNION_CASE) {
            type_spec = ((dds_ts_union_case_t*)state->cur_node)->branch_type;
          }
          if (type_spec == 0) {
            dds_ts_ostream_emit(stream, "??");
          } else {
            emit_type_spec(type_spec, stream);
          }
        }
        break;
      case dds_ts_walker_expr_emit_name:
	if (DDS_TS_IS_DEFINITION(state->cur_node->flags)) {
          dds_ts_ostream_emit(stream, ((dds_ts_definition_t*)state->cur_node)->name);
	}
        else {
          char buffer[40];
          sprintf(buffer, "?%x?", state->cur_node->flags);
          dds_ts_ostream_emit(stream, buffer);
        }
	break;
      case dds_ts_walker_expr_emit:
        dds_ts_ostream_emit(stream, expr->text);
	break;
      case dds_ts_walker_expr_end_def:
        break;
      case dds_ts_walker_expr_call_proc:
	for (dds_ts_walker_proc_def_t *proc_def = walker->proc_defs; proc_def != NULL; proc_def = proc_def->next) {
          if (strcmp(proc_def->name, expr->text) == 0) {
            dds_ts_walker_execute_expr(walker, proc_def->body, state, stream);
	    break;
	  }
	}
        break;
    }
  }
}

void dds_ts_walker_execute(dds_ts_walker_t *walker, char *buffer, size_t len)
{
  dds_ts_ostream_t stream;
  dds_ts_ostream_init(&stream, buffer, len);
  dds_ts_exec_state_t state;
  state.cur_node = walker->root_node;
  dds_ts_walker_execute_expr(walker, walker->main, &state, &stream);
}



