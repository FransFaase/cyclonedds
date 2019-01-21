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
#include "os/os.h"
#include "dds/ddsts/typetree.h"
#include "type_walker.h"

void ddsts_walk(ddsts_walk_exec_state_t *exec_state, ddsts_node_flags_t visit, ddsts_node_flags_t call, ddsts_walker_call_func_t func, ddsts_walk_context_t *context)
{
  ddsts_walk_exec_state_t child_exec_state;
  child_exec_state.call_parent = exec_state;
  for (ddsts_node_t *child = exec_state->node->children; child != NULL; child = child->next) {
    child_exec_state.node = child;
    if ((child->flags & call) != 0) {
      func(&child_exec_state, context);
    }
    if ((child->flags & visit) != 0) {
      ddsts_walk(&child_exec_state, visit, call, func, context);
    }
  }
}

typedef struct ddsts_walker_expr ddsts_walker_expr_t;
typedef struct ddsts_walker_proc_def ddsts_walker_proc_def_t;

struct ddsts_walker_proc_def {
  const char* name;
  ddsts_walker_expr_t *body;
  ddsts_walker_proc_def_t *next;
};

typedef enum {
  ddsts_walker_expr_for_all_children,
  ddsts_walker_expr_for_all_modules,
  ddsts_walker_expr_for_all_structs,
  ddsts_walker_expr_for_all_members,
  ddsts_walker_expr_for_all_declarators,
  ddsts_walker_expr_for_call_parent,
  ddsts_walker_expr_for_struct_member_type,
  ddsts_walker_expr_for_sequence_element_type,
  ddsts_walker_expr_if_is_type,
  ddsts_walker_expr_if_func,
  ddsts_walker_expr_emit,
  ddsts_walker_expr_emit_type,
  ddsts_walker_expr_emit_name,
  ddsts_walker_expr_end_def,
  ddsts_walker_expr_call_proc,
} ddsts_walker_expr_type_t;

struct ddsts_walker_expr {
  ddsts_walker_expr_t *parent;
  ddsts_walker_expr_type_t type;
  ddsts_walker_expr_t *sub1;
  ddsts_walker_expr_t *sub2;
  const char *text;
  ddsts_node_flags_t flags;
  char ch;
  bool (*cond_func)(ddsts_node_t *node);
  ddsts_walker_expr_t *next;
};

struct ddsts_walker {
  ddsts_node_t *root_node;
  ddsts_walker_proc_def_t *proc_defs;
  ddsts_walker_expr_t *main;
  ddsts_walker_expr_t *cur_parent_expr;
  ddsts_walker_expr_t **ref_next_expr;
};

ddsts_walker_t *ddsts_create_walker(ddsts_node_t *root_node)
{
  ddsts_walker_t *walker = (ddsts_walker_t*)os_malloc(sizeof(ddsts_walker_t));
  walker->root_node = root_node;
  walker->proc_defs = NULL;
  walker->main = NULL;
  walker->cur_parent_expr = NULL;
  walker->ref_next_expr = NULL;
  return walker;
}

extern void ddsts_walker_def_proc(ddsts_walker_t *walker, const char *name)
{
  ddsts_walker_proc_def_t *proc_def = (ddsts_walker_proc_def_t*)os_malloc(sizeof(ddsts_walker_proc_def_t));
  proc_def->name = name;
  proc_def->body = NULL;
  proc_def->next = walker->proc_defs;
  walker->proc_defs = proc_def;
  walker->ref_next_expr = &proc_def->body;
}

static ddsts_walker_expr_t *ddsts_create_expr(ddsts_walker_expr_type_t type, ddsts_walker_t *walker)
{
  ddsts_walker_expr_t *expr = (ddsts_walker_expr_t*)os_malloc(sizeof(ddsts_walker_expr_t));
  expr->parent = walker->cur_parent_expr;
  expr->type = type;
  expr->sub1 = NULL;
  expr->sub2 = NULL;
  expr->text = NULL;
  expr->flags = 0;
  expr->ch = '\0';
  expr->cond_func = 0;
  expr->next = NULL;
  *walker->ref_next_expr = expr;
  walker->ref_next_expr = &expr->next;
  return expr;
}

static void open_expr(ddsts_walker_t *walker, ddsts_walker_expr_t *expr)
{
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr;
}

static void close_expr(ddsts_walker_t *walker)
{
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
}

static void create_open_expr(ddsts_walker_t *walker, ddsts_walker_expr_type_t type)
{
  ddsts_walker_expr_t *expr = ddsts_create_expr(type, walker);
  open_expr(walker, expr);
}

extern void ddsts_walker_for_all_children(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_all_children);
}

extern void ddsts_walker_for_all_modules(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_all_modules);
}

extern void ddsts_walker_for_all_structs(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_all_structs);
}

extern void ddsts_walker_for_all_members(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_all_members);
}

extern void ddsts_walker_for_all_declarators(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_all_declarators);
}

extern void ddsts_walker_for_call_parent(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_call_parent);
}

extern void ddsts_walker_for_struct_member_type(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_struct_member_type);
}

extern void ddsts_walker_for_sequence_element_type(ddsts_walker_t *walker)
{
  create_open_expr(walker, ddsts_walker_expr_for_sequence_element_type);
}

extern void ddsts_walker_end_for(ddsts_walker_t *walker)
{
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
}

extern void ddsts_walker_if_is_type(ddsts_walker_t *walker, ddsts_node_flags_t flags)
{
  ddsts_walker_expr_t *expr = ddsts_create_expr(ddsts_walker_expr_if_is_type, walker);
  expr->flags = flags;
  open_expr(walker, expr);
}

extern void ddsts_walker_if_func(ddsts_walker_t *walker, bool (*func)(ddsts_node_t *node))
{
  ddsts_walker_expr_t *expr = ddsts_create_expr(ddsts_walker_expr_if_func, walker);
  expr->cond_func = func;
  open_expr(walker, expr);
}

extern void ddsts_walker_else(ddsts_walker_t *walker)
{
  walker->ref_next_expr = &walker->cur_parent_expr->sub2;
}

extern void ddsts_walker_end_if(ddsts_walker_t *walker)
{
  close_expr(walker);
}

extern void ddsts_walker_emit(ddsts_walker_t *walker, const char *text)
{
  ddsts_walker_expr_t *expr = ddsts_create_expr(ddsts_walker_expr_emit, walker);
  expr->text = text;
}

extern void ddsts_walker_emit_type(ddsts_walker_t *walker)
{
  ddsts_walker_expr_t *expr = ddsts_create_expr(ddsts_walker_expr_emit_type, walker);
  OS_UNUSED_ARG(expr);
}

extern void ddsts_walker_emit_name(ddsts_walker_t *walker)
{
  ddsts_walker_expr_t *expr = ddsts_create_expr(ddsts_walker_expr_emit_name, walker);
  OS_UNUSED_ARG(expr);
}

extern void ddsts_walker_end_def(ddsts_walker_t *walker)
{
  walker->ref_next_expr = NULL;
}

extern void ddsts_walker_call_proc(ddsts_walker_t *walker, const char *name)
{
  ddsts_walker_expr_t *expr = ddsts_create_expr(ddsts_walker_expr_call_proc, walker);
  expr->text = name;
}

extern void ddsts_walker_main(ddsts_walker_t *walker)
{
  walker->ref_next_expr = &walker->main;
}

extern void ddsts_walker_end(ddsts_walker_t *walker)
{
  walker->ref_next_expr = NULL;
}

static void ddsts_ostream_emit(ddsts_ostream_t *ostream, const char *s)
{
  ddsts_ostream_puts(ostream, s);
}

static void ddsts_ostream_emit_ull(ddsts_ostream_t *ostream, unsigned long long ull)
{
  char buffer[100];
  os_ulltostr(ull, buffer, 99, NULL);
  ddsts_ostream_emit(ostream, buffer);
}

extern void emit_type_spec(ddsts_type_spec_t *type_spec, ddsts_ostream_t *ostream)
{
  switch (type_spec->node.flags)
  {
    case DDSTS_SHORT_TYPE: ddsts_ostream_emit(ostream, "short"); break;
    case DDSTS_LONG_TYPE: ddsts_ostream_emit(ostream, "long"); break;
    case DDSTS_LONG_LONG_TYPE: ddsts_ostream_emit(ostream, "long long"); break;
    case DDSTS_UNSIGNED_SHORT_TYPE: ddsts_ostream_emit(ostream, "unsigned short"); break;
    case DDSTS_UNSIGNED_LONG_TYPE: ddsts_ostream_emit(ostream, "unsigned long"); break;
    case DDSTS_UNSIGNED_LONG_LONG_TYPE: ddsts_ostream_emit(ostream, "unsigned long long"); break;
    case DDSTS_CHAR_TYPE: ddsts_ostream_emit(ostream, "char"); break;
    case DDSTS_WIDE_CHAR_TYPE: ddsts_ostream_emit(ostream, "wchar"); break;
    case DDSTS_OCTET_TYPE: ddsts_ostream_emit(ostream, "octet"); break;
    case DDSTS_INT8_TYPE: ddsts_ostream_emit(ostream, "int8"); break;
    case DDSTS_UINT8_TYPE: ddsts_ostream_emit(ostream, "uint8"); break;
    case DDSTS_BOOLEAN_TYPE: ddsts_ostream_emit(ostream, "bool"); break;
    case DDSTS_FLOAT_TYPE: ddsts_ostream_emit(ostream, "float"); break;
    case DDSTS_DOUBLE_TYPE: ddsts_ostream_emit(ostream, "double"); break;
    case DDSTS_LONG_DOUBLE_TYPE: ddsts_ostream_emit(ostream, "long double"); break;
    case DDSTS_FIXED_PT_CONST_TYPE: ddsts_ostream_emit(ostream, "fixed"); break;
    case DDSTS_ANY_TYPE: ddsts_ostream_emit(ostream, "any"); break;
    case DDSTS_SEQUENCE:
      {
        ddsts_ostream_emit(ostream, "sequence<");
        ddsts_sequence_t *sequence = (ddsts_sequence_t*)type_spec;
        emit_type_spec(sequence->element_type.type_spec, ostream);
        if (sequence->bounded) {
          ddsts_ostream_emit(ostream, ",");
          ddsts_ostream_emit_ull(ostream, sequence->max);
        }
        ddsts_ostream_emit(ostream, ">");
      }
      break;
    case DDSTS_STRING:
      {
        ddsts_ostream_emit(ostream, "string");
        ddsts_string_t *string = (ddsts_string_t*)type_spec;
        if (string->bounded) {
          ddsts_ostream_emit(ostream, "<");
          ddsts_ostream_emit_ull(ostream, string->max);
          ddsts_ostream_emit(ostream, ">");
        }
      }
      break;
    case DDSTS_WIDE_STRING:
      {
        ddsts_ostream_emit(ostream, "wstring");
        ddsts_string_t *string = (ddsts_string_t*)type_spec;
        if (string->bounded) {
          ddsts_ostream_emit(ostream, "<");
          ddsts_ostream_emit_ull(ostream, string->max);
          ddsts_ostream_emit(ostream, ">");
        }
      }
      break;
    case DDSTS_FIXED_PT:
      {
        ddsts_ostream_emit(ostream, "fixed<");
        ddsts_fixed_pt_t *fixedpt = (ddsts_fixed_pt_t*)type_spec;
        ddsts_ostream_emit_ull(ostream, fixedpt->digits);
        ddsts_ostream_emit(ostream, ",");
        ddsts_ostream_emit_ull(ostream, fixedpt->fraction_digits);
        ddsts_ostream_emit(ostream, ">");
      }
      break;
    case DDSTS_MAP:
      {
        ddsts_ostream_emit(ostream, "map<");
        ddsts_map_t *map = (ddsts_map_t*)type_spec;
        emit_type_spec(map->key_type.type_spec, ostream);
        ddsts_ostream_emit(ostream, ",");
        emit_type_spec(map->value_type.type_spec, ostream);
        if (map->bounded) {
          ddsts_ostream_emit(ostream, ",");
          ddsts_ostream_emit_ull(ostream, map->max);
        }
        ddsts_ostream_emit(ostream, ">");
      }
      break;
    case DDSTS_STRUCT:
      {
        ddsts_ostream_emit(ostream, ((ddsts_struct_t*)type_spec)->def.name);
      }
      break;
    default:
      {
        ddsts_ostream_emit(ostream, "?");
        ddsts_ostream_emit_ull(ostream, type_spec->node.flags);
        ddsts_ostream_emit(ostream, "?");
      }
      break;
  }
}


static void ddsts_walker_execute_expr(ddsts_walker_t *walker, ddsts_walker_expr_t *expr, ddsts_walk_exec_state_t *state, void *context, ddsts_ostream_t *ostream)
{
  for (; expr != NULL; expr = expr->next) {
    switch(expr->type) {
      case ddsts_walker_expr_for_all_children:
        for (ddsts_node_t *node = state->node->children; node != NULL; node = node->next) {
          ddsts_walk_exec_state_t new_state;
          new_state.node = node;
          new_state.call_parent = state;
          ddsts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
        }
        break;
      case ddsts_walker_expr_for_all_modules:
        if (state->node->flags == DDSTS_MODULE) {
          for (ddsts_node_t *node = state->node->children; node != NULL; node = node->next) {
            if (node->flags == DDSTS_MODULE) {
              ddsts_walk_exec_state_t new_state;
              new_state.node = node;
              new_state.call_parent = state;
              ddsts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
            }
          }
        }
        break;
      case ddsts_walker_expr_for_all_structs:
        if (state->node->flags == DDSTS_MODULE) {
          for (ddsts_node_t *node = state->node->children; node != NULL; node = node->next) {
            if (node->flags == DDSTS_STRUCT) {
              ddsts_walk_exec_state_t new_state;
              new_state.node = node;
              new_state.call_parent = state;
              ddsts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
	    }
	  }
	}
	break;
      case ddsts_walker_expr_for_all_members:
        if (state->node->flags == DDSTS_STRUCT) {
          for (ddsts_node_t *node = state->node->children; node != NULL; node = node->next) {
            if (node->flags == DDSTS_STRUCT_MEMBER) {
              ddsts_walk_exec_state_t new_state;
              new_state.node = node;
              new_state.call_parent = state;
	      ddsts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
            }
	  }
	}
	break;
      case ddsts_walker_expr_for_all_declarators:
        if (state->node->flags == DDSTS_STRUCT_MEMBER) {
          for (ddsts_node_t *node = state->node->children; node != NULL; node = node->next) {
            ddsts_walk_exec_state_t new_state;
            new_state.node = node;
            new_state.call_parent = state;
            ddsts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
          }
        }
        break;
      case ddsts_walker_expr_for_call_parent:
        if (state->call_parent != NULL) {
          ddsts_walker_execute_expr(walker, expr->sub1, state->call_parent, context, ostream);
        }
        break;
      case ddsts_walker_expr_for_struct_member_type:
        if (state->node->flags == DDSTS_STRUCT_MEMBER) {
          ddsts_walk_exec_state_t new_state;
          new_state.node = &((ddsts_struct_member_t*)state->node)->member_type.type_spec->node;
          new_state.call_parent = state;
          ddsts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
        }
        break;
      case ddsts_walker_expr_for_sequence_element_type:
        if (state->node->flags == DDSTS_SEQUENCE) {
          ddsts_walk_exec_state_t new_state;
          new_state.node = &((ddsts_sequence_t*)state->node)->element_type.type_spec->node;
          new_state.call_parent = state;
          ddsts_walker_execute_expr(walker, expr->sub1, &new_state, context, ostream);
        }
        break;
      case ddsts_walker_expr_if_is_type:
        if (state->node->flags == expr->flags) {
          ddsts_walker_execute_expr(walker, expr->sub1, state, context, ostream);
        }
        break;
      case ddsts_walker_expr_if_func:
        if (expr->cond_func(state->node)) {
          ddsts_walker_execute_expr(walker, expr->sub1, state, context, ostream);
        }
        break;
      case ddsts_walker_expr_emit_type:
        {
          ddsts_type_spec_t *type_spec = 0;
          if (state->node->flags == DDSTS_STRUCT_MEMBER) {
            type_spec = ((ddsts_struct_member_t*)state->node)->member_type.type_spec;
          }
          if (type_spec == 0) {
            ddsts_ostream_emit(ostream, "??");
          }
          else {
            emit_type_spec(type_spec, ostream);
          }
        }
        break;
      case ddsts_walker_expr_emit_name:
	if (DDSTS_IS_DEFINITION(state->node->flags)) {
          ddsts_ostream_emit(ostream, ((ddsts_definition_t*)state->node)->name);
	}
        else {
          char buffer[40];
          os_ulltostr(state->node->flags, buffer, 39, NULL);
          ddsts_ostream_emit(ostream, "?");
          ddsts_ostream_emit(ostream, buffer);
          ddsts_ostream_emit(ostream, "?");
        }
	break;
      case ddsts_walker_expr_emit:
        ddsts_ostream_emit(ostream, expr->text);
	break;
      case ddsts_walker_expr_end_def:
        break;
      case ddsts_walker_expr_call_proc:
	for (ddsts_walker_proc_def_t *proc_def = walker->proc_defs; proc_def != NULL; proc_def = proc_def->next) {
          if (strcmp(proc_def->name, expr->text) == 0) {
            ddsts_walker_execute_expr(walker, proc_def->body, state, context, ostream);
	    break;
	  }
	}
        break;
    }
  }
}

extern void ddsts_walker_execute(ddsts_walker_t *walker, void *context, ddsts_ostream_t *ostream)
{
  ddsts_walk_exec_state_t state;
  state.node = walker->root_node;
  ddsts_walker_execute_expr(walker, walker->main, &state, context, ostream);
}

static void ddsts_walker_expr_free(ddsts_walker_expr_t *expr)
{
  while (expr != NULL) {
    ddsts_walker_expr_t *next = expr->next;
    ddsts_walker_expr_free(expr->sub1);
    ddsts_walker_expr_free(expr->sub2);
    os_free((void*)expr);
    expr = next;
  }
}

extern void ddsts_walker_free(ddsts_walker_t *walker)
{
  ddsts_walker_proc_def_t *proc_def;
  for (proc_def = walker->proc_defs; proc_def != NULL;) {
    ddsts_walker_proc_def_t *next = proc_def->next;
    ddsts_walker_expr_free(proc_def->body);
    os_free((void*)proc_def);
    proc_def = next;
  }
  ddsts_walker_expr_free(walker->main);
  os_free((void*)walker);
}


