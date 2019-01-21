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
#ifndef IDL_PARSER_H
#define IDL_PARSER_H

int ddsts_parse_file(const char *file, void (*error_func)(int line, int column, const char *msg), ddsts_node_t **ref_root_node);
int ddsts_parse_string(const char *str, void (*error_func)(int line, int column, const char *msg), ddsts_node_t **ref_root_node);

#endif /* IDL_PARSER_H */

