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

#include "typetree.h"
#include <assert.h>
#include "os/os.h"

static void free_node(dds_ts_node_t *node)
{
  os_free((void*)node);
}

static void free_type_spec(dds_ts_type_spec_t *type_spec)
{
  free_node((dds_ts_node_t*)type_spec);
}

static void free_base_type(dds_ts_base_type_t *base_type)
{
  free_type_spec((dds_ts_type_spec_t*)base_type);
}

static void free_sequence(dds_ts_sequence_t *sequence)
{
  free_type_spec((dds_ts_type_spec_t*)sequence->element_type);
  free_type_spec((dds_ts_type_spec_t*)sequence);
}

static void free_string(dds_ts_string_t *string)
{
  free_type_spec((dds_ts_type_spec_t*)string);
}

static void free_fixed_pt(dds_ts_fixed_pt_t *fixed_pt)
{
  free_type_spec((dds_ts_type_spec_t*)fixed_pt);
}

static void free_map(dds_ts_map_t *map)
{
  free_type_spec(map->key_type);
  free_type_spec(map->value_type);
  free_type_spec((dds_ts_type_spec_t*)map);
}

static void free_definition(dds_ts_definition_t *definition)
{
  os_free((void*)definition->name);
  free_type_spec((dds_ts_type_spec_t*)definition);
}

static void free_module(dds_ts_module_t *module)
{
  free_type_spec((dds_ts_type_spec_t*)module);
}

static void free_forward_declaration(dds_ts_forward_declaration_t *forward_declaration)
{
  free_definition((dds_ts_definition_t*)forward_declaration);
}

static void free_struct(dds_ts_struct_t *struct_decl)
{
  free_definition((dds_ts_definition_t*)struct_decl);
}

static void free_struct_member(dds_ts_struct_member_t *struct_member)
{
  free_type_spec(struct_member->member_type);
  free_node((dds_ts_node_t*)struct_member);
}

static void free_array_size(dds_ts_array_size_t *array_size)
{
  free_node((dds_ts_node_t*)array_size);
}

extern void dds_ts_free_node(dds_ts_node_t *node)
{
  while (node != NULL) {
    if (node->children != NULL)
      dds_ts_free_node(node->children);
    
    dds_ts_node_t *next = node->next;
    switch (node->flags) {
      case DDS_TS_SHORT_TYPE:
      case DDS_TS_LONG_TYPE:
      case DDS_TS_LONG_LONG_TYPE:
      case DDS_TS_UNSIGNED_SHORT_TYPE:
      case DDS_TS_UNSIGNED_LONG_TYPE:
      case DDS_TS_UNSIGNED_LONG_LONG_TYPE:
      case DDS_TS_CHAR_TYPE:
      case DDS_TS_WIDE_CHAR_TYPE:
      case DDS_TS_BOOLEAN_TYPE:
      case DDS_TS_OCTET_TYPE:
      case DDS_TS_INT8_TYPE:
      case DDS_TS_UINT8_TYPE:
      case DDS_TS_FLOAT_TYPE:
      case DDS_TS_DOUBLE_TYPE:
      case DDS_TS_LONG_DOUBLE_TYPE:
      case DDS_TS_FIXED_PT_CONST_TYPE:
      case DDS_TS_ANY_TYPE:
        free_base_type((dds_ts_base_type_t*)node);
        break;
      case DDS_TS_SEQUENCE:
        free_sequence((dds_ts_sequence_t*)node);
        break;
      case DDS_TS_STRING:
      case DDS_TS_WIDE_STRING:
        free_string((dds_ts_string_t*)node);
        break;
      case DDS_TS_FIXED_PT:
        free_fixed_pt((dds_ts_fixed_pt_t*)node);
        break;
      case DDS_TS_MAP:
        free_map((dds_ts_map_t*)node);
        break;
      case DDS_TS_MODULE:
        free_module((dds_ts_module_t*)node);
        break;
      case DDS_TS_FORWARD_STRUCT:
        free_forward_declaration((dds_ts_forward_declaration_t*)node);
        break;
      case DDS_TS_STRUCT:
        free_struct((dds_ts_struct_t*)node);
        break;
      case DDS_TS_DECLARATOR:
        free_definition((dds_ts_definition_t*)node);
        break;
      case DDS_TS_STRUCT_MEMBER:
        free_struct_member((dds_ts_struct_member_t*)node);
        break;
      case DDS_TS_ARRAY_SIZE:
        free_array_size((dds_ts_array_size_t*)node);
        break;
      default:
        assert(0);
        free_node(node);
        break;
    }
    node = next;
  }
}

