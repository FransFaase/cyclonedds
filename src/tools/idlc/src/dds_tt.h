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
#ifndef DDS_TT_TYPE_H
#define DDS_TT_TYPE_H

#include <stdbool.h>
#include <stdint.h>

//#define DDS_TT_ULLONG (BASIC_TYPE | 12)
//#define DDS_TT_LITERAL (1<<5)
//flags = DDS_TT_LITERAL | DDS_TT_ULLONG;

#define DDS_TT_CATEGORY(C)              ((C)<<6)
#define DDS_TT_IS_CATEGORY(X,C)         (((15<<6)&(X))==DDS_TT_CATEGORY(C))
#define DDS_TT_BASIC_TYPE(X)            (DDS_TT_CATEGORY(1) | (X))
#define DDS_TT_SHORT_TYPE               DDS_TT_BASIC_TYPE(1)
#define DDS_TT_LONG_TYPE                DDS_TT_BASIC_TYPE(2)
#define DDS_TT_LONG_LONG_TYPE           DDS_TT_BASIC_TYPE(3)
#define DDS_TT_UNSIGNED_SHORT_TYPE      DDS_TT_BASIC_TYPE(4)
#define DDS_TT_UNSIGNED_LONG_TYPE       DDS_TT_BASIC_TYPE(5)
#define DDS_TT_UNSIGNED_LONG_LONG_TYPE  DDS_TT_BASIC_TYPE(6)
#define DDS_TT_CHAR_TYPE                DDS_TT_BASIC_TYPE(7)
#define DDS_TT_WIDE_CHAR_TYPE           DDS_TT_BASIC_TYPE(8)
#define DDS_TT_BOOLEAN_TYPE             DDS_TT_BASIC_TYPE(9)
#define DDS_TT_OCTET_TYPE               DDS_TT_BASIC_TYPE(10)
#define DDS_TT_INT8_TYPE                DDS_TT_BASIC_TYPE(11)
#define DDS_TT_UINT8_TYPE               DDS_TT_BASIC_TYPE(12)
#define DDS_TT_FLOAT_TYPE               DDS_TT_BASIC_TYPE(13)
#define DDS_TT_DOUBLE_TYPE              DDS_TT_BASIC_TYPE(14)
#define DDS_TT_LONG_DOUBLE_TYPE         DDS_TT_BASIC_TYPE(15)
#define DDS_TT_FIXED_PT_CONST_TYPE      DDS_TT_BASIC_TYPE(16)
#define DDS_TT_ANY_TYPE                 DDS_TT_BASIC_TYPE(17)
#define DDS_TT_TYPE_SPEC(X)             (DDS_TT_CATEGORY(2) | (X))
#define DDS_TT_SEQUENCE_TYPE            DDS_TT_TYPE_SPEC(1)
#define DDS_TT_STRING_TYPE              DDS_TT_TYPE_SPEC(2)
#define DDS_TT_WIDE_STRING_TYPE         DDS_TT_TYPE_SPEC(3)
#define DDS_TT_FIXED_PT_TYPE            DDS_TT_TYPE_SPEC(4)
#define DDS_TT_MAP_TYPE                 DDS_TT_TYPE_SPEC(5)
#define DDS_TT_DEFINITION(X)            (DDS_TT_CATEGORY(3) | (X))
#define DDS_TT_IS_DEFINITION(X)         DDS_TT_IS_CATEGORY(X,3)
#define DDS_TT_MODULE                   DDS_TT_DEFINITION(1)
#define DDS_TT_FORWARD_STRUCT           DDS_TT_DEFINITION(2)
#define DDS_TT_FORWARD_UNION            DDS_TT_DEFINITION(3)
#define DDS_TT_STRUCT                   DDS_TT_DEFINITION(4)
#define DDS_TT_UNION                    DDS_TT_DEFINITION(4)
#define DDS_TT_ENUM                     DDS_TT_DEFINITION(6)
#define DDS_TT_CONST_DEF                DDS_TT_DEFINITION(10)
#define DDS_TT_BITSET                   DDS_TT_DEFINITION(11)
#define DDS_TT_BITMASK                  DDS_TT_DEFINITION(12)
#define DDS_TT_NATIVE                   DDS_TT_DEFINITION(13)
#define DDS_TT_TYPE_DEF                 DDS_TT_DEFINITION(14)
#define DDS_TT_ANNOTATION_DEF           DDS_TT_DEFINITION(15)
#define DDS_TT_PART(X)                  (DDS_TT_CATEGORY(4) | (X))
#define DDS_TT_ANNOTATION_PARM          DDS_TT_PART(1)
#define DDS_TT_STRUCT_MEMBER            DDS_TT_PART(2)
#define DDS_TT_ARRAY_SIZE               DDS_TT_PART(3)
#define DDS_TT_UNION_CASE               DDS_TT_PART(4)
#define DDS_TT_UNION_CASE_LABEL         DDS_TT_PART(5)
#define DDS_TT_UNION_CASE_DEFAULT       DDS_TT_PART(6)
#define DDS_TT_ENUM_VALUE               DDS_TT_PART(7)
#define DDS_TT_BIT_FIELD                DDS_TT_PART(8)
#define DDS_TT_BIT_VALUE                DDS_TT_PART(9)
#define DDS_PRAGMA                      DDS_TT_CATEGORY(5)

// Open issues:
// - How to represent array sizes (as tree elements or as an array of 
//   sizes). 
// - support any pragma or just the keyvalue pragma with respect to
//   backwards compatibility.
// - include file and line information in type definitions.

typedef char *dds_tt_identifier_t;

// Generic node

// The generic node serves as a basis for all other elements of the IDL type
// definitions

typedef struct dds_tt_node dds_tt_node_t;
typedef uint32_t dds_tt_node_flags_t;
struct dds_tt_node {
  dds_tt_node_flags_t flags;            // flags defining the kind of the node
  dds_tt_node_t *parent;     // pointer to the parent node
  dds_tt_node_t *children;   // pointer to the first child
  dds_tt_node_t *next;       // pointer to the next sibling
  // Maybe also needs information about the file and line where the node has 
  // been parsed from. This is maybe needed to determine for which parts
  // of the preprocessed output code needs to be generated.
};


// Literals

// Literals are values, either stated or calculated with an expression, that
// appear in the IDL declaration

// Literals (literal)
typedef struct {
  dds_tt_node_t;
} dds_tt_literal_t;

// Integer literal (integer_literal)
typedef struct {
  dds_tt_literal_t;
  long long value;
} dds_tt_integer_literal_t;

// Floating point literal (floating_pt_literal)
typedef struct {
  dds_tt_literal_t;
  long double value;
} dds_tt_floating_point_literal_t;

// Character literal (character_literal)
typedef struct {
  dds_tt_literal_t;
  char value;
} dds_tt_char_literal_t;

// Wide character literal (character_literal)
typedef struct {
  dds_tt_literal_t;
  unsigned long value;
} dds_tt_wide_char_literal_t;

// Boolean literal (boolean_literal)
typedef struct {
  dds_tt_literal_t;
  bool value;
} dds_tt_boolean_literal_t;

// (Wide) String literal (string_literal, wide_string_literal)
typedef struct {
  dds_tt_literal_t;
  char *value;
} dds_tt_string_literal_t;


// Annotations

// Annotation can be defined and applied to any IDL element. (The annotation
// definitions are given under the definitions.)

typedef struct dds_tt_annotation_definition dds_tt_annotation_definition_t;

// Annotation application (annotation_appl)
// - parameters appear as children
typedef struct {
  dds_tt_node_t;
  dds_tt_annotation_definition_t *definition;
} dds_tt_annotation_appl_t;

// Annotation parameter (annotation_appl_param)
typedef struct {
  dds_tt_literal_t;
  const char* name; // NULL when parameter not mentioned
} dds_tt_annotation_parm_t;

// The annotated node serves as a basis for most remaining IDL elements.
typedef struct {
  dds_tt_node_t;
  dds_tt_annotation_appl_t *annotations;
} dds_tt_annotated_node_t;


// Type specifications

// Type specification (type_spec)
typedef struct {
  dds_tt_annotated_node_t;
} dds_tt_type_spec_t;

// Basic type specification (base_type_spec)
typedef struct {
  dds_tt_type_spec_t;
} dds_tt_base_type_t;

// Sequence type (sequence_type)
typedef struct {
  dds_tt_type_spec_t;
  dds_tt_type_spec_t* base;
  bool bounded;
  unsigned long long max;
} dds_tt_sequence_type_t;

// (Wide) string type (string_type, wide_string_type)
typedef struct {
  dds_tt_type_spec_t;
  bool bounded;
  unsigned long long max;
} dds_tt_string_type_t;

// Fixed point type (fixed_pt_type)
typedef struct {
  dds_tt_type_spec_t;
  unsigned long long digits;
  unsigned long long fractional_digits;
} dds_tt_fixed_pt_type_t;

// Map type (map_type)
typedef struct {
  dds_tt_type_spec_t;
  dds_tt_type_spec_t *key_type;
  dds_tt_type_spec_t *value_type;
  unsigned long long max;
} dds_tt_map_type_t;


// Type definitions

// Definition (definition)
// (all definitions share a name)
typedef struct {
  dds_tt_annotated_node_t;
  dds_tt_identifier_t name;
} dds_tt_definition_t;

// Module declaration (module_dcl)
// - defintions appear as children
// (Probably needs extra member (and type) for definitions that are introduced
// by non-top scoped names when checking the scope rules.)
typedef struct {
  dds_tt_definition_t;
} dds_tt_module_t;

// Const declaration (const_dcl)
typedef struct {
  dds_tt_definition_t;
  dds_tt_type_spec_t *value_type;
  dds_tt_literal_t *value;
} dds_tt_const_definition_t;

// Forward declaration
typedef struct {
  dds_tt_definition_t;
  dds_tt_definition_t *definition;
} dds_tt_forward_declaration_t;

// Struct forward declaration (struct_forward_dcl)
// (no extra members)

// Union forward declaration (union_forward_dcl)
// (no extra members)

// Struct declaration (struct_def)
// - members appear as children
typedef struct {
  dds_tt_definition_t;
  dds_tt_definition_t *super; // used for extended struct type definition
} dds_tt_struct_t;

// Struct member (members)
// - declarators appear as chidren
typedef struct {
  dds_tt_annotated_node_t;
  dds_tt_type_spec_t *member_type;
} dds_tt_struct_member_t;

// Declarator (declarator)
// - fixed array sizes appear as children
//   (As an alternative have: int nr_dim; int *dims;)
// (no extra members)

// Fixed array size (fixed_array_size)
typedef struct {
  dds_tt_node_t;
  unsigned long long size;
} dds_tt_array_size_t;

// Union declaration (union_def)
// - cases appear as children
typedef struct {
  dds_tt_definition_t;
  dds_tt_base_type_t *switch_type;
} dds_tt_union_t;

// Union declaration (case)
// - declarator appears as child
// - labels are literals (or default case)
typedef struct {
  dds_tt_annotated_node_t;
  dds_tt_node_t *labels;
} dds_tt_union_case_t;

// Enumeration

// Enum declaration (enum_decl)
// - values appear as children
// (no extra members)

// Enum values (enumerator)
// (no extra members)

// Bitset (bitset_dcl)
// - fields appear as children
typedef struct dds_tt_bitset_dcl dds_tt_bitset_dcl_t;
struct dds_tt_bitset_dcl {
  dds_tt_definition_t;
  dds_tt_bitset_dcl_t *super;
};

// Bit field (bitfield)
// - identifiers appear as children
typedef struct {
  dds_tt_annotated_node_t;
  unsigned long long size;
  dds_tt_base_type_t *base_type;
} dds_tt_bitfiels_t;

// Bitmask (bitmask_dcl)
// - bit values (identifiers) appear as children
// (no extra members)

// Bit value (bit_value)
// (no extra members)

// Native defintion (native_dcl)
// (no extra members)

// Typedef definiton (typedef_dcl)
// - declarators appear as children
typedef struct {
  dds_tt_definition_t;
  dds_tt_type_spec_t *type;
} dds_tt_typedef_t; 

// Annotation declaration (annotation_dcl)
// - annotation bodies appear as children
struct dds_tt_annotation_definition {
  dds_tt_definition_t;
};

// Annotation member (annotation_member)
typedef struct {
  dds_tt_definition_t;
  dds_tt_type_spec_t *type;
  dds_tt_literal_t *default_value;
} dds_tt_annotation_member_t;

 
// Pragma (pragma) 
typedef struct {
  // Should we have just a dedicated pragma for keylist or a generic pragma?
} dds_tt_pragma_t;





#endif /* DDS_TT_TYPE_H */
