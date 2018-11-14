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

typedef struct idl_context idl_context_t;

typedef struct idl_module_it idl_module_it_t;
idl_module_it_t *idl_create_module_it_from_context(idl_context_t *context);
idl_module_it_t *idl_create_module_it(idl_module_it_t *module_it);
bool idl_module_it_more(idl_module_it_t *module_it);
void idl_module_it_next(idl_module_it_t *module_it);
const char* idl_module_it_name(idl_module_it_t *module_it);
void idl_free_module_it(idl_module_it_t **ref_module_it);

typedef struct idl_struct_it idl_struct_it_t;
idl_struct_it_t *idl_create_struct_it(idl_module_it_t *module_it);
bool idl_struct_it_more(idl_struct_it_t *struct_it);
void idl_struct_it_next(idl_struct_it_t *struct_it);
const char* idl_struct_it_name(idl_struct_it_t *enum_it);
void idl_free_struct_it(idl_struct_it_t **ref_struct_it);

typedef struct idl_struct_member_it idl_struct_member_it_t;
idl_struct_member_it_t *idl_create_struct_member_it(idl_struct_it_t *struct_it);
bool idl_struct_member_it_more(idl_struct_member_it_t *struct_member_it);
void idl_struct_member_it_next(idl_struct_member_it_t *struct_member_it);
void idl_free_struct_member_it(idl_struct_member_it_t **ref_struct_member_it);

typedef struct idl_union_it idl_union_it_t;
idl_union_it_t *idl_create_union_it(idl_module_it_t *module_it);
bool idl_union_it_more(idl_union_it_t *struct_it);
void idl_union_it_next(idl_union_it_t *struct_it);
const char* idl_union_it_name(idl_union_it_t *enum_it);
void idl_free_union_it(idl_union_it_t **ref_union_it);

typedef struct idl_union_case_it idl_union_case_it_t;
idl_union_case_it_t *idl_create_union_case_it(idl_union_it_t *struct_it);
bool idl_union_case_it_more(idl_union_case_it_t *struct_case_it);
void idl_union_case_it_next(idl_union_case_it_t *struct_case_it);
void idl_free_union_case_it(idl_union_case_it_t **ref_union_case_it);

typedef struct idl_union_case_label_it idl_union_case_label_it_t;
idl_union_case_label_it_t *idl_create_union_case_label_it(idl_union_case_it_t *struct_it);
bool idl_union_case_label_it_more(idl_union_case_label_it_t *struct_case_label_it);
void idl_union_case_label_it_next(idl_union_case_label_it_t *struct_case_label_it);
bool idl_union_case_label_it_is_default(idl_union_case_label_it_t *enum_it);
void idl_free_union_case_label_it(idl_union_case_label_it_t **ref_union_case_label_it);

typedef struct idl_declarator_it idl_declarator_it_t;
idl_declarator_it_t *idl_create_declarator_it_for_struct(idl_struct_member_it_t *struct_member_it);
idl_declarator_it_t *idl_create_declarator_it_for_union(idl_union_case_it_t *union_case_it);
bool idl_declarator_it_more(idl_declarator_it_t *declarator_it);
void idl_declarator_it_next(idl_declarator_it_t *declarator_it);
const char* idl_declarator_it_name(idl_declarator_it_t *enum_it);
void idl_free_declarator_it(idl_declarator_it_t **ref_declarator_it);

typedef struct idl_enum_it idl_enum_it_t;
idl_enum_it_t *idl_create_enum_it(idl_module_it_t *module_it);
bool idl_enum_it_more(idl_enum_it_t *enum_it);
void idl_enum_it_next(idl_enum_it_t *enum_it);
const char* idl_enum_it_name(idl_enum_it_t *enum_it);
void idl_free_enum_it(idl_enum_it_t **ref_enum_it);

typedef struct idl_enum_value_it idl_enum_value_it_t;
idl_enum_value_it_t *idl_create_enum_value_it(idl_enum_it_t *enum_it);
bool idl_enum_value_it_more(idl_enum_value_it_t *enum_value_it);
void idl_enum_value_it_next(idl_enum_value_it_t *enum_value_it);
const char* idl_enum_value_it_name(idl_enum_value_it_t *enum_value_it);
void idl_free_enum_value_it(idl_enum_value_it_t **ref_enum_value_it);

