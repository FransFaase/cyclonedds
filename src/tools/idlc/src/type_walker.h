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

typedef struct idl_walker idl_walker_t;

idl_walker_t *idl_create_walker(idl_context_t *context);
void idl_walker_def_proc(idl_walker_t *walker, const char *name);
void idl_walker_for_all_modules(idl_walker_t *walker);
void idl_walker_for_all_structs(idl_walker_t *walker);
void idl_walker_for_all_members(idl_walker_t *walker);
void idl_walker_for_all_unions(idl_walker_t *walker);
void idl_walker_for_all_cases(idl_walker_t *walker);
void idl_walker_for_all_case_labels(idl_walker_t *walker);
void idl_walker_if_default_case_label(idl_walker_t *walker);
void idl_walker_for_all_declarators(idl_walker_t *walker);
void idl_walker_for_all_enums(idl_walker_t *walker);
void idl_walker_for_all_enum_value(idl_walker_t *walker);
void idl_walker_else(idl_walker_t *walker);
void idl_walker_end_if(idl_walker_t *walker);
void idl_walker_end_for(idl_walker_t *walker);
void idl_walker_emit_name(idl_walker_t *walker);
void idl_walker_emit(idl_walker_t *walker, const char *text);
void idl_walker_end_def(idl_walker_t *walker);
void idl_walker_call_proc(idl_walker_t *walker, const char *name);
void idl_walker_main(idl_walker_t *walker);
void idl_walker_end(idl_walker_t *walker);
void idl_walker_execute(idl_walker_t *walker, char *buffer, size_t len);


