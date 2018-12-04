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
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "dds_tt.h"
#include "tt_create.h"

#include "os/os.h"

struct dds_ts_context {
  bool ignore_yyerror;
  dds_ts_node_t *root_node;
  dds_ts_node_t *cur_node;
  dds_ts_node_t *parent_for_declarator;
  dds_ts_union_case_t *cur_union_case;
};

/* (not used at this moment)
void dds_ts_print_literal(FILE* f, const dds_ts_literal_t *literal)
{
  switch (literal.flags) {
    case DDS_TS_LONG_LONG_TYPE:
      fprintf(f, "integer %lld", literal.llng);
      break;
    case DDS_TS_STRING_TYPE:
      fprintf(f, "string '%s'", literal.str);
      break;
    case DDS_TS_WIDE_STRING_TYPE:
      fprintf(f, "wstring '%s'", literal.str);
      break;
    case DDS_TS_CHAR_TYPE:
      fprintf(f, "char '%c'", literal.chr);
      break;
    case DDS_TS_WIDE_CHAR_TYPE:
      fprintf(f, "wchar u%4lX", literal.wchr);
      break;
    case DDS_TS_DOUBLE_TYPE:
      fprintf(f, "fixed %Lf", literal.ldbl);
      break;
    case DDS_TS_BOOLEAN_TYPE:
      fprintf(f, "boolean: %s", literal.bln ? "true" : "false");
      break;
    default:
      fprintf(f, "illegal-type");
      break;
  }
}
*/

/* (not used at this moment)
static int dds_ts_compare_literal(const dds_ts_literal_t *lhs, const dds_ts_literal_t *rhs)
{
  if (lhs.flags < rhs.flags) {
    return -1;
  }
  if (lhs.flags > rhs.flags) {
    return +1;
  }
  switch (lhs.flags) {
    case DDS_TS_LONG_LONG_TYPE:
      return lhs.llng < rhs.llng ? -1 : lhs.llng > rhs.llng ? +1 : 0;
      break;
    case DDS_TS_STRING_TYPE:
      return strcmp(lhs.str, rhs.str);
      break;
    case DDS_TS_WIDE_STRING_TYPE:
      assert(false);
      break;
    case DDS_TS_CHAR_TYPE:
      return lhs.chr < rhs.chr ? -1 : lhs.chr > rhs.chr ? +1 : 0;
      break;
    case DDS_TS_WIDE_CHAR_TYPE:
      assert(false);
      break;
    case DDS_TS_LONG_DOUBLE_TYPE:
      return lhs.ldbl < rhs.ldbl ? -1 : lhs.ldbl > rhs.ldbl ? +1 : 0;
      break;
    case DDS_TS_BOOLEAN_TYPE:
      return (!lhs.bln && rhs.bln) ? -1 : (lhs.bln && !rhs.bln) ? +1 : 0;
      break;
    default:
      assert(false);
      break;
  }
  return -1;
}
*/

/* (not used at this moment)
static bool dds_ts_check_literal_type(const dds_ts_literal_t *value, dds_ts_base_type_t *base_type)
{
  (void)value;
  (void)base_type;
  switch (value.type) {
    case DDS_TS_LONG_LONG_TYPE:
      switch (base_type->flags) {
	case DDS_TS_INT8_TYPE:
	  return -(1L << 7) <= value.llng && value.llng < (1L << 7);
	  break;
	case DDS_TS_SHORT_TYPE:
	  return -(1L << 15) <= value.llng && value.llng < (1L << 15);
	  break;
	case DDS_TS_LONG_TYPE:
	  return -(1LL << 31) <= value.llng && value.llng < (1LL << 31);
	  break;
	case DDS_TS_LONG_LONG_TYPE:
	  return true;
	  break;
	case DDS_TS_UINT8_TYPE:
	  return 0 <= value.llng && value.llng < (1L << 8);
	  break;
	case DDS_TS_UNSIGNED_SHORT_TYPE:
	  return 0 <= value.llng && value.llng < (1L << 16);
	  break;
	case DDS_TS_UNSIGNED_LONG_TYPE:
	  return 0 <= value.llng && value.llng < (1LL << 32);
	  return true;
	  break;
	case DDS_TS_UNSIGNED_LONG_LONG_TYPE:
	  return 0 <= value.llng;
	  break;
	default:
	  break;
      }
      break;
    case DDS_TS_CHAR_TYPE:
      return base_type->flags == DDS_TS_CHAR_TYPE;
      break;
    case DDS_TS_WIDE_CHAR_TYPE:
      return base_type->flags == DDS_TS_WIDE_CHAR_TYPE;
      break;
    case dds_ts_value_fixed_pt:
      return base_type->flags == DDS_TS_FIXED_PT_TYPE;
      break;
    case dds_ts_value_floating_pt:
      return base_type->flags == DDS_TS_FLOAT_TYPE;
      break;
    case dds_ts_value_boolean:
      return base_type->flags == DDS_TS_BOOLEAN_TYPE;
      break;
    default:
      assert(false);
      break;
  }
  return true;
}
*/

extern void dds_ts_eval_unary_oper(dds_ts_operator_type_t operator_type, const dds_ts_literal_t *operand, dds_ts_literal_t *result)
{
  if (operand->flags == DDS_TS_LONG_LONG_TYPE) {
    result->flags = DDS_TS_LONG_LONG_TYPE;
    switch (operator_type) {
      case dds_ts_operator_minus:
	result->llng = -operand->llng;
	break;
      case dds_ts_operator_plus:
	result->llng = operand->llng;
	break;
      case dds_ts_operator_inv:
	result->llng = ~operand->llng;
	break;
      default:
	assert(false);
	break;
    }
  } else {
    /* FIXME: error reporting */
  }
}

extern void dds_ts_eval_binary_oper(dds_ts_operator_type_t operator_type, const dds_ts_literal_t *lhs, const dds_ts_literal_t *rhs, dds_ts_literal_t *result)
{
  if (lhs->flags == DDS_TS_LONG_LONG_TYPE && rhs->flags == DDS_TS_LONG_LONG_TYPE) {
    result->flags = DDS_TS_LONG_LONG_TYPE;
    switch (operator_type) {
      case dds_ts_operator_or:
	result->llng = lhs->llng | rhs->llng;
	break;
      case dds_ts_operator_xor:
	result->llng = lhs->llng ^ rhs->llng;
	break;
      case dds_ts_operator_and:
	result->llng = lhs->llng & rhs->llng;
	break;
      case dds_ts_operator_shift_left:
	result->llng = lhs->llng << rhs->llng;
	break;
      case dds_ts_operator_shift_right:
	result->llng = lhs->llng >> rhs->llng;
	break;
      case dds_ts_operator_add:
	result->llng = lhs->llng + rhs->llng;
	break;
      case dds_ts_operator_sub:
	result->llng = lhs->llng - rhs->llng;
	break;
      case dds_ts_operator_times:
	result->llng = lhs->llng * rhs->llng;
	break;
      case dds_ts_operator_div:
	result->llng = lhs->llng / rhs->llng;
	break;
      case dds_ts_operator_mod:
	result->llng = lhs->llng % rhs->llng;
	break;
      default:
	assert(false);
	break;
    }
  } else {
    /* FIXME: other types + error reporting */
  }
}

static void add_child(dds_ts_node_t *node, dds_ts_node_t *parent)
{
  dds_ts_node_t **ref_def = &parent->children;
  while (*ref_def != 0) {
    ref_def = &(*ref_def)->next;
  }
  *ref_def = node;
}

static void init_node(dds_ts_node_t *node, dds_ts_node_flags_t flags, dds_ts_node_t *parent)
{
  node->flags = flags;
  node->parent = parent;
  node->children = NULL;
  node->next = NULL;
  if (parent != NULL) {
    add_child(node, parent);
  }
}

dds_ts_type_spec_t *dds_ts_new_base_type(dds_ts_context_t *context, dds_ts_node_flags_t flags)
{
  (void)context;
  dds_ts_base_type_t *base_type = (dds_ts_base_type_t*)os_malloc(sizeof(dds_ts_base_type_t));
  if (base_type == NULL) {
    abort();
  }
  init_node((dds_ts_node_t*)base_type, flags, NULL);
  return (dds_ts_type_spec_t*)base_type;
}

static dds_ts_type_spec_t *new_sequence_type(dds_ts_type_spec_t *element_type, bool bounded, unsigned long long max)
{
  dds_ts_sequence_type_t *sequence_type = (dds_ts_sequence_type_t*)os_malloc(sizeof(dds_ts_sequence_type_t));
  if (sequence_type == NULL) {
    abort();
  }
  init_node((dds_ts_node_t*)sequence_type, DDS_TS_SEQUENCE_TYPE, NULL);
  sequence_type->element_type = element_type;
  sequence_type->bounded = bounded;
  sequence_type->max = max;
  return (dds_ts_type_spec_t*)sequence_type;
}

dds_ts_type_spec_t *dds_ts_new_sequence_type(dds_ts_context_t *context, dds_ts_type_spec_t *base, const dds_ts_literal_t *size)
{
  (void)context;
  assert(size->flags == DDS_TS_LONG_LONG_TYPE);
  return new_sequence_type(base, true, size->ullng);
}

dds_ts_type_spec_t *dds_ts_new_sequence_type_unbound(dds_ts_context_t *context, dds_ts_type_spec_t *base)
{
  (void)context;
  return new_sequence_type(base, false, 0);
}

static dds_ts_type_spec_t *new_string_type(dds_ts_node_flags_t flags, bool bounded, unsigned long long max)
{
  dds_ts_string_type_t *string_type = (dds_ts_string_type_t*)os_malloc(sizeof(dds_ts_string_type_t));
  if (string_type == NULL) {
    abort();
  }
  init_node((dds_ts_node_t*)string_type, flags, NULL);
  string_type->bounded = bounded;
  string_type->max = max;
  return (dds_ts_type_spec_t*)string_type;
}

dds_ts_type_spec_t *dds_ts_new_string_type(dds_ts_context_t *context, const dds_ts_literal_t *size)
{
  (void)context;
  assert(size->flags == DDS_TS_LONG_LONG_TYPE);
  if (size->llng < 0) {
    /* FIXME: report error */
  }
  return new_string_type(DDS_TS_STRING_TYPE, true, (unsigned long long)size->llng);
}

dds_ts_type_spec_t *dds_ts_new_string_type_unbound(dds_ts_context_t *context)
{
  (void)context;
  return new_string_type(DDS_TS_STRING_TYPE, false, 0);
}

dds_ts_type_spec_t *dds_ts_new_wide_string_type(dds_ts_context_t *context, const dds_ts_literal_t *size)
{
  (void)context;
  assert(size->flags == DDS_TS_LONG_LONG_TYPE);
  if (size->llng < 0) {
    /* FIXME: report error */
  }
  return new_string_type(DDS_TS_WIDE_STRING_TYPE, true, (unsigned long long)size->llng);
}

dds_ts_type_spec_t *dds_ts_new_wide_string_type_unbound(dds_ts_context_t *context)
{
  (void)context;
  return new_string_type(DDS_TS_WIDE_STRING_TYPE, false, 0);
}

dds_ts_type_spec_t *dds_ts_new_fixed_type(dds_ts_context_t *context, const dds_ts_literal_t *digits, const dds_ts_literal_t *fraction_digits)
{
  (void)context;
  dds_ts_fixed_pt_type_t *fixed_pt_type = (dds_ts_fixed_pt_type_t*)os_malloc(sizeof(dds_ts_fixed_pt_type_t));
  if (fixed_pt_type == NULL) {
    abort();
  }
  init_node((dds_ts_node_t*)fixed_pt_type, DDS_TS_FIXED_PT_TYPE, NULL);
  assert(digits->flags == DDS_TS_LONG_LONG_TYPE);
  fixed_pt_type->digits = (unsigned long long)digits->llng;
  assert(fraction_digits->flags == DDS_TS_LONG_LONG_TYPE);
  fixed_pt_type->fraction_digits = (unsigned long long)fraction_digits->llng;
  return (dds_ts_type_spec_t*)fixed_pt_type;
}

static dds_ts_type_spec_t *new_map_type(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type, bool bounded, unsigned long long max)
{
  (void)context;
  dds_ts_map_type_t *map_type = (dds_ts_map_type_t*)os_malloc(sizeof(dds_ts_map_type_t));
  if (map_type == NULL) {
    abort();
  }
  init_node((dds_ts_node_t*)map_type, DDS_TS_MAP_TYPE, NULL);
  map_type->key_type = key_type;
  map_type->value_type = value_type;
  map_type->bounded = bounded;
  map_type->max = max;
  return (dds_ts_type_spec_t*)map_type;
}

dds_ts_type_spec_t *dds_ts_new_map_type(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type, const dds_ts_literal_t *size)
{
  assert(size->flags == DDS_TS_LONG_LONG_TYPE);
  return new_map_type(context, key_type, value_type, true, (unsigned long long)size->llng);
}

dds_ts_type_spec_t *dds_ts_new_map_type_unbound(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type)
{
  return new_map_type(context, key_type, value_type, false, 0);
}

dds_ts_scoped_name_t *dds_ts_new_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t* prev, bool top, dds_ts_identifier_t  name)
{
  assert(context != NULL);
  (void)context;
  (void)prev;
  (void)top;
  (void)name;
  return NULL;
}

dds_ts_node_flags_t dds_ts_get_base_type_of_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t* scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
  return DDS_TS_BOOLEAN_TYPE;
}

void dds_ts_get_value_of_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t *scoped_name, dds_ts_literal_t *result)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
  result->flags = DDS_TS_BOOLEAN_TYPE;
  result->bln = false;
}

static void init_annotated_node(dds_ts_context_t *context, dds_ts_annotated_node_t *annotated_node, dds_ts_node_flags_t flags, dds_ts_node_t *parent)
{
  init_node((dds_ts_node_t*)annotated_node, flags, parent);
  annotated_node->annotations = NULL; /* FIXME: should get these from the context */
  (void)context;

}
static void init_definition(dds_ts_context_t *context, dds_ts_definition_t *definition, dds_ts_identifier_t name, dds_ts_node_flags_t flags, dds_ts_node_t *parent)
{
  init_annotated_node(context, (dds_ts_annotated_node_t*)definition, flags, parent);
  if (name != NULL) {
    definition->name = os_strdup(name);
    if (definition->name == NULL) {
      abort();
    }
  }
  else {
    definition->name = NULL;
  }
}

static dds_ts_module_t *dds_ts_new_module_definition(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_node_t *parent)
{
  dds_ts_module_t *def = (dds_ts_module_t*)os_malloc(sizeof(dds_ts_module_t));
  if (def == NULL) {
    abort();
  }
  init_definition(context, (dds_ts_definition_t*)def, name, DDS_TS_MODULE, parent);
  return def;
}

extern dds_ts_context_t* dds_ts_create_context()
{
  dds_ts_context_t *context = (dds_ts_context_t*)os_malloc(sizeof(dds_ts_context_t));
  if (context == NULL) {
    abort();
  }
  context->ignore_yyerror = false;
  context->root_node = (dds_ts_node_t*)dds_ts_new_module_definition(context, NULL, NULL);
  context->cur_node = context->root_node;
  context->parent_for_declarator = NULL;
  context->cur_union_case = NULL;
  return context;
}

extern void dds_ts_context_set_ignore_yyerror(dds_ts_context_t *context, bool ignore_yyerror)
{
  assert(context != NULL);
  context->ignore_yyerror = ignore_yyerror;
}

extern bool dds_ts_context_get_ignore_yyerror(dds_ts_context_t *context)
{
  assert(context != NULL);
  return context->ignore_yyerror;
}

extern dds_ts_node_t* dds_ts_context_get_root_node(dds_ts_context_t *context)
{
  return context->root_node;
}

extern void dds_ts_free_context(dds_ts_context_t *context)
{
  assert(context != NULL);
  os_free(context);
}

#if (!defined(NDEBUG))
static bool cur_scope_is_definition_type(dds_ts_context_t *context, dds_ts_node_flags_t flags)
{
  assert(context != NULL && context->cur_node != NULL);
  return context->cur_node->flags == flags;
}
#endif

extern void dds_ts_module_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  dds_ts_module_t *module = dds_ts_new_module_definition(context, name, (dds_ts_node_t*)context->cur_node);
  /* Search the previous occurence of this module
  for (dds_ts_node_t *child = context->cur_node->children; child != NULL; child = child->next) {
    if (child->flags == DDS_TS_MODULE && strcmp(((dds_ts_definition_t*)child)->name, name) == 0) {
      / * FIXME: store result in module * /
    }
  }*/
  context->cur_node = (dds_ts_node_t*)module;
}

extern void dds_ts_module_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  assert(context->cur_node->parent != NULL);
  context->cur_node = context->cur_node->parent;
}

void dds_ts_add_struct_forward(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  /*
  dds_ts_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_ts_definition_struct_forward && strcmp(def->name, name) == 0) {
      / * FIXME: report warning: repeated forward declarationn * /
      return;
    } else if (def->type == dds_ts_definition_struct && strcmp(def->name, name) == 0) {
      / * FIXME: report error: forward declaration after definition * /
      return;
    }
  }
  */
  dds_ts_forward_declaration_t *forward_dcl = (dds_ts_forward_declaration_t*)os_malloc(sizeof(dds_ts_forward_declaration_t));
  if (forward_dcl == NULL) {
    abort();
  }
  init_definition(context, (dds_ts_definition_t*)forward_dcl, name, DDS_TS_FORWARD_STRUCT, context->cur_node);
  forward_dcl->definition = NULL;
}

void dds_ts_add_struct_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(   cur_scope_is_definition_type(context, DDS_TS_MODULE)
         || cur_scope_is_definition_type(context, DDS_TS_STRUCT));
/*
  assert(cur_scope_is_definition_type(context, dds_ts_definition_module));
  dds_ts_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_ts_definition_struct_forward && strcmp(def->name, name) == 0) {
      def->struct_forward_def->defined = true;
    } else if (def->type == dds_ts_definition_struct && strcmp(def->name, name) == 0) {
      / * FIXME: report error: repeated definition * /
    }
  }
*/
  dds_ts_struct_t *new_struct = (dds_ts_struct_t*)os_malloc(sizeof(dds_ts_struct_t));
  if (new_struct == NULL) {
    abort();
  }
  init_definition(context, (dds_ts_definition_t*)new_struct, name, DDS_TS_STRUCT, context->cur_node);
  new_struct->super = NULL;
  context->cur_node = (dds_ts_node_t*)new_struct;
}

void dds_ts_add_struct_extension_open(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_scoped_name_t *scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)scoped_name;
}

void dds_ts_add_struct_member(dds_ts_context_t *context, dds_ts_type_spec_t *type)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_STRUCT));
  dds_ts_struct_member_t *member = (dds_ts_struct_member_t*)os_malloc(sizeof(dds_ts_struct_member_t));
  if (member == NULL) {
    abort();
  }
  init_annotated_node(context, (dds_ts_annotated_node_t*)member, DDS_TS_STRUCT_MEMBER, context->cur_node);
  member->member_type = type;
  context->parent_for_declarator = (dds_ts_node_t*)member;
}

void dds_ts_struct_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_STRUCT));
  context->cur_node = context->cur_node->parent;
  context->parent_for_declarator = NULL;
}

void dds_ts_struct_empty_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_STRUCT));
  context->cur_node = context->cur_node->parent;
  context->parent_for_declarator = NULL;
}

void dds_ts_add_union_forward(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  (void)context;
  (void)name;
/*
  assert(cur_scope_is_definition_type(context, dds_ts_definition_module));
  dds_ts_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_ts_definition_union_forward && strcmp(def->name, name) == 0) {
      / * FIXME: report warning: repeated forward declarationn * /
      return;
    } else if (def->type == dds_ts_definition_union && strcmp(def->name, name) == 0) {
      / * FIXME: report error: forward declaration after definition * /
      return;
    }
  }
  */
  dds_ts_forward_declaration_t *forward_dcl = (dds_ts_forward_declaration_t*)os_malloc(sizeof(dds_ts_forward_declaration_t));
  if (forward_dcl == NULL) {
    abort();
  }
  init_definition(context, (dds_ts_definition_t*)forward_dcl, name, DDS_TS_FORWARD_UNION, context->cur_node);
  forward_dcl->definition = NULL;
}

void dds_ts_add_union_open(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_node_flags_t switch_type)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
/*
  assert(cur_scope_is_definition_type(context, dds_ts_definition_module));
  dds_ts_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_ts_definition_union_forward && strcmp(def->name, name) == 0) {
      def->union_forward_def->defined = true;
    } else if (def->type == dds_ts_definition_union && strcmp(def->name, name) == 0) {
      / * FIXME: report error: repeated definition * /
    }
  }
*/
  dds_ts_union_t *new_union = (dds_ts_union_t*)os_malloc(sizeof(dds_ts_union_t));
  if (new_union == NULL) {
    abort();
  }
  init_definition(context, (dds_ts_definition_t*)new_union, name, DDS_TS_UNION, context->cur_node);
  new_union->switch_type = switch_type;
  context->cur_node = (dds_ts_node_t*)new_union;
}

static void init_union_case(dds_ts_context_t *context)
{
  (void)context;
  assert(context != NULL);
  if (context->cur_union_case == NULL) {
    dds_ts_union_case_t *union_case = (dds_ts_union_case_t*)os_malloc(sizeof(dds_ts_union_case_t));
    if (union_case == NULL) {
      abort();
    }
    init_definition(context, (dds_ts_definition_t*)union_case, NULL, DDS_TS_UNION_CASE, context->cur_node);
    union_case->branch_type = 0;
    context->cur_union_case = union_case;
  }
}

void dds_ts_add_union_case_label(dds_ts_context_t *context, const dds_ts_literal_t *value)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_UNION));
  /* Check if value matches switch type
  if (dds_ts_check_literal_type(value, context->cur_node->union_def->switch_type)) {
    / * FIXME: report error: type of value does not match * /
  } */
  /* Check if value is repeated
  for (dds_ts_union_case_t *union_case = context->cur_node->union_def->cases; union_case != 0; union_case = union_case->next) {
    for (dds_ts_union_case_label_t *label = union_case->labels; label != 0; label = label->next) {
      if (!label->is_default && dds_ts_compare_literal(label->value, value)) {
	/ * FIXME: report error: value repeated * /
      }
    }
  } */
  init_union_case(context);
  dds_ts_union_case_label_t *label = (dds_ts_union_case_label_t*)os_malloc(sizeof(dds_ts_union_case_label_t));
  if (label == NULL) {
    abort();
  }
  init_annotated_node(context, (dds_ts_annotated_node_t*)label, DDS_TS_UNION_CASE_LABEL, (dds_ts_node_t*)context->cur_union_case);
  label->value = *value;
}

void dds_ts_add_union_case_default(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_UNION));
  /* Check if default is repeated
  for (dds_ts_union_case_t *union_case = context->cur_node->union_def->cases; union_case != 0; union_case = union_case->next) {
    for (dds_ts_union_case_label_t *label = union_case->labels; label != 0; label = label->next) {
      if (label->is_default) {
	/ * FIXME: report error: default repeated * /
      }
    }
  } */
  init_union_case(context);
  dds_ts_annotated_node_t *default_label = (dds_ts_annotated_node_t*)os_malloc(sizeof(dds_ts_annotated_node_t));
  if (default_label == NULL) {
    abort();
  }
  init_annotated_node(context, default_label, DDS_TS_UNION_CASE_DEFAULT, (dds_ts_node_t*)context->cur_union_case);
}

void dds_ts_add_union_element(dds_ts_context_t *context, dds_ts_type_spec_t *type)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_UNION));
  assert(context->cur_union_case != NULL);
  context->cur_union_case->branch_type = type;
  context->cur_union_case = NULL;
}

void dds_ts_union_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_UNION));
/*
  assert(cur_scope_is_definition_type(context, dds_ts_definition_union));
  context->ref_next_union_case = NULL;
  context->ref_next_union_case_label = NULL;
*/
  context->cur_node = context->cur_node->parent;
}

void dds_ts_add_typedef_open(dds_ts_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_ts_typedef_set_type(dds_ts_context_t *context, dds_ts_type_spec_t *type)
{
  assert(context != NULL);
  (void)context;
  (void)type;
}

void dds_ts_typedef_close(dds_ts_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_ts_add_declarator(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(context != NULL);
  if (context->parent_for_declarator != NULL && context->cur_node->flags == DDS_TS_STRUCT) {
    dds_ts_definition_t* declarator = (dds_ts_definition_t*)os_malloc(sizeof(dds_ts_definition_t));
    if (declarator == NULL) {
      abort();
    }
    init_definition(context, declarator, name, DDS_TS_DECLARATOR, context->parent_for_declarator);
    return;
  }
  if (context->cur_union_case != NULL && context->cur_node->flags == DDS_TS_UNION) {
    context->cur_union_case->name = os_strdup(name);
    return;
  }
  assert(false);
}

void dds_ts_add_const_def(dds_ts_context_t *context, dds_ts_type_spec_t *base_type, dds_ts_identifier_t name, const dds_ts_literal_t *value)
{
  assert(context != NULL);
  (void)context;
  (void)base_type;
  (void)name;
  (void)value;
}

void dds_ts_add_enum_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  dds_ts_definition_t *enum_def = (dds_ts_definition_t*)os_malloc(sizeof(dds_ts_definition_t));
  if (enum_def == NULL) {
    abort();
  }
  init_definition(context, enum_def, name, DDS_TS_ENUM, context->cur_node);
  context->cur_node = (dds_ts_node_t*)enum_def;
}

void dds_ts_add_enum_enumerator(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_ENUM));
  dds_ts_definition_t *enumerator = (dds_ts_definition_t*)os_malloc(sizeof(dds_ts_definition_t));
  if (enumerator == NULL) {
    abort();
  }
  init_definition(context, enumerator, name, DDS_TS_ENUMERATOR, context->cur_node);
}

void dds_ts_enum_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_ENUM));
  context->cur_node = context->cur_node->parent;
}

void dds_ts_add_array_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_ts_add_array_size(dds_ts_context_t *context, const dds_ts_literal_t *value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_ts_array_close(dds_ts_context_t* context)
{
  assert(context != NULL);
  (void)context;
}

void dds_ts_add_native(dds_ts_context_t* context, dds_ts_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_ts_add_annotation_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_ts_add_annotation_member_open(dds_ts_context_t *context, dds_ts_type_spec_t *base_type, dds_ts_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)base_type;
  (void)name;
}

void dds_ts_annotation_member_set_default(dds_ts_context_t *context, const dds_ts_literal_t *value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_ts_annotation_member_close(dds_ts_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_ts_annotation_close(dds_ts_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_ts_add_annotation_appl_open(dds_ts_context_t *context, dds_ts_scoped_name_t *scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
}

void dds_ts_add_annotation_appl_expr(dds_ts_context_t *context, const dds_ts_literal_t *value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_ts_add_annotation_appl_param(dds_ts_context_t *context, dds_ts_identifier_t name, const dds_ts_literal_t *value)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)value;
}

void dds_ts_annotation_appl_close(dds_ts_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

extern void dds_ts_add_const_definition(dds_ts_context_t *context, dds_ts_identifier_t name, const dds_ts_literal_t *value)
{
  (void)context;
  (void)name;
  (void)value;
/*
  assert(cur_scope_is_definition_type(context, dds_ts_definition_module));
  dds_ts_definition_t *def = dds_ts_new_definition(name, dds_ts_definition_const, context->cur_node);
  dds_ts_append_definition(def, &context->cur_node->module_def->definitions);
  def->const_def = (dds_ts_const_definition_t*)os_malloc(sizeof(dds_ts_const_definition_t));
  if (def->const_def == NULL) {
    abort();
  }
  def->const_def->value = value;
*/
}

dds_ts_const_definition_t *dds_ts_get_const_definition(dds_ts_definition_t *definitions, dds_ts_identifier_t name)
{
  (void)definitions;
  (void)name;
/*
  dds_ts_definition_t *definition = definitions;
  for (; definition != NULL; definition = definition->next) {
    if (strcmp(definition->name, name) == 0) {
      if (definition->type == dds_ts_definition_const) {
        return definition->const_def;
      } else {
	/ * FIXME: report error that definition is not a const definition * /
	return NULL;
      }
    }
  }
*/
  return NULL;
}

void dds_ts_add_bitset_open(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_type_spec_t *opt_type)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)opt_type;
}

void dds_ts_add_bitset_field(dds_ts_context_t *context, const dds_ts_literal_t *index)
{
  assert(context != NULL);
  (void)context;
  (void)index;
}

void dds_ts_add_bitset_field_to(dds_ts_context_t *context, const dds_ts_literal_t *index, dds_ts_node_flags_t dest_type)
{
  assert(context != NULL);
  (void)context;
  (void)index;
  (void)dest_type;
}

void dds_ts_add_bitset_ident(dds_ts_context_t *context, dds_ts_identifier_t ident)
{
  assert(context != NULL);
  (void)context;
  (void)ident;
}

void dds_ts_bitset_close(dds_ts_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_ts_add_bitmask_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_ts_add_bitmask_value(dds_ts_context_t *context, dds_ts_identifier_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_ts_bitmask_close(dds_ts_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

