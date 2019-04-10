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
#ifndef DDS_TS_TEMPL_SCRIPT_H
#define DDS_TS_TEMPL_SCRIPT_H

#include "dds/ddsts/typetree.h"

void dds_exec_templ_script(const char* file, ddsts_type_t *root_node, FILE* tsf, bool debugging);

#endif /* DDS_TS_TEMPL_SCRIPT_H */
