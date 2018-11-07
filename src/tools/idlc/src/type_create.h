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
#ifndef IDL_TYPE_CREATE_H
#define IDL_TYPE_CREATE_H

#include <stdbool.h>
#include "type.h"

typedef struct idl_type idl_type_t;

typedef struct idl_bit_value_list idl_bit_value_list_t;

typedef struct {
  idl_literal_type_t type;
  union {
    bool bln;
    char chr;
    char *str;
    long long llng;
    long double ldbl;
  } value;
} idl_literal_t;

typedef struct idl_scoped_name idl_scoped_name_t;

typedef char *idl_identifier_t;


void idl_eval_unary_oper(idl_operator_type_t operator_type, idl_literal_t operand, idl_literal_t *result);
void idl_eval_binary_oper(idl_operator_type_t operator_type, idl_literal_t lhs, idl_literal_t rhs, idl_literal_t *result);

typedef struct idl_context idl_context_t;
idl_context_t* idl_create_context();
void idl_context_set_ignore_yyerror(idl_context_t* context, bool ignore_yyerror);
bool idl_context_get_ignore_yyerror(idl_context_t* context);
void idl_free_context(idl_context_t* context);

idl_type_t *idl_new_basic_type(idl_context_t *context, idl_basic_type_t basic_type);
idl_type_t *idl_new_sequence_type(idl_context_t *context, idl_type_t *base, idl_literal_t size);
idl_type_t *idl_new_sequence_type_unbound(idl_context_t *context, idl_type_t *base);
idl_type_t *idl_new_string_type(idl_context_t *context, idl_literal_t size);
idl_type_t *idl_new_string_type_unbound(idl_context_t *context);
idl_type_t *idl_new_fixed_type(idl_context_t *context, idl_literal_t digits, idl_literal_t precision);
idl_type_t *idl_new_map_type(idl_context_t *context, idl_type_t *key_type, idl_type_t *value_type, idl_literal_t size);
idl_type_t *idl_new_map_type_unbound(idl_context_t *context, idl_type_t *key_type, idl_type_t *value_type);
idl_scoped_name_t *idl_new_scoped_name(idl_context_t *context, idl_scoped_name_t* prev, bool top, idl_identifier_t name);
idl_basic_type_t idl_get_basic_type_of_scoped_name(idl_context_t *context, idl_scoped_name_t *scoped_name);
idl_bit_value_list_t *idl_new_bit_value_list(idl_context_t *context, idl_identifier_t name, idl_bit_value_list_t *next);


void idl_module_open(idl_context_t *context, idl_identifier_t name);
void idl_module_close(idl_context_t *context);

void idl_add_struct_forward(idl_context_t *context, idl_identifier_t name);
void idl_add_struct_open(idl_context_t *context, idl_identifier_t name);
void idl_add_struct_extension_open(idl_context_t *context, idl_identifier_t name, idl_scoped_name_t *scoped_name);
void idl_add_struct_member(idl_context_t *context, idl_type_t *type);
void idl_struct_close(idl_context_t *context);
void idl_struct_empty_close(idl_context_t *context);

void idl_add_union_forward(idl_context_t *context, idl_identifier_t name);
void idl_add_union_open(idl_context_t *context, idl_identifier_t name, idl_basic_type_t basic_type);
void idl_add_union_case_label(idl_context_t *context, idl_literal_t value);
void idl_add_union_case_default(idl_context_t *context);
void idl_add_union_element(idl_context_t *context, idl_type_t *type);
void idl_union_close(idl_context_t *context);

void idl_add_typedef_open(idl_context_t *context);
void idl_typedef_set_type(idl_context_t *context, idl_type_t *type);
void idl_typedef_close(idl_context_t *context);

void idl_add_declarator(idl_context_t *context, idl_identifier_t name);

void idl_add_const_def(idl_context_t *context, idl_basic_type_t basic_type, idl_identifier_t name, idl_literal_t value);

void idl_add_enum_open(idl_context_t *context, idl_identifier_t name);
void idl_add_enum_enumerator(idl_context_t *context, idl_identifier_t name);
void idl_enum_close(idl_context_t *context);

void idl_add_array_open(idl_context_t *context, idl_identifier_t name);
void idl_add_array_size(idl_context_t *context, idl_literal_t value);
void idl_array_close(idl_context_t* context);

void idl_add_native(idl_context_t* context, idl_identifier_t name);

void idl_add_const_definition(idl_context_t *context, idl_identifier_t name, idl_literal_t value);

void idl_add_bitset_open(idl_context_t *context, idl_identifier_t name, idl_type_t *opt_type);
void idl_add_bitset_field(idl_context_t *context, idl_literal_t index);
void idl_add_bitset_field_to(idl_context_t *context, idl_literal_t index, idl_basic_type_t dest_type);
void idl_add_bitset_ident(idl_context_t *context, idl_identifier_t ident);
void idl_bitset_close(idl_context_t *context);

void idl_add_bitmask(idl_context_t *context, idl_identifier_t name, idl_bit_value_list_t *bit_value_list);

void idl_add_annotation_open(idl_context_t *context, idl_identifier_t name);
void idl_add_annotation_member_open(idl_context_t *context, idl_basic_type_t basic_type, idl_identifier_t name);
void idl_annotation_member_set_default(idl_context_t *context, idl_literal_t value);
void idl_annotation_member_close(idl_context_t *context);
void idl_annotation_close(idl_context_t *context);

void idl_add_annotation_appl_open(idl_context_t *context, idl_scoped_name_t *scoped_name);
void idl_add_annotation_appl_expr(idl_context_t *context, idl_literal_t value);
void idl_add_annotation_appl_param(idl_context_t *context, idl_identifier_t name, idl_literal_t value);
void idl_annotation_appl_close(idl_context_t *context);

#endif /* IDL_TYPE_CREATE_H */
