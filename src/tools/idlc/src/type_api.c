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
#include <stdbool.h>
#include "type.h"
#include "type_create.h"
#include "type_api.h"
#include "type_priv.h"
#include "os/os.h"

struct idl_module_it
{
   idl_definition_t *next;
   idl_definition_t *module;
};

idl_module_it_t *idl_create_module_it_from_context(idl_context_t *context)
{
  if (context == 0 || context->root_scope == 0 || context->root_scope->type != idl_definition_module) {
    assert(0);
    return 0;
  }
  idl_module_it_t *module_it = (idl_module_it_t*)os_malloc(sizeof(idl_module_it_t));
  module_it->module = context->root_scope;
  module_it->next = 0;
  return module_it;
}

idl_module_it_t *idl_create_module_it(idl_module_it_t *parent_module_it)
{
  if (parent_module_it == 0 || parent_module_it->module == 0) {
    assert(0);
    return 0;
  }
  idl_module_it_t *module_it = (idl_module_it_t*)os_malloc(sizeof(idl_module_it_t));
  module_it->next = parent_module_it->module->module_def->definitions;
  idl_module_it_next(module_it);
  return module_it;
}

bool idl_module_it_more(idl_module_it_t *module_it)
{
  if (module_it == 0) {
    assert(0);
    return false;
  }
  return module_it->module != 0;
}

void idl_module_it_next(idl_module_it_t *module_it)
{
  if (module_it == 0) {
    assert(0);
    return;
  }
  module_it->module = 0;
  for (; module_it->next != 0 && module_it->module == 0; module_it->next = module_it->next->next) {
    if (module_it->next->type == idl_definition_module) {
      module_it->module = module_it->next;
    }
  }
}

const char* idl_module_it_name(idl_module_it_t *module_it)
{
  if (module_it == 0) {
    assert(0);
    return "";
  }
  return module_it->module->name;
}

void idl_free_module_it(idl_module_it_t **ref_module_it)
{
  if (ref_module_it == 0 || *ref_module_it == 0) {
    assert(0);
    return;
  }
  os_free(*ref_module_it);
  *ref_module_it = 0;
}


struct idl_enum_it {
   idl_definition_t *next;
   idl_definition_t *enum_def;
};

idl_enum_it_t *idl_create_enum_it(idl_module_it_t *module_it)
{
  if (module_it == 0 || module_it->module == 0) {
    assert(0);
    return 0;
  }
  idl_enum_it_t *enum_it = (idl_enum_it_t*)os_malloc(sizeof(idl_enum_it_t));
  enum_it->next = module_it->module->module_def->definitions;
  idl_enum_it_next(enum_it);
  return enum_it;
}

bool idl_enum_it_more(idl_enum_it_t *enum_it)
{
  if (enum_it == 0) {
    assert(0);
    return false;
  }
  return enum_it->enum_def != 0;
}

void idl_enum_it_next(idl_enum_it_t *enum_it)
{
  if (enum_it == 0) {
    assert(0);
    return;
  }
  enum_it->enum_def = 0;
  for (; enum_it->next != 0 && enum_it->enum_def == 0; enum_it->next = enum_it->next->next) {
    if (enum_it->next->type == idl_definition_enum) {
      enum_it->enum_def = enum_it->next;
    }
  }
}

const char* idl_enum_it_name(idl_enum_it_t *enum_it)
{
  if (enum_it == 0) {
    assert(0);
    return "";
  }
  return enum_it->enum_def->name;
}

void idl_free_enum_it(idl_enum_it_t **ref_enum_it)
{
  *ref_enum_it = 0;
}


struct idl_enum_value_it {
};

idl_enum_value_it_t *idl_create_enum_value_it(idl_enum_it_t *enum_it)
{
  (void)enum_it;
  return 0;
}

bool idl_enum_value_it_more(idl_enum_value_it_t *enum_value_it)
{
  (void)enum_value_it;
  return false;
}

void idl_enum_value_it_next(idl_enum_value_it_t *enum_value_it)
{
  (void)enum_value_it;
}

const char* idl_enum_value_it_name(idl_enum_value_it_t *enum_value_it)
{
  (void)enum_value_it;
  return "";
}

void idl_free_enum_value_it(idl_enum_value_it_t **ref_enum_value_it)
{
  *ref_enum_value_it = 0;
}

