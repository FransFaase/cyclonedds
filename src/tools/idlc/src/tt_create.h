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
#ifndef DDS_TT_CREATE_H
#define DDS_TT_CREATE_H

#include <stdbool.h>

// Some types only used during parsing

typedef struct dds_tt_scoped_name dds_tt_scoped_name_t;
typedef struct dds_tt_context dds_tt_context_t;

typedef enum {
  dds_tt_operator_or,
  dds_tt_operator_xor,
  dds_tt_operator_and,
  dds_tt_operator_shift_left,
  dds_tt_operator_shift_right,
  dds_tt_operator_add,
  dds_tt_operator_sub,
  dds_tt_operator_times,
  dds_tt_operator_div,
  dds_tt_operator_mod,
  dds_tt_operator_minus,
  dds_tt_operator_plus,
  dds_tt_operator_inv
} dds_tt_operator_type_t;

void dds_tt_eval_unary_oper(dds_tt_operator_type_t operator_type, dds_tt_literal_t operand, dds_tt_literal_t *result);
void dds_tt_eval_binary_oper(dds_tt_operator_type_t operator_type, dds_tt_literal_t lhs, dds_tt_literal_t rhs, dds_tt_literal_t *result);

typedef struct dds_tt_context dds_tt_context_t;
dds_tt_context_t* dds_tt_create_context();
void dds_tt_context_set_ignore_yyerror(dds_tt_context_t* context, bool ignore_yyerror);
bool dds_tt_context_get_ignore_yyerror(dds_tt_context_t* context);
dds_tt_node_t* dds_tt_context_get_root_node();
void dds_tt_free_context(dds_tt_context_t* context);

dds_tt_type_spec_t *dds_tt_new_base_type(dds_tt_context_t *context, dds_tt_node_flags_t flags);
dds_tt_type_spec_t *dds_tt_new_sequence_type(dds_tt_context_t *context, dds_tt_type_spec_t *element_type, dds_tt_literal_t size);
dds_tt_type_spec_t *dds_tt_new_sequence_type_unbound(dds_tt_context_t *context, dds_tt_type_spec_t *base);
dds_tt_type_spec_t *dds_tt_new_string_type(dds_tt_context_t *context, dds_tt_literal_t size);
dds_tt_type_spec_t *dds_tt_new_string_type_unbound(dds_tt_context_t *context);
dds_tt_type_spec_t *dds_tt_new_wide_string_type(dds_tt_context_t *context, dds_tt_literal_t size);
dds_tt_type_spec_t *dds_tt_new_wide_string_type_unbound(dds_tt_context_t *context);
dds_tt_type_spec_t *dds_tt_new_fixed_type(dds_tt_context_t *context, dds_tt_literal_t digits, dds_tt_literal_t fraction_digits);
dds_tt_type_spec_t *dds_tt_new_map_type(dds_tt_context_t *context, dds_tt_type_spec_t *key_type, dds_tt_type_spec_t *value_type, dds_tt_literal_t size);
dds_tt_type_spec_t *dds_tt_new_map_type_unbound(dds_tt_context_t *context, dds_tt_type_spec_t *key_type, dds_tt_type_spec_t *value_type);
dds_tt_scoped_name_t *dds_tt_new_scoped_name(dds_tt_context_t *context, dds_tt_scoped_name_t* prev, bool top, dds_tt_identifier_t name);
dds_tt_node_flags_t dds_tt_get_base_type_of_scoped_name(dds_tt_context_t *context, dds_tt_scoped_name_t *scoped_name);
dds_tt_literal_t dds_tt_get_value_of_scoped_name(dds_tt_context_t *context, dds_tt_scoped_name_t *scoped_name);


void dds_tt_module_open(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_module_close(dds_tt_context_t *context);

void dds_tt_add_struct_forward(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_add_struct_open(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_add_struct_extension_open(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_scoped_name_t *scoped_name);
void dds_tt_add_struct_member(dds_tt_context_t *context, dds_tt_type_spec_t *type);
void dds_tt_struct_close(dds_tt_context_t *context);
void dds_tt_struct_empty_close(dds_tt_context_t *context);

void dds_tt_add_union_forward(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_add_union_open(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_node_flags_t base_type);
void dds_tt_add_union_case_label(dds_tt_context_t *context, dds_tt_literal_t value);
void dds_tt_add_union_case_default(dds_tt_context_t *context);
void dds_tt_add_union_element(dds_tt_context_t *context, dds_tt_type_spec_t *type);
void dds_tt_union_close(dds_tt_context_t *context);

void dds_tt_add_typedef_open(dds_tt_context_t *context);
void dds_tt_typedef_set_type(dds_tt_context_t *context, dds_tt_type_spec_t *type);
void dds_tt_typedef_close(dds_tt_context_t *context);

void dds_tt_add_declarator(dds_tt_context_t *context, dds_tt_identifier_t name);

void dds_tt_add_const_def(dds_tt_context_t *context, dds_tt_type_spec_t *base_type, dds_tt_identifier_t name, dds_tt_literal_t value);

void dds_tt_add_enum_open(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_add_enum_enumerator(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_enum_close(dds_tt_context_t *context);

void dds_tt_add_array_open(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_add_array_size(dds_tt_context_t *context, dds_tt_literal_t value);
void dds_tt_array_close(dds_tt_context_t* context);

void dds_tt_add_native(dds_tt_context_t* context, dds_tt_identifier_t name);

void dds_tt_add_const_definition(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_literal_t value);

void dds_tt_add_bitset_open(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_type_spec_t *opt_type);
void dds_tt_add_bitset_field(dds_tt_context_t *context, dds_tt_literal_t index);
void dds_tt_add_bitset_field_to(dds_tt_context_t *context, dds_tt_literal_t index, dds_tt_node_flags_t dest_type);
void dds_tt_add_bitset_ident(dds_tt_context_t *context, dds_tt_identifier_t ident);
void dds_tt_bitset_close(dds_tt_context_t *context);

void dds_tt_add_bitmask_open(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_add_bitmask_value(dds_tt_context_t *context, dds_tt_identifier_t value);
void dds_tt_bitmask_close(dds_tt_context_t *context);

void dds_tt_add_annotation_open(dds_tt_context_t *context, dds_tt_identifier_t name);
void dds_tt_add_annotation_member_open(dds_tt_context_t *context, dds_tt_type_spec_t *base_type, dds_tt_identifier_t name);
void dds_tt_annotation_member_set_default(dds_tt_context_t *context, dds_tt_literal_t value);
void dds_tt_annotation_member_close(dds_tt_context_t *context);
void dds_tt_annotation_close(dds_tt_context_t *context);

void dds_tt_add_annotation_appl_open(dds_tt_context_t *context, dds_tt_scoped_name_t *scoped_name);
void dds_tt_add_annotation_appl_expr(dds_tt_context_t *context, dds_tt_literal_t value);
void dds_tt_add_annotation_appl_param(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_literal_t value);
void dds_tt_annotation_appl_close(dds_tt_context_t *context);

#endif /* DDS_TT_CREATE_H */
