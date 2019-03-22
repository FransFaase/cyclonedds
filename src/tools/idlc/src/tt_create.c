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

#include "dds/ddsts/typetree.h"
#include "tt_create.h"

#include "os/os.h"

typedef struct array_size array_size_t;
struct array_size {
  unsigned long long size;
  array_size_t *next;
};

struct ddsts_context {
  ddsts_type_t *root_type;
  ddsts_type_t *cur_type;
  ddsts_type_t *type_for_declarator;
  array_size_t *array_sizes;
  bool out_of_memory;
  void (*error_func)(int line, int column, const char *msg);
};

static void add_module_member(ddsts_type_t *module, ddsts_type_t *member)
{
  if (module != NULL) {
    member->type.parent = module;
    ddsts_type_t **ref_child = &module->module.members;
    while (*ref_child != 0) {
      ref_child = &(*ref_child)->type.next;
    }
    *ref_child = member;
  }
}

static void add_struct_member(ddsts_type_t *struct_def, ddsts_type_t *member)
{
  if (struct_def != NULL) {
    member->type.parent = struct_def;
    ddsts_type_t **ref_child = &struct_def->struct_def.members;
    while (*ref_child != 0) {
      ref_child = &(*ref_child)->type.next;
    }
    *ref_child = member;
  }
}

extern bool ddsts_new_base_type(ddsts_context_t *context, ddsts_flags_t flags, ddsts_type_t **result)
{
  assert(context != NULL);
  ddsts_type_t *base_type = ddsts_create_base_type(flags);
  if (base_type == NULL) {
    context->out_of_memory = true;
    return false;
  }
  *result = base_type;
  return true;
}

static bool new_sequence(ddsts_context_t *context, ddsts_type_t *element_type, unsigned long long max, ddsts_type_t **result)
{
  assert(context != NULL);
  ddsts_type_t *sequence = ddsts_create_sequence(element_type, max);
  if (sequence == NULL) {
    context->out_of_memory = true;
    return false;
  }
  *result = sequence; 
  return true;
}

extern bool ddsts_new_sequence(ddsts_context_t *context, ddsts_type_t *base, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_sequence(context, base, size->value.ullng, result);
}

extern bool ddsts_new_sequence_unbound(ddsts_context_t *context, ddsts_type_t *base, ddsts_type_t **result)
{
  return new_sequence(context, base, 0ULL, result);
}

static bool new_string(ddsts_context_t *context, ddsts_flags_t flags, unsigned long long max, ddsts_type_t **result)
{
  assert(context != NULL);
  ddsts_type_t *string = ddsts_create_string(flags, max);
  if (string == NULL) {
    context->out_of_memory = true;
    return false;
  }
  *result = string;
  return true;
}

extern bool ddsts_new_string(ddsts_context_t *context, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_string(context, DDSTS_STRING, size->value.ullng, result);
}

extern bool ddsts_new_string_unbound(ddsts_context_t *context, ddsts_type_t **result)
{
  return new_string(context, DDSTS_STRING, 0ULL, result);
}

extern bool ddsts_new_wide_string(ddsts_context_t *context, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_string(context, DDSTS_WIDE_STRING, size->value.ullng, result);
}

extern bool ddsts_new_wide_string_unbound(ddsts_context_t *context, ddsts_type_t **result)
{
  return new_string(context, DDSTS_WIDE_STRING, 0ULL, result);
}

extern bool ddsts_new_fixed_pt(ddsts_context_t *context, ddsts_literal_t *digits, ddsts_literal_t *fraction_digits, ddsts_type_t **result)
{
  assert(context != NULL);
  assert(digits->flags == DDSTS_ULONGLONG);
  assert(fraction_digits->flags == DDSTS_ULONGLONG);
  ddsts_type_t *fixed_pt = ddsts_create_fixed_pt(digits->value.ullng, fraction_digits->value.ullng);
  if (fixed_pt == NULL) {
    context->out_of_memory = true;
    return false;
  }
  *result = fixed_pt;
  return true;
}

static bool new_map(ddsts_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, unsigned long long max, ddsts_type_t **result)
{
  ddsts_type_t *map = ddsts_create_map(key_type, value_type, max);
  if (map == NULL) {
    context->out_of_memory = true;
    return false;
  }
  *result = map;
  return true;
}

extern bool ddsts_new_map(ddsts_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, ddsts_literal_t *size, ddsts_type_t **result)
{
  assert(size->flags == DDSTS_ULONGLONG);
  return new_map(context, key_type, value_type, size->value.ullng, result);
}

extern bool ddsts_new_map_unbound(ddsts_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, ddsts_type_t **result)
{
  return new_map(context, key_type, value_type, 0UL, result);
}

struct ddsts_scoped_name {
  const char* name;
  bool top;
  ddsts_scoped_name_t *next;
};

extern bool ddsts_new_scoped_name(ddsts_context_t *context, ddsts_scoped_name_t* prev, bool top, ddsts_identifier_t name, ddsts_scoped_name_t **result)
{
  assert(context != NULL);
  ddsts_scoped_name_t *scoped_name = (ddsts_scoped_name_t*)os_malloc(sizeof(ddsts_scoped_name_t));
  if (scoped_name == NULL) {
    context->out_of_memory = true;
    return false;
  }
  scoped_name->name = name;
  scoped_name->top = top;
  scoped_name->next = NULL;
  if (prev == NULL) {
    *result = scoped_name;
  }
  else {
    ddsts_scoped_name_t **ref_scoped_name = &prev->next;
    while (*ref_scoped_name != NULL) {
      ref_scoped_name = &(*ref_scoped_name)->next;
    }
    *ref_scoped_name = scoped_name;
    *result = prev;
  }
  return true;
}

static bool resolve_scoped_name(ddsts_context_t *context, ddsts_scoped_name_t *scoped_name, ddsts_type_t **result)
{
  assert(context != NULL);
  assert(scoped_name != NULL);
  ddsts_type_t *cur_type = scoped_name->top ? context->root_type : context->cur_type;
  for (; cur_type != NULL; cur_type = cur_type->type.parent) {
    ddsts_type_t *found_node = cur_type;
    ddsts_scoped_name_t *cur_scoped_name;
    for (cur_scoped_name = scoped_name; cur_scoped_name != NULL && found_node != NULL; cur_scoped_name = cur_scoped_name->next) {
      ddsts_type_t *child =  found_node->type.flags == DDSTS_MODULE
                           ? found_node->module.members
                           : found_node->struct_def.members;
      for (; child != NULL; child = child->type.next) {
        if (DDSTS_IS_DEFINITION(child->type.flags) && strcmp(child->type.name, cur_scoped_name->name) == 0) {
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

static void free_scoped_name(ddsts_scoped_name_t *scoped_name)
{
  while (scoped_name != NULL) {
    ddsts_scoped_name_t *next = scoped_name->next;
    os_free((void*)scoped_name->name);
    os_free((void*)scoped_name);
    scoped_name = next;
  }
}

extern bool ddsts_get_type_from_scoped_name(ddsts_context_t *context, ddsts_scoped_name_t *scoped_name, ddsts_type_t **result)
{
  ddsts_type_t *definition;
  resolve_scoped_name(context, scoped_name, &definition);
  free_scoped_name(scoped_name);
  if (definition == NULL) {
    /* Create a Boolean type, just to prevent a NULL pointer */
    return ddsts_new_base_type(context, DDSTS_BOOLEAN, result);
  }
  *result = definition;
  return true;
}

static bool new_module_definition(ddsts_context_t *context, ddsts_identifier_t name, ddsts_type_t *parent, ddsts_type_t **result)
{
  ddsts_type_t *module = ddsts_create_module(name);
  if (module == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_module_member(parent, module);
  *result = module;
  return true;
}

extern ddsts_context_t* ddsts_create_context()
{
  ddsts_context_t *context = (ddsts_context_t*)os_malloc(sizeof(ddsts_context_t));
  if (context == NULL) {
    return NULL;
  }
  context->error_func = NULL;
  ddsts_type_t *module;
  if (!new_module_definition(context, NULL, NULL, &module)) {
    os_free(context);
    return NULL;
  }
  context->root_type = module;
  context->cur_type = context->root_type;
  context->type_for_declarator = NULL;
  context->array_sizes = NULL;
  return context;
}

extern void ddsts_context_error(ddsts_context_t *context, int line, int column, const char *msg)
{
  assert(context != NULL);
  if (context->error_func != NULL) {
    context->error_func(line, column, msg);
  }
}

extern void ddsts_context_set_error_func(ddsts_context_t *context, void (*error_func)(int line, int column, const char *msg))
{
  assert(context != NULL);
  context->error_func = error_func;
}

void ddsts_context_set_out_of_memory_error(ddsts_context_t* context)
{
  assert(context != NULL);
  context->out_of_memory = true;
}

bool ddsts_context_get_out_of_memory_error(ddsts_context_t* context)
{
  assert(context != NULL);
  return context->out_of_memory;
}

extern ddsts_type_t* ddsts_context_take_root_type(ddsts_context_t *context)
{
  ddsts_type_t* result = context->root_type;
  context->root_type = NULL;
  return result;
}

static void ddsts_context_free_array_sizes(ddsts_context_t *context)
{
  while (context->array_sizes != NULL) {
    array_size_t *next = context->array_sizes->next;
    os_free((void*)context->array_sizes);
    context->array_sizes = next;
  }
}

static void ddsts_context_close_member(ddsts_context_t *context)
{
  if (context->type_for_declarator != NULL && context->type_for_declarator->type.parent == NULL) {
    /* Normally, this should not happen */
    ddsts_free_type(context->type_for_declarator);
  }
  context->type_for_declarator = NULL;
  ddsts_context_free_array_sizes(context);
}

extern void ddsts_free_context(ddsts_context_t *context)
{
  assert(context != NULL);
  ddsts_context_close_member(context);
  ddsts_free_type(context->root_type);
  os_free(context);
}

#if (!defined(NDEBUG))
static bool cur_scope_is_definition_type(ddsts_context_t *context, ddsts_flags_t flags)
{
  assert(context != NULL && context->cur_type != NULL);
  return context->cur_type->type.flags == flags;
}
#endif

extern bool ddsts_module_open(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  ddsts_type_t *module;
  if (!new_module_definition(context, name, context->cur_type, &module)) {
    return false;
  }
  /* find previous open of this module, if any */
  ddsts_module_t *parent_module;
  for (parent_module = &context->cur_type->module; parent_module != NULL; parent_module = parent_module->previous) {
    ddsts_type_t *child;
    for (child = parent_module->members; child != NULL; child = child->type.next) {
      if (child->type.flags == DDSTS_MODULE && strcmp(child->type.name, name) == 0 && child != module) {
        module->module.previous = &child->module;
      }
    }
    if (module->module.previous != NULL) {
      break;
    }
  }
  context->cur_type = module;
  return true;
}

extern void ddsts_module_close(ddsts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  assert(context->cur_type->type.parent != NULL);
  context->cur_type = context->cur_type->type.parent;
}

extern bool ddsts_add_struct_forward(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  ddsts_type_t *forward_dcl = ddsts_create_struct_forward_dcl(name);
  if (forward_dcl == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_module_member(context->cur_type, forward_dcl);
  return true;
}

extern bool ddsts_add_struct_open(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(   cur_scope_is_definition_type(context, DDSTS_MODULE)
         || cur_scope_is_definition_type(context, DDSTS_STRUCT));
  ddsts_type_t *new_struct = ddsts_create_struct(name);
  if (new_struct == NULL) {
    context->out_of_memory = true;
    return false;
  }
  if (context->cur_type->type.flags == DDSTS_MODULE) {
    add_module_member(context->cur_type, new_struct);
  }
  else {
    add_struct_member(context->cur_type, new_struct);
  }
  context->cur_type = new_struct;
  /* find forward definitions of the struct and set their definition to this */
  if (new_struct->type.parent->type.flags == DDSTS_MODULE) {
    ddsts_module_t *parent_module;
    for (parent_module = &new_struct->type.parent->module; parent_module != NULL; parent_module = parent_module->previous) {
      ddsts_type_t *child;
      for (child = parent_module->members; child != NULL; child = child->type.next) {
        if (child->type.flags == DDSTS_FORWARD_STRUCT && strcmp(child->type.name, name) == 0) {
          child->forward.definition = new_struct;
        }
      }
    }
  }
  return true;
}

extern bool ddsts_add_struct_extension_open(ddsts_context_t *context, ddsts_identifier_t name, ddsts_scoped_name_t *scoped_name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  ddsts_type_t *new_struct = ddsts_create_struct(name);
  if (new_struct == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_module_member(context->cur_type, new_struct);
  /* find super */
  ddsts_type_t *definition;
  if (!resolve_scoped_name(context, scoped_name, &definition)) {
    free_scoped_name(scoped_name);
    return false;
  }
  free_scoped_name(scoped_name);
  if (definition != NULL && definition->type.flags == DDSTS_STRUCT) {
    new_struct->struct_def.super = definition;
  }
  context->cur_type = new_struct;
  return true;
}

extern bool ddsts_add_struct_member(ddsts_context_t *context, ddsts_type_t *spec_type)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  ddsts_context_close_member(context);
  context->type_for_declarator = spec_type;
  return true;
}

extern void ddsts_struct_close(ddsts_context_t *context, ddsts_type_t **result)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  ddsts_context_close_member(context);
  *result = context->cur_type;
  context->cur_type = context->cur_type->type.parent;
}

extern void ddsts_struct_empty_close(ddsts_context_t *context, ddsts_type_t **result)
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
  ddsts_type_t *array = ddsts_create_array(NULL, array_size->size);
  if (array == NULL) {
    return NULL;
  }
  ddsts_type_t *element_type = create_array_type(array_size->next, type);
  if (element_type == NULL) {
    ddsts_free_type(array);
    return NULL;
  }
  array->array.element_type = element_type;
  if (element_type->type.parent == NULL) {
    element_type->type.parent = array;
  }
  return array;
}
  
extern bool ddsts_add_declarator(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(context != NULL);
  if (context->cur_type->type.flags == DDSTS_STRUCT) {
    assert(context->type_for_declarator != NULL);
    ddsts_type_t *decl = ddsts_create_declaration(name, NULL);
    if (decl == NULL) {
      ddsts_context_free_array_sizes(context);
      context->out_of_memory = true;
      return false;
    }
    ddsts_type_t* type = create_array_type(context->array_sizes, context->type_for_declarator);
    if (type == NULL) {
      ddsts_context_free_array_sizes(context);
      ddsts_free_type(decl);
      context->out_of_memory = true;
      return false;
    }
    ddsts_context_free_array_sizes(context);
    decl->declaration.decl_type = type;
    if (type->type.parent == NULL) {
      type->type.parent = decl;
    }
    add_struct_member(context->cur_type, decl);
    return true;
  }
  assert(false);
  return false;
}

extern bool ddsts_add_array_size(ddsts_context_t *context, ddsts_literal_t *value)
{
  assert(context != NULL);
  assert(value->flags == DDSTS_ULONGLONG);
  array_size_t *array_size = (array_size_t*)os_malloc(sizeof(array_size_t));
  if (array_size == NULL) {
    context->out_of_memory = true;
    return false;
  }
  array_size->size = value->value.ullng;
  array_size->next = context->array_sizes;
  context->array_sizes = array_size;
  return true;
}

