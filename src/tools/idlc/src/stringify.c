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
#include <stdio.h>
#include <stdbool.h>

#include "dds_tt.h"
#include "type_walker.h"


extern void dds_tt_stringify(dds_tt_node_t *context, char *buffer, size_t len)
{
  dds_tt_walker_t *walker = dds_tt_create_walker(context);

  dds_tt_walker_def_proc(walker, "module");
    dds_tt_walker_for_all_modules(walker);
      dds_tt_walker_emit(walker, "module ");
      dds_tt_walker_emit_name(walker);
      dds_tt_walker_emit(walker, "{");
      dds_tt_walker_call_proc(walker, "module");
      dds_tt_walker_emit(walker, "}");
    dds_tt_walker_end_for(walker);

    dds_tt_walker_for_all_structs(walker);
      dds_tt_walker_emit(walker, "struct ");
      dds_tt_walker_emit_name(walker);
      dds_tt_walker_emit(walker, "{");
      dds_tt_walker_for_all_members(walker);
        dds_tt_walker_for_all_declarators(walker);
	  dds_tt_walker_emit_name(walker);
	  dds_tt_walker_emit(walker, ",");
        dds_tt_walker_end_for(walker);
        dds_tt_walker_emit(walker, ";");
      dds_tt_walker_end_for(walker);
      dds_tt_walker_emit(walker, "}");
    dds_tt_walker_end_for(walker);

    dds_tt_walker_for_all_unions(walker);
      dds_tt_walker_emit(walker, "union ");
      dds_tt_walker_emit_name(walker);
      dds_tt_walker_emit(walker, "{");
      dds_tt_walker_for_all_cases(walker);
        dds_tt_walker_for_all_case_labels(walker);
	  dds_tt_walker_if_default_case_label(walker);
	    dds_tt_walker_emit(walker, "d:");
	  dds_tt_walker_else(walker);
	    dds_tt_walker_emit(walker, "c:");
	  dds_tt_walker_end_if(walker);
	dds_tt_walker_end_for(walker);
        dds_tt_walker_for_all_declarators(walker);
	  dds_tt_walker_emit_name(walker);
	  dds_tt_walker_emit(walker, ",");
        dds_tt_walker_end_for(walker);
        dds_tt_walker_emit(walker, ";");
      dds_tt_walker_end_for(walker);
      dds_tt_walker_emit(walker, "}");
    dds_tt_walker_end_for(walker);

    dds_tt_walker_for_all_enums(walker);
      dds_tt_walker_emit(walker, "enum ");
      dds_tt_walker_emit_name(walker);
      dds_tt_walker_emit(walker, "{");
      dds_tt_walker_for_all_enum_value(walker);
        dds_tt_walker_emit_name(walker);
	dds_tt_walker_emit(walker, ",");
      dds_tt_walker_end_for(walker);
      dds_tt_walker_emit(walker, "}");
    dds_tt_walker_end_for(walker);
  dds_tt_walker_end_def(walker);

  dds_tt_walker_main(walker);
    dds_tt_walker_call_proc(walker, "module");
  dds_tt_walker_end(walker);

  dds_tt_walker_execute(walker, buffer, len);
}

