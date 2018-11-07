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

typedef struct idl_context idl_context_t;

typedef struct idl_module_it idl_module_it_t;
idl_module_it_t *idl_create_module_it_from_context(idl_context_t *context);
idl_module_it_t *idl_create_module_it(idl_module_it_t *module_it);
bool idl_module_it_more(idl_module_it_t *module_it);
void idl_module_it_next(idl_module_it_t *module_it);
const char* idl_module_it_name(idl_module_it_t *module_it);
void idl_free_module_it(idl_module_it_t **ref_module_it);

typedef struct idl_enum_it idl_enum_it_t;
idl_enum_it_t *idl_create_enum_it(idl_module_it_t *module_it);
bool idl_enum_it_more(idl_enum_it_t *enum_it);
void idl_enum_it_next(idl_enum_it_t *enum_it);
const char* idl_enum_it_name(idl_enum_it_t *enum_it);
void idl_free_enum_it(idl_enum_it_t **ref_enum_it);

typedef struct idl_enum_value_it idl_enum_value_it_t;
idl_enum_value_it_t *idl_create_enum_value_it(idl_enum_it_t *enum_it);
bool idl_enum_value_it_more(idl_enum_value_it_t *enum_value_it);
void idl_enum_value_it_next(idl_enum_value_it_t *enum_value_it);
const char* idl_enum_value_it_name(idl_enum_value_it_t *enum_value_it);
void idl_free_enum_value_it(idl_enum_value_it_t **ref_enum_value_it);

