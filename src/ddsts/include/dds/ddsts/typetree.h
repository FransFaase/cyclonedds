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
#ifndef DDSTS_TYPETREE_H
#define DDSTS_TYPETREE_H

#include <stdbool.h>
#include <stdint.h>
#include "dds/ddsts/export.h"

/*
 * The bits used for the flags are:
 * 0-7: the specific base type
 * 8-13: the type spec
 * 14-15: the definitions
 * 16-: the other nodes
 */

#define DDSTS_TYPE(X)                  (1L<<(X))

#define DDSTS_BASIC_TYPES              ((1L<<18)-1L)
#define DDSTS_SHORT_TYPE               DDSTS_TYPE(0)
#define DDSTS_LONG_TYPE                DDSTS_TYPE(1)
#define DDSTS_LONG_LONG_TYPE           DDSTS_TYPE(2)
#define DDSTS_UNSIGNED_SHORT_TYPE      DDSTS_TYPE(3)
#define DDSTS_UNSIGNED_LONG_TYPE       DDSTS_TYPE(4)
#define DDSTS_UNSIGNED_LONG_LONG_TYPE  DDSTS_TYPE(5)
#define DDSTS_CHAR_TYPE                DDSTS_TYPE(6)
#define DDSTS_WIDE_CHAR_TYPE           DDSTS_TYPE(7)
#define DDSTS_BOOLEAN_TYPE             DDSTS_TYPE(8)
#define DDSTS_OCTET_TYPE               DDSTS_TYPE(9)
#define DDSTS_INT8_TYPE                DDSTS_TYPE(10)
#define DDSTS_UINT8_TYPE               DDSTS_TYPE(11)
#define DDSTS_FLOAT_TYPE               DDSTS_TYPE(12)
#define DDSTS_DOUBLE_TYPE              DDSTS_TYPE(13)
#define DDSTS_LONG_DOUBLE_TYPE         DDSTS_TYPE(14)
#define DDSTS_FIXED_PT_CONST_TYPE      DDSTS_TYPE(15)
#define DDSTS_ANY_TYPE                 DDSTS_TYPE(16)
#define DDSTS_SEQUENCE                 DDSTS_TYPE(17)

#define DDSTS_STRING                   DDSTS_TYPE(18)
#define DDSTS_WIDE_STRING              DDSTS_TYPE(19)
#define DDSTS_FIXED_PT                 DDSTS_TYPE(20)
#define DDSTS_MAP                      DDSTS_TYPE(21)

#define DDSTS_DEFINITIONS              (((1L<<4)-1L)<<22)
#define DDSTS_MODULE                   DDSTS_TYPE(22)
#define DDSTS_FORWARD_STRUCT           DDSTS_TYPE(23)
#define DDSTS_STRUCT                   DDSTS_TYPE(24)
#define DDSTS_DECLARATOR               DDSTS_TYPE(25)

#define DDSTS_STRUCT_MEMBER            DDSTS_TYPE(26)
#define DDSTS_ARRAY_SIZE               DDSTS_TYPE(27)

#define DDSTS_TYPE_SPECS               ((1L<<26)-1L)
#define DDSTS_IS_TYPE_SPEC(X)          ((DDSTS_TYPE_SPECS & (X)) != 0)
#define DDSTS_IS_BASE_TYPE(X)          ((DDSTS_BASE_TYPES & (X)) != 0)
#define DDSTS_IS_DEFINITION(X)         ((DDSTS_DEFINITIONS & (X)) != 0)

/* Open issues:
 * - include file and line information in type definitions.
 */

typedef char *ddsts_identifier_t;

/* Literals
 *
 * Literals are values, either stated or calculated with an expression, that
 * appear in the IDL declaration. The literals only appear as members of
 * IDL elements, such as the constant definition and the case labels.
 */

typedef uint32_t ddsts_node_flags_t;

typedef struct {
  ddsts_node_flags_t flags; /* flags defining the kind of the literal */
  union {
    bool bln;
    char chr;
    unsigned long wchr;
    char *str;
    unsigned long long ullng;
    signed long long llng;
    long double ldbl;
  } value;
} ddsts_literal_t;


/* Generic node
 *
 * The generic node serves as a basis for all other elements of the IDL type
 * definitions
 */

typedef struct ddsts_node ddsts_node_t;
struct ddsts_node {
  ddsts_node_flags_t flags; /* flags defining the kind of the node */
  ddsts_node_t *parent;     /* pointer to the parent node */
  ddsts_node_t *children;   /* pointer to the first child */
  ddsts_node_t *next;       /* pointer to the next sibling */
  /* Maybe also needs information about the file and line where the node has
   * been parsed from. This is maybe needed to determine for which parts
   * of the preprocessed output code needs to be generated.
   */
  void (*free_func)(ddsts_node_t*);
};

DDSTS_EXPORT void ddsts_free_node(ddsts_node_t *node);


/* Type specifications */

/* Type specification (type_spec) */
typedef struct {
  ddsts_node_t node;
} ddsts_type_spec_t;

/* Base type specification (base_type_spec) */
typedef struct {
  ddsts_type_spec_t type_spec;
} ddsts_base_type_t;

DDSTS_EXPORT ddsts_base_type_t *ddsts_create_base_type(ddsts_node_flags_t flags);

/* Pointer to type spec */
typedef struct {
  ddsts_type_spec_t *type_spec;
  bool is_reference;
} ddsts_type_spec_ptr_t;

DDSTS_EXPORT void ddsts_type_spec_ptr_assign(ddsts_type_spec_ptr_t *type_spec_ptr, ddsts_type_spec_t *type_spec);
DDSTS_EXPORT void ddsts_type_spec_ptr_assign_reference(ddsts_type_spec_ptr_t *type_spec_ptr, ddsts_type_spec_t *type_spec);

/* Sequence type (sequence_type) */
typedef struct {
  ddsts_type_spec_t type_spec;
  ddsts_type_spec_ptr_t element_type;
  bool bounded;
  unsigned long long max;
} ddsts_sequence_t;

DDSTS_EXPORT ddsts_sequence_t *ddsts_create_sequence(ddsts_type_spec_ptr_t* element_type, bool bounded, unsigned long long max);

/* (Wide) string type (string_type, wide_string_type) */
typedef struct {
  ddsts_type_spec_t type_spec;
  bool bounded;
  unsigned long long max;
} ddsts_string_t;

DDSTS_EXPORT ddsts_string_t *ddsts_create_string(ddsts_node_flags_t flags, bool bounded, unsigned long long max);

/* Fixed point type (fixed_pt_type) */
typedef struct {
  ddsts_type_spec_t type_spec;
  unsigned long long digits;
  unsigned long long fraction_digits;
} ddsts_fixed_pt_t;

DDSTS_EXPORT ddsts_fixed_pt_t *ddsts_create_fixed_pt(unsigned long long digits, unsigned long long fraction_digits);

/* Map type (map_type) */
typedef struct {
  ddsts_type_spec_t type_spec;
  ddsts_type_spec_ptr_t key_type;
  ddsts_type_spec_ptr_t value_type;
  bool bounded;
  unsigned long long max;
} ddsts_map_t;

DDSTS_EXPORT ddsts_map_t *ddsts_create_map(ddsts_type_spec_ptr_t *key_type, ddsts_type_spec_ptr_t *value_type, bool bounded, unsigned long long max);


/* Type definitions */

/* Definition (definition)
 * (all definitions share a name)
 */
typedef struct {
  ddsts_type_spec_t type_spec;
  ddsts_identifier_t name;
} ddsts_definition_t;

/* Module declaration (module_dcl)
 * - defintions appear as children
 * (Probably needs extra member (and type) for definitions that are introduced
 * by non-top scoped names when checking the scope rules.)
 */
typedef struct ddsts_module ddsts_module_t;
struct ddsts_module {
  ddsts_definition_t def;
  ddsts_module_t *previous; /* to previous open of this module, if present */
};

DDSTS_EXPORT ddsts_module_t *ddsts_create_module(ddsts_identifier_t name);

/* Forward declaration */
typedef struct {
  ddsts_definition_t def;
  ddsts_definition_t *definition; /* reference to the actual definition */
} ddsts_forward_declaration_t;

/* Struct forward declaration (struct_forward_dcl)
 * ddsts_forward_declaration_t (no extra members)
 */

DDSTS_EXPORT ddsts_forward_declaration_t *ddsts_create_struct_forward_dcl(ddsts_identifier_t name);

/* Struct declaration (struct_def)
 * - members (and nested struct declarations) appear as children
 */
typedef struct {
  ddsts_definition_t def;
  ddsts_definition_t *super; /* used for extended struct type definition */
  bool part_of; /* true when used in other definition */
} ddsts_struct_t;

DDSTS_EXPORT ddsts_struct_t *ddsts_create_struct();

/* Struct member (members)
 * - declarators appear as chidren
 */
typedef struct {
  ddsts_node_t node;
  ddsts_type_spec_ptr_t member_type;
} ddsts_struct_member_t;

DDSTS_EXPORT ddsts_struct_member_t *ddsts_create_struct_member(ddsts_type_spec_ptr_t *member_type);

/* Declarator (declarator)
 * - fixed array sizes appear as children
 * use ddsts_definition_t (no extra members)
 */

DDSTS_EXPORT ddsts_definition_t *ddsts_create_declarator(ddsts_identifier_t name);

/* Fixed array size (fixed_array_size) */
typedef struct {
  ddsts_node_t node;
  unsigned long long size;
} ddsts_array_size_t;

DDSTS_EXPORT ddsts_array_size_t *ddsts_create_array_size(unsigned long long size);


#endif /* DDS_TYPE_TYPETREE_H */
