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
#ifndef DDS_TS_CREATE_H
#define DDS_TS_CREATE_H

#include <stdbool.h>

/* Some types only used during parsing */

typedef struct dds_ts_scoped_name dds_ts_scoped_name_t;
typedef struct dds_ts_context dds_ts_context_t;

typedef enum {
  dds_ts_operator_or,
  dds_ts_operator_xor,
  dds_ts_operator_and,
  dds_ts_operator_shift_left,
  dds_ts_operator_shift_right,
  dds_ts_operator_add,
  dds_ts_operator_sub,
  dds_ts_operator_times,
  dds_ts_operator_div,
  dds_ts_operator_mod,
  dds_ts_operator_minus,
  dds_ts_operator_plus,
  dds_ts_operator_inv
} dds_ts_operator_type_t;


void dds_ts_eval_unary_oper(dds_ts_operator_type_t operator_type, const dds_ts_literal_t *operand, dds_ts_literal_t *result);
void dds_ts_eval_binary_oper(dds_ts_operator_type_t operator_type, const dds_ts_literal_t *lhs, const dds_ts_literal_t *rhs, dds_ts_literal_t *result);

typedef struct dds_ts_context dds_ts_context_t;
dds_ts_context_t* dds_ts_create_context();
void dds_ts_context_set_ignore_yyerror(dds_ts_context_t* context, bool ignore_yyerror);
bool dds_ts_context_get_ignore_yyerror(dds_ts_context_t* context);
dds_ts_node_t* dds_ts_context_get_root_node();
void dds_ts_free_context(dds_ts_context_t* context);

dds_ts_type_spec_t *dds_ts_new_base_type(dds_ts_context_t *context, dds_ts_node_flags_t flags);
dds_ts_type_spec_t *dds_ts_new_sequence_type(dds_ts_context_t *context, dds_ts_type_spec_t *element_type, const dds_ts_literal_t *size);
dds_ts_type_spec_t *dds_ts_new_sequence_type_unbound(dds_ts_context_t *context, dds_ts_type_spec_t *base);
dds_ts_type_spec_t *dds_ts_new_string_type(dds_ts_context_t *context, const dds_ts_literal_t *size);
dds_ts_type_spec_t *dds_ts_new_string_type_unbound(dds_ts_context_t *context);
dds_ts_type_spec_t *dds_ts_new_wide_string_type(dds_ts_context_t *context, const dds_ts_literal_t *size);
dds_ts_type_spec_t *dds_ts_new_wide_string_type_unbound(dds_ts_context_t *context);
dds_ts_type_spec_t *dds_ts_new_fixed_type(dds_ts_context_t *context, const dds_ts_literal_t *digits, const dds_ts_literal_t *fraction_digits);
dds_ts_type_spec_t *dds_ts_new_map_type(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type, const dds_ts_literal_t *size);
dds_ts_type_spec_t *dds_ts_new_map_type_unbound(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type);
dds_ts_scoped_name_t *dds_ts_new_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t* prev, bool top, dds_ts_identifier_t name);
dds_ts_node_flags_t dds_ts_get_base_type_of_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t *scoped_name);
void dds_ts_get_value_of_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t *scoped_name, dds_ts_literal_t *result);


void dds_ts_module_open(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_module_close(dds_ts_context_t *context);

void dds_ts_add_struct_forward(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_add_struct_open(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_add_struct_extension_open(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_scoped_name_t *scoped_name);
void dds_ts_add_struct_member(dds_ts_context_t *context, dds_ts_type_spec_t *type);
void dds_ts_struct_close(dds_ts_context_t *context);
void dds_ts_struct_empty_close(dds_ts_context_t *context);

void dds_ts_add_union_forward(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_add_union_open(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_node_flags_t base_type);
void dds_ts_add_union_case_label(dds_ts_context_t *context, const dds_ts_literal_t *value);
void dds_ts_add_union_case_default(dds_ts_context_t *context);
void dds_ts_add_union_element(dds_ts_context_t *context, dds_ts_type_spec_t *type);
void dds_ts_union_close(dds_ts_context_t *context);

void dds_ts_add_typedef_open(dds_ts_context_t *context);
void dds_ts_typedef_set_type(dds_ts_context_t *context, dds_ts_type_spec_t *type);
void dds_ts_typedef_close(dds_ts_context_t *context);

void dds_ts_add_declarator(dds_ts_context_t *context, dds_ts_identifier_t name);

void dds_ts_add_const_def(dds_ts_context_t *context, dds_ts_type_spec_t *base_type, dds_ts_identifier_t name, const dds_ts_literal_t *value);

void dds_ts_add_enum_open(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_add_enum_enumerator(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_enum_close(dds_ts_context_t *context);

void dds_ts_add_array_open(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_add_array_size(dds_ts_context_t *context, const dds_ts_literal_t *value);
void dds_ts_array_close(dds_ts_context_t* context);

void dds_ts_add_native(dds_ts_context_t* context, dds_ts_identifier_t name);

void dds_ts_add_const_definition(dds_ts_context_t *context, dds_ts_identifier_t name, const dds_ts_literal_t *value);

void dds_ts_add_bitset_open(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_type_spec_t *opt_type);
void dds_ts_add_bitset_field(dds_ts_context_t *context, const dds_ts_literal_t *index);
void dds_ts_add_bitset_field_to(dds_ts_context_t *context, const dds_ts_literal_t *index, dds_ts_node_flags_t dest_type);
void dds_ts_add_bitset_ident(dds_ts_context_t *context, dds_ts_identifier_t ident);
void dds_ts_bitset_close(dds_ts_context_t *context);

void dds_ts_add_bitmask_open(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_add_bitmask_value(dds_ts_context_t *context, dds_ts_identifier_t value);
void dds_ts_bitmask_close(dds_ts_context_t *context);

void dds_ts_add_annotation_open(dds_ts_context_t *context, dds_ts_identifier_t name);
void dds_ts_add_annotation_member_open(dds_ts_context_t *context, dds_ts_type_spec_t *base_type, dds_ts_identifier_t name);
void dds_ts_annotation_member_set_default(dds_ts_context_t *context, const dds_ts_literal_t *value);
void dds_ts_annotation_member_close(dds_ts_context_t *context);
void dds_ts_annotation_close(dds_ts_context_t *context);

void dds_ts_add_annotation_appl_open(dds_ts_context_t *context, dds_ts_scoped_name_t *scoped_name);
void dds_ts_add_annotation_appl_expr(dds_ts_context_t *context, const dds_ts_literal_t *value);
void dds_ts_add_annotation_appl_param(dds_ts_context_t *context, dds_ts_identifier_t name, const dds_ts_literal_t *value);
void dds_ts_annotation_appl_close(dds_ts_context_t *context);

#endif /* DDS_TS_CREATE_H */
