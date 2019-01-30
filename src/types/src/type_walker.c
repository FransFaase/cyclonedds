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
#include <string.h>

#include "typetree.h"
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
  dds_ts_walker_expr_for_all_children,
  dds_ts_walker_expr_for_all_modules,
  dds_ts_walker_expr_for_all_structs,
  dds_ts_walker_expr_for_all_members,
  dds_ts_walker_expr_for_all_declarators,
  dds_ts_walker_expr_for_call_parent,
  dds_ts_walker_expr_for_struct_member_type,
  dds_ts_walker_expr_for_sequence_element_type,
  dds_ts_walker_expr_if_is_type,
  dds_ts_walker_expr_if_func,
  dds_ts_walker_expr_emit,
  dds_ts_walker_expr_emit_type,
  dds_ts_walker_expr_emit_name,
  dds_ts_walker_expr_end_def,
  dds_ts_walker_expr_call_proc,
  dds_ts_walker_expr_call_func,
} dds_ts_walker_expr_type_t;

struct dds_ts_walker_expr {
  dds_ts_walker_expr_t *parent;
  dds_ts_walker_expr_type_t type;
  dds_ts_walker_expr_t *sub1;
  dds_ts_walker_expr_t *sub2;
  const char *text;
  dds_ts_node_flags_t flags;
  char ch;
  bool (*cond_func)(dds_ts_node_t *node);
  dds_ts_walker_call_func_t call_func;
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

extern void dds_ts_walker_def_proc(dds_ts_walker_t *walker, const char *name)
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
  expr->flags = 0;
  expr->ch = '\0';
  expr->cond_func = 0;
  expr->call_func = 0;
  expr->next = NULL;
  *walker->ref_next_expr = expr;
  walker->ref_next_expr = &expr->next;
  return expr;
}

static void open_expr(dds_ts_walker_t *walker, dds_ts_walker_expr_t *expr)
{
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

static void close_expr(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
}

static void create_open_expr(dds_ts_walker_t *walker, dds_ts_walker_expr_type_t type)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(type, walker);
  open_expr(walker, expr);
}

extern void dds_ts_walker_for_all_children(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_all_children);
}

extern void dds_ts_walker_for_all_modules(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_all_modules);
}

extern void dds_ts_walker_for_all_structs(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_all_structs);
}

extern void dds_ts_walker_for_all_members(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_all_members);
}

extern void dds_ts_walker_for_all_declarators(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_all_declarators);
}

extern void dds_ts_walker_for_call_parent(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_call_parent);
}

extern void dds_ts_walker_for_struct_member_type(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_struct_member_type);
}

extern void dds_ts_walker_for_sequence_element_type(dds_ts_walker_t *walker)
{
  create_open_expr(walker, dds_ts_walker_expr_for_sequence_element_type);
}

extern void dds_ts_walker_end_for(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
}

extern void dds_ts_walker_if_is_type(dds_ts_walker_t *walker, dds_ts_node_flags_t flags)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_if_is_type, walker);
  expr->flags = flags;
  open_expr(walker, expr);
}

extern void dds_ts_walker_if_func(dds_ts_walker_t *walker, bool (*func)(dds_ts_node_t *node))
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_if_func, walker);
  expr->cond_func = func;
  open_expr(walker, expr);
}

extern void dds_ts_walker_else(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = &walker->cur_parent_expr->sub2;
}

extern void dds_ts_walker_end_if(dds_ts_walker_t *walker)
{
  close_expr(walker);
}

extern void dds_ts_walker_emit(dds_ts_walker_t *walker, const char *text)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_emit, walker);
  expr->text = text;
}

extern void dds_ts_walker_emit_type(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_emit_type, walker);
  OS_UNUSED_ARG(expr);
}

extern void dds_ts_walker_emit_name(dds_ts_walker_t *walker)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_emit_name, walker);
  OS_UNUSED_ARG(expr);
}

extern void dds_ts_walker_end_def(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = NULL;
}

extern void dds_ts_walker_call_proc(dds_ts_walker_t *walker, const char *name)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_call_proc, walker);
  expr->text = name;
}

extern void dds_ts_walker_call_func(dds_ts_walker_t *walker, dds_ts_walker_call_func_t func)
{
  dds_ts_walker_expr_t *expr = dds_ts_create_expr(dds_ts_walker_expr_call_func, walker);
  expr->call_func = func;
}

extern void dds_ts_walker_main(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = &walker->main;
}

extern void dds_ts_walker_end(dds_ts_walker_t *walker)
{
  walker->ref_next_expr = NULL;
}

static void dds_ts_ostream_emit(dds_ts_ostream_t *ostream, const char *s)
{
  dds_ts_ostream_puts(ostream, s);
}

static void dds_ts_ostream_emit_ull(dds_ts_ostream_t *ostream, unsigned long long ull)
{
  char buffer[100];
  os_ulltostr(ull, buffer, 99, NULL);
  dds_ts_ostream_emit(ostream, buffer);
}

extern void emit_type_spec(dds_ts_type_spec_t *type_spec, dds_ts_ostream_t *ostream)
{
  switch (type_spec->node.flags)
  {
    case DDS_TS_SHORT_TYPE: dds_ts_ostream_emit(ostream, "short"); break;
    case DDS_TS_LONG_TYPE: dds_ts_ostream_emit(ostream, "long"); break;
    case DDS_TS_LONG_LONG_TYPE: dds_ts_ostream_emit(ostream, "long long"); break;
    case DDS_TS_UNSIGNED_SHORT_TYPE: dds_ts_ostream_emit(ostream, "unsigned short"); break;
    case DDS_TS_UNSIGNED_LONG_TYPE: dds_ts_ostream_emit(ostream, "unsigned long"); break;
    case DDS_TS_UNSIGNED_LONG_LONG_TYPE: dds_ts_ostream_emit(ostream, "unsigned long long"); break;
    case DDS_TS_CHAR_TYPE: dds_ts_ostream_emit(ostream, "char"); break;
    case DDS_TS_WIDE_CHAR_TYPE: dds_ts_ostream_emit(ostream, "wchar"); break;
    case DDS_TS_OCTET_TYPE: dds_ts_ostream_emit(ostream, "octet"); break;
    case DDS_TS_INT8_TYPE: dds_ts_ostream_emit(ostream, "int8"); break;
    case DDS_TS_UINT8_TYPE: dds_ts_ostream_emit(ostream, "uint8"); break;
    case DDS_TS_BOOLEAN_TYPE: dds_ts_ostream_emit(ostream, "bool"); break;
    case DDS_TS_FLOAT_TYPE: dds_ts_ostream_emit(ostream, "float"); break;
    case DDS_TS_DOUBLE_TYPE: dds_ts_ostream_emit(ostream, "double"); break;
    case DDS_TS_LONG_DOUBLE_TYPE: dds_ts_ostream_emit(ostream, "long double"); break;
    case DDS_TS_FIXED_PT_CONST_TYPE: dds_ts_ostream_emit(ostream, "fixed"); break;
    case DDS_TS_ANY_TYPE: dds_ts_ostream_emit(ostream, "any"); break;
    case DDS_TS_SEQUENCE:
      {
        dds_ts_ostream_emit(ostream, "sequence<");
        dds_ts_sequence_t *sequence = (dds_ts_sequence_t*)type_spec;
        emit_type_spec(sequence->element_type.type_spec, ostream);
        if (sequence->bounded) {
          dds_ts_ostream_emit(ostream, ",");
          dds_ts_ostream_emit_ull(ostream, sequence->max);
        }
        dds_ts_ostream_emit(ostream, ">");
      }
      break;
    case DDS_TS_STRING:
      {
        dds_ts_ostream_emit(ostream, "string");
        dds_ts_string_t *string = (dds_ts_string_t*)type_spec;
        if (string->bounded) {
          dds_ts_ostream_emit(ostream, "<");
          dds_ts_ostream_emit_ull(ostream, string->max);
          dds_ts_ostream_emit(ostream, ">");
        }
      }
      break;
    case DDS_TS_WIDE_STRING:
      {
        dds_ts_ostream_emit(ostream, "wstring");
        dds_ts_string_t *string = (dds_ts_string_t*)type_spec;
        if (string->bounded) {
          dds_ts_ostream_emit(ostream, "<");
          dds_ts_ostream_emit_ull(ostream, string->max);
          dds_ts_ostream_emit(ostream, ">");
        }
      }
      break;
    case DDS_TS_FIXED_PT:
      {
        dds_ts_ostream_emit(ostream, "fixed<");
        dds_ts_fixed_pt_t *fixedpt = (dds_ts_fixed_pt_t*)type_spec;
        dds_ts_ostream_emit_ull(ostream, fixedpt->digits);
        dds_ts_ostream_emit(ostream, ",");
        dds_ts_ostream_emit_ull(ostream, fixedpt->fraction_digits);
        dds_ts_ostream_emit(ostream, ">");
      }
      break;
    case DDS_TS_MAP:
      {
        dds_ts_ostream_emit(ostream, "map<");
        dds_ts_map_t *map = (dds_ts_map_t*)type_spec;
        emit_type_spec(map->key_type.type_spec, ostream);
        dds_ts_ostream_emit(ostream, ",");
        emit_type_spec(map->value_type.type_spec, ostream);
        if (map->bounded) {
          dds_ts_ostream_emit(ostream, ",");
          dds_ts_ostream_emit_ull(ostream, map->max);
        }
        dds_ts_ostream_emit(ostream, ">");
      }
      break;
    case DDS_TS_STRUCT:
      {
        dds_ts_ostream_emit(ostream, ((dds_ts_struct_t*)type_spec)->def.name);
      }
      break;
    default:
      {
        dds_ts_ostream_emit(ostream, "?");
        dds_ts_ostream_emit_ull(ostream, type_spec->node.flags);
        dds_ts_ostream_emit(ostream, "?");
      }
      break;
  }
}


static void dds_ts_walker_execute_expr(dds_ts_walker_t *walker, dds_ts_walker_expr_t *expr, dds_ts_walker_exec_state_t *state, void *context, dds_ts_ostream_t *ostream)
{
  for (; expr != NULL; expr = expr->next) {
    switch(expr->type) {
      case dds_ts_walker_expr_for_all_children:
        for (dds_ts_node_t *node = state->node->children; node != NULL; node = node->next) {
          dds_ts_walker_exec_state_t new_state;
          new_state.node = node;
          new_state.call_parent = state;
          dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
        }
        break;
      case dds_ts_walker_expr_for_all_modules:
        if (state->node->flags == DDS_TS_MODULE) {
          for (dds_ts_node_t *node = state->node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TS_MODULE) {
              dds_ts_walker_exec_state_t new_state;
              new_state.node = node;
              new_state.call_parent = state;
              dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
            }
          }
        }
        break;
      case dds_ts_walker_expr_for_all_structs:
        if (state->node->flags == DDS_TS_MODULE) {
          for (dds_ts_node_t *node = state->node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TS_STRUCT) {
              dds_ts_walker_exec_state_t new_state;
              new_state.node = node;
              new_state.call_parent = state;
              dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
	    }
	  }
	}
	break;
      case dds_ts_walker_expr_for_all_members:
        if (state->node->flags == DDS_TS_STRUCT) {
          for (dds_ts_node_t *node = state->node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TS_STRUCT_MEMBER) {
              dds_ts_walker_exec_state_t new_state;
              new_state.node = node;
              new_state.call_parent = state;
	      dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
            }
	  }
	}
	break;
      case dds_ts_walker_expr_for_all_declarators:
        if (state->node->flags == DDS_TS_STRUCT_MEMBER) {
          for (dds_ts_node_t *node = state->node->children; node != NULL; node = node->next) {
            dds_ts_walker_exec_state_t new_state;
            new_state.node = node;
            new_state.call_parent = state;
            dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
          }
        }
        break;
      case dds_ts_walker_expr_for_call_parent:
        if (state->call_parent != NULL) {
          dds_ts_walker_execute_expr(walker, expr->sub1, state->call_parent, context, ostream);
        }
        break;
      case dds_ts_walker_expr_for_struct_member_type:
        if (state->node->flags == DDS_TS_STRUCT_MEMBER) {
          dds_ts_walker_exec_state_t new_state;
          new_state.node = &((dds_ts_struct_member_t*)state->node)->member_type.type_spec->node;
          new_state.call_parent = state;
          dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
        }
        break;
      case dds_ts_walker_expr_for_sequence_element_type:
        if (state->node->flags == DDS_TS_SEQUENCE) {
          dds_ts_walker_exec_state_t new_state;
          new_state.node = &((dds_ts_sequence_t*)state->node)->element_type.type_spec->node;
          new_state.call_parent = state;
          dds_ts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
        }
        break;
      case dds_ts_walker_expr_if_is_type:
        if (state->node->flags == expr->flags) {
          dds_ts_walker_execute_expr(walker, expr->sub1, state, context, ostream);
        }
        break;
      case dds_ts_walker_expr_if_func:
        if (expr->cond_func(state->node)) {
          dds_ts_walker_execute_expr(walker, expr->sub1, state, context, ostream);
        }
        break;
      case dds_ts_walker_expr_emit_type:
        {
          dds_ts_type_spec_t *type_spec = 0;
          if (state->node->flags == DDS_TS_STRUCT_MEMBER) {
            type_spec = ((dds_ts_struct_member_t*)state->node)->member_type.type_spec;
          }
          if (type_spec == 0) {
            dds_ts_ostream_emit(ostream, "??");
          }
          else {
            emit_type_spec(type_spec, ostream);
          }
        }
        break;
      case dds_ts_walker_expr_emit_name:
	if (DDS_TS_IS_DEFINITION(state->node->flags)) {
          dds_ts_ostream_emit(ostream, ((dds_ts_definition_t*)state->node)->name);
	}
        else {
          char buffer[40];
          os_ulltostr(state->node->flags, buffer, 39, NULL);
          dds_ts_ostream_emit(ostream, "?");
          dds_ts_ostream_emit(ostream, buffer);
          dds_ts_ostream_emit(ostream, "?");
        }
	break;
      case dds_ts_walker_expr_emit:
        dds_ts_ostream_emit(ostream, expr->text);
	break;
      case dds_ts_walker_expr_end_def:
        break;
      case dds_ts_walker_expr_call_proc:
	for (dds_ts_walker_proc_def_t *proc_def = walker->proc_defs; proc_def != NULL; proc_def = proc_def->next) {
          if (strcmp(proc_def->name, expr->text) == 0) {
            dds_ts_walker_execute_expr(walker, proc_def->body, state, context, ostream);
	    break;
	  }
	}
        break;
      case dds_ts_walker_expr_call_func:
        expr->call_func(state, context, ostream);
    }
  }
}

extern void dds_ts_walker_execute(dds_ts_walker_t *walker, void *context, dds_ts_ostream_t *ostream)
{
  dds_ts_walker_exec_state_t state;
  state.node = walker->root_node;
  dds_ts_walker_execute_expr(walker, walker->main, &state, context, ostream);
}

static void dds_ts_walker_expr_free(dds_ts_walker_expr_t *expr)
{
  while (expr != NULL) {
    dds_ts_walker_expr_t *next = expr->next;
    dds_ts_walker_expr_free(expr->sub1);
    dds_ts_walker_expr_free(expr->sub2);
    os_free((void*)expr);
    expr = next;
  }
}

extern void dds_ts_walker_free(dds_ts_walker_t *walker)
{
  dds_ts_walker_proc_def_t *proc_def;
  for (proc_def = walker->proc_defs; proc_def != NULL;) {
    dds_ts_walker_proc_def_t *next = proc_def->next;
    dds_ts_walker_expr_free(proc_def->body);
    os_free((void*)proc_def);
    proc_def = next;
  }
  dds_ts_walker_expr_free(walker->main);
  os_free((void*)walker);
}


