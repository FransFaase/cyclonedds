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
	assert(0);
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
	assert(0);
	break;
    }
  } else {
    /* FIXME: other types + error reporting */
  }
}

idl_type_t *idl_new_basic_type(idl_context_t *context, idl_basic_type_t type)
{
  (void)context;
  (void)type;
  return 0;
}

idl_type_t *idl_new_sequence_type(idl_context_t *context, idl_type_t *base, idl_literal_t size)
{
  (void)context;
  (void)base;
  (void)size;
  return 0;
}

idl_type_t *idl_new_sequence_type_unbound(idl_context_t *context, idl_type_t *base)
{
  (void)context;
  (void)base;
  return 0;
}

idl_type_t *idl_new_string_type(idl_context_t *context, idl_literal_t size)
{
  (void)context;
  (void)size;
  return 0;
}

idl_type_t *idl_new_string_type_unbound(idl_context_t *context)
{
  (void)context;
  return 0;
}

idl_type_t *idl_new_fixed_type(idl_context_t *context, idl_literal_t digits, idl_literal_t precision)
{
  (void)context;
  (void)digits;
  (void)precision;
  return 0;
}

idl_type_t *idl_new_map_type(idl_context_t *context, idl_type_t *key_type, idl_type_t *value_type, idl_literal_t size)
{
  (void)context;
  (void)key_type;
  (void)value_type;
  (void)size;
  return 0;
}

idl_type_t *idl_new_map_type_unbound(idl_context_t *context, idl_type_t *key_type, idl_type_t *value_type)
{
  (void)context;
  (void)key_type;
  (void)value_type;
  return 0;
}

idl_scoped_name_t *idl_new_scoped_name(idl_context_t *context, idl_scoped_name_t* prev, bool top, idl_identifier_t  name)
{
  (void)context;
  (void)prev;
  (void)top;
  (void)name;
  return 0;
}

idl_basic_type_t idl_get_basic_type_of_scoped_name(idl_context_t *context, idl_scoped_name_t* scoped_name)
{
  (void)context;
  (void)scoped_name;
  return idl_boolean;
}

idl_bit_value_list_t *idl_new_bit_value_list(idl_context_t *context, idl_identifier_t name, idl_bit_value_list_t *next)
{
  (void)context;
  (void)name;
  (void)next;
  return 0;
}

extern idl_context_t* idl_create_context()
{
  idl_context_t *context = (idl_context_t*)os_malloc(sizeof(idl_context_t));
  context->ignore_yyerror = false;
  context->root_definitions = 0;
  context->cur_definitions = &context->root_definitions;
  return context;
}

extern void idl_context_set_ignore_yyerror(idl_context_t *context, bool ignore_yyerror)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  context->ignore_yyerror = ignore_yyerror;
}

extern bool idl_context_get_ignore_yyerror(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return false;
  }
  return context->ignore_yyerror;
}

extern void idl_free_context(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  os_free(context);
}

extern void idl_module_open(idl_context_t *context, idl_identifier_t name)
{ 
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name; 
}

extern void idl_module_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_struct_forward(idl_context_t *context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_struct_open(idl_context_t *context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_struct_extension_open(idl_context_t *context, idl_identifier_t name, idl_scoped_name_t *scoped_name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
  (void)scoped_name;
}

void idl_add_struct_member(idl_context_t *context, idl_type_t *type)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)type;
}

void idl_struct_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_struct_empty_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_union_forward(idl_context_t *context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_union_open(idl_context_t *context, idl_identifier_t name, idl_basic_type_t basic_type)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
  (void)basic_type;
}

void idl_add_union_case_label(idl_context_t *context, idl_literal_t value)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)value;
}

void idl_add_union_case_default(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_union_element(idl_context_t *context, idl_type_t *type)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)type;
}

void idl_union_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_typedef_open(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_typedef_set_type(idl_context_t *context, idl_type_t *type)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)type;
}

void idl_typedef_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_declarator(idl_context_t *context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_const_def(idl_context_t *context, idl_basic_type_t basic_type, idl_identifier_t name, idl_literal_t value)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)basic_type;
  (void)name;
  (void)value;
}

void idl_add_enum_open(idl_context_t *context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_enum_enumerator(idl_context_t *context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_enum_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_array_open(idl_context_t *context, idl_identifier_t  name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_array_size(idl_context_t *context, idl_literal_t value)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)value;
}

void idl_array_close(idl_context_t* context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_native(idl_context_t* context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_annotation_open(idl_context_t *context, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
}

void idl_add_annotation_member_open(idl_context_t *context, idl_basic_type_t basic_type, idl_identifier_t name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)basic_type;
  (void)name;
}

void idl_annotation_member_set_default(idl_context_t *context, idl_literal_t value)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)value;
}

void idl_annotation_member_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_annotation_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_annotation_appl_open(idl_context_t *context, idl_scoped_name_t *scoped_name)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)scoped_name;
}

void idl_add_annotation_appl_expr(idl_context_t *context, idl_literal_t value)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)value;
}

void idl_add_annotation_appl_param(idl_context_t *context, idl_identifier_t name, idl_literal_t value)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
  (void)value;
}

void idl_annotation_appl_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

static idl_definition_t *idl_new_definition(idl_identifier_t  name, idl_definition_type_t type)
{
  idl_definition_t *result = (idl_definition_t*)os_malloc(sizeof(idl_definition_t));
  result->name = os_strdup(name);
  result->type = type;
  result->next = 0;
  /* result->def not initialized */
  return result;
}

extern void idl_add_const_definition(idl_context_t *context, idl_identifier_t name, idl_literal_t value)
{
  idl_definition_t **ref_definitions = 0;

  assert(context != 0);
  if (context == 0) {
    return;
  }

  ref_definitions = context->cur_definitions;
  while (*ref_definitions != 0) {
    ref_definitions = &(*ref_definitions)->next;
  }

  *ref_definitions = idl_new_definition(name, idl_definition_const);
  (*ref_definitions)->def.const_def = (idl_const_definition_t*)os_malloc(sizeof(idl_const_definition_t));
  (*ref_definitions)->def.const_def->value = value;
}
  
idl_const_definition_t *idl_get_const_definition(idl_definition_t *definitions, idl_identifier_t name)
{
  idl_definition_t *definition = definitions;
  for (; definition != 0; definition = definition->next) {
    if (strcmp(definition->name, name) == 0) {
      if (definition->type == idl_definition_const) {
        return definition->def.const_def;
      } else {
	/* FIXME: report error that definition is not a const definition */
	return 0;
      }
    }
  }
  return 0;
}

void idl_add_bitset_open(idl_context_t *context, idl_identifier_t name, idl_type_t *opt_type)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
  (void)opt_type;
}

void idl_add_bitset_field(idl_context_t *context, idl_literal_t index)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)index;
}

void idl_add_bitset_field_to(idl_context_t *context, idl_literal_t index, idl_basic_type_t dest_type)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)index;
  (void)dest_type;
}

void idl_add_bitset_ident(idl_context_t *context, idl_identifier_t ident)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)ident;
}

void idl_bitset_close(idl_context_t *context)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
}

void idl_add_bitmask(idl_context_t *context, idl_identifier_t name, idl_bit_value_list_t *bit_value_list)
{
  assert(context != 0);
  if (context == 0) {
    return;
  }
  (void)name;
  (void)bit_value_list;
}


