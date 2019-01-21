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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "dds/ddsts/typetree.h"
#include "dds/ddsts/type_walk.h"

void ddsts_walk(ddsts_walk_exec_state_t *exec_state, ddsts_node_flags_t visit, ddsts_node_flags_t call, ddsts_walk_call_func_t func, void *context)
{
  ddsts_walk_exec_state_t child_exec_state;
  child_exec_state.call_parent = exec_state;
  for (ddsts_node_t *child = exec_state->node->children; child != NULL; child = child->next) {
    child_exec_state.node = child;
    if ((child->flags & call) != 0) {
      func(&child_exec_state, context);
    }
    if ((child->flags & visit) != 0) {
      ddsts_walk(&child_exec_state, visit, call, func, context);
    }
  }
}

