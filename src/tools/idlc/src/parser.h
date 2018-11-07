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
#ifndef IDL_PARSER_H
#define IDL_PARSER_H

int idl_parse_file(const char *file);
int idl_parse_string(const char *str, bool ignore_yyerror);

// For testing:
int idl_parse_string_stringify(const char *str, char *buffer, size_t len);

#endif /* IDL_PARSER_H */

