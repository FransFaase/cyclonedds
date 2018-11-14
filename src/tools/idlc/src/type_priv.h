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
#ifndef IDL_TYPE_H
#define IDL_TYPE_H

//#include <stdint.h>

typedef struct
{
  idl_type_t* base;
  bool bounded;
  unsigned long long max;
} idl_sequence_type_t;

typedef struct
{
  bool bounded;
  unsigned long long max;
} idl_string_type_t;

struct idl_bit_value_list {
  const char *name;
  idl_bit_value_list_t *next;
};

typedef enum {
  idl_type_basic,
  idl_type_sequence,
  idl_type_string,
  idl_type_enum,
  idl_type_enum_value
} idl_type_type_t;

struct idl_type {
  idl_type_type_t type;
  union {
    idl_basic_type_t basic;
    idl_sequence_type_t *sequence;
    idl_string_type_t *string;
  };
};

void idl_print_literal(FILE *f, idl_literal_t literal);

struct idl_scoped_name {
  bool top;
  const char *name;
  idl_scoped_name_t *next;
};

typedef struct idl_definition idl_definition_t;

typedef enum {
  idl_definition_module,
  idl_definition_struct_forward,
  idl_definition_struct,
  idl_definition_union_forward,
  idl_definition_union,
  idl_definition_declarator,
  idl_definition_enum,
  idl_definition_enum_value,
  idl_definition_const
} idl_definition_type_t;

// Module:

typedef struct {
  idl_definition_t *definitions;
} idl_module_definition_t;

// Struct:

typedef struct {
  bool defined;
} idl_struct_forward_t;

typedef struct idl_struct_member idl_struct_member_t;
struct idl_struct_member {
  idl_type_t *type;
  idl_definition_t *declarators;
  idl_struct_member_t *next;
};

typedef struct {
  idl_struct_member_t *members;
} idl_struct_t;

// Union:

typedef struct {
  bool defined;
} idl_union_forward_t;

typedef struct idl_union_case_label idl_union_case_label_t;
struct idl_union_case_label {
  bool is_default;
  idl_literal_t value;
  idl_union_case_label_t *next;
};

typedef struct idl_union_case idl_union_case_t;
struct idl_union_case {
  idl_union_case_label_t *labels;
  idl_type_t *element_type;
  idl_definition_t *declarators;
  idl_union_case_t *next;
};

typedef struct {
  idl_basic_type_t switch_type;
  idl_union_case_t *cases;
} idl_union_t;

// Enum:

typedef struct idl_enum_value_definition idl_enum_value_definition_t;

typedef struct {
  idl_enum_value_definition_t *values;
  int nr_values;
} idl_enum_definition_t;

struct idl_enum_value_definition {
  idl_definition_t *def;
  idl_enum_definition_t *enum_def;
  idl_enum_value_definition_t *next;
  int nr;
};

// Const:

typedef struct {
  idl_literal_t value;
} idl_const_definition_t;

// A definition:

struct idl_definition {
  const char *name;
  idl_definition_type_t type;
  union {
    idl_module_definition_t *module_def;
    idl_struct_forward_t *struct_forward_def;
    idl_struct_t *struct_def;
    idl_union_forward_t *union_forward_def;
    idl_union_t *union_def;
    idl_enum_definition_t *enum_def;
    idl_enum_value_definition_t *enum_value_def;
    idl_const_definition_t *const_def;
  };
  struct idl_definition *parent;
  struct idl_definition *next;
};

struct idl_context {
  bool ignore_yyerror;
  idl_definition_t *root_scope;
  idl_definition_t *cur_scope;
  idl_definition_t **ref_next_declarator;
  idl_union_case_t **ref_next_union_case;
  idl_union_case_label_t **ref_next_union_case_label;
};

#endif /* IDL_TYPE_H */
