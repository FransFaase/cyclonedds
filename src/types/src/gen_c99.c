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

#include "os/os.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "gen_c99.h"
#include "type_walker.h"

/* Traverse process */

/* The traverse process takes care of flattening the definitions into a
 * sequence of calls to functions for:
 * - structure definition
 * - sequence structure definition
 * - forward structure declaration
 */

/* Structure for the call-back functions */

typedef struct
{
  void (*open_module)(void *context, dds_ts_module_t *module);
  void (*close_module)(void *context);
  void (*forward_struct)(void *context, dds_ts_struct_t *struct_def);
  void (*unembedded_sequence)(void *context, dds_ts_struct_t *base_type, const char *decl_name, dds_ts_struct_t *element_type);
  void (*unembedded_sequence_with_name)(void *context, dds_ts_struct_t *base_type, const char *decl_name, const char *element_type_name);
  void (*open_struct)(void *context, dds_ts_struct_t *struct_def);
  void (*close_struct)(void *context, dds_ts_struct_t *struct_def);
  void (*open_struct_declarator)(void *context, dds_ts_definition_t *declarator);
  void (*open_struct_declarator_with_type)(void *context, dds_ts_definition_t *declarator, dds_ts_type_spec_t *type);
  void (*close_struct_declarator)(void *context);
  void (*open_array)(void *context, unsigned long long size);
  void (*close_array)(void *context, unsigned long long size);
  void (*basic_type)(void *context, dds_ts_type_spec_t *type);
  void (*type_reference)(void *context, const char *name);
  void (*open_sequence)(void *context);
  void (*close_sequence)(void *sequence);
  bool unembed_embedded_structs;
} traverse_funcs_t;


/* Some administration used during the traverse process */

/* This keeps a list of structures for which open_struct or forward_struct
 * has been called. It is also used to determine if forward_struct needs to
 * called before a call to unembedded_sequence.
 */

typedef struct struct_def_used struct_def_used_t;
struct struct_def_used {
  dds_ts_struct_t *struct_def;
  bool as_forward;
  bool as_definition;
  struct_def_used_t *next;
};

typedef struct {
  struct_def_used_t *struct_def_used;
} traverse_admin_t;

static void init_traverse_admin(traverse_admin_t *traverse_admin)
{
  traverse_admin->struct_def_used = NULL;
}

static void free_traverse_admin(traverse_admin_t *traverse_admin)
{
  struct_def_used_t *struct_def_used;
  for (struct_def_used = traverse_admin->struct_def_used; struct_def_used != NULL;) {
    struct_def_used_t *next = struct_def_used->next;
    os_free(struct_def_used);
    struct_def_used = next;
  }
}

static struct_def_used_t *find_struct_def_used(traverse_admin_t *traverse_admin, dds_ts_struct_t *struct_def)
{
  struct_def_used_t *struct_def_used;
  for (struct_def_used = traverse_admin->struct_def_used; struct_def_used != NULL; struct_def_used = struct_def_used->next)
    if (struct_def_used->struct_def == struct_def)
      return struct_def_used;
  struct_def_used = (struct_def_used_t*)os_malloc(sizeof(struct_def_used_t));
  if (struct_def_used == NULL) {
    fprintf(stderr, "Error: memory allocation failed\n");
    return NULL;
  }
  struct_def_used->struct_def = struct_def;
  struct_def_used->as_forward = false;
  struct_def_used->as_definition = false;
  struct_def_used->next = traverse_admin->struct_def_used;
  traverse_admin->struct_def_used = struct_def_used;
  return struct_def_used;
}

/* Included modules and structs */

/* Given a struct, find all modules and structs that need to be included because
   they are used (recursively) */

typedef struct included_node included_node_t;
struct included_node {
  dds_ts_node_t *node;
  included_node_t *next;
};

static void included_nodes_add(included_node_t **ref_included_nodes, dds_ts_node_t *node)
{
  included_node_t *new_included_node = (included_node_t*)malloc(sizeof(included_node_t));
  if (new_included_node == NULL) {
    return; /* do not report out-of-memory */
  }
  new_included_node->node = node;
  new_included_node->next = *ref_included_nodes;
  *ref_included_nodes = new_included_node;
}

static void included_nodes_free(included_node_t *included_nodes)
{
  while (included_nodes != NULL) {
    included_node_t *next = included_nodes->next;
    os_free((void*)included_nodes);
    included_nodes = next;
  }
}

static bool included_nodes_contains(included_node_t *included_nodes, dds_ts_node_t *node)
{
  for (; included_nodes != NULL; included_nodes = included_nodes->next)
    if (included_nodes->node == node)
      return true;
  return false;
}

static void find_used_structs(included_node_t **ref_included_nodes, dds_ts_struct_t *struct_def);

static void find_used_structs_in_members(included_node_t **ref_included_nodes, dds_ts_struct_t *struct_def)
{
  dds_ts_node_t *child;
  for (child = struct_def->def.type_spec.node.children; child != NULL; child = child->next) {
    if (child->flags == DDS_TS_STRUCT)
      find_used_structs_in_members(ref_included_nodes, (dds_ts_struct_t*)child);
    else if (child->flags == DDS_TS_STRUCT_MEMBER) {
      dds_ts_struct_member_t *member = (dds_ts_struct_member_t*)child;
      if (member->member_type.type_spec->node.flags == DDS_TS_SEQUENCE) {
        dds_ts_sequence_t *seq_member_type = (dds_ts_sequence_t*)member->member_type.type_spec;
        dds_ts_type_spec_t *element_type = seq_member_type->element_type.type_spec;
        if (element_type->node.flags == DDS_TS_STRUCT)
          find_used_structs(ref_included_nodes, (dds_ts_struct_t*)element_type);
        else if (element_type->node.flags == DDS_TS_FORWARD_STRUCT) {
          dds_ts_forward_declaration_t *forward_decl = (dds_ts_forward_declaration_t*)element_type;
          if (forward_decl->definition != NULL)
            find_used_structs(ref_included_nodes, (dds_ts_struct_t*)forward_decl->definition);
        }
      }
    }
  }
}

static void find_used_structs(included_node_t **ref_included_nodes, dds_ts_struct_t *struct_def)
{
  dds_ts_node_t *node = &struct_def->def.type_spec.node;
  if (included_nodes_contains(*ref_included_nodes, node))
    return;
  included_nodes_add(ref_included_nodes, node);

  /* include modules in which this struct occurs */
  for (node = node->parent; node != NULL && node->parent != NULL ; node = node->parent) {
    if (included_nodes_contains(*ref_included_nodes, node))
      break;
    included_nodes_add(ref_included_nodes, node);
  }

  find_used_structs_in_members(ref_included_nodes, struct_def);
}

/* Output function with named string arguments */

static void output(dds_ts_ostream_t *ostream, const char *fmt, ...)
{
  const char *s;
  for (s = fmt; *s != '\0'; s++)
    if (*s == '$') {
      s++;
      if (*s == '$')
        dds_ts_ostream_put(ostream, *s);
      else {
        va_list args;
        va_start(args, fmt);
        for (;;) {
          char letter = (char)va_arg(args, int);
          if (letter == '\0')
            break;
          const char *str = va_arg(args, const char*);
          if (letter == *s) {
            const char *t;
            for (t = str; *t != '\0'; t++)
              dds_ts_ostream_put(ostream, *t);
            break;
          }
        }
        va_end(args);
      }
    }
    else
      dds_ts_ostream_put(ostream, *s);
}


/* Finding specific parent of a node */

static dds_ts_node_t *dds_ts_node_get_parent(dds_ts_node_t *node, dds_ts_node_flags_t flags)
{
  while (node != NULL) {
    node = node->parent;
    if (node != NULL && node->flags == flags) {
      return node;
    }
  }
  return NULL;
}

/* Generating output */

static const char *output_file_name(const char* file_name, const char *ext)
{
  size_t file_name_len = strlen(file_name);
  if (file_name_len < 4 || strcmp(file_name + file_name_len - 4, ".idl") != 0) {
    fprintf(stderr, "Error: File name '%s' does not have '.idl' extension\n", file_name);
    return NULL;
  }
  size_t result_len = file_name_len - 2 + strlen(ext);
  char *result = (char*)os_malloc(sizeof(char)*(result_len));
  if (result == NULL) {
    fprintf(stderr, "Error: memory allocation failed\n");
    return NULL;
  }
  os_strlcpy(result, file_name, file_name_len - 2);
  os_strlcat(result, ext, result_len);
  return result;
}

static const char *uppercase_file_name(const char* file_name)
{
  size_t file_name_len = strlen(file_name);
  if (file_name_len < 4 || strcmp(file_name + file_name_len - 4, ".idl") != 0) {
    fprintf(stderr, "Error: File name '%s' does not have '.idl' extension\n", file_name);
    return NULL;
  }
  char *result = (char*)os_malloc(sizeof(char)*(file_name_len - 3));
  if (result == NULL) {
    fprintf(stderr, "Error: memory allocation failed\n");
    return NULL;
  }
  size_t i;
  for (i = 0; i < file_name_len - 4; i++)
    result[i] = (char)toupper(file_name[i]);
  result[i] = '\0';
  return result;
}

/* Generate header file */

static void write_copyright_header(dds_ts_ostream_t *ostream, const char *target_file_name, const char *source_file_name)
{
  os_time time_now = os_timeGet();
  char time_descr[OS_CTIME_R_BUFSIZE+1];
  os_ctime_r(&time_now, time_descr, OS_CTIME_R_BUFSIZE);

  output(ostream,
         "/*\n"
         " * Copyright(c) 2006 to 2018 ADLINK Technology Limited and others\n"
         " *\n"
         " * This program and the accompanying materials are made available under the\n"
         " * terms of the Eclipse Public License v. 2.0 which is available at\n"
         " * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License\n"
         " * v. 1.0 which is available at\n"
         " * http://www.eclipse.org/org/documents/edl-v10.php.\n"
         " *\n"
         " * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause\n"
         " */\n"
         "/****************************************************************\n"
         "\n"
         "  Generated by Cyclone DDS IDL to C Translator\n"
         "  File name: $T\n"
         "  Source: $S\n"
         "  Generated: $D\n"
         "  Cyclone DDS: V0.1.0\n"
         "\n"
         "*****************************************************************/\n"
         "\n",
         'T', target_file_name,
         'S', source_file_name,
         'D', time_descr,
         '\0');
}

static void write_header_intro(dds_ts_ostream_t *ostream, const char *file_name)
{
  const char *uc_file_name = uppercase_file_name(file_name);
  if (uc_file_name == NULL)
    return;

  output(ostream,
          "#include \"ddsc/dds_public_impl.h\"\n"
         "\n"
         "#ifndef _DDSL_$U_H_\n"
         "#define _DDSL_$U_H_\n"
         "\n"
         "\n"
         "#ifdef __cplusplus\n"
         "extern \"C\" {\n"
         "#endif\n"
         "\n",
         'U', uc_file_name,
         '\0');

  os_free((void*)uc_file_name);
}

static void write_header_close(dds_ts_ostream_t *ostream, const char *file_name)
{
  const char *uc_file_name = uppercase_file_name(file_name);
  if (uc_file_name == NULL)
    return;

  output(ostream,
         "#ifdef __cplusplus\n"
         "}\n"
         "#endif\n"
         "#endif /* _DDSL_$U_H_ */\n",
         'U', uc_file_name,
         '\0');

  os_free((void*)uc_file_name);
}

static const char *name_with_module_prefix(dds_ts_definition_t *def_node)
{
  size_t len = 0;
  dds_ts_definition_t *cur = def_node;
  for (cur = def_node; cur != NULL;) {
    len += strlen(cur->name);
    if (   cur->type_spec.node.parent != NULL
        && (   cur->type_spec.node.parent->flags == DDS_TS_MODULE
            || cur->type_spec.node.parent->flags == DDS_TS_STRUCT)
        && cur->type_spec.node.parent->parent != NULL) {
      cur = (dds_ts_definition_t*)(cur->type_spec.node.parent);
      len++;
    }
    else
      cur = NULL;
  }
  char *result = (char*)os_malloc(sizeof(char)*(len+1));
  if (result == NULL) {
    fprintf(stderr, "Error: memory allocation failed\n");
    return NULL;
  }
  result[len] = '\0';
  for (cur = def_node; cur != NULL;) {
    size_t cur_name_len = strlen(cur->name);
    len -= cur_name_len;
    size_t i;
    for (i = 0; i < cur_name_len; i++) {
       result[len + i] = cur->name[i];
    }
    if (   cur->type_spec.node.parent != NULL
        && (   cur->type_spec.node.parent->flags == DDS_TS_MODULE
            || cur->type_spec.node.parent->flags == DDS_TS_STRUCT)
        && cur->type_spec.node.parent->parent != NULL) {
      cur = (dds_ts_definition_t*)cur->type_spec.node.parent;
      result[--len] = '_';
    }
    else {
      cur = NULL;
    }
  }
  return result;
}

static void write_header_forward_struct(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  traverse_admin_t *admin = (traverse_admin_t*)context;

  dds_ts_struct_t *struct_def = NULL;
  if (exec_state->node->flags == DDS_TS_STRUCT) {
    struct_def = (dds_ts_struct_t*)exec_state->node;
  }
  else if (exec_state->node->flags == DDS_TS_FORWARD_STRUCT) {
    struct_def = (dds_ts_struct_t*)((dds_ts_forward_declaration_t*)exec_state->node)->definition;
  }
  if (struct_def == NULL) {
    return;
  }
  struct_def_used_t *struct_def_used = find_struct_def_used(admin, struct_def);
  if (struct_def_used == NULL || struct_def_used->as_forward || struct_def_used->as_definition) {
    return;
  }

  const char *full_name = name_with_module_prefix(&struct_def->def);
  if (full_name == NULL)
    return;

  output(ostream,
         "typedef struct $N $N;\n"
         "\n",
         'N', full_name,
         '\0');

  os_free((void*)full_name);
}

static bool member_type_is_sequence_of_struct(dds_ts_node_t *node)
{
  assert(node->flags == DDS_TS_STRUCT_MEMBER);

  dds_ts_struct_member_t *member = (dds_ts_struct_member_t*)node;
  if (member->member_type.type_spec->node.flags == DDS_TS_SEQUENCE) {
    dds_ts_sequence_t *seq_member_type = (dds_ts_sequence_t*)member->member_type.type_spec;
    dds_ts_type_spec_t *element_type = seq_member_type->element_type.type_spec;
    return element_type->node.flags == DDS_TS_STRUCT || element_type->node.flags == DDS_TS_FORWARD_STRUCT;
  }
  return false;
}

static bool member_type_is_sequence_of_sequence(dds_ts_node_t *node)
{
  assert(node->flags == DDS_TS_STRUCT_MEMBER);

  dds_ts_struct_member_t *member = (dds_ts_struct_member_t*)node;
  if (member->member_type.type_spec->node.flags == DDS_TS_SEQUENCE) {
    dds_ts_sequence_t *seq_member_type = (dds_ts_sequence_t*)member->member_type.type_spec;
    dds_ts_type_spec_t *element_type = seq_member_type->element_type.type_spec;
    return element_type->node.flags == DDS_TS_SEQUENCE;
  }
  return false;
}

static void write_header_seq_struct_with_name(dds_ts_struct_t *struct_def, const char *decl_name, const char *element_type_name, dds_ts_ostream_t *ostream)
{
  const char *base_name = name_with_module_prefix(&struct_def->def);
  if (base_name == NULL)
    return;

  output(ostream,
         "typedef struct $B_$D_seq\n"
         "{\n"
         "  uint32_t _maximum;\n"
         "  uint32_t _length;\n"
         "  $E *_buffer;\n"
         "  bool _release;\n"
         "} $B_$D_seq;\n"
         "\n"
         "#define $B_$D_seq__alloc() \\\n"
         "(($B_$D_seq*) dds_alloc (sizeof ($B_$D_seq)));\n"
         "\n"
         "#define $B_$D_seq_allocbuf(l) \\\n"
         "(($E *) dds_alloc ((l) * sizeof ($E)))\n"
         "\n",
         'B', base_name,
         'D', decl_name,
         'E', element_type_name,
         '\0');

  os_free((void*)base_name);
}

static void write_header_sequence_struct(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_DECLARATOR);
  assert(exec_state->call_parent != NULL && exec_state->call_parent->node->flags == DDS_TS_STRUCT_MEMBER);
  assert(exec_state->call_parent->call_parent != NULL && exec_state->call_parent->call_parent->node->flags == DDS_TS_STRUCT);

  const char *decl_name = ((dds_ts_definition_t*)exec_state->node)->name;

  dds_ts_struct_t *struct_def = (dds_ts_struct_t*)exec_state->call_parent->call_parent->node;

  dds_ts_type_spec_t *member_type = ((dds_ts_struct_member_t*)exec_state->call_parent->node)->member_type.type_spec;
  assert(member_type->node.flags == DDS_TS_SEQUENCE);
  
  dds_ts_type_spec_t *sequence_element_type = ((dds_ts_sequence_t*)member_type)->element_type.type_spec;
  assert(sequence_element_type->node.flags == DDS_TS_STRUCT || sequence_element_type->node.flags == DDS_TS_FORWARD_STRUCT);

  const char *element_type_name = name_with_module_prefix((dds_ts_definition_t*)sequence_element_type);
  if (element_type_name == NULL)
    return;

  write_header_seq_struct_with_name(struct_def, decl_name, element_type_name, ostream);

  os_free((void*)element_type_name);
}

static void write_header_sequence_sequence(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_DECLARATOR);
  assert(exec_state->call_parent != NULL && exec_state->call_parent->node->flags == DDS_TS_STRUCT_MEMBER);
  assert(exec_state->call_parent->call_parent != NULL && exec_state->call_parent->call_parent->node->flags == DDS_TS_STRUCT);

  const char *decl_name = ((dds_ts_definition_t*)exec_state->node)->name;

  dds_ts_struct_t *struct_def = (dds_ts_struct_t*)exec_state->call_parent->call_parent->node;

  dds_ts_type_spec_t *member_type = ((dds_ts_struct_member_t*)exec_state->call_parent->node)->member_type.type_spec;
  assert(member_type->node.flags == DDS_TS_SEQUENCE);
  
  dds_ts_type_spec_t *sequence_element_type = ((dds_ts_sequence_t*)member_type)->element_type.type_spec;
  assert(sequence_element_type->node.flags == DDS_TS_SEQUENCE);

  write_header_seq_struct_with_name(struct_def, decl_name, "dds_sequence_t", ostream);
}

static void write_header_open_struct(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_STRUCT);

  const char *full_name = name_with_module_prefix(&((dds_ts_struct_t*)exec_state->node)->def);
  if (full_name == NULL)
    return;

  output(ostream,
         "typedef struct $N\n"
         "{\n",
         'N', full_name,
         '\0');

  os_free((void*)full_name);
}

static void write_header_close_struct(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_STRUCT);
  dds_ts_struct_t *struct_def = (dds_ts_struct_t*)exec_state->node;

  const char *full_name = name_with_module_prefix(&struct_def->def);
  if (full_name == NULL)
    return;

  output(ostream,
         "} $N;\n"
         "\n",
         'N', full_name,
         '\0');
  if (!struct_def->part_of) {
    output(ostream,
           "extern const dds_topic_descriptor_t $N_desc;\n"
           "\n"
           "#define $N__alloc() \\\n"
           "(($N*) dds_alloc (sizeof ($N)));\n"
           "\n"
           "#define $N_free(d,o) \\\n"
           "dds_sample_free ((d), &$N_desc, (o))\n",
           'N', full_name,
           '\0');
  }
  output(ostream, "\n", '\0');

  os_free((void*)full_name);
}

static void write_header_struct_member(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_DECLARATOR);
  assert(exec_state->call_parent->node->flags == DDS_TS_STRUCT_MEMBER);

  dds_ts_definition_t* declarator = (dds_ts_definition_t*)exec_state->node;
  const char *decl_name = declarator->name;
  dds_ts_type_spec_t *type_spec = ((dds_ts_struct_member_t*)exec_state->call_parent->node)->member_type.type_spec;
  
  switch (type_spec->node.flags) {
    case DDS_TS_SHORT_TYPE:              output(ostream, "int16_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_LONG_TYPE:               output(ostream, "int32_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_LONG_LONG_TYPE:          output(ostream, "int64_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_UNSIGNED_SHORT_TYPE:     output(ostream, "uint16_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_UNSIGNED_LONG_TYPE:      output(ostream, "uint32_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_UNSIGNED_LONG_LONG_TYPE: output(ostream, "uint64_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_CHAR_TYPE:               output(ostream, "char $D", 'D', decl_name, '\0'); break;
    case DDS_TS_BOOLEAN_TYPE:            output(ostream, "bool $D", 'D', decl_name, '\0'); break;
    case DDS_TS_OCTET_TYPE:              output(ostream, "uint8_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_INT8_TYPE:               output(ostream, "int8_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_UINT8_TYPE:              output(ostream, "uint8_t $D", 'D', decl_name, '\0'); break;
    case DDS_TS_FLOAT_TYPE:              output(ostream, "float $D", 'D', decl_name, '\0'); break;
    case DDS_TS_DOUBLE_TYPE:             output(ostream, "double $D", 'D', decl_name, '\0'); break;
    case DDS_TS_STRING: {
      dds_ts_string_t *string_type = (dds_ts_string_t*)type_spec;
      if (string_type->bounded) {
        char size_string[30];
        os_ulltostr(string_type->max + 1, size_string, 30, NULL);
        output(ostream, "char $D[$S]", 'D', decl_name, 'S', size_string, '\0');
      }
      else
        output(ostream, "char * $D", 'D', decl_name, '\0');
      break;
    }
    case DDS_TS_STRUCT: {
      dds_ts_struct_t *struct_type = (dds_ts_struct_t*)type_spec;
      const char *struct_name = name_with_module_prefix(&struct_type->def);
      if (struct_name != NULL) {
        output(ostream, "$S $D", 'S', struct_name, 'D', decl_name, '\0');
        os_free((void*)struct_name);
      }
      break;
    }
    case DDS_TS_SEQUENCE: {
      dds_ts_sequence_t *sequence_type = (dds_ts_sequence_t*)type_spec;
      if (   sequence_type->element_type.type_spec->node.flags == DDS_TS_STRUCT
          || sequence_type->element_type.type_spec->node.flags == DDS_TS_FORWARD_STRUCT
          || sequence_type->element_type.type_spec->node.flags == DDS_TS_SEQUENCE) {
        dds_ts_node_t *struct_node = dds_ts_node_get_parent(&declarator->type_spec.node, DDS_TS_STRUCT);
        if (struct_node != NULL) {
          const char *full_name = name_with_module_prefix(&((dds_ts_struct_t*)struct_node)->def);
          if (full_name != NULL) {
            output(ostream, "$N_$D_seq $D", 'N', full_name, 'D', decl_name, '\0');
            os_free((void*)full_name);
          }
        }
      }
      else
        output(ostream, "dds_sequence_t $D", 'D', decl_name, '\0');
      break;
    }
    default:
      output(ostream, "// type not supported: $D", 'D', decl_name, '\0');
      break;
  }
}

static void write_header_array_size(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_ARRAY_SIZE);
  unsigned long long size = ((dds_ts_array_size_t*)exec_state->node)->size; 
  char size_string[30];
  os_ulltostr(size, size_string, 30, NULL);
  output(ostream, "[$S]", 'S', size_string, '\0');
}

static void generate_header_file(const char* file_name, dds_ts_node_t *root_node, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(root_node);
  const char *h_file_name = output_file_name(file_name, "h");
  if (h_file_name == NULL)
    return;
  if (!dds_ts_ostream_open(ostream, h_file_name)) {
    fprintf(stderr, "Could not open file '%s' for writing\n", h_file_name);
    return;
  }

  write_copyright_header(ostream, h_file_name, file_name);
  write_header_intro(ostream, file_name);

  dds_ts_walker_t *walker = dds_ts_create_walker(root_node);

  dds_ts_walker_def_proc(walker, "module");

    dds_ts_walker_for_all_children(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_MODULE);
        dds_ts_walker_call_proc(walker, "module");
      dds_ts_walker_end_if(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_STRUCT);
        dds_ts_walker_call_proc(walker, "struct");
      dds_ts_walker_end_if(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_FORWARD_STRUCT);
        dds_ts_walker_call_func(walker, write_header_forward_struct);
      dds_ts_walker_end_if(walker);
    dds_ts_walker_end_for(walker);
  
  dds_ts_walker_end_def(walker);

  dds_ts_walker_def_proc(walker, "struct");

    dds_ts_walker_for_all_children(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_STRUCT);
        dds_ts_walker_call_proc(walker, "struct");
      dds_ts_walker_end_if(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_STRUCT_MEMBER);
        dds_ts_walker_if_func(walker, member_type_is_sequence_of_struct);
          dds_ts_walker_for_struct_member_type(walker);
            dds_ts_walker_for_sequence_element_type(walker);
              dds_ts_walker_call_proc(walker, "forward_struct");
            dds_ts_walker_end_for(walker);
          dds_ts_walker_end_for(walker);
          dds_ts_walker_for_all_declarators(walker);
            dds_ts_walker_call_func(walker, write_header_sequence_struct);
          dds_ts_walker_end_for(walker);
        dds_ts_walker_end_if(walker);
        dds_ts_walker_if_func(walker, member_type_is_sequence_of_sequence);
          dds_ts_walker_for_all_declarators(walker);
            dds_ts_walker_call_func(walker, write_header_sequence_sequence);
          dds_ts_walker_end_for(walker);
        dds_ts_walker_end_if(walker);
      dds_ts_walker_end_if(walker);
    dds_ts_walker_end_for(walker);

    dds_ts_walker_call_func(walker, write_header_open_struct);
    dds_ts_walker_for_all_members(walker);
      dds_ts_walker_for_all_declarators(walker);
        dds_ts_walker_emit(walker, "  ");
        dds_ts_walker_call_func(walker, write_header_struct_member);
        dds_ts_walker_for_all_children(walker);
          dds_ts_walker_call_func(walker, write_header_array_size);
        dds_ts_walker_end_for(walker);
        dds_ts_walker_emit(walker, ";\n");
      dds_ts_walker_end_for(walker);
    dds_ts_walker_end_for(walker);
    dds_ts_walker_call_func(walker, write_header_close_struct);
              
  dds_ts_walker_end_def(walker);

  dds_ts_walker_main(walker);
    dds_ts_walker_call_proc(walker, "module");
  dds_ts_walker_end(walker);

  traverse_admin_t admin;
  init_traverse_admin(&admin);
  dds_ts_walker_execute(walker, &admin, ostream);
  free_traverse_admin(&admin);

  dds_ts_walker_free(walker);

  write_header_close(ostream, file_name);

  dds_ts_ostream_close(ostream);
  os_free((void*)h_file_name);
}

/* Generate source file */

static void write_meta_data_open_array(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_ARRAY_SIZE);
  unsigned long long size = ((dds_ts_array_size_t*)exec_state->node)->size; 
  char size_string[30];
  os_ulltostr(size, size_string, 30, NULL);
  output(ostream, "<Array size=\\\"$S\\\">", 'S', size_string, '\0');
}

static void write_meta_data_basic_type(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  dds_ts_type_spec_t *type_spec = (dds_ts_type_spec_t*)exec_state->node;
  switch (type_spec->node.flags) {
    case DDS_TS_SHORT_TYPE:              output(ostream, "<Short/>", '\0'); break;
    case DDS_TS_LONG_TYPE:               output(ostream, "<Long/>", '\0'); break;
    case DDS_TS_LONG_LONG_TYPE:          output(ostream, "<LongLong/>", '\0'); break;
    case DDS_TS_UNSIGNED_SHORT_TYPE:     output(ostream, "<UShort/>", '\0'); break;
    case DDS_TS_UNSIGNED_LONG_TYPE:      output(ostream, "<ULong/>", '\0'); break;
    case DDS_TS_UNSIGNED_LONG_LONG_TYPE: output(ostream, "<ULongLong/>", '\0'); break;
    case DDS_TS_CHAR_TYPE:               output(ostream, "<Char/>", '\0'); break;
    case DDS_TS_BOOLEAN_TYPE:            output(ostream, "<Boolean/>", '\0'); break;
    case DDS_TS_OCTET_TYPE:              output(ostream, "<Octet/>", '\0'); break;
    case DDS_TS_INT8_TYPE:               output(ostream, "<Int8/>", '\0'); break;
    case DDS_TS_UINT8_TYPE:              output(ostream, "<UInt8/>", '\0'); break;
    case DDS_TS_FLOAT_TYPE:              output(ostream, "<Float/>", '\0'); break;
    case DDS_TS_DOUBLE_TYPE:             output(ostream, "<Double/>", '\0'); break;
    case DDS_TS_STRING: {
      dds_ts_string_t *string_type = (dds_ts_string_t*)type_spec;
      if (string_type->bounded) {
        char size_string[30];
        os_ulltostr(string_type->max, size_string, 30, NULL);
        output(ostream, "<String length=\\\"$S\\\"/>", 'S', size_string, '\0');
      }
      else
        output(ostream, "<String/>", '\0');
      break;
    }
  }
}

/* Is embedded struct */

static bool is_embedded_struct(dds_ts_node_t *node)
{
  return node->flags == DDS_TS_STRUCT && node->parent != NULL && node->parent->flags == DDS_TS_STRUCT;
}

static void write_meta_data(dds_ts_ostream_t *ostream, dds_ts_struct_t *struct_def)
{
  /* Determine which structs should be include */
  included_node_t *included_nodes = NULL;
  find_used_structs(&included_nodes, struct_def);

  dds_ts_node_t *root_node = &struct_def->def.type_spec.node;
  while (root_node->parent != NULL)
    root_node = root_node->parent;

  output(ostream, "<MetaData version=\\\"1.0.0\\\">", '\0');

  dds_ts_walker_t *walker = dds_ts_create_walker(root_node);

  dds_ts_walker_def_proc(walker, "module");

    dds_ts_walker_for_all_children(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_MODULE);
        dds_ts_walker_emit(walker, "<Module name=\\\"");
        dds_ts_walker_emit_name(walker);
        dds_ts_walker_emit(walker, "\\\">");
        dds_ts_walker_call_proc(walker, "module");
        dds_ts_walker_emit(walker, "</Module>");
      dds_ts_walker_end_if(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_STRUCT);
        dds_ts_walker_call_proc(walker, "struct");
      dds_ts_walker_end_if(walker);
    dds_ts_walker_end_for(walker);
  
  dds_ts_walker_end_def(walker);

  dds_ts_walker_def_proc(walker, "struct");

    dds_ts_walker_emit(walker, "<Struct name=\\\"");
    dds_ts_walker_emit_name(walker);
    dds_ts_walker_emit(walker, "\\\">");
    dds_ts_walker_for_all_members(walker);
      dds_ts_walker_for_all_declarators(walker);
        dds_ts_walker_emit(walker, "<Member name=\\\"");
        dds_ts_walker_emit_name(walker);
        dds_ts_walker_emit(walker, "\\\">");
        dds_ts_walker_for_all_children(walker);
          dds_ts_walker_call_func(walker, write_meta_data_open_array);
        dds_ts_walker_end_for(walker);
        dds_ts_walker_for_call_parent(walker);
          dds_ts_walker_for_struct_member_type(walker);
            dds_ts_walker_call_proc(walker, "type_spec");
          dds_ts_walker_end_for(walker);
        dds_ts_walker_end_for(walker);
        dds_ts_walker_for_all_children(walker);
          dds_ts_walker_emit(walker, "</Array>");
        dds_ts_walker_end_for(walker);
        dds_ts_walker_emit(walker, "</Member>");
      dds_ts_walker_end_for(walker);
    dds_ts_walker_end_for(walker);
    dds_ts_walker_emit(walker, "</Struct>");
              
  dds_ts_walker_end_def(walker);

  dds_ts_walker_def_proc(walker, "type_spec");
    dds_ts_walker_call_func(walker, write_meta_data_basic_type);
    dds_ts_walker_if_is_type(walker, DDS_TS_SEQUENCE);
      dds_ts_walker_emit(walker, "<Sequence>");
        dds_ts_walker_for_sequence_element_type(walker);
          dds_ts_walker_call_proc(walker, "type_spec");
        dds_ts_walker_end_for(walker);
      dds_ts_walker_emit(walker, "</Sequence>");
    dds_ts_walker_end_if(walker);
    dds_ts_walker_if_is_type(walker, DDS_TS_STRUCT);
      dds_ts_walker_if_func(walker, is_embedded_struct);
        dds_ts_walker_call_proc(walker, "struct");
      dds_ts_walker_else(walker);
        dds_ts_walker_emit(walker, "<Type name=\\\"");
        dds_ts_walker_emit_name(walker);
        dds_ts_walker_emit(walker, "/>");
      dds_ts_walker_end_if(walker);
    dds_ts_walker_end_if(walker);
  dds_ts_walker_end_def(walker);

  dds_ts_walker_main(walker);
    dds_ts_walker_call_proc(walker, "module");
  dds_ts_walker_end(walker);

  traverse_admin_t admin;
  init_traverse_admin(&admin);
  dds_ts_walker_execute(walker, &admin, ostream);
  free_traverse_admin(&admin);

  dds_ts_walker_free(walker);
  output(ostream, "</MetaData>", '\0');

  included_nodes_free(included_nodes);
}


static void write_source_struct(dds_ts_walker_exec_state_t *exec_state, void *context, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDS_TS_STRUCT);
  dds_ts_struct_t *struct_def = (dds_ts_struct_t*)exec_state->node;
  if (struct_def->part_of)
    return;

  const char *full_name = name_with_module_prefix(&struct_def->def);
  if (full_name == NULL)
    return;

  output(ostream,
         "\n\n"
         "static const dds_key_descriptor_t $N_keys[0] =\n"
         "{\n"
         "};\n"
         "\n"
         "static const uint32_t $N_ops [] =\n"
         "{\n"
         "};\n"
         "\n"
         "const dds_topic_descriptor_t $N_desc =\n"
         "{\n"
         "  sizeof ($N),\n"
         "  8u,\n"
         "  DDS_TOPIC_FIXED_KEY | DDS_TOPIC_NO_OPTIMIZE,\n"
         "  3u,\n"
         "  \"\",\n"
         "  $N_keys,\n"
         "  45,\n"
         "  $N_ops,\n"
         "  \"",
         'N', full_name,
         '\0');
  write_meta_data(ostream, struct_def);
  output(ostream,
         "\"\n"
         "};\n",
         '\0');

  os_free((void*)full_name);
}


static void generate_source_file(const char* file_name, dds_ts_node_t *root_node, dds_ts_ostream_t *ostream)
{
  OS_UNUSED_ARG(root_node);
  const char *c_file_name = output_file_name(file_name, "c");
  if (c_file_name == NULL)
    return;
  if (!dds_ts_ostream_open(ostream, c_file_name)) {
    fprintf(stderr, "Could not open file '%s' for writing\n", c_file_name);
    return;
  }
  const char *h_file_name = output_file_name(file_name, "h");
  if (h_file_name == NULL)
    return;

  write_copyright_header(ostream, c_file_name, file_name);
  output(ostream, "#include \"$F\"\n\n\n\n", 'F', h_file_name, '\0');

  dds_ts_walker_t *walker = dds_ts_create_walker(root_node);

  dds_ts_walker_def_proc(walker, "module");

    dds_ts_walker_for_all_children(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_MODULE);
        dds_ts_walker_call_proc(walker, "module");
      dds_ts_walker_end_if(walker);
      dds_ts_walker_if_is_type(walker, DDS_TS_STRUCT);
        dds_ts_walker_call_func(walker, write_source_struct);
      dds_ts_walker_end_if(walker);
    dds_ts_walker_end_for(walker);
  
  dds_ts_walker_end_def(walker);

  dds_ts_walker_main(walker);
    dds_ts_walker_call_proc(walker, "module");
  dds_ts_walker_end(walker);

  dds_ts_walker_execute(walker, NULL, ostream);

  dds_ts_walker_free(walker);

  dds_ts_ostream_close(ostream);
  os_free((void*)h_file_name);
  os_free((void*)c_file_name);
}

void dds_ts_generate_C99(const char* file_name, dds_ts_node_t *root_node, dds_ts_ostream_t *ostream)
{
  generate_header_file(file_name, root_node, ostream);
  generate_source_file(file_name, root_node, ostream);
}

