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
   idl_definition_t *module;
   idl_definition_t *next;
};

idl_module_it_t *idl_create_module_it_from_context(idl_context_t *context)
{
  assert(context != NULL && context->root_scope != NULL && context->root_scope->type == idl_definition_module);
  idl_module_it_t *module_it = (idl_module_it_t*)os_malloc(sizeof(idl_module_it_t));
  if (module_it == NULL) {
    assert(false);
    return NULL;
  }
  module_it->module = context->root_scope;
  module_it->next = NULL;
  return module_it;
}

idl_module_it_t *idl_create_module_it(idl_module_it_t *parent_module_it)
{
  assert(parent_module_it != NULL && parent_module_it->module != NULL);
  idl_module_it_t *module_it = (idl_module_it_t*)os_malloc(sizeof(idl_module_it_t));
  if (module_it == NULL) {
    assert(false);
    return NULL;
  }
  module_it->next = parent_module_it->module->module_def->definitions;
  idl_module_it_next(module_it);
  return module_it;
}

bool idl_module_it_more(idl_module_it_t *module_it)
{
  assert(module_it != NULL);
  return module_it->module != NULL;
}

void idl_module_it_next(idl_module_it_t *module_it)
{
  assert(module_it != NULL);
  module_it->module = NULL;
  for (; module_it->next != NULL && module_it->module == NULL; module_it->next = module_it->next->next) {
    if (module_it->next->type == idl_definition_module) {
      module_it->module = module_it->next;
    }
  }
}

const char* idl_module_it_name(idl_module_it_t *module_it)
{
  assert(module_it != NULL);
  return module_it->module->name;
}

void idl_free_module_it(idl_module_it_t **ref_module_it)
{
  assert(ref_module_it != NULL && *ref_module_it != NULL);
  os_free(*ref_module_it);
  *ref_module_it = NULL;
}


struct idl_struct_it {
   idl_definition_t *cur;
   idl_definition_t *next;
};

idl_struct_it_t *idl_create_struct_it(idl_module_it_t *module_it)
{
  assert(module_it != NULL && module_it->module != NULL);
  idl_struct_it_t *struct_it = (idl_struct_it_t*)os_malloc(sizeof(idl_struct_it_t));
  if (struct_it == NULL) {
    assert(false);
    return NULL;
  }
  struct_it->next = module_it->module->module_def->definitions;
  idl_struct_it_next(struct_it);
  return struct_it;
}

bool idl_struct_it_more(idl_struct_it_t *struct_it)
{
  assert(struct_it != NULL);
  return struct_it->cur != NULL;
}

void idl_struct_it_next(idl_struct_it_t *struct_it)
{
  assert(struct_it != NULL);
  struct_it->cur = NULL;
  for (; struct_it->next != NULL && struct_it->cur == NULL; struct_it->next = struct_it->next->next) {
    if (struct_it->next->type == idl_definition_struct) {
      struct_it->cur = struct_it->next;
    }
  }
}

const char* idl_struct_it_name(idl_struct_it_t *struct_it)
{
  assert(struct_it != NULL);
  return struct_it->cur->name;
}

void idl_free_struct_it(idl_struct_it_t **ref_struct_it)
{
  assert(struct_it != NULL && *ref_struct_it != NULL);
  os_free(*ref_struct_it);
  *ref_struct_it = NULL;
}


struct idl_struct_member_it {
  idl_struct_member_t *struct_member;
};

idl_struct_member_it_t *idl_create_struct_member_it(idl_struct_it_t *struct_it)
{
  assert(enum_it != NULL && enum_it->cur != NULL);
  idl_struct_member_it_t *struct_member_it = (idl_struct_member_it_t*)os_malloc(sizeof(idl_struct_member_it_t));
  if (struct_member_it == NULL) {
    assert(false);
    return NULL;
  }
  struct_member_it->struct_member = struct_it->cur->struct_def->members;
  return struct_member_it;
}

bool idl_struct_member_it_more(idl_struct_member_it_t *struct_member_it)
{
  assert(struct_member_it != NULL);
  return struct_member_it->struct_member != NULL;
}

void idl_struct_member_it_next(idl_struct_member_it_t *struct_member_it)
{
  assert(struct_member_it != NULL);
  struct_member_it->struct_member = struct_member_it->struct_member->next;
}

void idl_free_struct_member_it(idl_struct_member_it_t **ref_struct_member_it)
{
  assert(ref_struct_member_it != NULL && *ref_struct_member_it != NULL);
  os_free(*ref_struct_member_it);
  *ref_struct_member_it = NULL;
}


struct idl_union_it {
   idl_definition_t *cur;
   idl_definition_t *next;
};

idl_union_it_t *idl_create_union_it(idl_module_it_t *module_it)
{
  assert(module_it != NULL && module_it->module != NULL);
  idl_union_it_t *union_it = (idl_union_it_t*)os_malloc(sizeof(idl_union_it_t));
  if (union_it == NULL) {
    assert(false);
    return NULL;
  }
  union_it->next = module_it->module->module_def->definitions;
  idl_union_it_next(union_it);
  return union_it;
}

bool idl_union_it_more(idl_union_it_t *union_it)
{
  assert(union_it != NULL);
  return union_it->cur != NULL;
}

void idl_union_it_next(idl_union_it_t *union_it)
{
  assert(union_it != NULL);
  union_it->cur = NULL;
  for (; union_it->next != NULL && union_it->cur == NULL; union_it->next = union_it->next->next) {
    if (union_it->next->type == idl_definition_union) {
      union_it->cur = union_it->next;
    }
  }
}

const char* idl_union_it_name(idl_union_it_t *union_it)
{
  assert(union_it != NULL);
  return union_it->cur->name;
}

void idl_free_union_it(idl_union_it_t **ref_union_it)
{
  assert(union_it != NULL && *ref_union_it != NULL);
  os_free(*ref_union_it);
  *ref_union_it = NULL;
}


struct idl_union_case_it {
  idl_union_case_t *union_case;
};

idl_union_case_it_t *idl_create_union_case_it(idl_union_it_t *union_it)
{
  assert(enum_it != NULL && enum_it->cur != NULL);
  idl_union_case_it_t *union_case_it = (idl_union_case_it_t*)os_malloc(sizeof(idl_union_case_it_t));
  if (union_case_it == NULL) {
    assert(false);
    return NULL;
  }
  union_case_it->union_case = union_it->cur->union_def->cases;
  return union_case_it;
}

bool idl_union_case_it_more(idl_union_case_it_t *union_case_it)
{
  assert(union_case_it != NULL);
  return union_case_it->union_case != NULL;
}

void idl_union_case_it_next(idl_union_case_it_t *union_case_it)
{
  assert(union_case_it != NULL);
  union_case_it->union_case = union_case_it->union_case->next;
}

void idl_free_union_case_it(idl_union_case_it_t **ref_union_case_it)
{
  assert(ref_union_case_it != NULL && *ref_union_case_it != NULL);
  os_free(*ref_union_case_it);
  *ref_union_case_it = NULL;
}


struct idl_union_case_label_it {
  idl_union_case_label_t *union_case_label;
};

idl_union_case_label_it_t *idl_create_union_case_label_it(idl_union_case_it_t *union_case_it)
{
  assert(enum_it != NULL && enum_it->cur != NULL);
  idl_union_case_label_it_t *union_case_label_it = (idl_union_case_label_it_t*)os_malloc(sizeof(idl_union_case_label_it_t));
  if (union_case_label_it == NULL) {
    assert(false);
    return NULL;
  }
  union_case_label_it->union_case_label = union_case_it->union_case->labels;
  return union_case_label_it;
}

bool idl_union_case_label_it_more(idl_union_case_label_it_t *union_case_label_it)
{
  assert(union_case_label_it != NULL);
  return union_case_label_it->union_case_label != NULL;
}

void idl_union_case_label_it_next(idl_union_case_label_it_t *union_case_label_it)
{
  assert(union_case_label_it != NULL);
  union_case_label_it->union_case_label = union_case_label_it->union_case_label->next;
}

bool idl_union_case_label_it_is_default(idl_union_case_label_it_t *union_case_label_it)
{
  assert(union_case_label_it != NULL);
  return union_case_label_it->union_case_label->is_default;
}

void idl_free_union_case_label_it(idl_union_case_label_it_t **ref_union_case_label_it)
{
  assert(ref_union_case_label_it != NULL && *ref_union_case_label_it != NULL);
  os_free(*ref_union_case_label_it);
  *ref_union_case_label_it = NULL;
}


struct idl_declarator_it {
   idl_definition_t *cur;
};

idl_declarator_it_t *idl_create_declarator_it_for_struct(idl_struct_member_it_t *struct_member_it)
{
  assert(module_it != NULL && module_it->module != NULL);
  idl_declarator_it_t *declarator_it = (idl_declarator_it_t*)os_malloc(sizeof(idl_declarator_it_t));
  if (declarator_it == NULL) {
    assert(false);
    return NULL;
  }
  declarator_it->cur = struct_member_it->struct_member->declarators;
  return declarator_it;
}

idl_declarator_it_t *idl_create_declarator_it_for_union(idl_union_case_it_t *union_case_it)
{
  assert(module_it != NULL && module_it->module != NULL);
  idl_declarator_it_t *declarator_it = (idl_declarator_it_t*)os_malloc(sizeof(idl_declarator_it_t));
  if (declarator_it == NULL) {
    assert(false);
    return NULL;
  }
  declarator_it->cur = union_case_it->union_case->declarators;
  return declarator_it;
}

bool idl_declarator_it_more(idl_declarator_it_t *declarator_it)
{
  assert(declarator_it != NULL);
  return declarator_it->cur != NULL;
}

void idl_declarator_it_next(idl_declarator_it_t *declarator_it)
{
  assert(declarator_it != NULL);
  declarator_it->cur = declarator_it->cur->next;
}

const char* idl_declarator_it_name(idl_declarator_it_t *declarator_it)
{
  assert(declarator_it != NULL);
  return declarator_it->cur->name;
}

void idl_free_declarator_it(idl_declarator_it_t **ref_declarator_it)
{
  assert(declarator_it != NULL && *ref_declarator_it != NULL);
  os_free(*ref_declarator_it);
  *ref_declarator_it = NULL;
}


struct idl_enum_it {
   idl_definition_t *cur;
   idl_definition_t *next;
};

idl_enum_it_t *idl_create_enum_it(idl_module_it_t *module_it)
{
  assert(module_it != NULL && module_it->module != NULL);
  idl_enum_it_t *enum_it = (idl_enum_it_t*)os_malloc(sizeof(idl_enum_it_t));
  if (enum_it == NULL) {
    assert(false);
    return NULL;
  }
  enum_it->next = module_it->module->module_def->definitions;
  idl_enum_it_next(enum_it);
  return enum_it;
}

bool idl_enum_it_more(idl_enum_it_t *enum_it)
{
  assert(enum_it != NULL);
  return enum_it->cur != NULL;
}

void idl_enum_it_next(idl_enum_it_t *enum_it)
{
  assert(enum_it != NULL);
  enum_it->cur = NULL;
  for (; enum_it->next != NULL && enum_it->cur == NULL; enum_it->next = enum_it->next->next) {
    if (enum_it->next->type == idl_definition_enum) {
      enum_it->cur = enum_it->next;
    }
  }
}

const char* idl_enum_it_name(idl_enum_it_t *enum_it)
{
  assert(enum_it != NULL);
  return enum_it->cur->name;
}

void idl_free_enum_it(idl_enum_it_t **ref_enum_it)
{
  assert(enum_it != NULL && *ref_enum_it != NULL);
  os_free(*ref_enum_it);
  *ref_enum_it = NULL;
}


struct idl_enum_value_it {
  idl_enum_value_definition_t *enum_value;
};

idl_enum_value_it_t *idl_create_enum_value_it(idl_enum_it_t *enum_it)
{
  assert(enum_it != NULL && enum_it->cur != NULL);
  idl_enum_value_it_t *enum_value_it = (idl_enum_value_it_t*)os_malloc(sizeof(idl_enum_value_it_t));
  if (enum_value_it == NULL) {
    assert(false);
    return NULL;
  }
  enum_value_it->enum_value = enum_it->cur->enum_def->values;
  return enum_value_it;
}

bool idl_enum_value_it_more(idl_enum_value_it_t *enum_value_it)
{
  assert(enum_value_it != NULL);
  return enum_value_it->enum_value != NULL;
}

void idl_enum_value_it_next(idl_enum_value_it_t *enum_value_it)
{
  assert(enum_value_it != NULL);
  enum_value_it->enum_value = enum_value_it->enum_value->next;
}

const char* idl_enum_value_it_name(idl_enum_value_it_t *enum_value_it)
{
  assert(enum_value_it != NULL && enum_value_it->enum_value != NULL);
  return enum_value_it->enum_value->def->name;
}

void idl_free_enum_value_it(idl_enum_value_it_t **ref_enum_value_it)
{
  assert(ref_enum_value_it != NULL && *ref_enum_value_it != NULL);
  os_free(*ref_enum_value_it);
  *ref_enum_value_it = NULL;
}

