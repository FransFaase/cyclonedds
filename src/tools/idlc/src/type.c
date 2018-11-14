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

#include "type.h"
#include "type_create.h"
#include "type_priv.h"

#include "os/os.h"

void idl_print_literal(FILE* f, idl_literal_t literal)
{
  switch (literal.type) {
    case idl_integer_literal:
      fprintf(f, "integer %lld", literal.value.llng);
      break;
    case idl_string_literal:
      fprintf(f, "string '%s'", literal.value.str);
      break;
    case idl_wide_string_literal:
      fprintf(f, "wstring");
      break;
    case idl_character_literal:
      fprintf(f, "char '%c'", literal.value.chr);
      break;
    case idl_wide_character_literal:
      fprintf(f, "wchar");
      break;
    case idl_fixed_pt_literal:
      fprintf(f, "fixed");
      break;
    case idl_floating_pt_literal:
      fprintf(f, "float %Lf", literal.value.ldbl);
      break;
    case idl_boolean_literal:
      fprintf(f, "boolean: %s", literal.value.bln ? "true" : "false");
      break;
    default:
      fprintf(f, "illegal-type");
      break;
  }
}

static int idl_compare_literal(idl_literal_t lhs, idl_literal_t rhs)
{
  if (lhs.type < rhs.type) {
    return -1;
  }
  if (lhs.type > rhs.type) {
    return +1;
  }
  switch (lhs.type) {
    case idl_integer_literal:
      return lhs.value.llng < rhs.value.llng ? -1 : lhs.value.llng > rhs.value.llng ? +1 : 0;
      break;
    case idl_string_literal:
      return strcmp(lhs.value.str, rhs.value.str);
      break;
    case idl_wide_string_literal:
      assert(false);
      break;
    case idl_character_literal:
      return lhs.value.chr < rhs.value.chr ? -1 : lhs.value.chr > rhs.value.chr ? +1 : 0;
      break;
    case idl_wide_character_literal:
      assert(false);
      break;
    case idl_fixed_pt_literal:
      assert(false);
      break;
    case idl_floating_pt_literal:
      return lhs.value.ldbl < rhs.value.ldbl ? -1 : lhs.value.ldbl > rhs.value.ldbl ? +1 : 0;
      break;
    case idl_boolean_literal:
      return (!lhs.value.bln && rhs.value.bln) ? -1 : (lhs.value.bln && !rhs.value.bln) ? +1 : 0;
      break;
    default:
      assert(false);
      break;
  }
  return -1;
}

static bool idl_check_literal_type(idl_literal_t value, idl_basic_type_t basic_type)
{
  switch (value.type) {
    case idl_integer_literal:
      switch (basic_type) {
	case idl_int8:
	  return -(1L << 7) <= value.value.llng && value.value.llng < (1L << 7);
	  break;
	case idl_short:
	  return -(1L << 15) <= value.value.llng && value.value.llng < (1L << 15);
	  break;
	case idl_long:
	  return -(1LL << 31) <= value.value.llng && value.value.llng < (1LL << 31);
	  break;
	case idl_longlong:
	  return true;
	  break;
	case idl_uint8:
	  return 0 <= value.value.llng && value.value.llng < (1L << 8);
	  break;
	case idl_ushort:
	  return 0 <= value.value.llng && value.value.llng < (1L << 16);
	  break;
	case idl_ulong:
	  return 0 <= value.value.llng && value.value.llng < (1LL << 32);
	  return true;
	  break;
	case idl_ulonglong:
	  return 0 <= value.value.llng;
	  break;
	default:
	  break;
      }
      break;
    case idl_character_literal:
      return basic_type == idl_char;
      break;
    case idl_wide_character_literal:
      return basic_type == idl_wchar;
      break;
    case idl_fixed_pt_literal:
      return basic_type == idl_fixed;
      break;
    case idl_floating_pt_literal:
      return basic_type == idl_float;
      break;
    case idl_boolean_literal:
      return basic_type == idl_boolean;
      break;
    default:
      assert(false);
      break;
  }
  return false;
}

extern void idl_eval_unary_oper(idl_operator_type_t operator_type, idl_literal_t operand, idl_literal_t *result)
{
  if (operand.type == idl_integer_literal) {
    result->type = idl_integer_literal;
    switch (operator_type) {
      case idl_operator_minus:
	result->value.llng = -operand.value.llng;
	break;
      case idl_operator_plus:
	result->value.llng = operand.value.llng;
	break;
      case idl_operator_inv:
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

extern void idl_eval_binary_oper(idl_operator_type_t operator_type, idl_literal_t lhs, idl_literal_t rhs, idl_literal_t *result)
{
  if (lhs.type == idl_integer_literal && rhs.type == idl_integer_literal) {
    result->type = idl_integer_literal;
    switch (operator_type) {
      case idl_operator_or:
	result->value.llng = lhs.value.llng | rhs.value.llng;
	break;
      case idl_operator_xor:
	result->value.llng = lhs.value.llng ^ rhs.value.llng;
	break;
      case idl_operator_and:
	result->value.llng = lhs.value.llng & rhs.value.llng;
	break;
      case idl_operator_shift_left:
	result->value.llng = lhs.value.llng << rhs.value.llng;
	break;
      case idl_operator_shift_right:
	result->value.llng = lhs.value.llng >> rhs.value.llng;
	break;
      case idl_operator_add:
	result->value.llng = lhs.value.llng + rhs.value.llng;
	break;
      case idl_operator_sub:
	result->value.llng = lhs.value.llng - rhs.value.llng;
	break;
      case idl_operator_times:
	result->value.llng = lhs.value.llng * rhs.value.llng;
	break;
      case idl_operator_div:
	result->value.llng = lhs.value.llng / rhs.value.llng;
	break;
      case idl_operator_mod:
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

idl_type_t *idl_new_basic_type(idl_context_t *context, idl_basic_type_t basic_type)
{
  assert(context != NULL);
  (void)context;
  (void)basic_type;
  return NULL;
}

idl_type_t *idl_new_sequence_type(idl_context_t *context, idl_type_t *base, idl_literal_t size)
{
  assert(context != NULL);
  (void)context;
  (void)base;
  (void)size;
  return NULL;
}

idl_type_t *idl_new_sequence_type_unbound(idl_context_t *context, idl_type_t *base)
{
  assert(context != NULL);
  (void)context;
  (void)base;
  return NULL;
}

idl_type_t *idl_new_string_type(idl_context_t *context, idl_literal_t size)
{
  assert(context != NULL);
  (void)context;
  (void)size;
  return NULL;
}

idl_type_t *idl_new_string_type_unbound(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
  return NULL;
}

idl_type_t *idl_new_fixed_type(idl_context_t *context, idl_literal_t digits, idl_literal_t precision)
{
  assert(context != NULL);
  (void)context;
  (void)digits;
  (void)precision;
  return NULL;
}

idl_type_t *idl_new_map_type(idl_context_t *context, idl_type_t *key_type, idl_type_t *value_type, idl_literal_t size)
{
  assert(context != NULL);
  (void)context;
  (void)key_type;
  (void)value_type;
  (void)size;
  return NULL;
}

idl_type_t *idl_new_map_type_unbound(idl_context_t *context, idl_type_t *key_type, idl_type_t *value_type)
{
  assert(context != NULL);
  (void)context;
  (void)key_type;
  (void)value_type;
  return NULL;
}

idl_scoped_name_t *idl_new_scoped_name(idl_context_t *context, idl_scoped_name_t* prev, bool top, idl_identifier_t  name)
{
  assert(context != NULL);
  (void)context;
  (void)prev;
  (void)top;
  (void)name;
  return NULL;
}

idl_basic_type_t idl_get_basic_type_of_scoped_name(idl_context_t *context, idl_scoped_name_t* scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
  return idl_boolean;
}

idl_bit_value_list_t *idl_new_bit_value_list(idl_context_t *context, idl_identifier_t name, idl_bit_value_list_t *next)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)next;
  return NULL;
}

static idl_definition_t *idl_new_definition(idl_identifier_t  name, idl_definition_type_t type, idl_definition_t *parent)
{
  idl_definition_t *result = (idl_definition_t*)os_malloc(sizeof(idl_definition_t));
  if (result == NULL) {
    abort();
  }
  result->name = os_strdup(name);
  if (result->name == NULL) {
    abort();
  }
  result->type = type;
  result->parent = parent;
  result->next = NULL;
  /* result->def not initialized */
  return result;
}

static void idl_append_definition(idl_definition_t *definition, idl_definition_t **ref_definition)
{
  while ((*ref_definition) != NULL) {
    ref_definition = &(*ref_definition)->next;
  }
  (*ref_definition) = definition;
}

static idl_definition_t *idl_new_module_definition(idl_identifier_t name, idl_definition_t *parent)
{
  idl_definition_t *def = idl_new_definition(name, idl_definition_module, parent);
  def->module_def = (idl_module_definition_t*)os_malloc(sizeof(idl_module_definition_t));
  if (def->module_def == NULL) {
    abort();
  }
  def->module_def->definitions = NULL;
  if (parent != NULL && parent->type == idl_definition_module) {
    idl_append_definition(def, &parent->module_def->definitions);
  }
  return def;
}

extern idl_context_t* idl_create_context()
{
  idl_context_t *context = (idl_context_t*)os_malloc(sizeof(idl_context_t));
  if (context == NULL) {
    abort();
  }
  context->ignore_yyerror = false;
  context->root_scope = idl_new_module_definition("", NULL);
  context->cur_scope = context->root_scope;
  context->ref_next_declarator = NULL;
  context->ref_next_union_case = NULL;
  context->ref_next_union_case_label = NULL;
  return context;
}

extern void idl_context_set_ignore_yyerror(idl_context_t *context, bool ignore_yyerror)
{
  assert(context != NULL);
  context->ignore_yyerror = ignore_yyerror;
}

extern bool idl_context_get_ignore_yyerror(idl_context_t *context)
{
  assert(context != NULL);
  return context->ignore_yyerror;
}

extern void idl_free_context(idl_context_t *context)
{
  assert(context != NULL);
  os_free(context);
}

static bool cur_scope_is_definition_type(idl_context_t *context, idl_definition_type_t type)
{
  assert(context != NULL && context->cur_scope != NULL);
  return context->cur_scope->type == type;
}

extern void idl_module_open(idl_context_t *context, idl_identifier_t name)
{
  //fprintf(stderr, "idl_module_open %s\n", name);	
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  idl_definition_t *def = context->cur_scope->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == idl_definition_module && strcmp(def->name, name) == 0) {
      // Open existing scope
      context->cur_scope = def;
      return;
    }
  }
  // Create new scope
  idl_definition_t *new_module = idl_new_module_definition(name, context->cur_scope);
  context->cur_scope = new_module;
  //fprintf(stderr, " cur scope %s\n", context->cur_scope->name);
}

extern void idl_module_close(idl_context_t *context)
{
  //fprintf(stderr, "idl_module_close\n");
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  assert(context->cur_scope->parent != NULL);
  context->cur_scope = context->cur_scope->parent;
  //fprintf(stderr, " cur scope %s\n", context->cur_scope->name);
}

void idl_add_struct_forward(idl_context_t *context, idl_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  idl_definition_t *def = context->cur_scope->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == idl_definition_struct_forward && strcmp(def->name, name) == 0) {
      /* FIXME: report warning: repeated forward declarationn */
      return;
    } else if (def->type == idl_definition_struct && strcmp(def->name, name) == 0) {
      /* FIXME: report error: forward declaration after definition */
      return;
    }
  }

  idl_definition_t *new_struct_forward = idl_new_definition(name, idl_definition_struct_forward, context->cur_scope);
  new_struct_forward->struct_forward_def = (idl_struct_forward_t*)os_malloc(sizeof(idl_struct_forward_t));
  if (new_struct_forward->struct_forward_def == NULL) {
    abort();
  }
  new_struct_forward->struct_forward_def->defined = false;
}

void idl_add_struct_open(idl_context_t *context, idl_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  idl_definition_t *def = context->cur_scope->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == idl_definition_struct_forward && strcmp(def->name, name) == 0) {
      def->struct_forward_def->defined = true;
    } else if (def->type == idl_definition_struct && strcmp(def->name, name) == 0) {
      /* FIXME: report error: repeated definition */
    }
  }

  idl_definition_t *new_struct = idl_new_definition(name, idl_definition_struct, context->cur_scope);
  idl_append_definition(new_struct, &context->cur_scope->module_def->definitions);
  new_struct->struct_def = (idl_struct_t*)os_malloc(sizeof(idl_struct_t));
  if (new_struct->struct_def == NULL) {
    abort();
  }
  new_struct->struct_def->members = NULL;
  context->cur_scope = new_struct;
}

void idl_add_struct_extension_open(idl_context_t *context, idl_identifier_t name, idl_scoped_name_t *scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)scoped_name;
}

void idl_add_struct_member(idl_context_t *context, idl_type_t *type)
{
  assert(cur_scope_is_definition_type(context, idl_definition_struct));
  context->ref_next_declarator = NULL;
  idl_struct_member_t **ref_member = &context->cur_scope->struct_def->members;
  while ((*ref_member) != NULL) {
    ref_member = &(*ref_member)->next;
  }
  (*ref_member) = (idl_struct_member_t*)os_malloc(sizeof(idl_struct_member_t));
  if ((*ref_member) == NULL) {
    abort();
  }
  (*ref_member)->type = type;
  (*ref_member)->declarators = NULL;
  (*ref_member)->next = NULL;
  context->ref_next_declarator = &(*ref_member)->declarators;
}

void idl_struct_close(idl_context_t *context)
{
  assert(cur_scope_is_definition_type(context, idl_definition_struct));
  context->ref_next_declarator = NULL;
  context->cur_scope = context->cur_scope->parent;
}

void idl_struct_empty_close(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void idl_add_union_forward(idl_context_t *context, idl_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  idl_definition_t *def = context->cur_scope->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == idl_definition_union_forward && strcmp(def->name, name) == 0) {
      /* FIXME: report warning: repeated forward declarationn */
      return;
    } else if (def->type == idl_definition_union && strcmp(def->name, name) == 0) {
      /* FIXME: report error: forward declaration after definition */
      return;
    }
  }

  idl_definition_t *new_union_forward = idl_new_definition(name, idl_definition_union_forward, context->cur_scope);
  new_union_forward->union_forward_def = (idl_union_forward_t*)os_malloc(sizeof(idl_union_forward_t));
  if (new_union_forward->union_forward_def == NULL) {
    abort();
  }
  new_union_forward->union_forward_def->defined = false;
}

void idl_add_union_open(idl_context_t *context, idl_identifier_t name, idl_basic_type_t basic_type)
{
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  idl_definition_t *def = context->cur_scope->module_def->definitions;
  for (; def != NULL; def = def->next) {
    if (def->type == idl_definition_union_forward && strcmp(def->name, name) == 0) {
      def->union_forward_def->defined = true;
    } else if (def->type == idl_definition_union && strcmp(def->name, name) == 0) {
      /* FIXME: report error: repeated definition */
    }
  }

  idl_definition_t *new_union = idl_new_definition(name, idl_definition_union, context->cur_scope);
  idl_append_definition(new_union, &context->cur_scope->module_def->definitions);
  new_union->union_def = (idl_union_t*)os_malloc(sizeof(idl_union_t));
  if (new_union->union_def == NULL) {
    abort();
  }
  new_union->union_def->switch_type = basic_type;
  new_union->union_def->cases = NULL;
  context->cur_scope = new_union;
  context->ref_next_union_case = &new_union->union_def->cases;
  context->ref_next_union_case_label = NULL;
}

static idl_union_case_label_t *idl_new_union_case_label(idl_context_t *context)
{
  assert(context != NULL);
  assert(context->ref_next_union_case != NULL);
  if ((*context->ref_next_union_case) == NULL) {
    // First label of case: create the case
    context->ref_next_declarator = NULL;
    (*context->ref_next_union_case) = (idl_union_case_t*)os_malloc(sizeof(idl_union_case_t));
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
  idl_union_case_label_t *case_label = (idl_union_case_label_t*)os_malloc(sizeof(idl_union_case_label_t));
  if (case_label == NULL) {
    abort();
  }
  case_label->is_default = false;
  case_label->next = NULL;
  (*context->ref_next_union_case_label) = case_label;
  context->ref_next_union_case_label = &case_label->next;
  return case_label;
}

void idl_add_union_case_label(idl_context_t *context, idl_literal_t value)
{
  assert(cur_scope_is_definition_type(context, idl_definition_union));
  // Check if value matches switch type
  if (idl_check_literal_type(value, context->cur_scope->union_def->switch_type)) {
    /* FIXME: report error: type of value does not match */
  }
  // Check if value is repeated
  for (idl_union_case_t *union_case = context->cur_scope->union_def->cases; union_case != 0; union_case = union_case->next) {
    for (idl_union_case_label_t *label = union_case->labels; label != 0; label = label->next) {
      if (!label->is_default && idl_compare_literal(label->value, value)) {
	/* FIXME: report error: value repeated */
      }
    }
  }
  idl_union_case_label_t * case_label = idl_new_union_case_label(context);
  case_label->value = value;
}

void idl_add_union_case_default(idl_context_t *context)
{
  assert(cur_scope_is_definition_type(context, idl_definition_union));
  // Check if default is repeated
  for (idl_union_case_t *union_case = context->cur_scope->union_def->cases; union_case != 0; union_case = union_case->next) {
    for (idl_union_case_label_t *label = union_case->labels; label != 0; label = label->next) {
      if (label->is_default) {
	/* FIXME: report error: default repeated */
      }
    }
  }
  idl_union_case_label_t *case_label = idl_new_union_case_label(context);
  case_label->is_default = true;
}

void idl_add_union_element(idl_context_t *context, idl_type_t *type)
{
  assert(cur_scope_is_definition_type(context, idl_definition_union));
  assert(context->ref_next_union_case != NULL);
  (*context->ref_next_union_case)->element_type = type;
  context->ref_next_declarator = &(*context->ref_next_union_case)->declarators;
  context->ref_next_union_case = &(*context->ref_next_union_case)->next;
  context->ref_next_union_case_label = NULL;
}

void idl_union_close(idl_context_t *context)
{
  assert(cur_scope_is_definition_type(context, idl_definition_union));
  context->ref_next_union_case = NULL;
  context->ref_next_union_case_label = NULL;
  context->cur_scope = context->cur_scope->parent;
}

void idl_add_typedef_open(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void idl_typedef_set_type(idl_context_t *context, idl_type_t *type)
{
  assert(context != NULL);
  (void)context;
  (void)type;
}

void idl_typedef_close(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void idl_add_declarator(idl_context_t *context, idl_identifier_t name)
{
  assert(context != NULL);
  if (context->ref_next_declarator == NULL) {
    /*FIXME: change into assert when done */
    return;
  }
  (*context->ref_next_declarator) = idl_new_definition(name, idl_definition_declarator, context->cur_scope);
  if ((*context->ref_next_declarator) == NULL) {
    abort();
  }
  context->ref_next_declarator = &(*context->ref_next_declarator)->next;
}

void idl_add_const_def(idl_context_t *context, idl_basic_type_t basic_type, idl_identifier_t name, idl_literal_t value)
{
  assert(context != NULL);
  (void)context;
  (void)basic_type;
  (void)name;
  (void)value;
}

void idl_add_enum_open(idl_context_t *context, idl_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  idl_definition_t *def = idl_new_definition(name, idl_definition_enum, context->cur_scope);
  idl_append_definition(def, &context->cur_scope->module_def->definitions);
  def->enum_def = (idl_enum_definition_t*)os_malloc(sizeof(idl_enum_definition_t));
  if (def->enum_def == NULL) {
    abort();
  }
  def->enum_def->values = NULL;
  def->enum_def->nr_values = 0;
  context->cur_scope = def;
}

void idl_add_enum_value(idl_context_t *context, idl_identifier_t name)
{
  //fprintf(stderr, "add_enum_value %s\n", name);
  assert(cur_scope_is_definition_type(context, idl_definition_enum));
  assert(context->cur_scope->parent != NULL && context->cur_scope->parent->type == idl_definition_module);
  idl_definition_t *def = idl_new_definition(name, idl_definition_enum_value, context->cur_scope);
  idl_append_definition(def, &context->cur_scope->parent->module_def->definitions);
  def->enum_value_def = (idl_enum_value_definition_t*)os_malloc(sizeof(idl_enum_value_definition_t));
  if (def->enum_value_def == NULL) {
    abort();
  }
  def->enum_value_def->def = def;
  def->enum_value_def->enum_def = context->cur_scope->enum_def;
  def->enum_value_def->next = NULL;
  def->enum_value_def->nr = context->cur_scope->enum_def->nr_values++;

  idl_enum_value_definition_t **ref_value = &context->cur_scope->enum_def->values;
  while ((*ref_value) != NULL) {
    ref_value = &(*ref_value)->next;
  }
  (*ref_value) = def->enum_value_def;
  //fprintf(stderr, " added to %s\n", context->cur_scope->name);
}

void idl_enum_close(idl_context_t *context)
{
  assert(cur_scope_is_definition_type(context, idl_definition_enum));
  context->cur_scope = context->cur_scope->parent;
}

void idl_add_array_open(idl_context_t *context, idl_identifier_t  name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void idl_add_array_size(idl_context_t *context, idl_literal_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void idl_array_close(idl_context_t* context)
{
  assert(context != NULL);
  (void)context;
}

void idl_add_native(idl_context_t* context, idl_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void idl_add_annotation_open(idl_context_t *context, idl_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)name;
}

void idl_add_annotation_member_open(idl_context_t *context, idl_basic_type_t basic_type, idl_identifier_t name)
{
  assert(context != NULL);
  (void)context;
  (void)basic_type;
  (void)name;
}

void idl_annotation_member_set_default(idl_context_t *context, idl_literal_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void idl_annotation_member_close(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void idl_annotation_close(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void idl_add_annotation_appl_open(idl_context_t *context, idl_scoped_name_t *scoped_name)
{
  assert(context != NULL);
  (void)context;
  (void)scoped_name;
}

void idl_add_annotation_appl_expr(idl_context_t *context, idl_literal_t value)
{
  assert(context != NULL);
  (void)context;
  (void)value;
}

void idl_add_annotation_appl_param(idl_context_t *context, idl_identifier_t name, idl_literal_t value)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)value;
}

void idl_annotation_appl_close(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

extern void idl_add_const_definition(idl_context_t *context, idl_identifier_t name, idl_literal_t value)
{
  assert(cur_scope_is_definition_type(context, idl_definition_module));
  idl_definition_t *def = idl_new_definition(name, idl_definition_const, context->cur_scope);
  idl_append_definition(def, &context->cur_scope->module_def->definitions);
  def->const_def = (idl_const_definition_t*)os_malloc(sizeof(idl_const_definition_t));
  if (def->const_def == NULL) {
    abort();
  }
  def->const_def->value = value;
}
  
idl_const_definition_t *idl_get_const_definition(idl_definition_t *definitions, idl_identifier_t name)
{
  idl_definition_t *definition = definitions;
  for (; definition != NULL; definition = definition->next) {
    if (strcmp(definition->name, name) == 0) {
      if (definition->type == idl_definition_const) {
        return definition->const_def;
      } else {
	/* FIXME: report error that definition is not a const definition */
	return NULL;
      }
    }
  }
  return NULL;
}

void idl_add_bitset_open(idl_context_t *context, idl_identifier_t name, idl_type_t *opt_type)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)opt_type;
}

void idl_add_bitset_field(idl_context_t *context, idl_literal_t index)
{
  assert(context != NULL);
  (void)context;
  (void)index;
}

void idl_add_bitset_field_to(idl_context_t *context, idl_literal_t index, idl_basic_type_t dest_type)
{
  assert(context != NULL);
  (void)context;
  (void)index;
  (void)dest_type;
}

void idl_add_bitset_ident(idl_context_t *context, idl_identifier_t ident)
{
  assert(context != NULL);
  (void)context;
  (void)ident;
}

void idl_bitset_close(idl_context_t *context)
{
  assert(context != NULL);
  (void)context;
}

void idl_add_bitmask(idl_context_t *context, idl_identifier_t name, idl_bit_value_list_t *bit_value_list)
{
  assert(context != NULL);
  (void)context;
  (void)name;
  (void)bit_value_list;
}


