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
#ifndef IDL_TYPE_ENUMS_H
#define IDL_TYPE_ENUMS_H

typedef enum {
  idl_int8,
  idl_uint8,
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

typedef enum {
  idl_operator_or,
  idl_operator_xor,
  idl_operator_and,
  idl_operator_shift_left,
  idl_operator_shift_right,
  idl_operator_add,
  idl_operator_sub,
  idl_operator_times,
  idl_operator_div,
  idl_operator_mod,
  idl_operator_minus,
  idl_operator_plus,
  idl_operator_inv
} idl_operator_type_t;

#endif /* IDL_TYPE_ENUMS_H */
