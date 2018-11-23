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

typedef struct dds_tt_walker_expr dds_tt_walker_expr_t;
typedef struct dds_tt_walker_proc_def dds_tt_walker_proc_def_t;

struct dds_tt_walker_proc_def {
  const char* name;
  dds_tt_walker_expr_t *body;
  dds_tt_walker_proc_def_t *next;
};

typedef enum {
  dds_tt_walker_expr_for_all_modules,
  dds_tt_walker_expr_for_all_structs,
  dds_tt_walker_expr_for_all_members,
  dds_tt_walker_expr_for_all_unions,
  dds_tt_walker_expr_for_all_cases,
  dds_tt_walker_expr_for_all_case_labels,
  dds_tt_walker_expr_if_default_case_label,
  dds_tt_walker_expr_for_all_declarators,
  dds_tt_walker_expr_for_all_enums,
  dds_tt_walker_expr_for_all_enum_values,
  dds_tt_walker_expr_else,
  dds_tt_walker_expr_end_if,
  dds_tt_walker_expr_end_for,
  dds_tt_walker_expr_emit_name,
  dds_tt_walker_expr_emit,
  dds_tt_walker_expr_end_def,
  dds_tt_walker_expr_call_proc,
} dds_tt_walker_expr_type_t;

struct dds_tt_walker_expr {
  dds_tt_walker_expr_t *parent;
  dds_tt_walker_expr_type_t type;
  dds_tt_walker_expr_t *sub1;
  dds_tt_walker_expr_t *sub2;
  const char *text;
  dds_tt_walker_expr_t *next;
};

struct dds_tt_walker {
  dds_tt_node_t *root_node;
  dds_tt_walker_proc_def_t *proc_defs;
  dds_tt_walker_expr_t *main;
  dds_tt_walker_expr_t *cur_parent_expr;
  dds_tt_walker_expr_t **ref_next_expr;
};

dds_tt_walker_t *dds_tt_create_walker(dds_tt_node_t *root_node)
{
  //fprintf(stderr, "\n");
  dds_tt_walker_t *walker = (dds_tt_walker_t*)os_malloc(sizeof(dds_tt_walker_t));
  walker->root_node = root_node;
  walker->proc_defs = NULL;
  walker->main = NULL;
  walker->cur_parent_expr = NULL;
  walker->ref_next_expr = NULL;
  return walker;
}

void dds_tt_walker_def_proc(dds_tt_walker_t *walker, const char *name)
{
  //fprintf(stderr, "\n");
  dds_tt_walker_proc_def_t *proc_def = (dds_tt_walker_proc_def_t*)os_malloc(sizeof(dds_tt_walker_proc_def_t));
  proc_def->name = name;
  proc_def->body = NULL;
  proc_def->next = walker->proc_defs;
  walker->proc_defs = proc_def;
  walker->ref_next_expr = &proc_def->body;
}

static dds_tt_walker_expr_t *dds_tt_create_expr(dds_tt_walker_expr_type_t type, dds_tt_walker_t *walker)
{
  dds_tt_walker_expr_t *expr = (dds_tt_walker_expr_t*)os_malloc(sizeof(dds_tt_walker_expr_t));
  //fprintf(stderr, "expr %p\n", expr);
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

void dds_tt_walker_for_all_modules(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all modules\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_modules, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_for_all_structs(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all structs\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_structs, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
  (void)walker;
}

void dds_tt_walker_for_all_members(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all members\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_members, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_for_all_unions(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all unions\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_unions, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_for_all_cases(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all cases\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_cases, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_for_all_case_labels(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all labels\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_case_labels, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_if_default_case_label(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "if def\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_if_default_case_label, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_for_all_declarators(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all declaratiors\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_declarators, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_for_all_enums(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all enums\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_enums, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_for_all_enum_value(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "all enum values\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_for_all_enum_values, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void dds_tt_walker_else(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "else\n");
  if (walker->cur_parent_expr->type != dds_tt_walker_expr_if_default_case_label) {
    //fprintf(stderr, "ERROR: else does not match if\n");
  }
  walker->ref_next_expr = &walker->cur_parent_expr->sub2;
}

void dds_tt_walker_end_if(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "end if %p\n", walker->cur_parent_expr);
  if (walker->cur_parent_expr->type != dds_tt_walker_expr_if_default_case_label) {
    //fprintf(stderr, "ERROR: end if does not match if\n");
  }
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
  //fprintf(stderr, "new parent %p\n", walker->cur_parent_expr);
}

void dds_tt_walker_end_for(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "end for %p\n", walker->cur_parent_expr);
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  //fprintf(stderr, "1\n");
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
  //fprintf(stderr, "new parent %p\n", walker->cur_parent_expr);
}

void dds_tt_walker_emit_name(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "emit name\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_emit_name, walker);
  (void)expr;
}

void dds_tt_walker_emit(dds_tt_walker_t *walker, const char *text)
{
  //fprintf(stderr, "emit\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_emit, walker);
  expr->text = text;
}

void dds_tt_walker_end_def(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "end_def\n");
  walker->ref_next_expr = NULL;
}

void dds_tt_walker_call_proc(dds_tt_walker_t *walker, const char *name)
{
  //fprintf(stderr, "call\n");
  dds_tt_walker_expr_t *expr = dds_tt_create_expr(dds_tt_walker_expr_call_proc, walker);
  expr->text = name;
}

void dds_tt_walker_main(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "main\n");
  walker->ref_next_expr = &walker->main;
}

void dds_tt_walker_end(dds_tt_walker_t *walker)
{
  //fprintf(stderr, "end\n");
  walker->ref_next_expr = NULL;
}

typedef struct
{
  char *s;
  const char *e;
} dds_tt_ostream_t;

static void dds_tt_ostream_init(dds_tt_ostream_t *stream, char *buffer, size_t len)
{
  stream->s = buffer;
  stream->e = buffer + len - 1;
}

static void dds_tt_ostream_emit(dds_tt_ostream_t *stream, const char *s)
{
  while(*s != '\0' && stream->s < stream->e) {
    *stream->s++ = *s++;
  }
  *stream->s = '\0';
}

typedef struct {
  dds_tt_node_t *cur_node;
} dds_tt_exec_state_t;

void dds_tt_walker_execute_expr(dds_tt_walker_t *walker, dds_tt_walker_expr_t *expr, dds_tt_exec_state_t *state, dds_tt_ostream_t *stream)
{
  //if (state->cur_node == 0)
  //  fprintf(stderr, "node == NULL\n");
  //else
  //  fprintf(stderr, "node->flags = %d\n", state->cur_node->flags);
  for (; expr != NULL; expr = expr->next) {
    switch(expr->type) {
      case dds_tt_walker_expr_for_all_modules:
        if (state->cur_node->flags == DDS_TT_MODULE) {
          for (dds_tt_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TT_MODULE) {
              dds_tt_exec_state_t new_state = *state;
              new_state.cur_node = node;
              dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
            }
          }
        }
        break;
      case dds_tt_walker_expr_for_all_structs:
        if (state->cur_node->flags == DDS_TT_MODULE) {
          for (dds_tt_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TT_STRUCT) {
              dds_tt_exec_state_t new_state = *state;
              new_state.cur_node = node;
              dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	     }
	   }
	 }
	 break;
      case dds_tt_walker_expr_for_all_members:
        if (state->cur_node->flags == DDS_TT_STRUCT) {
          dds_tt_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	     dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_tt_walker_expr_for_all_unions:
	if (state->cur_node->flags == DDS_TT_MODULE) {
          for (dds_tt_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TT_UNION) {
	      dds_tt_exec_state_t new_state = *state;
	      new_state.cur_node = node;
	      dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	    }
	  }
	}
	break;
      case dds_tt_walker_expr_for_all_cases:
	if (state->cur_node->flags == DDS_TT_UNION) {
          dds_tt_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	    dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_tt_walker_expr_for_all_case_labels:
	if (state->cur_node->flags == DDS_TT_UNION_CASE) {
	  dds_tt_exec_state_t new_state = *state;
          for (new_state.cur_node = ((dds_tt_union_case_t*)state->cur_node)->labels; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	     dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_tt_walker_expr_if_default_case_label:
	if (state->cur_node->flags == DDS_TT_UNION_CASE_DEFAULT || state->cur_node->flags == DDS_TT_UNION_CASE_LABEL) {
          dds_tt_walker_execute_expr(walker, state->cur_node->flags == DDS_TT_UNION_CASE_DEFAULT ? expr->sub1 : expr->sub2, state, stream);
	}
	break;
      case dds_tt_walker_expr_for_all_declarators:
	if (state->cur_node->flags == DDS_TT_STRUCT_MEMBER || state->cur_node->flags == DDS_TT_UNION_CASE) {
          dds_tt_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
	    dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_tt_walker_expr_for_all_enums:
	if (state->cur_node->flags == DDS_TT_MODULE) {
          for (dds_tt_node_t *node = state->cur_node->children; node != NULL; node = node->next) {
            if (node->flags == DDS_TT_ENUM) {
	      dds_tt_exec_state_t new_state = *state;
	      new_state.cur_node = node;
	      dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	    }
	  }
	}
	break;
      case dds_tt_walker_expr_for_all_enum_values:
	if (state->cur_node->flags == DDS_TT_ENUM) {
	  dds_tt_exec_state_t new_state = *state;
          for (new_state.cur_node = state->cur_node->children; new_state.cur_node != NULL; new_state.cur_node = new_state.cur_node->next) {
            dds_tt_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	  }
	}
	break;
      case dds_tt_walker_expr_else:
	break;
      case dds_tt_walker_expr_end_if:
	break;
      case dds_tt_walker_expr_end_for:
        break;
      case dds_tt_walker_expr_emit_name:
	if (DDS_TT_IS_DEFINITION(state->cur_node->flags)) {
          dds_tt_ostream_emit(stream, ((dds_tt_definition_t*)state->cur_node)->name);
	}
	break;
      case dds_tt_walker_expr_emit:
        dds_tt_ostream_emit(stream, expr->text);
	break;
      case dds_tt_walker_expr_end_def:
        break;
      case dds_tt_walker_expr_call_proc:
	for (dds_tt_walker_proc_def_t *proc_def = walker->proc_defs; proc_def != NULL; proc_def = proc_def->next) {
          if (strcmp(proc_def->name, expr->text) == 0) {
            dds_tt_walker_execute_expr(walker, proc_def->body, state, stream);
	    break;
	  }
	}
        break;
    }
  }
}

void dds_tt_walker_execute(dds_tt_walker_t *walker, char *buffer, size_t len)
{
  //fprintf(stderr, "exeute\n");
  dds_tt_ostream_t stream;
  dds_tt_ostream_init(&stream, buffer, len);
  dds_tt_exec_state_t state;
  state.cur_node = walker->root_node;
  dds_tt_walker_execute_expr(walker, walker->main, &state, &stream);
}



