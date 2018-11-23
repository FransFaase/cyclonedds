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

typedef struct dds_tt_walker dds_tt_walker_t;

dds_tt_walker_t *dds_tt_create_walker(dds_tt_node_t *root_node);
void dds_tt_walker_def_proc(dds_tt_walker_t *walker, const char *name);
void dds_tt_walker_for_all_modules(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_structs(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_members(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_unions(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_cases(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_case_labels(dds_tt_walker_t *walker);
void dds_tt_walker_if_default_case_label(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_declarators(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_enums(dds_tt_walker_t *walker);
void dds_tt_walker_for_all_enum_value(dds_tt_walker_t *walker);
void dds_tt_walker_else(dds_tt_walker_t *walker);
void dds_tt_walker_end_if(dds_tt_walker_t *walker);
void dds_tt_walker_end_for(dds_tt_walker_t *walker);
void dds_tt_walker_emit_name(dds_tt_walker_t *walker);
void dds_tt_walker_emit(dds_tt_walker_t *walker, const char *text);
void dds_tt_walker_end_def(dds_tt_walker_t *walker);
void dds_tt_walker_call_proc(dds_tt_walker_t *walker, const char *name);
void dds_tt_walker_main(dds_tt_walker_t *walker);
void dds_tt_walker_end(dds_tt_walker_t *walker);
void dds_tt_walker_execute(dds_tt_walker_t *walker, char *buffer, size_t len);


