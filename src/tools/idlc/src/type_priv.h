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
  idl_definition_enum,
  idl_definition_enum_value,
  idl_definition_const
} idl_definition_type_t;

typedef struct {
  idl_definition_t *definitions;
} idl_module_definition_t;

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

typedef struct {
  idl_literal_t value;
} idl_const_definition_t;

struct idl_definition {
  const char *name;
  idl_definition_type_t type;
  union {
    idl_module_definition_t *module_def;
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
};

#endif /* IDL_TYPE_H */
