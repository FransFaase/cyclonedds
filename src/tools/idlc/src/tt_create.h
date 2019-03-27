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
#ifndef DDSTS_CREATE_H
#define DDSTS_CREATE_H

#include <stdbool.h>
#include "dds/ddsrt/retcode.h"

/* Some types only used during parsing */

typedef struct dds_scoped_name dds_scoped_name_t;
typedef struct dds_context dds_context_t;

dds_context_t* dds_create_context();
void dds_context_error(dds_context_t *context, int line, int column, const char *msg);
void dds_context_set_error_func(dds_context_t *context, void (*error)(int line, int column, const char *msg));
dds_retcode_t dds_context_get_retcode(dds_context_t* context);
ddsts_type_t* dds_context_take_root_type();
void dds_free_context(dds_context_t* context);

bool dds_context_copy_identifier(dds_context_t *context, ddsts_identifier_t source, ddsts_identifier_t *dest);

bool dds_new_base_type(dds_context_t *context, ddsts_flags_t flags, ddsts_type_t **result);
bool dds_new_sequence(dds_context_t *context, ddsts_type_t *element_type, ddsts_literal_t *size, ddsts_type_t **result);
bool dds_new_sequence_unbound(dds_context_t *context, ddsts_type_t *base, ddsts_type_t **result);
bool dds_new_string(dds_context_t *context, ddsts_literal_t *size, ddsts_type_t **result);
bool dds_new_string_unbound(dds_context_t *context, ddsts_type_t **result);
bool dds_new_wide_string(dds_context_t *context, ddsts_literal_t *size, ddsts_type_t **result);
bool dds_new_wide_string_unbound(dds_context_t *context, ddsts_type_t **result);
bool dds_new_fixed_pt(dds_context_t *context, ddsts_literal_t *digits, ddsts_literal_t *fraction_digits, ddsts_type_t **result);
bool dds_new_map(dds_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, ddsts_literal_t *size, ddsts_type_t **result);
bool dds_new_map_unbound(dds_context_t *context, ddsts_type_t *key_type, ddsts_type_t *value_type, ddsts_type_t **result);
bool dds_new_scoped_name(dds_context_t *context, dds_scoped_name_t* prev, bool top, ddsts_identifier_t name, dds_scoped_name_t **result);
bool dds_get_type_from_scoped_name(dds_context_t *context, dds_scoped_name_t *scoped_name, ddsts_type_t **result);
void dds_free_scoped_name(dds_scoped_name_t *scoped_name);

bool dds_module_open(dds_context_t *context, ddsts_identifier_t name);
void dds_module_close(dds_context_t *context);

bool dds_add_struct_forward(dds_context_t *context, ddsts_identifier_t name);
bool dds_add_struct_open(dds_context_t *context, ddsts_identifier_t name);
bool dds_add_struct_extension_open(dds_context_t *context, ddsts_identifier_t name, dds_scoped_name_t *scoped_name);
bool dds_add_struct_member(dds_context_t *context, ddsts_type_t **ref_type);
void dds_struct_close(dds_context_t *context, ddsts_type_t **result);
void dds_struct_empty_close(dds_context_t *context, ddsts_type_t **result);

bool dds_add_declarator(dds_context_t *context, ddsts_identifier_t name);

bool dds_add_array_size(dds_context_t *context, ddsts_literal_t *value);

void dds_accept(dds_context_t *context);

#endif /* DDSTS_CREATE_H */
