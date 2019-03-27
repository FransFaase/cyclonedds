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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "dds/ddsrt/retcode.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/string.h"
#include "dds/ddsts/typetree.h"
#include "tt_create.h"


typedef struct array_size array_size_t;
struct array_size {
  unsigned long long size;
  array_size_t *next;
};

struct dds_context {
  ddsts_type_t *root_type;
  ddsts_type_t *cur_type;
  ddsts_type_t *type_for_declarator;
  array_size_t *array_sizes;
  dds_retcode_t retcode;
  ddsts_identifier_t dangling_identifier;
  void (*error_func)(int line, int column, const char *msg);
};

extern bool dds_new_base_type(dds_context_t *context, ddsts_flags_t flags, ddsts_type_t **result)
{
  assert(context != NULL);
  ddsts_type_t *base_type;
  dds_retcode_t rc = ddsts_create_base_type(flags, &base_type);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  *result = base_type;
  return true;
}

static bool new_sequence(dds_context_t *context, ddsts_type_t *element_type, unsigned long long max, ddsts_type_t **result)
{
  assert(context != NULL);
  ddsts_type_t *sequence;
  dds_retcode_t rc = ddsts_create_sequence(element_type, max, &sequence);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  *result = sequence;
  return true;
}

extern bool dds_new_sequence(dds_context_t *context, ddsts_type_t *base, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_sequence(context, base, size->value.ullng, result);
}

extern bool dds_new_sequence_unbound(dds_context_t *context, ddsts_type_t *base, ddsts_type_t **result)
{
  return new_sequence(context, base, 0ULL, result);
}

static bool new_string(dds_context_t *context, ddsts_flags_t flags, unsigned long long max, ddsts_type_t **result)
{
  assert(context != NULL);
  ddsts_type_t *string;
  dds_retcode_t rc = ddsts_create_string(flags, max, &string);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  *result = string;
  return true;
}

extern bool dds_new_string(dds_context_t *context, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_string(context, DDSTS_STRING, size->value.ullng, result);
}

extern bool dds_new_string_unbound(dds_context_t *context, ddsts_type_t **result)
{
  return new_string(context, DDSTS_STRING, 0ULL, result);
}

extern bool dds_new_wide_string(dds_context_t *context, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_string(context, DDSTS_WIDE_STRING, size->value.ullng, result);
}

extern bool dds_new_wide_string_unbound(dds_context_t *context, ddsts_type_t **result)
{
  return new_string(context, DDSTS_WIDE_STRING, 0ULL, result);
}

extern bool dds_new_fixed_pt(dds_context_t *context, ddsts_literal_t *digits, ddsts_literal_t *fraction_digits, ddsts_type_t **result)
{
  assert(context != NULL);
  assert(digits->flags == DDSTS_ULONGLONG);
  assert(fraction_digits->flags == DDSTS_ULONGLONG);
  ddsts_type_t *fixed_pt;
  dds_retcode_t rc = ddsts_create_fixed_pt(digits->value.ullng, fraction_digits->value.ullng, &fixed_pt);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  *result = fixed_pt;
  return true;
}

static bool new_map(dds_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, unsigned long long max, ddsts_type_t **result)
{
  ddsts_type_t *map;
  dds_retcode_t rc = ddsts_create_map(key_type, value_type, max, &map);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  *result = map;
  return true;
}

extern bool dds_new_map(dds_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_map(context, key_type, value_type, size->value.ullng, result);
}

extern bool dds_new_map_unbound(dds_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, ddsts_type_t **result)
{
  return new_map(context, key_type, value_type, 0UL, result);
}

struct dds_scoped_name {
  const char* name;
  bool top;
  dds_scoped_name_t *next;
};

extern bool dds_new_scoped_name(dds_context_t *context, dds_scoped_name_t* prev, bool top, ddsts_identifier_t name, dds_scoped_name_t **result)
{
  assert(context != NULL);
  assert(context->dangling_identifier == name);
  dds_scoped_name_t *scoped_name = (dds_scoped_name_t*)ddsrt_malloc(sizeof(dds_scoped_name_t));
  if (scoped_name == NULL) {
    context->retcode = DDS_RETCODE_OUT_OF_RESOURCES;
    return false;
  }
  context->dangling_identifier = NULL;
  scoped_name->name = name;
  scoped_name->top = top;
  scoped_name->next = NULL;
  if (prev == NULL) {
    *result = scoped_name;
  }
  else {
    dds_scoped_name_t **ref_scoped_name = &prev->next;
    while (*ref_scoped_name != NULL) {
      ref_scoped_name = &(*ref_scoped_name)->next;
    }
    *ref_scoped_name = scoped_name;
    *result = prev;
  }
  return true;
}

static bool resolve_scoped_name(dds_context_t *context, dds_scoped_name_t *scoped_name, ddsts_type_t **result)
{
  assert(context != NULL);
  assert(scoped_name != NULL);
  ddsts_type_t *cur_type = scoped_name->top ? context->root_type : context->cur_type;
  for (; cur_type != NULL; cur_type = cur_type->type.parent) {
    ddsts_type_t *found_node = cur_type;
    dds_scoped_name_t *cur_scoped_name;
    for (cur_scoped_name = scoped_name; cur_scoped_name != NULL && found_node != NULL; cur_scoped_name = cur_scoped_name->next) {
      ddsts_type_t *child =  DDSTS_IS_TYPE(found_node, DDSTS_MODULE)
                           ? found_node->module.members
                           : found_node->struct_def.members;
      for (; child != NULL; child = child->type.next) {
        if (DDSTS_IS_DEFINITION(child) && strcmp(child->type.name, cur_scoped_name->name) == 0) {
          break;
        }
      }
      found_node = child;
    }
    if (found_node != NULL) {
      *result = found_node;
      return true;
    }
  }
  /* Could not resolve scoped name */
  *result = NULL;
  return false;
}

extern void dds_free_scoped_name(dds_scoped_name_t *scoped_name)
{
  while (scoped_name != NULL) {
    dds_scoped_name_t *next = scoped_name->next;
    ddsrt_free((void*)scoped_name->name);
    ddsrt_free((void*)scoped_name);
    scoped_name = next;
  }
}

extern bool dds_get_type_from_scoped_name(dds_context_t *context, dds_scoped_name_t *scoped_name, ddsts_type_t **result)
{
  ddsts_type_t *definition;
  resolve_scoped_name(context, scoped_name, &definition);
  dds_free_scoped_name(scoped_name);
  if (definition == NULL) {
    /* Create a Boolean type, just to prevent a NULL pointer */
    return dds_new_base_type(context, DDSTS_BOOLEAN, result);
  }
  *result = definition;
  return true;
}

static bool new_module_definition(dds_context_t *context, ddsts_identifier_t name, ddsts_type_t *parent, ddsts_type_t **result)
{
  assert(context != NULL);
  assert(context->dangling_identifier == name);
  ddsts_type_t *module;
  dds_retcode_t rc = ddsts_create_module(name, &module);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  context->dangling_identifier = NULL;
  ddsts_module_add_member(parent, module);
  *result = module;
  return true;
}

extern dds_context_t* dds_create_context()
{
  dds_context_t *context = (dds_context_t*)ddsrt_malloc(sizeof(dds_context_t));
  if (context == NULL) {
    return NULL;
  }
  context->error_func = NULL;
  context->dangling_identifier = NULL;
  ddsts_type_t *module;
  if (!new_module_definition(context, NULL, NULL, &module)) {
    ddsrt_free(context);
    return NULL;
  }
  context->root_type = module;
  context->cur_type = context->root_type;
  context->type_for_declarator = NULL;
  context->array_sizes = NULL;
  context->retcode = DDS_RETCODE_ERROR;
  return context;
}

extern void dds_context_error(dds_context_t *context, int line, int column, const char *msg)
{
  assert(context != NULL);
  if (context->error_func != NULL) {
    context->error_func(line, column, msg);
  }
}

extern void dds_context_set_error_func(dds_context_t *context, void (*error_func)(int line, int column, const char *msg))
{
  assert(context != NULL);
  context->error_func = error_func;
}

dds_retcode_t dds_context_get_retcode(dds_context_t* context)
{
  assert(context != NULL);
  return context->retcode;
}

extern ddsts_type_t* dds_context_take_root_type(dds_context_t *context)
{
  ddsts_type_t* result = context->root_type;
  context->root_type = NULL;
  return result;
}

static void dds_context_free_array_sizes(dds_context_t *context)
{
  while (context->array_sizes != NULL) {
    array_size_t *next = context->array_sizes->next;
    ddsrt_free((void*)context->array_sizes);
    context->array_sizes = next;
  }
}

static void dds_context_close_member(dds_context_t *context)
{
  if (context->type_for_declarator != NULL && context->type_for_declarator->type.parent == NULL) {
    ddsts_free_type(context->type_for_declarator);
  }
  context->type_for_declarator = NULL;
  dds_context_free_array_sizes(context);
}

extern void dds_free_context(dds_context_t *context)
{
  assert(context != NULL);
  dds_context_close_member(context);
  ddsts_free_type(context->root_type);
  ddsrt_free(context->dangling_identifier);
  ddsrt_free(context);
}

bool dds_context_copy_identifier(dds_context_t *context, ddsts_identifier_t source, ddsts_identifier_t *dest)
{
  assert(context != NULL && source != NULL);
  assert(context->dangling_identifier == NULL);
  context->dangling_identifier = *dest = ddsrt_strdup(source);
  if (context->dangling_identifier == NULL) {
    context->retcode = DDS_RETCODE_OUT_OF_RESOURCES;
    return false;
  }
  return true;
}

#if (!defined(NDEBUG))
static bool cur_scope_is_definition_type(dds_context_t *context, ddsts_flags_t flags)
{
  assert(context != NULL && context->cur_type != NULL);
  return DDSTS_IS_TYPE(context->cur_type, flags);
}
#endif

extern bool dds_module_open(dds_context_t *context, ddsts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  ddsts_type_t *module;
  if (!new_module_definition(context, name, context->cur_type, &module)) {
    return false;
  }
  context->cur_type = module;
  return true;
}

extern void dds_module_close(dds_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  assert(context->cur_type->type.parent != NULL);
  context->cur_type = context->cur_type->type.parent;
}

extern bool dds_add_struct_forward(dds_context_t *context, ddsts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  assert(context->dangling_identifier == name);
  ddsts_type_t *forward_dcl;
  dds_retcode_t rc = ddsts_create_struct_forward_dcl(name, &forward_dcl);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  context->dangling_identifier = NULL;
  ddsts_module_add_member(context->cur_type, forward_dcl);
  return true;
}

extern bool dds_add_struct_open(dds_context_t *context, ddsts_identifier_t name)
{
  assert(   cur_scope_is_definition_type(context, DDSTS_MODULE)
         || cur_scope_is_definition_type(context, DDSTS_STRUCT));
  assert(context->dangling_identifier == name);
  ddsts_type_t *new_struct;
  dds_retcode_t rc = ddsts_create_struct(name, &new_struct);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  context->dangling_identifier = NULL;
  if (DDSTS_IS_TYPE(context->cur_type, DDSTS_MODULE)) {
    ddsts_module_add_member(context->cur_type, new_struct);
  }
  else {
    ddsts_struct_add_member(context->cur_type, new_struct);
  }
  context->cur_type = new_struct;
  /* find forward definitions of the struct and set their definition to this */
  if (DDSTS_IS_TYPE(new_struct->type.parent, DDSTS_MODULE)) {
    ddsts_module_t *parent_module;
    for (parent_module = &new_struct->type.parent->module; parent_module != NULL; parent_module = parent_module->previous) {
      ddsts_type_t *child;
      for (child = parent_module->members; child != NULL; child = child->type.next) {
        if (DDSTS_IS_TYPE(child, DDSTS_FORWARD_STRUCT) && strcmp(child->type.name, name) == 0) {
          child->forward.definition = new_struct;
        }
      }
    }
  }
  return true;
}

extern bool dds_add_struct_extension_open(dds_context_t *context, ddsts_identifier_t name, dds_scoped_name_t *scoped_name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  assert(context->dangling_identifier == name);
  ddsts_type_t *new_struct;
  dds_retcode_t rc = ddsts_create_struct(name, &new_struct);
  if (rc != DDS_RETCODE_OK) {
    context->retcode = rc;
    return false;
  }
  context->dangling_identifier = NULL;
  ddsts_module_add_member(context->cur_type, new_struct);
  /* find super */
  ddsts_type_t *definition;
  if (!resolve_scoped_name(context, scoped_name, &definition)) {
    dds_free_scoped_name(scoped_name);
    return false;
  }
  dds_free_scoped_name(scoped_name);
  if (definition != NULL && DDSTS_IS_TYPE(definition, DDSTS_STRUCT)) {
    new_struct->struct_def.super = definition;
  }
  context->cur_type = new_struct;
  return true;
}

extern bool dds_add_struct_member(dds_context_t *context, ddsts_type_t **ref_spec_type)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  dds_context_close_member(context);
  context->type_for_declarator = *ref_spec_type;
  *ref_spec_type = NULL;
  return true;
}

extern void dds_struct_close(dds_context_t *context, ddsts_type_t **result)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  dds_context_close_member(context);
  *result = context->cur_type;
  context->cur_type = context->cur_type->type.parent;
}

extern void dds_struct_empty_close(dds_context_t *context, ddsts_type_t **result)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  *result = context->cur_type;
  context->cur_type = context->cur_type->type.parent;
}

static ddsts_type_t *create_array_type(array_size_t *array_size, ddsts_type_t *type)
{
  if (array_size == NULL) {
    return type;
  }
  ddsts_type_t *array;
  dds_retcode_t rc = ddsts_create_array(NULL, array_size->size, &array);
  if (rc != DDS_RETCODE_OK) {
    return NULL;
  }
  ddsts_type_t *element_type = create_array_type(array_size->next, type);
  if (element_type == NULL) {
    ddsts_free_type(array);
    return NULL;
  }
  ddsts_array_set_element_type(array, element_type);
  return array;
}

extern bool dds_add_declarator(dds_context_t *context, ddsts_identifier_t name)
{
  assert(context != NULL);
  assert(context->dangling_identifier == name);
  if (DDSTS_IS_TYPE(context->cur_type, DDSTS_STRUCT)) {
    assert(context->type_for_declarator != NULL);
    ddsts_type_t *decl;
    dds_retcode_t rc = ddsts_create_declaration(name, NULL, &decl);
    if (rc != DDS_RETCODE_OK) {
      dds_context_free_array_sizes(context);
      context->retcode = rc;
      return false;
    }
    context->dangling_identifier = NULL;
    ddsts_type_t* type = create_array_type(context->array_sizes, context->type_for_declarator);
    if (type == NULL) {
      dds_context_free_array_sizes(context);
      ddsts_free_type(decl);
      context->retcode = DDS_RETCODE_OUT_OF_RESOURCES;
      return false;
    }
    dds_context_free_array_sizes(context);
    ddsts_declaration_set_type(decl, type);
    ddsts_struct_add_member(context->cur_type, decl);
    return true;
  }
  assert(false);
  return false;
}

extern bool dds_add_array_size(dds_context_t *context, ddsts_literal_t *value)
{
  assert(context != NULL);
  assert(value->flags == DDSTS_ULONGLONG);
  array_size_t **ref_array_size = &context->array_sizes;
  while (*ref_array_size != NULL) {
    ref_array_size = &(*ref_array_size)->next;
  }
  *ref_array_size = (array_size_t*)ddsrt_malloc(sizeof(array_size_t));
  if (*ref_array_size == NULL) {
    context->retcode = DDS_RETCODE_OUT_OF_RESOURCES;
    return false;
  }
  (*ref_array_size)->size = value->value.ullng;
  (*ref_array_size)->next = NULL;
  return true;
}

extern void dds_accept(dds_context_t *context)
{
  assert(context != NULL);
  context->retcode = DDS_RETCODE_OK;
}

