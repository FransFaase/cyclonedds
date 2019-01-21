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

#include "gen_ostream.h"

typedef struct ddsts_walker ddsts_walker_t;

typedef struct ddsts_walk_exec_state_t ddsts_walk_exec_state_t;
struct ddsts_walk_exec_state_t {
  ddsts_node_t *node;
  ddsts_walk_exec_state_t *call_parent;
};

typedef struct {
  ddsts_ostream_t *ostream;
} ddsts_walk_context_t;

typedef void (*ddsts_walker_call_func_t)(ddsts_walk_exec_state_t *exec_state, ddsts_walk_context_t *context);

void ddsts_walk(ddsts_walk_exec_state_t *exec_state, ddsts_node_flags_t visit, ddsts_node_flags_t call, ddsts_walker_call_func_t func, ddsts_walk_context_t *context);

ddsts_walker_t *ddsts_create_walker(ddsts_node_t *root_node);
void ddsts_walker_def_proc(ddsts_walker_t *walker, const char *name);
void ddsts_walker_for_all_children(ddsts_walker_t *walker);
void ddsts_walker_for_all_modules(ddsts_walker_t *walker);
void ddsts_walker_for_all_structs(ddsts_walker_t *walker);
void ddsts_walker_for_all_members(ddsts_walker_t *walker);
void ddsts_walker_for_all_unions(ddsts_walker_t *walker);
void ddsts_walker_for_all_declarators(ddsts_walker_t *walker);
void ddsts_walker_for_call_parent(ddsts_walker_t *walker);
void ddsts_walker_for_struct_member_type(ddsts_walker_t *walker);
void ddsts_walker_for_sequence_element_type(ddsts_walker_t *walker);
void ddsts_walker_end_for(ddsts_walker_t *walker);
void ddsts_walker_if_is_type(ddsts_walker_t *walker, ddsts_node_flags_t flags);
void ddsts_walker_if_func(ddsts_walker_t *walker, bool (*func)(ddsts_node_t *node));
void ddsts_walker_else(ddsts_walker_t *walker);
void ddsts_walker_end_if(ddsts_walker_t *walker);
void ddsts_walker_emit(ddsts_walker_t *walker, const char *text);
void ddsts_walker_emit_type(ddsts_walker_t *walker);
void ddsts_walker_emit_name(ddsts_walker_t *walker);
void ddsts_walker_end_def(ddsts_walker_t *walker);
void ddsts_walker_call_proc(ddsts_walker_t *walker, const char *name);
void ddsts_walker_main(ddsts_walker_t *walker);
void ddsts_walker_end(ddsts_walker_t *walker);
void ddsts_walker_execute(ddsts_walker_t *walker, void *context, ddsts_ostream_t *ostream);
void ddsts_walker_free(ddsts_walker_t *walker);
