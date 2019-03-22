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


void ddsts_free_type(ddsts_type_t *type)
{
  if (type != NULL) {
    type->type.free_func(type);
  }
}

static void set_type_ref(ddsts_type_t **ref_type, ddsts_type_t *type, ddsts_type_t *parent)
{
  *ref_type = type;
  if (type != NULL && type->type.parent == NULL) {
    type->type.parent = parent;
  }
}

static void free_type_ref(ddsts_type_t *type, ddsts_type_t *parent)
{
  if (type != NULL && type->type.parent == parent) {
    ddsts_free_type(type);
  }
}

static void free_children(ddsts_type_t *type)
{
  while (type != NULL) {
    ddsts_type_t *next = type->type.next;
    ddsts_free_type(type);
    type = next;
  }
}

static void init_type(ddsts_type_t *type, void (*free_func)(ddsts_type_t*), ddsts_flags_t flags, ddsts_identifier_t name)
{
  type->type.flags = flags;
  type->type.name = name;
  type->type.parent = NULL;
  type->type.next = NULL;
  type->type.free_func = free_func;
}

static void free_type(ddsts_type_t *type)
{
  os_free((void*)type->type.name);
  os_free((void*)type);
}

/* ddsts_base_type_t */

ddsts_type_t *ddsts_create_base_type(ddsts_flags_t flags)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_base_type_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_type, flags, NULL);
  return type;
}

/* ddsts_sequence_t */

static void free_sequence(ddsts_type_t *type)
{
  free_type_ref(type->sequence.element_type, type);
  free_type(type);
}

ddsts_type_t *ddsts_create_sequence(ddsts_type_t* element_type, unsigned long long max)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_sequence_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_sequence, DDSTS_SEQUENCE, NULL);
  set_type_ref(&type->sequence.element_type, element_type, type);
  type->sequence.max = max;
  return type;
}

/* ddsts_array_t */

static void free_array(ddsts_type_t *type)
{
  free_type_ref(type->array.element_type, type);
  free_type(type);
}

ddsts_type_t *ddsts_create_array(ddsts_type_t* element_type, unsigned long long size)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_array_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_array, DDSTS_SEQUENCE, NULL);
  set_type_ref(&type->array.element_type, element_type, type);
  type->array.size = size;
  return type;
}

/* ddsts_string_t */

static void free_string(ddsts_type_t *type)
{
  free_type(type);
}

ddsts_type_t *ddsts_create_string(ddsts_flags_t flags, unsigned long long max)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_string_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_string, flags, NULL);
  type->string.max = max;
  return type;
}

/* ddsts_fixed_pt_t */

static void free_fixed_pt(ddsts_type_t *type)
{
  free_type(type);
}

ddsts_type_t *ddsts_create_fixed_pt(unsigned long long digits, unsigned long long fraction_digits)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_fixed_pt_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_fixed_pt, DDSTS_FIXED_PT, NULL);
  type->fixed_pt.digits = digits;
  type->fixed_pt.fraction_digits = fraction_digits;
  return type;
}

/* ddsts_map_t */

static void free_map(ddsts_type_t *type)
{
  free_type_ref(type->map.key_type, type);
  free_type_ref(type->map.value_type, type);
  free_type(type);
}

ddsts_type_t *ddsts_create_map(ddsts_type_t *key_type, ddsts_type_t *value_type, unsigned long long max)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_map_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_map, DDSTS_MAP, NULL);
  set_type_ref(&type->map.key_type, key_type, type);
  set_type_ref(&type->map.value_type, value_type, type);
  type->map.max = max;
  return type;
}

/* ddsts_module_t */

static void free_module(ddsts_type_t *type)
{
  free_children(type->module.members);
  free_type(type);
}

ddsts_type_t *ddsts_create_module(ddsts_identifier_t name)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_module_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_module, DDSTS_MODULE, name);
  type->module.members = NULL;
  type->module.previous = NULL;
  return type;
}

/* ddsts_forward_declaration_t */

static void free_forward(ddsts_type_t *type)
{
  free_type(type);
}

ddsts_type_t *ddsts_create_struct_forward_dcl(ddsts_identifier_t name)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_forward_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_forward, DDSTS_FORWARD_STRUCT, name);
  type->forward.definition = NULL;
  return type;
}

/* ddsts_struct_t */

static void free_struct(ddsts_type_t *type)
{
  free_children(type->struct_def.members);
  free_type(type);
}

ddsts_type_t *ddsts_create_struct(ddsts_identifier_t name)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_struct_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_struct, DDSTS_STRUCT, name);
  type->struct_def.members = NULL;
  type->struct_def.super = NULL;
  return type;
}

/* ddsts_declarator_t */

static void free_declaration(ddsts_type_t *type)
{
  free_type_ref(type->declaration.decl_type, type);
  free_type(type);
}

ddsts_type_t *ddsts_create_declaration(ddsts_identifier_t name, ddsts_type_t *decl_type)
{
  ddsts_type_t *type = (ddsts_type_t*)os_malloc(sizeof(ddsts_declaration_t));
  if (type == NULL) {
    return NULL;
  }
  init_type(type, free_declaration, DDSTS_DECLARATION, name);
  type->declaration.decl_type = decl_type;
  return type;
}

