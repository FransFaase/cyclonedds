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
#ifndef DDS_TS_TYPE_H
#define DDS_TS_TYPE_H

#include <stdbool.h>
#include <stdint.h>

/*
 * The bits used for the flags are:
 * 0-7: the specific base type
 * 8-13: the type spec
 * 14-15: the definitions
 * 16-: the other nodes
 */

#define DDS_TS_TYPE_SPEC(X)             (((X)&7)<<8)
#define DDS_TS_IS_TYPE_SPEC(T)          (((7<<8)&(T)) != 0)
#define DDS_TS_BASE_TYPE(X)             (DDS_TS_TYPE_SPEC(1) | (X))
#define DDS_TS_IS_BASE_TYPE(T)          (((7<<8)&(T)) == 1)
#define DDS_TS_GET_BASE_TYPE(T)         (((7<<8)|31)&(T))
#define DDS_TS_SHORT_TYPE               DDS_TS_BASE_TYPE(1)
#define DDS_TS_LONG_TYPE                DDS_TS_BASE_TYPE(2)
#define DDS_TS_LONG_LONG_TYPE           DDS_TS_BASE_TYPE(3)
#define DDS_TS_UNSIGNED_SHORT_TYPE      DDS_TS_BASE_TYPE(4)
#define DDS_TS_UNSIGNED_LONG_TYPE       DDS_TS_BASE_TYPE(5)
#define DDS_TS_UNSIGNED_LONG_LONG_TYPE  DDS_TS_BASE_TYPE(6)
#define DDS_TS_CHAR_TYPE                DDS_TS_BASE_TYPE(7)
#define DDS_TS_WIDE_CHAR_TYPE           DDS_TS_BASE_TYPE(8)
#define DDS_TS_BOOLEAN_TYPE             DDS_TS_BASE_TYPE(9)
#define DDS_TS_OCTET_TYPE               DDS_TS_BASE_TYPE(10)
#define DDS_TS_INT8_TYPE                DDS_TS_BASE_TYPE(11)
#define DDS_TS_UINT8_TYPE               DDS_TS_BASE_TYPE(12)
#define DDS_TS_FLOAT_TYPE               DDS_TS_BASE_TYPE(13)
#define DDS_TS_DOUBLE_TYPE              DDS_TS_BASE_TYPE(14)
#define DDS_TS_LONG_DOUBLE_TYPE         DDS_TS_BASE_TYPE(15)
#define DDS_TS_FIXED_PT_CONST_TYPE      DDS_TS_BASE_TYPE(16)
#define DDS_TS_ANY_TYPE                 DDS_TS_BASE_TYPE(17)
#define DDS_TS_SEQUENCE_TYPE            DDS_TS_TYPE_SPEC(2)
#define DDS_TS_STRING_TYPE              DDS_TS_TYPE_SPEC(3)
#define DDS_TS_WIDE_STRING_TYPE         DDS_TS_TYPE_SPEC(4)
#define DDS_TS_FIXED_PT_TYPE            DDS_TS_TYPE_SPEC(5)
#define DDS_TS_MAP_TYPE                 DDS_TS_TYPE_SPEC(6)
#define DDS_TS_DEFINITION(X)            (((X)&15)<<12)
#define DDS_TS_IS_DEFINITION(T)         (((15<<12)&(T)) != 0)
#define DDS_TS_MODULE                   DDS_TS_DEFINITION(1)
#define DDS_TS_FORWARD_STRUCT           DDS_TS_DEFINITION(2)
#define DDS_TS_FORWARD_UNION            DDS_TS_DEFINITION(3)
#define DDS_TS_STRUCT                   DDS_TS_DEFINITION(4)
#define DDS_TS_UNION                    DDS_TS_DEFINITION(5)
#define DDS_TS_UNION_CASE               DDS_TS_DEFINITION(6)
#define DDS_TS_DECLARATOR               DDS_TS_DEFINITION(7)
#define DDS_TS_ENUM                     DDS_TS_DEFINITION(8)
#define DDS_TS_ENUMERATOR               DDS_TS_DEFINITION(9)
#define DDS_TS_CONST_DEF                DDS_TS_DEFINITION(10)
#define DDS_TS_BITSET                   DDS_TS_DEFINITION(11)
#define DDS_TS_BITMASK                  DDS_TS_DEFINITION(12)
#define DDS_TS_NATIVE                   DDS_TS_DEFINITION(13)
#define DDS_TS_TYPE_DEF                 DDS_TS_DEFINITION(14)
#define DDS_TS_ANNOTATION_DEF           DDS_TS_DEFINITION(15)
#define DDS_TS_OTHER(X)                 ((X)<<16)
#define DDS_TS_ANNOTATION_PARM          DDS_TS_OTHER(1)
#define DDS_TS_STRUCT_MEMBER            DDS_TS_OTHER(2)
#define DDS_TS_ARRAY_SIZE               DDS_TS_OTHER(3)
#define DDS_TS_UNION_CASE_LABEL         DDS_TS_OTHER(4)
#define DDS_TS_UNION_CASE_DEFAULT       DDS_TS_OTHER(5)
#define DDS_TS_BIT_FIELD                DDS_TS_OTHER(6)
#define DDS_TS_BIT_VALUE                DDS_TS_OTHER(7)
#define DDS_PRAGMA                      DDS_TS_OTHER(8)

/* Open issues:
 * - How to represent array sizes (as tree elements or as an array of
 *   sizes).
 * - support any pragma or just the keyvalue pragma with respect to
 *   backwards compatibility.
 * - include file and line information in type definitions.
 */

typedef char *dds_ts_identifier_t;

/* Literals
 *
 * Literals are values, either stated or calculated with an expression, that
 * appear in the IDL declaration. The literals only appear as members of
 * IDL elements, such as the constant definition and the case labels.
 */

typedef uint32_t dds_ts_node_flags_t;

typedef struct {
  dds_ts_node_flags_t flags; /* flags defining the kind of the literal */
  union {
    bool bln;
    char chr;
    unsigned long wchr;
    char *str;
    unsigned long long ullng;
    signed long long llng;
    long double ldbl;
  };
} dds_ts_literal_t;


/* Generic node
 *
 * The generic node serves as a basis for all other elements of the IDL type
 * definitions
 */

typedef struct dds_ts_node dds_ts_node_t;
struct dds_ts_node {
  dds_ts_node_flags_t flags; /* flags defining the kind of the node */
  dds_ts_node_t *parent;     /* pointer to the parent node */
  dds_ts_node_t *children;   /* pointer to the first child */
  dds_ts_node_t *next;       /* pointer to the next sibling */
  /* Maybe also needs information about the file and line where the node has
   * been parsed from. This is maybe needed to determine for which parts
   * of the preprocessed output code needs to be generated.
   */
};


/* Annotations
 *
 * Annotation can be defined and applied to any IDL element. (The annotation
 * definitions are given under the definitions.)
 */

typedef struct dds_ts_annotation_definition dds_ts_annotation_definition_t;

/* Annotation application (annotation_appl)
 * - parameters appear as children
 */
typedef struct {
  dds_ts_node_t;
  const char* name;
  dds_ts_annotation_definition_t *definition; /* NULL for build-in annotations */
} dds_ts_annotation_appl_t;

/* Annotation parameter (annotation_appl_param) */
typedef struct {
  dds_ts_literal_t;
  const char* name; /* NULL when parameter not mentioned */
} dds_ts_annotation_parm_t;

/* The annotated node serves as a basis for most remaining IDL elements. */
typedef struct {
  dds_ts_node_t;
  dds_ts_annotation_appl_t *annotations;
} dds_ts_annotated_node_t;


/* Type specifications */

/* Type specification (type_spec) */
typedef struct {
  dds_ts_annotated_node_t;
} dds_ts_type_spec_t;

/* Base type specification (base_type_spec) */
typedef struct {
  dds_ts_type_spec_t;
} dds_ts_base_type_t;

/* Sequence type (sequence_type) */
typedef struct {
  dds_ts_type_spec_t;
  dds_ts_type_spec_t* element_type;
  bool bounded;
  unsigned long long max;
} dds_ts_sequence_type_t;

/* (Wide) string type (string_type, wide_string_type) */
typedef struct {
  dds_ts_type_spec_t;
  bool bounded;
  unsigned long long max;
} dds_ts_string_type_t;

/* Fixed point type (fixed_pt_type) */
typedef struct {
  dds_ts_type_spec_t;
  unsigned long long digits;
  unsigned long long fraction_digits;
} dds_ts_fixed_pt_type_t;

/* Map type (map_type) */
typedef struct {
  dds_ts_type_spec_t;
  dds_ts_type_spec_t *key_type;
  dds_ts_type_spec_t *value_type;
  bool bounded;
  unsigned long long max;
} dds_ts_map_type_t;


/* Type definitions */

/* Definition (definition)
 * (all definitions share a name)
 */
typedef struct {
  dds_ts_annotated_node_t;
  dds_ts_identifier_t name;
} dds_ts_definition_t;

/* Module declaration (module_dcl)
 * - defintions appear as children
 * (Probably needs extra member (and type) for definitions that are introduced
 * by non-top scoped names when checking the scope rules.)
 */
typedef struct {
  dds_ts_definition_t;
} dds_ts_module_t;

/* Const declaration (const_dcl) */
typedef struct {
  dds_ts_definition_t;
  dds_ts_type_spec_t *value_type;
  dds_ts_literal_t *value;
} dds_ts_const_definition_t;

/* Forward declaration */
typedef struct {
  dds_ts_definition_t;
  dds_ts_definition_t *definition; /* reference to the actual definition */
} dds_ts_forward_declaration_t;

/* Struct forward declaration (struct_forward_dcl)
 * (no extra members)
 */

/* Union forward declaration (union_forward_dcl)
 * (no extra members)
 */

/* Struct declaration (struct_def)
 * - members appear as children
 */
typedef struct {
  dds_ts_definition_t;
  dds_ts_definition_t *super; /* used for extended struct type definition */
} dds_ts_struct_t;

/* Struct member (members)
 * - declarators appear as chidren
 */
typedef struct {
  dds_ts_annotated_node_t;
  dds_ts_type_spec_t *member_type;
} dds_ts_struct_member_t;

/* Declarator (declarator)
 * - fixed array sizes appear as children
 *   (As an alternative have: int nr_dim; int *dims;)
 * (no extra members)
 */

/* Fixed array size (fixed_array_size) */
typedef struct {
  dds_ts_node_t;
  unsigned long long size;
} dds_ts_array_size_t;

/* Union declaration (union_def)
 * - cases appear as children
 */
typedef struct {
  dds_ts_definition_t;
  dds_ts_node_flags_t switch_type;
} dds_ts_union_t;

/* Union cases (case)
 * - labels (including default) appear as children
 */
typedef struct {
  dds_ts_definition_t; /* name of definition is the branch name */
  dds_ts_type_spec_t *branch_type;
} dds_ts_union_case_t;

/* Union case labels (case_label) */
typedef struct {
  dds_ts_annotated_node_t;
  dds_ts_literal_t value;
} dds_ts_union_case_label_t;

/* Enumeration */

/* Enum declaration (enum_decl)
 * - values appear as children
 * (no extra members)
 */

/* Enum values (enumerator)
 * (no extra members)
 */

/* Bitset (bitset_dcl)
 * - fields appear as children
 */
typedef struct dds_ts_bitset_dcl dds_ts_bitset_dcl_t;
struct dds_ts_bitset_dcl {
  dds_ts_definition_t;
  dds_ts_bitset_dcl_t *super;
};

/* Bit field (bitfield)
 * - identifiers appear as children
 */
typedef struct {
  dds_ts_annotated_node_t;
  unsigned long long size;
  dds_ts_base_type_t *base_type;
} dds_ts_bitfiels_t;

/* Bitmask (bitmask_dcl)
 * - bit values (identifiers) appear as children
 * (no extra members)
 */

/* Bit value (bit_value)
 * (no extra members)
 */

/* Native defintion (native_dcl)
 * (no extra members)
 */

/* Typedef definiton (typedef_dcl)
 * - declarators appear as children
 */
typedef struct {
  dds_ts_definition_t;
  dds_ts_type_spec_t *type;
} dds_ts_typedef_t;

/* Annotation declaration (annotation_dcl)
 * - annotation bodies appear as children
 */
struct dds_ts_annotation_definition {
  dds_ts_definition_t;
};

/* Annotation member (annotation_member) */
typedef struct {
  dds_ts_definition_t;
  dds_ts_type_spec_t *type;
  dds_ts_literal_t *default_value;
} dds_ts_annotation_member_t;


/* Pragma (pragma) */
typedef struct {
  /* Should we have just a dedicated pragma for keylist or a generic pragma? */
  int dummy; /* temporarily add a dummy member to prevent VS compile error */
} dds_ts_pragma_t;





#endif /* dds_ts_TYPE_H */
