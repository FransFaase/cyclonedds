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

#include "stringify.h"


extern void ddsts_stringify(ddsts_node_t *root_node, ddsts_ostream_t *ostream)
{
  ddsts_walker_t *walker = ddsts_create_walker(root_node);

  ddsts_walker_def_proc(walker, "module");

    ddsts_walker_for_all_modules(walker);
      ddsts_walker_emit(walker, "module ");
      ddsts_walker_emit_name(walker);
      ddsts_walker_emit(walker, "{");
      ddsts_walker_call_proc(walker, "module");
      ddsts_walker_emit(walker, "}");
    ddsts_walker_end_for(walker);

    ddsts_walker_for_all_structs(walker);
      ddsts_walker_emit(walker, "struct ");
      ddsts_walker_emit_name(walker);
      ddsts_walker_emit(walker, "{");
      ddsts_walker_for_all_members(walker);
        ddsts_walker_emit_type(walker);
        ddsts_walker_emit(walker, " ");
        ddsts_walker_for_all_declarators(walker);
	  ddsts_walker_emit_name(walker);
	  ddsts_walker_emit(walker, ",");
        ddsts_walker_end_for(walker);
        ddsts_walker_emit(walker, ";");
      ddsts_walker_end_for(walker);
      ddsts_walker_emit(walker, "}");
    ddsts_walker_end_for(walker);

  ddsts_walker_end_def(walker);

  ddsts_walker_main(walker);
    ddsts_walker_call_proc(walker, "module");
  ddsts_walker_end(walker);

  ddsts_walker_execute(walker, NULL, ostream);

  ddsts_walker_free(walker);
}

