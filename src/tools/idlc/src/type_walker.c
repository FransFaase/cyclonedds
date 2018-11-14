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

#include "type.h"
#include "type_create.h"
#include "type_priv.h"
#include "type_walker.h"
#include "os/os.h"

typedef struct idl_walker_expr idl_walker_expr_t;
typedef struct idl_walker_proc_def idl_walker_proc_def_t;

struct idl_walker_proc_def {
  const char* name;
  idl_walker_expr_t *body;
  idl_walker_proc_def_t *next;
};

typedef enum {
  idl_walker_expr_for_all_modules,
  idl_walker_expr_for_all_structs,
  idl_walker_expr_for_all_members,
  idl_walker_expr_for_all_unions,
  idl_walker_expr_for_all_cases,
  idl_walker_expr_for_all_case_labels,
  idl_walker_expr_if_default_case_label,
  idl_walker_expr_for_all_declarators,
  idl_walker_expr_for_all_enums,
  idl_walker_expr_for_all_enum_values,
  idl_walker_expr_else,
  idl_walker_expr_end_if,
  idl_walker_expr_end_for,
  idl_walker_expr_emit_name,
  idl_walker_expr_emit,
  idl_walker_expr_end_def,
  idl_walker_expr_call_proc,
} idl_walker_expr_type_t;

struct idl_walker_expr {
  idl_walker_expr_t *parent;
  idl_walker_expr_type_t type;
  idl_walker_expr_t *sub1;
  idl_walker_expr_t *sub2;
  const char *text;
  idl_walker_expr_t *next;
};

struct idl_walker {
  idl_context_t *context;
  idl_walker_proc_def_t *proc_defs;
  idl_walker_expr_t *main;
  idl_walker_expr_t *cur_parent_expr;
  idl_walker_expr_t **ref_next_expr;
};

idl_walker_t *idl_create_walker(idl_context_t *context)
{
  //fprintf(stderr, "\n");
  idl_walker_t *walker = (idl_walker_t*)os_malloc(sizeof(idl_walker_t));
  walker->context = context;
  walker->proc_defs = NULL;
  walker->main = NULL;
  walker->cur_parent_expr = NULL;
  walker->ref_next_expr = NULL;
  return walker;
}

void idl_walker_def_proc(idl_walker_t *walker, const char *name)
{
  //fprintf(stderr, "\n");
  idl_walker_proc_def_t *proc_def = (idl_walker_proc_def_t*)os_malloc(sizeof(idl_walker_proc_def_t));
  proc_def->name = name;
  proc_def->body = NULL;
  proc_def->next = walker->proc_defs;
  walker->proc_defs = proc_def;
  walker->ref_next_expr = &proc_def->body;
}

static idl_walker_expr_t *idl_create_expr(idl_walker_expr_type_t type, idl_walker_t *walker)
{
  idl_walker_expr_t *expr = (idl_walker_expr_t*)os_malloc(sizeof(idl_walker_expr_t));
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

void idl_walker_for_all_modules(idl_walker_t *walker)
{
  //fprintf(stderr, "all modules\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_modules, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_for_all_structs(idl_walker_t *walker)
{
  //fprintf(stderr, "all structs\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_structs, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
  (void)walker;
}

void idl_walker_for_all_members(idl_walker_t *walker)
{
  //fprintf(stderr, "all members\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_members, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_for_all_unions(idl_walker_t *walker)
{
  //fprintf(stderr, "all unions\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_unions, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_for_all_cases(idl_walker_t *walker)
{
  //fprintf(stderr, "all cases\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_cases, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_for_all_case_labels(idl_walker_t *walker)
{
  //fprintf(stderr, "all labels\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_case_labels, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_if_default_case_label(idl_walker_t *walker)
{
  //fprintf(stderr, "if def\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_if_default_case_label, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_for_all_declarators(idl_walker_t *walker)
{
  //fprintf(stderr, "all declaratiors\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_declarators, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_for_all_enums(idl_walker_t *walker)
{
  //fprintf(stderr, "all enums\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_enums, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_for_all_enum_value(idl_walker_t *walker)
{
  //fprintf(stderr, "all enum values\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_for_all_enum_values, walker);
  walker->ref_next_expr = &expr->sub1;
  walker->cur_parent_expr = expr; 
}

void idl_walker_else(idl_walker_t *walker)
{
  //fprintf(stderr, "else\n");
  if (walker->cur_parent_expr->type != idl_walker_expr_if_default_case_label) {
    //fprintf(stderr, "ERROR: else does not match if\n");
  }
  walker->ref_next_expr = &walker->cur_parent_expr->sub2;
}

void idl_walker_end_if(idl_walker_t *walker)
{
  //fprintf(stderr, "end if %p\n", walker->cur_parent_expr);
  if (walker->cur_parent_expr->type != idl_walker_expr_if_default_case_label) {
    //fprintf(stderr, "ERROR: end if does not match if\n");
  }
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
  //fprintf(stderr, "new parent %p\n", walker->cur_parent_expr);
}

void idl_walker_end_for(idl_walker_t *walker)
{
  //fprintf(stderr, "end for %p\n", walker->cur_parent_expr);
  walker->ref_next_expr = &walker->cur_parent_expr->next;
  //fprintf(stderr, "1\n");
  walker->cur_parent_expr = walker->cur_parent_expr->parent;
  //fprintf(stderr, "new parent %p\n", walker->cur_parent_expr);
}

void idl_walker_emit_name(idl_walker_t *walker)
{
  //fprintf(stderr, "emit name\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_emit_name, walker);
  (void)expr;
}

void idl_walker_emit(idl_walker_t *walker, const char *text)
{
  //fprintf(stderr, "emit\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_emit, walker);
  expr->text = text;
}

void idl_walker_end_def(idl_walker_t *walker)
{
  //fprintf(stderr, "end_def\n");
  walker->ref_next_expr = NULL;
}

void idl_walker_call_proc(idl_walker_t *walker, const char *name)
{
  //fprintf(stderr, "call\n");
  idl_walker_expr_t *expr = idl_create_expr(idl_walker_expr_call_proc, walker);
  expr->text = name;
}

void idl_walker_main(idl_walker_t *walker)
{
  //fprintf(stderr, "main\n");
  walker->ref_next_expr = &walker->main;
}

void idl_walker_end(idl_walker_t *walker)
{
  //fprintf(stderr, "end\n");
  walker->ref_next_expr = NULL;
}

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

static void idl_ostream_emit(idl_ostream_t *stream, const char *s)
{
  while(*s != '\0' && stream->s < stream->e) {
    *stream->s++ = *s++;
  }
  *stream->s = '\0';
}

typedef struct {
  idl_definition_t *cur_def;
  idl_struct_member_t *struct_member;
  idl_union_case_t *union_case;
  idl_union_case_label_t *union_case_label;
  idl_enum_value_definition_t *enum_value;
} idl_exec_state_t;

void idl_walker_execute_expr(idl_walker_t *walker, idl_walker_expr_t *expr, idl_exec_state_t *state, idl_ostream_t *stream)
{
  for (; expr != NULL; expr = expr->next) {
    switch(expr->type) {
      case idl_walker_expr_for_all_modules:
        if (state->cur_def->type == idl_definition_module) {
          for (idl_definition_t *def = state->cur_def->module_def->definitions; def != NULL; def = def->next) {
            if (def->type == idl_definition_module) {
              idl_exec_state_t new_state = *state;
              new_state.cur_def = def;
              idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
            }
          }
        }
        break;
      case idl_walker_expr_for_all_structs:
        if (state->cur_def->type == idl_definition_module) {
          for (idl_definition_t *def = state->cur_def->module_def->definitions; def != NULL; def = def->next) {
            if (def->type == idl_definition_struct) {
              idl_exec_state_t new_state = *state;
              new_state.cur_def = def;
              idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	     }
	   }
	 }
	 break;
       case idl_walker_expr_for_all_members:
	 if (state->cur_def->type == idl_definition_struct) {
           idl_exec_state_t new_state = *state;
	   for (new_state.struct_member = state->cur_def->struct_def->members; new_state.struct_member != NULL; new_state.struct_member = new_state.struct_member->next) {
	     idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	   }
	 }
	 break;
       case idl_walker_expr_for_all_unions:
	 if (state->cur_def->type == idl_definition_module) {
           for (idl_definition_t *def = state->cur_def->module_def->definitions; def != NULL; def = def->next) {
             if (def->type == idl_definition_union) {
	       idl_exec_state_t new_state = *state;
	       new_state.cur_def = def;
	       idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	     }
	   }
	 }
	 break;
       case idl_walker_expr_for_all_cases:
	 if (state->cur_def->type == idl_definition_union) {
           idl_exec_state_t new_state = *state;
	   for (new_state.union_case = state->cur_def->union_def->cases; new_state.union_case != NULL; new_state.union_case = new_state.union_case->next) {
	     idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	   }
	 }
	 break;
       case idl_walker_expr_for_all_case_labels:
	 if (state->union_case != NULL) {
	   idl_exec_state_t new_state = *state;
	   for (new_state.union_case_label = state->union_case->labels; new_state.union_case_label != NULL; new_state.union_case_label = new_state.union_case_label->next) {
	     idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	   }
	 }
	 break;
       case idl_walker_expr_if_default_case_label:
	 if (state->union_case_label != NULL) {
           idl_walker_execute_expr(walker, state->union_case_label->is_default ? expr->sub1 : expr->sub2, state, stream);
	 }
	 break;
       case idl_walker_expr_for_all_declarators:
	 if (state->struct_member != NULL) {
           idl_exec_state_t new_state = *state;
           for (new_state.cur_def = state->struct_member->declarators; new_state.cur_def != NULL; new_state.cur_def = new_state.cur_def->next) {
	     idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	   }
	 } else if (state->union_case != NULL) {
           idl_exec_state_t new_state = *state;
           for (new_state.cur_def = state->union_case->declarators; new_state.cur_def != NULL; new_state.cur_def = new_state.cur_def->next) {
	     idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	   }
	 }
	 break;
       case idl_walker_expr_for_all_enums:
	 if (state->cur_def->type == idl_definition_module) {
           for (idl_definition_t *def = state->cur_def->module_def->definitions; def != NULL; def = def->next) {
             if (def->type == idl_definition_enum) {
	       idl_exec_state_t new_state = *state;
	       new_state.cur_def = def;
	       idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	     }
	   }
	 }
	 break;
       case idl_walker_expr_for_all_enum_values:
	 if (state->cur_def->type == idl_definition_enum) {
	   idl_exec_state_t new_state = *state;
	   for (new_state.enum_value = state->cur_def->enum_def->values; new_state.enum_value != 0; new_state.enum_value = new_state.enum_value->next) {
             idl_walker_execute_expr(walker, expr->sub1, &new_state, stream);
	   }
	 }
	 break;
       case idl_walker_expr_else:
	 break;
       case idl_walker_expr_end_if:
	 break;
       case idl_walker_expr_end_for:
	 break;
       case idl_walker_expr_emit_name:
	 if (state->enum_value != NULL) {
           idl_ostream_emit(stream, state->enum_value->def->name);
	 } else if (state->cur_def != NULL) {
           idl_ostream_emit(stream, state->cur_def->name);
	 }
	 break;
       case idl_walker_expr_emit:
         idl_ostream_emit(stream, expr->text);
	 break;
       case idl_walker_expr_end_def:
        break;
      case idl_walker_expr_call_proc:
	for (idl_walker_proc_def_t *proc_def = walker->proc_defs; proc_def != NULL; proc_def = proc_def->next) {
          if (strcmp(proc_def->name, expr->text) == 0) {
            idl_walker_execute_expr(walker, proc_def->body, state, stream);
	    break;
	  }
	}
        break;
    }
  }
}

void idl_walker_execute(idl_walker_t *walker, char *buffer, size_t len)
{
  //fprintf(stderr, "exeute\n");
  idl_ostream_t stream;
  idl_ostream_init(&stream, buffer, len);
  idl_exec_state_t state;
  state.cur_def = walker->context->root_scope;
  state.struct_member = NULL;
  state.union_case = NULL;
  state.union_case_label = NULL;
  state.enum_value = NULL;
  idl_walker_execute_expr(walker, walker->main, &state, &stream);
}



