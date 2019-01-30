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

typedef struct dds_ts_walker dds_ts_walker_t;

typedef struct dds_ts_walker_exec_state_t dds_ts_walker_exec_state_t;
struct dds_ts_walker_exec_state_t {
  dds_ts_node_t *node;
  dds_ts_walker_exec_state_t *call_parent;
};

typedef void (*dds_ts_walker_call_func_t)(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream);

dds_ts_walker_t *dds_ts_create_walker(dds_ts_node_t *root_node);
void dds_ts_walker_def_proc(dds_ts_walker_t *walker, const char *name);
void dds_ts_walker_for_all_children(dds_ts_walker_t *walker);
void dds_ts_walker_for_all_modules(dds_ts_walker_t *walker);
void dds_ts_walker_for_all_structs(dds_ts_walker_t *walker);
void dds_ts_walker_for_all_members(dds_ts_walker_t *walker);
void dds_ts_walker_for_all_unions(dds_ts_walker_t *walker);
void dds_ts_walker_for_all_declarators(dds_ts_walker_t *walker);
void dds_ts_walker_for_call_parent(dds_ts_walker_t *walker);
void dds_ts_walker_for_struct_member_type(dds_ts_walker_t *walker);
void dds_ts_walker_for_sequence_element_type(dds_ts_walker_t *walker);
void dds_ts_walker_end_for(dds_ts_walker_t *walker);
void dds_ts_walker_if_is_type(dds_ts_walker_t *walker, dds_ts_node_flags_t flags);
void dds_ts_walker_if_func(dds_ts_walker_t *walker, bool (*func)(dds_ts_node_t *node));
void dds_ts_walker_else(dds_ts_walker_t *walker);
void dds_ts_walker_end_if(dds_ts_walker_t *walker);
void dds_ts_walker_emit(dds_ts_walker_t *walker, const char *text);
void dds_ts_walker_emit_type(dds_ts_walker_t *walker);
void dds_ts_walker_emit_name(dds_ts_walker_t *walker);
void dds_ts_walker_end_def(dds_ts_walker_t *walker);
void dds_ts_walker_call_proc(dds_ts_walker_t *walker, const char *name);
void dds_ts_walker_call_func(dds_ts_walker_t *walker, dds_ts_walker_call_func_t func);
void dds_ts_walker_main(dds_ts_walker_t *walker);
void dds_ts_walker_end(dds_ts_walker_t *walker);
void dds_ts_walker_execute(dds_ts_walker_t *walker, void *context, dds_ts_ostream_t *ostream);
void dds_ts_walker_free(dds_ts_walker_t *walker);
