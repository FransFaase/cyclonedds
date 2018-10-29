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
#ifndef IDL_TYPE_H
#define IDL_TYPE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  idl_float,
  idl_fixed,
  idl_double,
  idl_longdouble,
  idl_short,
  idl_long,
  idl_longlong,
  idl_ushort,
  idl_ulong,
  idl_ulonglong,
  idl_char,
  idl_wchar,
  idl_boolean,
  idl_octet,
  idl_any
} idl_basic_type_t;

typedef enum {
  idl_integer_literal,
  idl_string_literal,
  idl_wide_string_literal,
  idl_character_literal,
  idl_wide_character_literal,
  idl_fixed_pt_literal,
  idl_floating_pt_literal,
  idl_boolean_literal,
} idl_literal_type_t;

typedef struct {
  idl_literal_type_t type;
  union {
    bool bln;
    char chr;
    char *str;
    long long llng;
    long double ldbl;
  } value;
} idl_literal_t;

typedef char *idl_identifier_t;


#endif /* IDL_TYPE_H */
