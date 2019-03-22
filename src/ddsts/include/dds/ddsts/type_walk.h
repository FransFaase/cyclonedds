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

#ifndef DDSTS_TYPE_WALK_H
#define DDSTS_TYPE_WALK_H

typedef struct ddsts_walk_exec_state_t ddsts_walk_exec_state_t;
struct ddsts_walk_exec_state_t {
  ddsts_type_t *type;
  ddsts_walk_exec_state_t *call_parent;
};

typedef void (*ddsts_walk_call_func_t)(ddsts_walk_exec_state_t *exec_state, void *context);

void ddsts_walk(ddsts_walk_exec_state_t *exec_state, ddsts_flags_t visit, ddsts_flags_t call, ddsts_walk_call_func_t func, void *context);

#endif /* DDSTS_TYPE_WALK_H */
