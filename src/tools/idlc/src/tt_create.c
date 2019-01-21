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

struct ddsts_context {
  ddsts_node_t *root_node;
  ddsts_node_t *cur_node;
  ddsts_node_t *parent_for_declarator;
  ddsts_node_t *array_sizes;
  bool out_of_memory;
  void (*error_func)(int line, int column, const char *msg);
};

static void add_child(ddsts_node_t *node, ddsts_node_t *parent)
{
  if (parent != NULL) {
    node->parent = parent;
    ddsts_node_t **ref_def = &parent->children;
    while (*ref_def != 0) {
      ref_def = &(*ref_def)->next;
    }
    *ref_def = node;
  }
}

static void attach_children(ddsts_node_t *parent, ddsts_node_t *children)
{
  ddsts_node_t **ref_child = &parent->children;
  while (*ref_child != NULL) {
    ref_child = &(*ref_child)->next;
  }
  *ref_child = children;
  ddsts_node_t *child;
  for (child = children; child != NULL; child = child->next) {
    child->parent = parent;
  }
}

extern bool ddsts_new_base_type(ddsts_context_t *context, ddsts_node_flags_t flags, ddsts_type_spec_ptr_t *result)
{
  assert(context != NULL);
  ddsts_base_type_t *base_type = ddsts_create_base_type(flags);
  if (base_type == NULL) {
    context->out_of_memory = true;
    return false;
  }
  ddsts_type_spec_ptr_assign(result, (ddsts_type_spec_t*)base_type);
  return true;
}

static bool new_sequence(ddsts_context_t *context, ddsts_type_spec_ptr_t *element_type, bool bounded, unsigned long long max, ddsts_type_spec_ptr_t *result)
{
  assert(context != NULL);
  ddsts_sequence_t *sequence = ddsts_create_sequence(element_type, bounded, max);
  if (sequence == NULL) {
    context->out_of_memory = true;
    return false;
  }
  ddsts_type_spec_ptr_assign(result, (ddsts_type_spec_t*)sequence);
  return true;
}

extern bool ddsts_new_sequence(ddsts_context_t *context, ddsts_type_spec_ptr_t *base, ddsts_literal_t *size, ddsts_type_spec_ptr_t *result)
{
  assert(size->flags == DDSTS_UNSIGNED_LONG_LONG_TYPE);
  return new_sequence(context, base, true, size->value.ullng, result);
}

extern bool ddsts_new_sequence_unbound(ddsts_context_t *context, ddsts_type_spec_ptr_t *base, ddsts_type_spec_ptr_t *result)
{
  return new_sequence(context, base, false, 0, result);
}

static bool new_string(ddsts_context_t *context, ddsts_node_flags_t flags, bool bounded, unsigned long long max, ddsts_type_spec_ptr_t *result)
{
  assert(context != NULL);
  ddsts_string_t *string = ddsts_create_string(flags, bounded, max);
  if (string == NULL) {
    context->out_of_memory = true;
    return false;
  }
  ddsts_type_spec_ptr_assign(result, (ddsts_type_spec_t*)string);
  return true;
}

extern bool ddsts_new_string(ddsts_context_t *context, ddsts_literal_t *size, ddsts_type_spec_ptr_t *result)
{
  assert(size->flags == DDSTS_UNSIGNED_LONG_LONG_TYPE);
  return new_string(context, DDSTS_STRING, true, size->value.ullng, result);
}

extern bool ddsts_new_string_unbound(ddsts_context_t *context, ddsts_type_spec_ptr_t *result)
{
  return new_string(context, DDSTS_STRING, false, 0, result);
}

extern bool ddsts_new_wide_string(ddsts_context_t *context, ddsts_literal_t *size, ddsts_type_spec_ptr_t *result)
{
  assert(size->flags == DDSTS_UNSIGNED_LONG_LONG_TYPE);
  return new_string(context, DDSTS_WIDE_STRING, true, size->value.ullng, result);
}

extern bool ddsts_new_wide_string_unbound(ddsts_context_t *context, ddsts_type_spec_ptr_t *result)
{
  return new_string(context, DDSTS_WIDE_STRING, false, 0, result);
}

extern bool ddsts_new_fixed_pt(ddsts_context_t *context, ddsts_literal_t *digits, ddsts_literal_t *fraction_digits, ddsts_type_spec_ptr_t *result)
{
  assert(context != NULL);
  assert(digits->flags == DDSTS_UNSIGNED_LONG_LONG_TYPE);
  assert(fraction_digits->flags == DDSTS_UNSIGNED_LONG_LONG_TYPE);
  ddsts_fixed_pt_t *fixed_pt = ddsts_create_fixed_pt(digits->value.ullng, fraction_digits->value.ullng);
  if (fixed_pt == NULL) {
    context->out_of_memory = true;
    return false;
  }
  ddsts_type_spec_ptr_assign(result, (ddsts_type_spec_t*)fixed_pt);
  return true;
}

static bool new_map(ddsts_context_t *context, ddsts_type_spec_ptr_t *key_type, ddsts_type_spec_ptr_t *value_type, bool bounded, unsigned long long max, ddsts_type_spec_ptr_t *result)
{
  ddsts_map_t *map = ddsts_create_map(key_type, value_type, bounded, max);
  if (map == NULL) {
    context->out_of_memory = true;
    return false;
  }
  ddsts_type_spec_ptr_assign(result, (ddsts_type_spec_t*)map);
  return true;
}

extern bool ddsts_new_map(ddsts_context_t *context, ddsts_type_spec_ptr_t *key_type, ddsts_type_spec_ptr_t *value_type, ddsts_literal_t *size, ddsts_type_spec_ptr_t *result)
{
  assert(size->flags == DDSTS_UNSIGNED_LONG_LONG_TYPE);
  return new_map(context, key_type, value_type, true, size->value.ullng, result);
}

extern bool ddsts_new_map_unbound(ddsts_context_t *context, ddsts_type_spec_ptr_t *key_type, ddsts_type_spec_ptr_t *value_type, ddsts_type_spec_ptr_t *result)
{
  return new_map(context, key_type, value_type, false, 0, result);
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

static bool resolve_scoped_name(ddsts_context_t *context, ddsts_scoped_name_t *scoped_name, ddsts_definition_t **result)
{
  assert(context != NULL);
  assert(scoped_name != NULL);
  ddsts_node_t *cur_node = scoped_name->top ? context->root_node : context->cur_node;
  for (; cur_node != NULL; cur_node = cur_node->parent) {
    ddsts_node_t *found_node = cur_node;
    ddsts_scoped_name_t *cur_scoped_name;
    for (cur_scoped_name = scoped_name; cur_scoped_name != NULL && found_node != NULL; cur_scoped_name = cur_scoped_name->next) {
      ddsts_node_t *child;
      for (child = found_node->children; child != NULL; child = child->next) {
        if (DDSTS_IS_DEFINITION(child->flags) && strcmp(((ddsts_definition_t*)child)->name, cur_scoped_name->name) == 0) {
          break;
        }
      }
      found_node = child;
    }
    if (found_node != NULL) {
      *result = (ddsts_definition_t*)found_node;
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

extern bool ddsts_get_type_spec_from_scoped_name(ddsts_context_t *context, ddsts_scoped_name_t *scoped_name, ddsts_type_spec_ptr_t *result)
{
  ddsts_definition_t *definition;
  resolve_scoped_name(context, scoped_name, &definition);
  free_scoped_name(scoped_name);
  if (definition == NULL) {
    /* Create a Boolean type, just to prevent a NULL pointer */
    return ddsts_new_base_type(context, DDSTS_BOOLEAN_TYPE, result);
  }
  ddsts_type_spec_ptr_assign_reference(result, (ddsts_type_spec_t*)definition);
  return true;
}

static bool new_module_definition(ddsts_context_t *context, ddsts_identifier_t name, ddsts_node_t *parent, ddsts_module_t **result)
{
  ddsts_module_t *module = ddsts_create_module(name);
  if (module == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_child(&module->def.type_spec.node, parent);
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
  ddsts_module_t *module;
  if (!new_module_definition(context, NULL, NULL, &module)) {
    os_free(context);
    return NULL;
  }
  context->root_node = (ddsts_node_t*)module;
  context->cur_node = context->root_node;
  context->parent_for_declarator = NULL;
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

extern ddsts_node_t* ddsts_context_take_root_node(ddsts_context_t *context)
{
  ddsts_node_t* result = context->root_node;
  context->root_node = NULL;
  return result;
}

extern void ddsts_free_context(ddsts_context_t *context)
{
  assert(context != NULL);
  ddsts_free_node(context->root_node);
  os_free(context);
}

#if (!defined(NDEBUG))
static bool cur_scope_is_definition_type(ddsts_context_t *context, ddsts_node_flags_t flags)
{
  assert(context != NULL && context->cur_node != NULL);
  return context->cur_node->flags == flags;
}
#endif

extern bool ddsts_module_open(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  ddsts_module_t *module;
  if (!new_module_definition(context, name, (ddsts_node_t*)context->cur_node, &module)) {
    return false;
  }
  /* find previous open of this module, if any */
  ddsts_module_t *parent_module;
  for (parent_module = (ddsts_module_t*)context->cur_node; parent_module != NULL; parent_module = parent_module->previous) {
    ddsts_node_t *child;
    for (child = parent_module->def.type_spec.node.children; child != NULL; child = child->next) {
      if (child->flags == DDSTS_MODULE && strcmp(((ddsts_module_t*)child)->def.name, name) == 0 && (ddsts_module_t*)child != module) {
        module->previous = (ddsts_module_t*)child;
      }
    }
    if (module->previous != NULL) {
      break;
    }
  }
  context->cur_node = (ddsts_node_t*)module;
  return true;
}

extern void ddsts_module_close(ddsts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  assert(context->cur_node->parent != NULL);
  context->cur_node = context->cur_node->parent;
}

extern bool ddsts_add_struct_forward(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  ddsts_forward_declaration_t *forward_dcl = ddsts_create_struct_forward_dcl(name);
  if (forward_dcl == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_child(&forward_dcl->def.type_spec.node, context->cur_node);
  return true;
}

extern bool ddsts_add_struct_open(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(   cur_scope_is_definition_type(context, DDSTS_MODULE)
         || cur_scope_is_definition_type(context, DDSTS_STRUCT));
  ddsts_struct_t *new_struct = ddsts_create_struct(name);
  if (new_struct == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_child(&new_struct->def.type_spec.node, context->cur_node);
  context->cur_node = (ddsts_node_t*)new_struct;
  /* find forward definitions of the struct and set their definition to this */
  if (new_struct->def.type_spec.node.parent->flags == DDSTS_MODULE) {
    ddsts_module_t *parent_module;
    for (parent_module = (ddsts_module_t*)new_struct->def.type_spec.node.parent; parent_module != NULL; parent_module = parent_module->previous) {
      ddsts_node_t *child;
      for (child = parent_module->def.type_spec.node.children; child != NULL; child = child->next) {
        if (child->flags == DDSTS_FORWARD_STRUCT && strcmp(((ddsts_forward_declaration_t*)child)->def.name, name) == 0) {
          ((ddsts_forward_declaration_t*)child)->definition = &new_struct->def;
        }
      }
    }
  }
  return true;
}

extern bool ddsts_add_struct_extension_open(ddsts_context_t *context, ddsts_identifier_t name, ddsts_scoped_name_t *scoped_name)
{
  assert(cur_scope_is_definition_type(context, DDSTS_MODULE));
  ddsts_struct_t *new_struct = ddsts_create_struct(name);
  if (new_struct == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_child(&new_struct->def.type_spec.node, context->cur_node);
  /* find super */
  ddsts_definition_t *definition;
  if (!resolve_scoped_name(context, scoped_name, &definition)) {
    free_scoped_name(scoped_name);
    return false;
  }
  free_scoped_name(scoped_name);
  if (definition != NULL && definition->type_spec.node.flags == DDSTS_STRUCT) {
    new_struct->super = definition;
  }
  context->cur_node = (ddsts_node_t*)new_struct;
  return true;
}

static void mark_structs_as_part_of(ddsts_type_spec_t *type)
{
  if (type->node.flags == DDSTS_STRUCT)
  {
    /* fprintf(stderr, "%s is part of %s\n", ((ddsts_struct_t*)type)->name, ((ddsts_definition_t*)context->cur_node)->name); */
    ((ddsts_struct_t*)type)->part_of = true;
  }
  else if (type->node.flags == DDSTS_FORWARD_STRUCT)
  {
    /* fprintf(stderr, "forward %s is part of %s through sequence\n", ((ddsts_forward_declaration_t*)seq_type->element_type)->name, ((ddsts_definition_t*)context->cur_node)->name); */
    ddsts_forward_declaration_t *forward_struct = (ddsts_forward_declaration_t*)type;
    if (forward_struct->definition != NULL && forward_struct->definition->type_spec.node.flags == DDSTS_STRUCT)
      ((ddsts_struct_t*)forward_struct->definition)->part_of = true;
    /* FIXME: set value on forward. */
  }
  else if (type->node.flags == DDSTS_SEQUENCE) {
    mark_structs_as_part_of(((ddsts_sequence_t*)type)->element_type.type_spec);
  }
  else if (type->node.flags == DDSTS_MAP) {
    mark_structs_as_part_of(((ddsts_map_t*)type)->key_type.type_spec);
    mark_structs_as_part_of(((ddsts_map_t*)type)->value_type.type_spec);
  }
}

extern bool ddsts_add_struct_member(ddsts_context_t *context, ddsts_type_spec_ptr_t *spec_type_ptr)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  ddsts_struct_member_t *member = ddsts_create_struct_member(spec_type_ptr);
  if (member == NULL) {
    context->out_of_memory = true;
    return false;
  }
  add_child(&member->node, context->cur_node);
  context->parent_for_declarator = (ddsts_node_t*)member;
  mark_structs_as_part_of(spec_type_ptr->type_spec);
  return true;
}

extern void ddsts_struct_close(ddsts_context_t *context, ddsts_type_spec_ptr_t *result)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  ddsts_type_spec_t *type_spec = (ddsts_type_spec_t*)context->cur_node;
  context->cur_node = context->cur_node->parent;
  context->parent_for_declarator = NULL;
  context->array_sizes = NULL;
  ddsts_type_spec_ptr_assign_reference(result, type_spec);
}

extern void ddsts_struct_empty_close(ddsts_context_t *context, ddsts_type_spec_ptr_t *result)
{
  assert(cur_scope_is_definition_type(context, DDSTS_STRUCT));
  ddsts_type_spec_t *type_spec = (ddsts_type_spec_t*)context->cur_node;
  context->cur_node = context->cur_node->parent;
  context->parent_for_declarator = NULL;
  context->array_sizes = NULL;
  ddsts_type_spec_ptr_assign_reference(result, type_spec);
}

extern bool ddsts_add_declarator(ddsts_context_t *context, ddsts_identifier_t name)
{
  assert(context != NULL);
  if (context->parent_for_declarator != NULL && context->cur_node->flags == DDSTS_STRUCT) {
    ddsts_definition_t* declarator = ddsts_create_declarator(name);
    if (declarator == NULL) {
      context->out_of_memory = true;
      return false;
    }
    add_child(&declarator->type_spec.node, context->parent_for_declarator);
    if (context->array_sizes != NULL) {
      attach_children(&declarator->type_spec.node, context->array_sizes);
      context->array_sizes = NULL;
    }
    return true;
  }
  assert(false);
  return false;
}

extern bool ddsts_add_array_size(ddsts_context_t *context, ddsts_literal_t *value)
{
  assert(context != NULL);
  assert(value->flags == DDSTS_UNSIGNED_LONG_LONG_TYPE);
  ddsts_array_size_t *array_size = ddsts_create_array_size(value->value.ullng);
  if (array_size == NULL) {
    context->out_of_memory = true;
    return false;
  }
  ddsts_node_t **ref_child = &context->array_sizes;
  while (*ref_child != NULL) {
    ref_child = &(*ref_child)->next;
  }
  *ref_child = &array_size->node;
  return true;
}

