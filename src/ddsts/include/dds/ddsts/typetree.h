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

#define DDSTS_TYPE(X)                  (1L<<(X))

#define DDSTS_BASIC_TYPES              ((1L<<(16+1)-1L)
#define DDSTS_SHORT                    DDSTS_TYPE(0)
#define DDSTS_LONG                     DDSTS_TYPE(1)
#define DDSTS_LONGLONG                 DDSTS_TYPE(2)
#define DDSTS_USHORT                   DDSTS_TYPE(3)
#define DDSTS_ULONG                    DDSTS_TYPE(4)
#define DDSTS_ULONGLONG                DDSTS_TYPE(5)
#define DDSTS_CHAR                     DDSTS_TYPE(6)
#define DDSTS_WIDE_CHAR                DDSTS_TYPE(7)
#define DDSTS_BOOLEAN                  DDSTS_TYPE(8)
#define DDSTS_OCTET                    DDSTS_TYPE(9)
#define DDSTS_INT8                     DDSTS_TYPE(10)
#define DDSTS_UINT8                    DDSTS_TYPE(11)
#define DDSTS_FLOAT                    DDSTS_TYPE(12)
#define DDSTS_DOUBLE                   DDSTS_TYPE(13)
#define DDSTS_LONGDOUBLE               DDSTS_TYPE(14)
#define DDSTS_FIXED_PT_CONST           DDSTS_TYPE(15)
#define DDSTS_ANY                      DDSTS_TYPE(16)

#define DDSTS_SEQUENCE                 DDSTS_TYPE(17)
#define DDSTS_ARRAY                    DDSTS_TYPE(18)
#define DDSTS_STRING                   DDSTS_TYPE(19)
#define DDSTS_WIDE_STRING              DDSTS_TYPE(20)
#define DDSTS_FIXED_PT                 DDSTS_TYPE(21)
#define DDSTS_MAP                      DDSTS_TYPE(22)

#define DDSTS_DEFINITIONS              (((1L<<(26-23+1))-1L)<<23)
#define DDSTS_MODULE                   DDSTS_TYPE(23)
#define DDSTS_FORWARD_STRUCT           DDSTS_TYPE(24)
#define DDSTS_STRUCT                   DDSTS_TYPE(25)
#define DDSTS_DECLARATION              DDSTS_TYPE(26)

#define DDSTS_TYPE_SPECS               ((1L<<(26+1)-1L)
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

typedef uint32_t ddsts_flags_t;

typedef struct {
  ddsts_flags_t flags; /* flags defining the kind of the literal */
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


/* Type specification
 *
 * The union ddsts_type_t is used to contain all possible type specifications,
 * where the struct ddsts_typespec_t serves as a basis for these type specifications.
 */

typedef union ddsts_type ddsts_type_t;

DDSTS_EXPORT void ddsts_free_type(ddsts_type_t *type);

typedef struct ddsts_typespec ddsts_typespec_t;
struct ddsts_typespec {
  ddsts_flags_t flags; /* flags defining the kind of the node */
  ddsts_identifier_t name;
  ddsts_type_t *parent;     /* pointer to the parent node */
  ddsts_type_t *next;       /* pointer to the next sibling */
  /* Maybe also needs information about the file and line where the node has
   * been parsed from. This is maybe needed to determine for which parts
   * of the preprocessed output code needs to be generated.
   */
  void (*free_func)(ddsts_type_t*);
};

/* Base type specification (base_type_spec) */
typedef struct {
  ddsts_typespec_t typespec;
} ddsts_base_type_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_base_type(ddsts_flags_t flags);

/* Sequence type (sequence_type) */
typedef struct {
  ddsts_typespec_t typespec;
  ddsts_type_t *element_type;
  unsigned long long max;
} ddsts_sequence_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_sequence(ddsts_type_t* element_type, unsigned long long max);

/* Array type */
typedef struct {
  ddsts_typespec_t typespec;
  ddsts_type_t *element_type;
  unsigned long long size;
} ddsts_array_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_array(ddsts_type_t* element_type, unsigned long long max);

/* (Wide) string type (string_type, wide_string_type) */
typedef struct {
  ddsts_typespec_t typespec;
  unsigned long long max;
} ddsts_string_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_string(ddsts_flags_t flags, unsigned long long max);

/* Fixed point type (fixed_pt_type) */
typedef struct {
  ddsts_typespec_t typespec;
  unsigned long long digits;
  unsigned long long fraction_digits;
} ddsts_fixed_pt_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_fixed_pt(unsigned long long digits, unsigned long long fraction_digits);

/* Map type (map_type) */
typedef struct {
  ddsts_typespec_t typespec;
  ddsts_type_t *key_type;
  ddsts_type_t *value_type;
  unsigned long long max;
} ddsts_map_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_map(ddsts_type_t *key_type, ddsts_type_t *value_type, unsigned long long max);

/* Module declaration (module_dcl)
 * (Probably needs extra member (and type) for definitions that are introduced
 * by non-top scoped names when checking the scope rules.)
 */
typedef struct ddsts_module ddsts_module_t;
struct ddsts_module {
  ddsts_typespec_t type;
  ddsts_type_t *members;
  ddsts_module_t *previous; /* to previous open of this module, if present */
};

DDSTS_EXPORT ddsts_type_t *ddsts_create_module(ddsts_identifier_t name);

/* Forward declaration */
typedef struct {
  ddsts_typespec_t type;
  ddsts_type_t *definition; /* reference to the actual definition */
} ddsts_forward_t;

/* Struct forward declaration (struct_forward_dcl)
 * ddsts_forward_t (no extra members)
 */

DDSTS_EXPORT ddsts_type_t *ddsts_create_struct_forward_dcl(ddsts_identifier_t name);

/* Struct declaration (struct_def)
 */
typedef struct {
  ddsts_typespec_t type;
  ddsts_type_t *super; /* used for extended struct type definition */
  ddsts_type_t *members;
} ddsts_struct_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_struct();

/* Declaration
 */
typedef struct {
  ddsts_typespec_t type;
  ddsts_type_t *decl_type;
} ddsts_declaration_t;

DDSTS_EXPORT ddsts_type_t *ddsts_create_declaration(ddsts_identifier_t name, ddsts_type_t *decl_type);


/* The union of all type specs */
union ddsts_type {
  ddsts_typespec_t type;
  ddsts_base_type_t base_type;
  ddsts_sequence_t sequence;
  ddsts_array_t array;
  ddsts_string_t string;
  ddsts_fixed_pt_t fixed_pt;
  ddsts_map_t map;
  ddsts_module_t module;
  ddsts_forward_t forward;
  ddsts_struct_t struct_def;
  ddsts_declaration_t declaration;
};

#endif /* DDS_TYPE_TYPETREE_H */
