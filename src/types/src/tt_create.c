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

#include "typetree.h"
#include "tt_create.h"

#include "os/os.h"

struct dds_ts_context {
  dds_ts_node_t *root_node;
  dds_ts_node_t *cur_node;
  dds_ts_node_t *parent_for_declarator;
  bool out_of_memory;
  void (*error_func)(int line, int column, const char *msg);
};

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

extern bool dds_ts_new_base_type(dds_ts_context_t *context, dds_ts_node_flags_t flags, dds_ts_type_spec_t **result)
{
  assert(context != NULL);
  dds_ts_base_type_t *base_type = (dds_ts_base_type_t*)os_malloc(sizeof(dds_ts_base_type_t));
  if (base_type == NULL) {
    context->out_of_memory = true;
    return false;
  }
  init_node((dds_ts_node_t*)base_type, flags, NULL);
  *result = (dds_ts_type_spec_t*)base_type;
  return true;
}

static bool new_sequence(dds_ts_context_t *context, dds_ts_type_spec_t *element_type, bool bounded, unsigned long long max, dds_ts_type_spec_t **result)
{
  assert(context != NULL);
  dds_ts_sequence_t *sequence = (dds_ts_sequence_t*)os_malloc(sizeof(dds_ts_sequence_t));
  if (sequence == NULL) {
    context->out_of_memory = true;
    return false;
  }
  init_node((dds_ts_node_t*)sequence, DDS_TS_SEQUENCE, NULL);
  sequence->element_type = element_type;
  sequence->bounded = bounded;
  sequence->max = max;
  *result = (dds_ts_type_spec_t*)sequence;
  return true;
}

extern bool dds_ts_new_sequence(dds_ts_context_t *context, dds_ts_type_spec_t *base, dds_ts_literal_t *size, dds_ts_type_spec_t **result)
{
  assert(size->flags == DDS_TS_UNSIGNED_LONG_LONG_TYPE);
  return new_sequence(context, base, true, size->value.ullng, result);
}

extern bool dds_ts_new_sequence_unbound(dds_ts_context_t *context, dds_ts_type_spec_t *base, dds_ts_type_spec_t **result)
{
  return new_sequence(context, base, false, 0, result);
}

static bool new_string(dds_ts_context_t *context, dds_ts_node_flags_t flags, bool bounded, unsigned long long max, dds_ts_type_spec_t **result)
{
  assert(context != NULL);
  dds_ts_string_t *string = (dds_ts_string_t*)os_malloc(sizeof(dds_ts_string_t));
  if (string == NULL) {
    context->out_of_memory = true;
    return false;
  }
  init_node((dds_ts_node_t*)string, flags, NULL);
  string->bounded = bounded;
  string->max = max;
  *result = (dds_ts_type_spec_t*)string;
  return true;
}

extern bool dds_ts_new_string(dds_ts_context_t *context, dds_ts_literal_t *size, dds_ts_type_spec_t **result)
{
  assert(size->flags == DDS_TS_UNSIGNED_LONG_LONG_TYPE);
  return new_string(context, DDS_TS_STRING, true, size->value.ullng, result);
}

extern bool dds_ts_new_string_unbound(dds_ts_context_t *context, dds_ts_type_spec_t **result)
{
  return new_string(context, DDS_TS_STRING, false, 0, result);
}

extern bool dds_ts_new_wide_string(dds_ts_context_t *context, dds_ts_literal_t *size, dds_ts_type_spec_t **result)
{
  assert(size->flags == DDS_TS_UNSIGNED_LONG_LONG_TYPE);
  return new_string(context, DDS_TS_WIDE_STRING, true, size->value.ullng, result);
}

extern bool dds_ts_new_wide_string_unbound(dds_ts_context_t *context, dds_ts_type_spec_t **result)
{
  return new_string(context, DDS_TS_WIDE_STRING, false, 0, result);
}

extern bool dds_ts_new_fixed_pt(dds_ts_context_t *context, dds_ts_literal_t *digits, dds_ts_literal_t *fraction_digits, dds_ts_type_spec_t **result)
{
  assert(context != NULL);
  assert(digits->flags == DDS_TS_UNSIGNED_LONG_LONG_TYPE);
  assert(fraction_digits->flags == DDS_TS_UNSIGNED_LONG_LONG_TYPE);
  dds_ts_fixed_pt_t *fixed_pt = (dds_ts_fixed_pt_t*)os_malloc(sizeof(dds_ts_fixed_pt_t));
  if (fixed_pt == NULL) {
    context->out_of_memory = true;
    return false;
  }
  init_node((dds_ts_node_t*)fixed_pt, DDS_TS_FIXED_PT, NULL);
  fixed_pt->digits = digits->value.ullng;
  fixed_pt->fraction_digits = fraction_digits->value.ullng;
  *result = (dds_ts_type_spec_t*)fixed_pt;
  return true;
}

static bool new_map(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type, bool bounded, unsigned long long max, dds_ts_type_spec_t **result)
{
  dds_ts_map_t *map = (dds_ts_map_t*)os_malloc(sizeof(dds_ts_map_t));
  if (map == NULL) {
    context->out_of_memory = true;
    return false;
  }
  init_node((dds_ts_node_t*)map, DDS_TS_MAP, NULL);
  map->key_type = key_type;
  map->value_type = value_type;
  map->bounded = bounded;
  map->max = max;
  *result = (dds_ts_type_spec_t*)map;
  return true;
}

extern bool dds_ts_new_map(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type, dds_ts_literal_t *size, dds_ts_type_spec_t **result)
{
  assert(size->flags == DDS_TS_UNSIGNED_LONG_LONG_TYPE);
  return new_map(context, key_type, value_type, true, size->value.ullng, result);
}

extern bool dds_ts_new_map_unbound(dds_ts_context_t *context, dds_ts_type_spec_t *key_type, dds_ts_type_spec_t *value_type, dds_ts_type_spec_t **result)
{
  return new_map(context, key_type, value_type, false, 0, result);
}

struct dds_ts_scoped_name {
  const char* name;
  bool top;
  dds_ts_scoped_name_t *next;
};

extern bool dds_ts_new_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t* prev, bool top, dds_ts_identifier_t name, dds_ts_scoped_name_t **result)
{
  assert(context != NULL);
  dds_ts_scoped_name_t *scoped_name = (dds_ts_scoped_name_t*)os_malloc(sizeof(dds_ts_scoped_name_t));
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
    dds_ts_scoped_name_t **ref_scoped_name = &prev->next;
    while (*ref_scoped_name != NULL) {
      ref_scoped_name = &(*ref_scoped_name)->next;
    }
    *ref_scoped_name = scoped_name;
    *result = prev;
  }
  return true;
}

static bool resolve_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t *scoped_name, dds_ts_definition_t **result)
{
  assert(context != NULL);
  assert(scoped_name != NULL);
  dds_ts_node_t *cur_node = scoped_name->top ? context->root_node : context->cur_node;
  for (; cur_node != NULL; cur_node = cur_node->parent) {
    dds_ts_node_t *found_node = cur_node;
    dds_ts_scoped_name_t *cur_scoped_name;
    for (cur_scoped_name = scoped_name; cur_scoped_name != NULL && found_node != NULL; cur_scoped_name = cur_scoped_name->next) {
      dds_ts_node_t *child;
      for (child = found_node->children; child != NULL; child = child->next) {
        if (DDS_TS_IS_DEFINITION(child->flags) && strcmp(((dds_ts_definition_t*)child)->name, cur_scoped_name->name) == 0) {
          break;
        }
      }
      found_node = child;
    }
    if (found_node != NULL) {
      *result = (dds_ts_definition_t*)found_node;
      return true;
    }
  }
  /* Could not resolve scoped name */
  *result = NULL;
  return false;
}

extern bool dds_ts_get_type_spec_from_scoped_name(dds_ts_context_t *context, dds_ts_scoped_name_t *scoped_name, dds_ts_type_spec_t **result)
{
  dds_ts_definition_t *definition;
  resolve_scoped_name(context, scoped_name, &definition);
  if (definition == NULL) {
    /* Create a Boolean type, just to prevent a NULL pointer */
    return dds_ts_new_base_type(context, DDS_TS_BOOLEAN_TYPE, result);
  }
  *result = (dds_ts_type_spec_t*)definition;
  return true;
}

static bool init_definition(dds_ts_context_t *context, dds_ts_definition_t *definition, dds_ts_identifier_t name, dds_ts_node_flags_t flags, dds_ts_node_t *parent)
{
  init_node((dds_ts_node_t*)definition, flags, parent);
  if (name != NULL) {
    definition->name = name;
    if (definition->name == NULL) {
      context->out_of_memory = true;
      return false;
    }
  }
  else {
    definition->name = NULL;
  }
  return true;
}

static bool new_module_definition(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_node_t *parent, dds_ts_module_t **result)
{
  dds_ts_module_t *def = (dds_ts_module_t*)os_malloc(sizeof(dds_ts_module_t));
  if (def == NULL) {
    context->out_of_memory = true;
    return false;
  }
  if (!init_definition(context, (dds_ts_definition_t*)def, name, DDS_TS_MODULE, parent)) {
    os_free((void*)def);
    return false;
  }
  *result = def;
  return true;
}

extern dds_ts_context_t* dds_ts_create_context()
{
  dds_ts_context_t *context = (dds_ts_context_t*)os_malloc(sizeof(dds_ts_context_t));
  if (context == NULL) {
    return NULL;
  }
  context->error_func = NULL;
  dds_ts_module_t *module;
  if (!new_module_definition(context, NULL, NULL, &module)) {
    os_free(context);
    return NULL;
  }
  context->root_node = (dds_ts_node_t*)module;
  context->cur_node = context->root_node;
  context->parent_for_declarator = NULL;
  return context;
}

extern void dds_ts_context_error(dds_ts_context_t *context, int line, int column, const char *msg)
{
  assert(context != NULL);
  if (context->error_func != NULL)
    context->error_func(line, column, msg);
}

extern void dds_ts_context_set_error_func(dds_ts_context_t *context, void (*error_func)(int line, int column, const char *msg))
{
  assert(context != NULL);
  context->error_func = error_func;
}

void dds_ts_context_set_out_of_memory_error(dds_ts_context_t* context)
{
  assert(context != NULL);
  context->out_of_memory = true;
}

bool dds_ts_context_get_out_of_memory_error(dds_ts_context_t* context)
{
  assert(context != NULL);
  return context->out_of_memory;
}

extern dds_ts_node_t* dds_ts_context_get_root_node(dds_ts_context_t *context)
{
  return context->root_node;
}

extern void dds_ts_free_context(dds_ts_context_t *context)
{
  assert(context != NULL);
  dds_ts_free_node(context->root_node);
  os_free(context);
}

#if (!defined(NDEBUG))
static bool cur_scope_is_definition_type(dds_ts_context_t *context, dds_ts_node_flags_t flags)
{
  assert(context != NULL && context->cur_node != NULL);
  return context->cur_node->flags == flags;
}
#endif

extern bool dds_ts_module_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  dds_ts_module_t *module;
  if (!new_module_definition(context, name, (dds_ts_node_t*)context->cur_node, &module))
    return false;
  context->cur_node = (dds_ts_node_t*)module;
  return true;
}

extern void dds_ts_module_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  assert(context->cur_node->parent != NULL);
  context->cur_node = context->cur_node->parent;
}

extern bool dds_ts_add_struct_forward(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  dds_ts_forward_declaration_t *forward_dcl = (dds_ts_forward_declaration_t*)os_malloc(sizeof(dds_ts_forward_declaration_t));
  if (forward_dcl == NULL) {
    context->out_of_memory = true;
    return false;
  }
  if (!init_definition(context, (dds_ts_definition_t*)forward_dcl, name, DDS_TS_FORWARD_STRUCT, context->cur_node)) {
    os_free((void*)forward_dcl);
    return false;
  }
  forward_dcl->definition = NULL;
  return true;
}

extern bool dds_ts_add_struct_open(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  dds_ts_struct_t *new_struct = (dds_ts_struct_t*)os_malloc(sizeof(dds_ts_struct_t));
  if (new_struct == NULL) {
    context->out_of_memory = true;
    return false;
  }
  if (!init_definition(context, (dds_ts_definition_t*)new_struct, name, DDS_TS_STRUCT, context->cur_node)) {
    os_free((void*)new_struct);
    return false;
  }
  new_struct->super = NULL;
  context->cur_node = (dds_ts_node_t*)new_struct;
  return true;
}

extern bool dds_ts_add_struct_extension_open(dds_ts_context_t *context, dds_ts_identifier_t name, dds_ts_scoped_name_t *scoped_name)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_MODULE));
  dds_ts_struct_t *new_struct = (dds_ts_struct_t*)os_malloc(sizeof(dds_ts_struct_t));
  if (new_struct == NULL) {
    context->out_of_memory = true;
    return false;
  }
  if (!init_definition(context, (dds_ts_definition_t*)new_struct, name, DDS_TS_STRUCT, context->cur_node)) {
    os_free((void*)new_struct);
    return false;
  }
  new_struct->super = NULL;
  dds_ts_definition_t *definition;
  if (!resolve_scoped_name(context, scoped_name, &definition)) {
    os_free((void*)new_struct);
    return false;
  }
  if (definition != NULL && definition->type_spec.node.flags == DDS_TS_STRUCT) {
    new_struct->super = definition;
  }
  context->cur_node = (dds_ts_node_t*)new_struct;
  return true;
}

extern bool dds_ts_add_struct_member(dds_ts_context_t *context, dds_ts_type_spec_t *type)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_STRUCT));
  dds_ts_struct_member_t *member = (dds_ts_struct_member_t*)os_malloc(sizeof(dds_ts_struct_member_t));
  if (member == NULL) {
    context->out_of_memory = true;
    return false;
  }
  init_node((dds_ts_node_t*)member, DDS_TS_STRUCT_MEMBER, context->cur_node);
  member->member_type = type;
  context->parent_for_declarator = (dds_ts_node_t*)member;
  return true;
}

extern void dds_ts_struct_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_STRUCT));
  context->cur_node = context->cur_node->parent;
  context->parent_for_declarator = NULL;
}

extern void dds_ts_struct_empty_close(dds_ts_context_t *context)
{
  assert(cur_scope_is_definition_type(context, DDS_TS_STRUCT));
  context->cur_node = context->cur_node->parent;
  context->parent_for_declarator = NULL;
}

extern bool dds_ts_add_declarator(dds_ts_context_t *context, dds_ts_identifier_t name)
{
  assert(context != NULL);
  if (context->parent_for_declarator != NULL && context->cur_node->flags == DDS_TS_STRUCT) {
    dds_ts_definition_t* declarator = (dds_ts_definition_t*)os_malloc(sizeof(dds_ts_definition_t));
    if (declarator == NULL) {
      context->out_of_memory = true;
      return false;
    }
    if (!init_definition(context, declarator, name, DDS_TS_DECLARATOR, context->parent_for_declarator)) {
      os_free((void*)declarator);
      return false;
    }
    return true;
  }
  assert(false);
  return false;
}

extern bool dds_ts_add_array_size(dds_ts_context_t *context, dds_ts_literal_t *value)
{
  assert(context != NULL);
  assert(value->flags == DDS_TS_UNSIGNED_LONG_LONG_TYPE);
  dds_ts_array_size_t *array_size = (dds_ts_array_size_t*)os_malloc(sizeof(dds_ts_array_size_t));
  if (array_size == NULL) {
    context->out_of_memory = true;
    return false;
  }
  init_node((dds_ts_node_t*)array_size, DDS_TS_ARRAY_SIZE, context->cur_node);
  array_size->size = value->value.ullng;
  return true;
}

