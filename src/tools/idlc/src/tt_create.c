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

struct dds_tt_context {
  bool ignore_yyerror;
  dds_tt_node_t *root_node;
  dds_tt_node_t *cur_node;
  dds_tt_definition_t **ref_next_declarator;
  dds_tt_union_case_t **ref_next_union_case;
  //dds_tt_union_case_label_t **ref_next_union_case_label;
};

void dds_tt_print_literal(FILE* f, dds_tt_value_t literal)
{
  switch (literal.type) {
    case dds_tt_value_integer:
      fprintf(f, "integer %lld", literal.value.llng);
      break;
    case dds_tt_value_string:
      fprintf(f, "string '%s'", literal.value.str);
      break;
    case dds_tt_value_wide_string:
      fprintf(f, "wstring '%s'", literal.value.str);
      break;
    case dds_tt_value_character:
      fprintf(f, "char '%c'", literal.value.chr);
      break;
    case dds_tt_value_wide_character:
      fprintf(f, "wchar u%4lX", literal.value.wchr);
      break;
    case dds_tt_value_fixed_pt:
      fprintf(f, "fixed %Lf", literal.value.ldbl);
      break;
    case dds_tt_value_floating_pt:
      fprintf(f, "float %Lf", literal.value.ldbl);
      break;
    case dds_tt_value_boolean:
      fprintf(f, "boolean: %s", literal.value.bln ? "true" : "false");
      break;
    default:
      fprintf(f, "illegal-type");
      break;
  }
}

static int dds_tt_compare_literal(dds_tt_value_t lhs, dds_tt_value_t rhs)
{
  if (lhs.type < rhs.type) {
    return -1;
  }
  if (lhs.type > rhs.type) {
    return +1;
  }
  switch (lhs.type) {
    case dds_tt_value_integer:
      return lhs.value.llng < rhs.value.llng ? -1 : lhs.value.llng > rhs.value.llng ? +1 : 0;
      break;
    case dds_tt_value_string:
      return strcmp(lhs.value.str, rhs.value.str);
      break;
    case dds_tt_value_wide_string:
      assert(false);
      break;
    case dds_tt_value_character:
      return lhs.value.chr < rhs.value.chr ? -1 : lhs.value.chr > rhs.value.chr ? +1 : 0;
      break;
    case dds_tt_value_wide_character:
      assert(false);
      break;
    case dds_tt_value_fixed_pt:
      assert(false);
      break;
    case dds_tt_value_floating_pt:
      return lhs.value.ldbl < rhs.value.ldbl ? -1 : lhs.value.ldbl > rhs.value.ldbl ? +1 : 0;
      break;
    case dds_tt_value_boolean:
      return (!lhs.value.bln && rhs.value.bln) ? -1 : (lhs.value.bln && !rhs.value.bln) ? +1 : 0;
      break;
    default:
      assert(false);
      break;
  }
  return -1;
}

static bool dds_tt_check_literal_type(dds_tt_value_t value, dds_tt_base_type_t *base_type)
{
  switch (value.type) {
    case dds_tt_value_integer:
      switch (base_type->flags) {
	case DDS_TT_INT8_TYPE:
	  return -(1L << 7) <= value.value.llng && value.value.llng < (1L << 7);
	  break;
	case DDS_TT_SHORT_TYPE:
	  return -(1L << 15) <= value.value.llng && value.value.llng < (1L << 15);
	  break;
	case DDS_TT_LONG_TYPE:
	  return -(1LL << 31) <= value.value.llng && value.value.llng < (1LL << 31);
	  break;
	case DDS_TT_LONG_LONG_TYPE:
	  return true;
	  break;
	case DDS_TT_UINT8_TYPE:
	  return 0 <= value.value.llng && value.value.llng < (1L << 8);
	  break;
	case DDS_TT_UNSIGNED_SHORT_TYPE:
	  return 0 <= value.value.llng && value.value.llng < (1L << 16);
	  break;
	case DDS_TT_UNSIGNED_LONG_TYPE:
	  return 0 <= value.value.llng && value.value.llng < (1LL << 32);
	  return true;
	  break;
	case DDS_TT_UNSIGNED_LONG_LONG_TYPE:
	  return 0 <= value.value.llng;
	  break;
	default:
	  break;
      }
      break;
    case dds_tt_value_character:
      return base_type->flags == DDS_TT_CHAR_TYPE;
      break;
    case dds_tt_value_wide_character:
      return base_type->flags == DDS_TT_WIDE_CHAR_TYPE;
      break;
    case dds_tt_value_fixed_pt:
      return base_type->flags == DDS_TT_FIXED_PT_TYPE;
      break;
    case dds_tt_value_floating_pt:
      return base_type->flags == DDS_TT_FLOAT_TYPE;
      break;
    case dds_tt_value_boolean:
      return base_type->flags == DDS_TT_BOOLEAN_TYPE;
      break;
    default:
      assert(false);
      break;
  }
  return false;
}

extern void dds_tt_eval_unary_oper(dds_tt_operator_type_t operator_type, dds_tt_value_t operand, dds_tt_value_t *result)
{
  if (operand.type == dds_tt_value_integer) {
    result->type = dds_tt_value_integer;
    switch (operator_type) {
      case dds_tt_operator_minus:
	result->value.llng = -operand.value.llng;
	break;
      case dds_tt_operator_plus:
	result->value.llng = operand.value.llng;
	break;
      case dds_tt_operator_inv:
	result->value.llng = ~operand.value.llng;
	break;
      default:
	assert(false);
	break;
    }
  } else {
    /* FIXME: error reporting */
  }
}

extern void dds_tt_eval_binary_oper(dds_tt_operator_type_t operator_type, dds_tt_value_t lhs, dds_tt_value_t rhs, dds_tt_value_t *result)
{
  if (lhs.type == dds_tt_value_integer && rhs.type == dds_tt_value_integer) {
    result->type = dds_tt_value_integer;
    switch (operator_type) {
      case dds_tt_operator_or:
	result->value.llng = lhs.value.llng | rhs.value.llng;
	break;
      case dds_tt_operator_xor:
	result->value.llng = lhs.value.llng ^ rhs.value.llng;
	break;
      case dds_tt_operator_and:
	result->value.llng = lhs.value.llng & rhs.value.llng;
	break;
      case dds_tt_operator_shift_left:
	result->value.llng = lhs.value.llng << rhs.value.llng;
	break;
      case dds_tt_operator_shift_right:
	result->value.llng = lhs.value.llng >> rhs.value.llng;
	break;
      case dds_tt_operator_add:
	result->value.llng = lhs.value.llng + rhs.value.llng;
	break;
      case dds_tt_operator_sub:
	result->value.llng = lhs.value.llng - rhs.value.llng;
	break;
      case dds_tt_operator_times:
	result->value.llng = lhs.value.llng * rhs.value.llng;
	break;
      case dds_tt_operator_div:
	result->value.llng = lhs.value.llng / rhs.value.llng;
	break;
      case dds_tt_operator_mod:
	result->value.llng = lhs.value.llng % rhs.value.llng;
	break;
      default:
	assert(false);
	break;
    }
  } else {
    /* FIXME: other types + error reporting */
  }
}

static void add_child(dds_tt_node_t *node, dds_tt_node_t *parent)
{
  dds_tt_node_t **ref_def = &parent->children;
  while (*ref_def != 0) {
    ref_def = &(*ref_def)->next;
  }
  *ref_def = node;    
}

static void init_node(dds_tt_node_t *node, dds_tt_node_flags_t flags, dds_tt_node_t *parent)
{
  node->flags = flags;
  node->parent = parent;
  node->children = NULL;
  if (parent != NULL) {
    add_child(node, parent);
  }
}

dds_tt_type_spec_t *dds_tt_new_base_type(dds_tt_context_t *context, dds_tt_node_flags_t flags)
{
  (void)context;
  dds_tt_base_type_t *base_type = (dds_tt_base_type_t*)os_malloc(sizeof(dds_tt_base_type_t));
  if (base_type == NULL) {
    abort();
  }
  init_node((dds_tt_node_t*)base_type, flags, NULL);
  return (dds_tt_type_spec_t*)base_type;
}

static dds_tt_type_spec_t *new_sequence_type(dds_tt_type_spec_t *base, bool bounded, unsigned long long max)
{
  dds_tt_sequence_type_t *sequence_type = (dds_tt_base_type_t*)os_malloc(sizeof(dds_tt_base_type_t));
  if (sequence_type == NULL) {
    abort();
  }
  init_node((dds_tt_node_t*)sequence_type, DDS_TT_SEQUENCE_TYPE, NULL);
  sequence_type->base = base;
  sequence_type->bounded = bounded;
  sequence_type->max = max;
  return (dds_tt_type_spec_t*)sequence_type;
}

dds_tt_type_spec_t *dds_tt_new_sequence_type(dds_tt_context_t *context, dds_tt_type_spec_t *base, dds_tt_value_t size)
{
  (void)context;
  assert(size.type == dds_tt_value_integer);
  return new_sequence_type(base, false, size.value.llng);
}

dds_tt_type_spec_t *dds_tt_new_sequence_type_unbound(dds_tt_context_t *context, dds_tt_type_spec_t *base)
{
  (void)context;
  return new_sequence_type(base, true, 0);
}

static dds_tt_type_spec_t *new_string_type(dds_tt_node_flags_t flags, bool bounded, unsigned long long max)
{
  dds_tt_string_type_t *string_type = (dds_tt_string_type_t*)os_malloc(sizeof(dds_tt_string_type_t));
  if (string_type == NULL) {
    abort();
  }
  init_node((dds_tt_node_t*)string_type, flags, NULL);
  string_type->bounded = bounded;
  string_type->max = max;
  return (dds_tt_type_spec_t*)string_type;
}

dds_tt_type_spec_t *dds_tt_new_string_type(dds_tt_context_t *context, dds_tt_value_t size)
{
  (void)context;
  assert(size.type == dds_tt_value_integer);
  if (size.value.llng < 0) {
    /* FIXME: report error */
  }  
  return new_string_type(DDS_TT_STRING_TYPE, false, (unsigned long long)size.value.llng);
}

dds_tt_type_spec_t *dds_tt_new_string_type_unbound(dds_tt_context_t *context)
{
  (void)context;
  return new_string_type(DDS_TT_STRING_TYPE, true, 0);
}

dds_tt_type_spec_t *dds_tt_new_wide_string_type(dds_tt_context_t *context, dds_tt_value_t size)
{
  (void)context;
  assert(size.type == dds_tt_value_integer);
  if (size.value.llng < 0) {
    /* FIXME: report error */
  }  
  return new_string_type(DDS_TT_WIDE_STRING_TYPE, false, (unsigned long long)size.value.llng);
}

dds_tt_type_spec_t *dds_tt_new_wide_string_type_unbound(dds_tt_context_t *context)
{
  (void)context;
  return new_string_type(DDS_TT_WIDE_STRING_TYPE, true, 0);
}

dds_tt_type_spec_t *dds_tt_new_fixed_type(dds_tt_context_t *context, dds_tt_value_t digits, dds_tt_value_t fraction_digits)
{
  assert(context != NULL);
  (void)context;
  (void)digits;
  (void)fraction_digits;
  return NULL;
}

dds_tt_type_spec_t *dds_tt_new_map_type(dds_tt_context_t *context, dds_tt_type_spec_t *key_type, dds_tt_type_spec_t *value_type, dds_tt_value_t size)
{
  assert(context != NULL);
  (void)context;
  (void)key_type;
  (void)value_type;
  (void)size;
  return NULL;
}

dds_tt_type_spec_t *dds_tt_new_map_type_unbound(dds_tt_context_t *context, dds_tt_type_spec_t *key_type, dds_tt_type_spec_t *value_type)
{
  assert(context != NULL);
  (void)context;
  (void)key_type;
  (void)value_type;
  return NULL;
}

dds_tt_scoped_name_t *dds_tt_new_scoped_name(dds_tt_context_t *context, dds_tt_scoped_name_t* prev, bool top, dds_tt_identifier_t  name)
{
  assert(context != NULL);
  (void)context;
  (void)prev;
  (void)top;
  (void)name;
  return NULL;
}

dds_tt_node_flags_t dds_tt_get_base_type_of_scoped_name(dds_tt_context_t *context, dds_tt_scoped_name_t* scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
  return DDS_TT_BOOLEAN_TYPE;
}

dds_tt_value_t dds_tt_get_value_of_scoped_name(dds_tt_context_t *context, dds_tt_scoped_name_t *scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
  dds_tt_value_t result;
  result.type = dds_tt_value_boolean;
  result.value.bln = false;
  return result;
}

static void init_definition(dds_tt_definition_t *definition, dds_tt_identifier_t name, dds_tt_node_flags_t flags, dds_tt_node_t *parent)
{
  init_node((dds_tt_node_t*)definition, flags, parent);
  definition->name = os_strdup(name);
  if (definition->name == NULL) {
    abort();
  }
}

static dds_tt_module_t *dds_tt_new_module_definition(dds_tt_identifier_t name, dds_tt_node_t *parent)
{
  dds_tt_module_t *def = (dds_tt_module_t*)os_malloc(sizeof(dds_tt_module_t));
  if (def == NULL) {
    abort();
  }
  init_definition((dds_tt_definition_t*)def, name, DDS_TT_MODULE, parent);
  return def;
}

extern dds_tt_context_t* dds_tt_create_context()
{
  dds_tt_context_t *context = (dds_tt_context_t*)os_malloc(sizeof(dds_tt_context_t));
  if (context == NULL) {
    abort();
  }
  context->ignore_yyerror = false;
  context->root_node = (dds_tt_node_t*)dds_tt_new_module_definition("", NULL);
  context->cur_node = context->root_node;
  context->ref_next_declarator = NULL;
  context->ref_next_union_case = NULL;
  //context->ref_next_union_case_label = NULL;
  return context;
}

extern void dds_tt_context_set_ignore_yyerror(dds_tt_context_t *context, bool ignore_yyerror)
{
  assert(context != NULL);
  context->ignore_yyerror = ignore_yyerror;
}

extern bool dds_tt_context_get_ignore_yyerror(dds_tt_context_t *context)
{
  assert(context != NULL);
  return context->ignore_yyerror;
}

extern dds_tt_node_t* dds_tt_context_get_root_node(dds_tt_context_t *context)
{
  return context->root_node;
}

extern void dds_tt_free_context(dds_tt_context_t *context)
{
  assert(context != NULL);
  os_free(context);
}

static bool cur_scope_is_definition_type(dds_tt_context_t *context, dds_tt_node_flags_t flags)
{
  assert(context != NULL && context->cur_node != NULL);
  return context->cur_node->flags == flags;
}

extern void dds_tt_module_open(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  //fprintf(stderr, "dds_tt_module_open %s\n", name);	
  assert(cur_scope_is_definition_type(context, DDS_TT_MODULE));
  context->cur_node = (dds_tt_node_t*)dds_tt_new_module_definition(name, (dds_tt_node_t*)context->cur_node);
  //fprintf(stderr, " cur scope %s\n", context->cur_node->name);
}

extern void dds_tt_module_close(dds_tt_context_t *context)
{
  //fprintf(stderr, "dds_tt_module_close\n");
  assert(cur_scope_is_definition_type(context, DDS_TT_MODULE));
  assert(context->cur_node->parent != NULL);
  context->cur_node = context->cur_node->parent;
  //fprintf(stderr, " cur scope %s\n", context->cur_node->name);
}

void dds_tt_add_struct_forward(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TT_MODULE));
  (void)context;
  (void)name;
  /*
  dds_tt_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_tt_definition_struct_forward && strcmp(def->name, name) == 0) {
      / * FIXME: report warning: repeated forward declarationn * /
      return;
    } else if (def->type == dds_tt_definition_struct && strcmp(def->name, name) == 0) {
      / * FIXME: report error: forward declaration after definition * /
      return;
    }
  }

  dds_tt_definition_t *new_struct_forward = dds_tt_new_definition(name, dds_tt_definition_struct_forward, context->cur_node);
  new_struct_forward->struct_forward_def = (dds_tt_struct_forward_t*)os_malloc(sizeof(dds_tt_struct_forward_t));
  if (new_struct_forward->struct_forward_def == NULL) {
    abort();
  }
  new_struct_forward->struct_forward_def->defined = false;
  */
}

void dds_tt_add_struct_open(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  (void)context;
  (void)name;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_module));
  dds_tt_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_tt_definition_struct_forward && strcmp(def->name, name) == 0) {
      def->struct_forward_def->defined = true;
    } else if (def->type == dds_tt_definition_struct && strcmp(def->name, name) == 0) {
      / * FIXME: report error: repeated definition * /
    }
  }

  dds_tt_definition_t *new_struct = dds_tt_new_definition(name, dds_tt_definition_struct, context->cur_node);
  dds_tt_append_definition(new_struct, &context->cur_node->module_def->definitions);
  new_struct->struct_def = (dds_tt_struct_t*)os_malloc(sizeof(dds_tt_struct_t));
  if (new_struct->struct_def == NULL) {
    abort();
  }
  new_struct->struct_def->members = NULL;
  context->cur_node = new_struct;
*/
}

void dds_tt_add_struct_extension_open(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_scoped_name_t *scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)scoped_name;
}

void dds_tt_add_struct_member(dds_tt_context_t *context, dds_tt_type_spec_t *type)
{
  (void)context;
  (void)type;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_struct));
  context->ref_next_declarator = NULL;
  dds_tt_struct_member_t **ref_member = &context->cur_node->struct_def->members;
  while ((*ref_member) != NULL) {
    ref_member = &(*ref_member)->next;
  }
  (*ref_member) = (dds_tt_struct_member_t*)os_malloc(sizeof(dds_tt_struct_member_t));
  if ((*ref_member) == NULL) {
    abort();
  }
  (*ref_member)->type = type;
  (*ref_member)->declarators = NULL;
  (*ref_member)->next = NULL;
  context->ref_next_declarator = &(*ref_member)->declarators;
*/
}

void dds_tt_struct_close(dds_tt_context_t *context)
{
  (void)context;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_struct));
  context->ref_next_declarator = NULL;
  context->cur_node = context->cur_node->parent;
*/
}

void dds_tt_struct_empty_close(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_tt_add_union_forward(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  (void)context;
  (void)name;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_module));
  dds_tt_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_tt_definition_union_forward && strcmp(def->name, name) == 0) {
      / * FIXME: report warning: repeated forward declarationn * /
      return;
    } else if (def->type == dds_tt_definition_union && strcmp(def->name, name) == 0) {
      / * FIXME: report error: forward declaration after definition * /
      return;
    }
  }

  dds_tt_definition_t *new_union_forward = dds_tt_new_definition(name, dds_tt_definition_union_forward, context->cur_node);
  new_union_forward->union_forward_def = (dds_tt_union_forward_t*)os_malloc(sizeof(dds_tt_union_forward_t));
  if (new_union_forward->union_forward_def == NULL) {
    abort();
  }
  new_union_forward->union_forward_def->defined = false;
*/
}

void dds_tt_add_union_open(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_node_flags_t base_type)
{
  //fprintf(stderr, "union_open\n");
  (void)context;
  (void)name;
  (void)base_type;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_module));
  dds_tt_definition_t *def = context->cur_node->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == dds_tt_definition_union_forward && strcmp(def->name, name) == 0) {
      def->union_forward_def->defined = true;
    } else if (def->type == dds_tt_definition_union && strcmp(def->name, name) == 0) {
      / * FIXME: report error: repeated definition * /
    }
  }

  dds_tt_definition_t *new_union = dds_tt_new_definition(name, dds_tt_definition_union, context->cur_node);
  dds_tt_append_definition(new_union, &context->cur_node->module_def->definitions);
  new_union->union_def = (dds_tt_union_t*)os_malloc(sizeof(dds_tt_union_t));
  if (new_union->union_def == NULL) {
    abort();
  }
  new_union->union_def->switch_type = base_type;
  new_union->union_def->cases = NULL;
  context->cur_node = new_union;
  context->ref_next_union_case = &new_union->union_def->cases;
  context->ref_next_union_case_label = NULL;
*/
}

/*
static dds_tt_union_case_label_t *dds_tt_new_union_case_label(dds_tt_context_t *context)
{
  (void)context;
  assert(context != NULL);
  assert(context->ref_next_union_case != NULL);
  if ((*context->ref_next_union_case) == NULL) {
    // First label of case: create the case
    context->ref_next_declarator = NULL;
    (*context->ref_next_union_case) = (dds_tt_union_case_t*)os_malloc(sizeof(dds_tt_union_case_t));
    if ((*context->ref_next_union_case) == NULL) {
      abort();
    }
    (*context->ref_next_union_case)->labels = NULL;
    (*context->ref_next_union_case)->element_type = NULL;
    (*context->ref_next_union_case)->declarators = NULL;
    (*context->ref_next_union_case)->next = NULL;
    context->ref_next_union_case_label = &(*context->ref_next_union_case)->labels;
  }
  assert(context->ref_next_union_case_label != NULL);
  dds_tt_union_case_label_t *case_label = (dds_tt_union_case_label_t*)os_malloc(sizeof(dds_tt_union_case_label_t));
  if (case_label == NULL) {
    abort();
  }
  case_label->is_default = false;
  case_label->next = NULL;
  (*context->ref_next_union_case_label) = case_label;
  context->ref_next_union_case_label = &case_label->next;
  return case_label;
}
*/

void dds_tt_add_union_case_label(dds_tt_context_t *context, dds_tt_value_t value)
{
  //fprintf(stderr, "union_case_label\n");
  (void)context;
  (void)value;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_union));
  // Check if value matches switch type
  if (dds_tt_check_literal_type(value, context->cur_node->union_def->switch_type)) {
    / * FIXME: report error: type of value does not match * /
  }
  // Check if value is repeated
  for (dds_tt_union_case_t *union_case = context->cur_node->union_def->cases; union_case != 0; union_case = union_case->next) {
    for (dds_tt_union_case_label_t *label = union_case->labels; label != 0; label = label->next) {
      if (!label->is_default && dds_tt_compare_literal(label->value, value)) {
	/ * FIXME: report error: value repeated * /
      }
    }
  }
  dds_tt_union_case_label_t * case_label = dds_tt_new_union_case_label(context);
  case_label->value = value;
*/
}

void dds_tt_add_union_case_default(dds_tt_context_t *context)
{
  //fprintf(stderr, "union_case_default\n");
  (void)context;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_union));
  // Check if default is repeated
  for (dds_tt_union_case_t *union_case = context->cur_node->union_def->cases; union_case != 0; union_case = union_case->next) {
    for (dds_tt_union_case_label_t *label = union_case->labels; label != 0; label = label->next) {
      if (label->is_default) {
	/ * FIXME: report error: default repeated * /
      }
    }
  }
  dds_tt_union_case_label_t *case_label = dds_tt_new_union_case_label(context);
  case_label->is_default = true;
*/
}

void dds_tt_add_union_element(dds_tt_context_t *context, dds_tt_type_spec_t *type)
{
  //fprintf(stderr, "union_element\n");
  (void)context;
  (void)type;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_union));
  assert(context->ref_next_union_case != NULL);
  (*context->ref_next_union_case)->element_type = type;
  context->ref_next_declarator = &(*context->ref_next_union_case)->declarators;
  context->ref_next_union_case = &(*context->ref_next_union_case)->next;
  context->ref_next_union_case_label = NULL;
*/
}

void dds_tt_union_close(dds_tt_context_t *context)
{
  //fprintf(stderr, "union_close\n");
  (void)context;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_union));
  context->ref_next_union_case = NULL;
  context->ref_next_union_case_label = NULL;
  context->cur_node = context->cur_node->parent;
*/
}

void dds_tt_add_typedef_open(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_tt_typedef_set_type(dds_tt_context_t *context, dds_tt_type_spec_t *type)
{
  assert(context != NULL);
  (void)context;
  (void)type;
}

void dds_tt_typedef_close(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_tt_add_declarator(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  //fprintf(stderr, "declarator %s\n", name);
  (void)context;
  (void)name;
/*
  assert(context != NULL);
  if (context->ref_next_declarator == NULL) {
    / *FIXME: change into assert when done * /
    return;
  }
  (*context->ref_next_declarator) = dds_tt_new_definition(name, dds_tt_definition_declarator, context->cur_node);
  if ((*context->ref_next_declarator) == NULL) {
    abort();
  }
  context->ref_next_declarator = &(*context->ref_next_declarator)->next;
*/
}

void dds_tt_add_const_def(dds_tt_context_t *context, dds_tt_type_spec_t *base_type, dds_tt_identifier_t name, dds_tt_value_t value)
{
  assert(context != NULL);
  (void)context;
  (void)base_type;
  (void)name;
  (void)value;
}

void dds_tt_add_enum_open(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  (void)context;
  (void)name;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_module));
  dds_tt_definition_t *def = dds_tt_new_definition(name, dds_tt_definition_enum, context->cur_node);
  dds_tt_append_definition(def, &context->cur_node->module_def->definitions);
  def->enum_def = (dds_tt_enum_definition_t*)os_malloc(sizeof(dds_tt_enum_definition_t));
  if (def->enum_def == NULL) {
    abort();
  }
  def->enum_def->values = NULL;
  def->enum_def->nr_values = 0;
  context->cur_node = def;
*/
}

void dds_tt_add_enum_enumerator(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  (void)context;
  (void)name;
/*
  //fprintf(stderr, "add_enum_value %s\n", name);
  assert(cur_scope_is_definition_type(context, dds_tt_definition_enum));
  assert(context->cur_node->parent != NULL && context->cur_node->parent->type == dds_tt_definition_module);
  dds_tt_definition_t *def = dds_tt_new_definition(name, dds_tt_definition_enum_value, context->cur_node);
  dds_tt_append_definition(def, &context->cur_node->parent->module_def->definitions);
  def->enum_value_def = (dds_tt_enum_value_definition_t*)os_malloc(sizeof(dds_tt_enum_value_definition_t));
  if (def->enum_value_def == NULL) {
    abort();
  }
  def->enum_value_def->def = def;
  def->enum_value_def->enum_def = context->cur_node->enum_def;
  def->enum_value_def->next = NULL;
  def->enum_value_def->nr = context->cur_node->enum_def->nr_values++;

  dds_tt_enum_value_definition_t **ref_value = &context->cur_node->enum_def->values;
  while ((*ref_value) != NULL) {
    ref_value = &(*ref_value)->next;
  }
  (*ref_value) = def->enum_value_def;
  //fprintf(stderr, " added to %s\n", context->cur_node->name);
*/
}

void dds_tt_enum_close(dds_tt_context_t *context)
{
  (void)context;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_enum));
  context->cur_node = context->cur_node->parent;
*/
}

void dds_tt_add_array_open(dds_tt_context_t *context, dds_tt_identifier_t  name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_tt_add_array_size(dds_tt_context_t *context, dds_tt_value_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_tt_array_close(dds_tt_context_t* context)
{
  assert(context != NULL);
  (void)context;
}

void dds_tt_add_native(dds_tt_context_t* context, dds_tt_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_tt_add_annotation_open(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_tt_add_annotation_member_open(dds_tt_context_t *context, dds_tt_type_spec_t *base_type, dds_tt_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)base_type;
  (void)name;
}

void dds_tt_annotation_member_set_default(dds_tt_context_t *context, dds_tt_value_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_tt_annotation_member_close(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_tt_annotation_close(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_tt_add_annotation_appl_open(dds_tt_context_t *context, dds_tt_scoped_name_t *scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
}

void dds_tt_add_annotation_appl_expr(dds_tt_context_t *context, dds_tt_value_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_tt_add_annotation_appl_param(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_value_t value)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)value;
}

void dds_tt_annotation_appl_close(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

extern void dds_tt_add_const_definition(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_value_t value)
{
  (void)context;
  (void)name;
  (void)value;
/*
  assert(cur_scope_is_definition_type(context, dds_tt_definition_module));
  dds_tt_definition_t *def = dds_tt_new_definition(name, dds_tt_definition_const, context->cur_node);
  dds_tt_append_definition(def, &context->cur_node->module_def->definitions);
  def->const_def = (dds_tt_const_definition_t*)os_malloc(sizeof(dds_tt_const_definition_t));
  if (def->const_def == NULL) {
    abort();
  }
  def->const_def->value = value;
*/
}
  
dds_tt_const_definition_t *dds_tt_get_const_definition(dds_tt_definition_t *definitions, dds_tt_identifier_t name)
{
  (void)definitions;
  (void)name;
/*
  dds_tt_definition_t *definition = definitions;
  for (; definition != NULL; definition = definition->next) {
    if (strcmp(definition->name, name) == 0) {
      if (definition->type == dds_tt_definition_const) {
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

void dds_tt_add_bitset_open(dds_tt_context_t *context, dds_tt_identifier_t name, dds_tt_type_spec_t *opt_type)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)opt_type;
}

void dds_tt_add_bitset_field(dds_tt_context_t *context, dds_tt_value_t index)
{
  assert(context != NULL);
  (void)context;
  (void)index;
}

void dds_tt_add_bitset_field_to(dds_tt_context_t *context, dds_tt_value_t index, dds_tt_node_flags_t dest_type)
{
  assert(context != NULL);
  (void)context;
  (void)index;
  (void)dest_type;
}

void dds_tt_add_bitset_ident(dds_tt_context_t *context, dds_tt_identifier_t ident)
{
  assert(context != NULL);
  (void)context;
  (void)ident;
}

void dds_tt_bitset_close(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void dds_tt_add_bitmask_open(dds_tt_context_t *context, dds_tt_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void dds_tt_add_bitmask_value(dds_tt_context_t *context, dds_tt_identifier_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void dds_tt_bitmask_close(dds_tt_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

