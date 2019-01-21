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

#include "dds/ddsts/typetree.h"
#include <assert.h>
#include "os/os.h"

/* ddsts_node_t */

extern void ddsts_free_node(ddsts_node_t *node)
{
  if (node != NULL) {
    node->free_func(node);
  }
}

static void init_node(ddsts_node_t *node, void (*free_func)(ddsts_node_t*), ddsts_node_flags_t flags)
{
  node->free_func = free_func;
  node->flags = flags;
  node->parent = NULL;
  node->children = NULL;
  node->next = NULL;
}

static void free_node(ddsts_node_t *node)
{
  ddsts_node_t *child;
  for (child = node->children; child != NULL;) {
    ddsts_node_t *next = child->next;
    ddsts_free_node(child);
    child = next;
  }
  os_free((void*)node);
}

/* ddsts_type_spec_t (is abstract) */

static void init_type_spec(ddsts_type_spec_t *type_spec, void (*free_func)(ddsts_node_t*), ddsts_node_flags_t flags)
{
  init_node(&type_spec->node, free_func, flags);
}

static void free_type_spec(ddsts_type_spec_t *type_spec)
{
  free_node(&type_spec->node);
}

/* ddsts_type_spec_ptr_t */

void ddsts_type_spec_ptr_assign(ddsts_type_spec_ptr_t *type_spec_ptr, ddsts_type_spec_t *type_spec)
{
  type_spec_ptr->type_spec = type_spec;
  type_spec_ptr->is_reference = false;
}

void ddsts_type_spec_ptr_assign_reference(ddsts_type_spec_ptr_t *type_spec_ptr, ddsts_type_spec_t *type_spec)
{
  type_spec_ptr->type_spec = type_spec;
  type_spec_ptr->is_reference = true;
}

/* ddsts_base_type_t */

static void init_base_type(ddsts_base_type_t *base_type, void (*free_func)(ddsts_node_t*), ddsts_node_flags_t flags)
{
  init_type_spec(&base_type->type_spec, free_func, flags);
}

static void free_base_type(ddsts_node_t *node)
{
  free_type_spec(&((ddsts_base_type_t*)node)->type_spec);
}

extern ddsts_base_type_t *ddsts_create_base_type(ddsts_node_flags_t flags)
{
  ddsts_base_type_t *base_type = (ddsts_base_type_t*)os_malloc(sizeof(ddsts_base_type_t));
  if (base_type == NULL) {
    return NULL;
  }
  init_base_type(base_type, free_base_type, flags);
  return base_type;
}

/* ddsts_type_spec_ptr_t */

static void free_type_spec_ptr(ddsts_type_spec_ptr_t* type_spec_ptr)
{
  if (!type_spec_ptr->is_reference) {
    ddsts_free_node(&type_spec_ptr->type_spec->node);
  }
}

/* ddsts_sequence_t */

static void free_sequence(ddsts_node_t *node)
{
  free_type_spec_ptr(&((ddsts_sequence_t*)node)->element_type);
  free_type_spec(&((ddsts_sequence_t*)node)->type_spec);
}

extern ddsts_sequence_t *ddsts_create_sequence(ddsts_type_spec_ptr_t* element_type, bool bounded, unsigned long long max)
{
  ddsts_sequence_t *sequence = (ddsts_sequence_t*)os_malloc(sizeof(ddsts_sequence_t));
  if (sequence == NULL) {
    return NULL;
  }
  init_type_spec(&sequence->type_spec, free_sequence, DDSTS_SEQUENCE);
  sequence->element_type = *element_type;
  sequence->bounded = bounded;
  sequence->max = max;
  return sequence;
}

/* ddsts_string_t */

static void free_string(ddsts_node_t *node)
{
  free_type_spec(&((ddsts_string_t*)node)->type_spec);
}

extern ddsts_string_t *ddsts_create_string(ddsts_node_flags_t flags, bool bounded, unsigned long long max)
{
  ddsts_string_t *string = (ddsts_string_t*)os_malloc(sizeof(ddsts_string_t));
  if (string == NULL) {
    return NULL;
  }
  init_type_spec(&string->type_spec, free_string, flags);
  string->bounded = bounded;
  string->max = max;
  return string;
}

/* ddsts_fixed_pt_t */

static void free_fixed_pt(ddsts_node_t *node)
{
  free_type_spec(&((ddsts_fixed_pt_t*)node)->type_spec);
}

extern ddsts_fixed_pt_t *ddsts_create_fixed_pt(unsigned long long digits, unsigned long long fraction_digits)
{
  ddsts_fixed_pt_t *fixed_pt = (ddsts_fixed_pt_t*)os_malloc(sizeof(ddsts_fixed_pt_t));
  if (fixed_pt == NULL) {
    return NULL;
  }
  init_type_spec(&fixed_pt->type_spec, free_fixed_pt, DDSTS_FIXED_PT);
  fixed_pt->digits = digits;
  fixed_pt->fraction_digits = fraction_digits;
  return fixed_pt;
}

/* ddsts_map_t */

static void free_map(ddsts_node_t *node)
{
  free_type_spec_ptr(&((ddsts_map_t*)node)->key_type);
  free_type_spec_ptr(&((ddsts_map_t*)node)->value_type);
  free_type_spec(&((ddsts_map_t*)node)->type_spec);
}

extern ddsts_map_t *ddsts_create_map(ddsts_type_spec_ptr_t *key_type, ddsts_type_spec_ptr_t *value_type, bool bounded, unsigned long long max)
{
  ddsts_map_t *map = (ddsts_map_t*)os_malloc(sizeof(ddsts_map_t));
  if (map == NULL) {
    return NULL;
  }
  init_type_spec(&map->type_spec, free_map, DDSTS_MAP);
  map->key_type = *key_type;
  map->value_type = *value_type;
  map->bounded = bounded;
  map->max = max;
  return map;
}

/* ddsts_definition_t (is abstract) */

static void init_definition(ddsts_definition_t *definition, void (*free_func)(ddsts_node_t*), ddsts_node_flags_t flags, ddsts_identifier_t name)
{
  init_type_spec(&definition->type_spec, free_func, flags);
  definition->name = name;
}

static void free_definition(ddsts_node_t *node)
{
  os_free((void*)((ddsts_definition_t*)node)->name);
  free_type_spec(&((ddsts_definition_t*)node)->type_spec);
}

/* ddsts_module_t */

static void free_module(ddsts_node_t *node)
{
  free_definition(node);
}

ddsts_module_t *ddsts_create_module(ddsts_identifier_t name)
{
  ddsts_module_t *module = (ddsts_module_t*)os_malloc(sizeof(ddsts_module_t));
  if (module == NULL) {
    return NULL;
  }
  init_definition(&module->def, free_module, DDSTS_MODULE, name);
  module->previous = NULL;
  return module;
}

/* ddsts_forward_declaration_t */

static void free_forward_declaration(ddsts_node_t *node)
{
  free_definition(node);
}

extern ddsts_forward_declaration_t *ddsts_create_struct_forward_dcl(ddsts_identifier_t name)
{
  ddsts_forward_declaration_t *forward_dcl = (ddsts_forward_declaration_t*)os_malloc(sizeof(ddsts_forward_declaration_t));
  if (forward_dcl == NULL) {
    return NULL;
  }
  init_definition(&forward_dcl->def, free_forward_declaration, DDSTS_FORWARD_STRUCT, name);
  forward_dcl->definition = NULL;
  return forward_dcl;
}

/* ddsts_struct_t */

static void free_struct(ddsts_node_t *node)
{
  free_definition(node);
}

extern ddsts_struct_t *ddsts_create_struct(ddsts_identifier_t name)
{
  ddsts_struct_t *new_struct = (ddsts_struct_t*)os_malloc(sizeof(ddsts_struct_t));
  if (new_struct == NULL) {
    return NULL;
  }
  init_definition(&new_struct->def, free_struct, DDSTS_STRUCT, name);
  new_struct->super = NULL;
  new_struct->part_of = false;
  return new_struct;
}

/* ddsts_struct_member_t */

static void free_struct_member(ddsts_node_t *node)
{
  free_type_spec_ptr(&((ddsts_struct_member_t*)node)->member_type);
  free_node(&((ddsts_struct_member_t*)node)->node);
}

extern ddsts_struct_member_t *ddsts_create_struct_member(ddsts_type_spec_ptr_t *member_type)
{
  ddsts_struct_member_t *member = (ddsts_struct_member_t*)os_malloc(sizeof(ddsts_struct_member_t));
  if (member == NULL) {
    return NULL;
  }
  init_node(&member->node, free_struct_member, DDSTS_STRUCT_MEMBER);
  member->member_type = *member_type;
  return member;
}

/* Declarator */

extern ddsts_definition_t *ddsts_create_declarator(ddsts_identifier_t name)
{
  ddsts_definition_t* declarator = (ddsts_definition_t*)os_malloc(sizeof(ddsts_definition_t));
  if (declarator == NULL) {
    return NULL;
  }
  init_definition(declarator, free_definition, DDSTS_DECLARATOR, name);
  return declarator;
}

/* ddsts_array_size_t */

static void free_array_size(ddsts_node_t *node)
{
  free_node(&((ddsts_array_size_t*)node)->node);
}

extern ddsts_array_size_t *ddsts_create_array_size(unsigned long long size)
{
  ddsts_array_size_t *array_size = (ddsts_array_size_t*)os_malloc(sizeof(ddsts_array_size_t));
  if (array_size == NULL) {
    return NULL;
  }
  init_node(&array_size->node, free_array_size, DDSTS_ARRAY_SIZE);
  array_size->size = size;
  return array_size;
}

