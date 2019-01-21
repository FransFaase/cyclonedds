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
#include "dds/ddsts/typetree.h"
#include "dds/ddsts/type_walk.h"
#include "gen_ostream.h"
#include "gen_c99.h"

/* Some administration used during the traverse process */

/* For each sequence of struct in IDL, a separate struct is defined in the C header,
 * which uses the struct definition generated for the struct (from IDL). In case
 * for this IDL struct no struct has been generated in the C header, a forward
 * declaration (with a typedef) needs to be generated. For this reason we need
 * to track for which IDL structs a forward definition and/or a struct declaration
 * have occured.
 */

typedef struct struct_def_used struct_def_used_t;
struct struct_def_used {
  ddsts_struct_t *struct_def;
  bool as_forward;
  bool as_definition;
  struct_def_used_t *next;
};

typedef struct {
  ddsts_ostream_t *ostream;
} output_context_t;

typedef struct {
  output_context_t output_context;
  struct_def_used_t *struct_def_used;
} gen_header_context_t;

static void gen_header_context_init(gen_header_context_t *gen_header_context, ddsts_ostream_t *ostream)
{
  gen_header_context->output_context.ostream = ostream;
  gen_header_context->struct_def_used = NULL;
}

static void free_gen_header_context(gen_header_context_t *gen_header_context)
{
  struct_def_used_t *struct_def_used;
  for (struct_def_used = gen_header_context->struct_def_used; struct_def_used != NULL;) {
    struct_def_used_t *next = struct_def_used->next;
    os_free(struct_def_used);
    struct_def_used = next;
  }
}

static struct_def_used_t *find_struct_def_used(gen_header_context_t *gen_header_context, ddsts_struct_t *struct_def)
{
  struct_def_used_t *struct_def_used;
  for (struct_def_used = gen_header_context->struct_def_used; struct_def_used != NULL; struct_def_used = struct_def_used->next)
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
  struct_def_used->next = gen_header_context->struct_def_used;
  gen_header_context->struct_def_used = struct_def_used;
  return struct_def_used;
}

/* Included modules and structs */

/* Given a struct, find all modules and structs that need to be included because
 * they are used (recursively)
 */

typedef struct included_node included_node_t;
struct included_node {
  ddsts_node_t *node;
  included_node_t *next;
};

static void included_nodes_add(included_node_t **ref_included_nodes, ddsts_node_t *node)
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

static bool included_nodes_contains(included_node_t *included_nodes, ddsts_node_t *node)
{
  for (; included_nodes != NULL; included_nodes = included_nodes->next)
    if (included_nodes->node == node)
      return true;
  return false;
}

static void find_used_structs(included_node_t **ref_included_nodes, ddsts_struct_t *struct_def);

static void find_used_structs_in_type_def(included_node_t **ref_included_nodes, ddsts_type_spec_t *type_spec)
{
  switch (type_spec->node.flags) {
    case DDSTS_SEQUENCE:
      find_used_structs_in_type_def(ref_included_nodes, ((ddsts_sequence_t*)type_spec)->element_type.type_spec);
      break;
    case DDSTS_STRUCT:
      find_used_structs(ref_included_nodes, (ddsts_struct_t*)type_spec);
      break;
    case DDSTS_FORWARD_STRUCT: {
      ddsts_forward_declaration_t *forward_decl = (ddsts_forward_declaration_t*)type_spec;
      if (forward_decl->definition != NULL)
        find_used_structs(ref_included_nodes, (ddsts_struct_t*)forward_decl->definition);
      break;
    }
    default:
      break;
  }
}

static void find_used_structs(included_node_t **ref_included_nodes, ddsts_struct_t *struct_def)
{
  ddsts_node_t *node = &struct_def->def.type_spec.node;
  if (included_nodes_contains(*ref_included_nodes, node))
    return;
  included_nodes_add(ref_included_nodes, node);

  /* include modules in which this struct occurs */
  for (node = node->parent; node != NULL && node->parent != NULL ; node = node->parent) {
    if (included_nodes_contains(*ref_included_nodes, node))
      break;
    included_nodes_add(ref_included_nodes, node);
  }

  ddsts_node_t *child;
  for (child = struct_def->def.type_spec.node.children; child != NULL; child = child->next) {
    if (child->flags == DDSTS_STRUCT_MEMBER) {
      ddsts_struct_member_t *member = (ddsts_struct_member_t*)child;
      find_used_structs_in_type_def(ref_included_nodes, member->member_type.type_spec);
    }
  }
}

/* Finding specific parent of a node */

static ddsts_node_t *ddsts_node_get_parent(ddsts_node_t *node, ddsts_node_flags_t flags)
{
  while (node != NULL) {
    node = node->parent;
    if (node != NULL && node->flags == flags) {
      return node;
    }
  }
  return NULL;
}


/* Output function with named string arguments */

static void output(ddsts_ostream_t *ostream, const char *fmt, ...)
{
  const char *values[26];
  for (int i = 0; i < 26; i++) {
    values[i] = "";
  }

  /* Parse variable arguments: pairs of a capital letter and a string */
  va_list args;
  va_start(args, fmt);
  for (;;) {
    char letter = (char)va_arg(args, int);
    if (letter < 'A' || letter > 'Z') {
      break;
    }
    values[letter - 'A'] = va_arg(args, const char*);
  }
  va_end(args);

  const char *s;
  for (s = fmt; *s != '\0'; s++) {
    if (*s == '$') {
      s++;
      if (*s == '$')
        ddsts_ostream_put(ostream, *s);
      else if ('A' <= *s && *s <= 'Z') {
        const char *t;
        for (t = values[*s - 'A']; *t != '\0'; t++) {
          ddsts_ostream_put(ostream, *t);
        }
      }
    }
    else {
      ddsts_ostream_put(ostream, *s);
    }
  }
}


/* File name functions */

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

/* Generate copyright headere */

static void write_copyright_header(ddsts_ostream_t *ostream, const char *target_file_name, const char *source_file_name)
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

/* Generate C99 header file */

static void write_header_intro(ddsts_ostream_t *ostream, const char *file_name)
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

static void write_header_close(ddsts_ostream_t *ostream, const char *file_name)
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

static const char *name_with_module_prefix(ddsts_definition_t *def_node)
{
  size_t len = 0;
  ddsts_definition_t *cur = def_node;
  for (cur = def_node; cur != NULL;) {
    len += strlen(cur->name);
    if (   cur->type_spec.node.parent != NULL
        && (   cur->type_spec.node.parent->flags == DDSTS_MODULE
            || cur->type_spec.node.parent->flags == DDSTS_STRUCT)
        && cur->type_spec.node.parent->parent != NULL) {
      cur = (ddsts_definition_t*)(cur->type_spec.node.parent);
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
        && (   cur->type_spec.node.parent->flags == DDSTS_MODULE
            || cur->type_spec.node.parent->flags == DDSTS_STRUCT)
        && cur->type_spec.node.parent->parent != NULL) {
      cur = (ddsts_definition_t*)cur->type_spec.node.parent;
      result[--len] = '_';
    }
    else {
      cur = NULL;
    }
  }
  return result;
}

/* Functions called from walker for the include file */

static void write_header_forward_struct(ddsts_walk_exec_state_t *exec_state, void *context)
{
  ddsts_struct_t *struct_def = NULL;
  if (exec_state->node->flags == DDSTS_STRUCT) {
    struct_def = (ddsts_struct_t*)exec_state->node;
  }
  else if (exec_state->node->flags == DDSTS_FORWARD_STRUCT) {
    struct_def = (ddsts_struct_t*)((ddsts_forward_declaration_t*)exec_state->node)->definition;
  }
  if (struct_def == NULL) {
    return;
  }
  struct_def_used_t *struct_def_used = find_struct_def_used((gen_header_context_t*)context, struct_def);
  if (struct_def_used == NULL || struct_def_used->as_forward || struct_def_used->as_definition) {
    return;
  }
  struct_def_used->as_forward = true;

  const char *full_name = name_with_module_prefix(&struct_def->def);
  if (full_name == NULL)
    return;

  output(((output_context_t*)context)->ostream,
         "typedef struct $N $N;\n"
         "\n",
         'N', full_name,
         '\0');

  os_free((void*)full_name);
}

static void write_header_seq_struct_with_name(ddsts_struct_t *struct_def, const char *decl_name, const char *element_type_name, ddsts_ostream_t *ostream)
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

static void write_header_sequence_struct(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDSTS_DECLARATOR);
  assert(exec_state->call_parent != NULL && exec_state->call_parent->node->flags == DDSTS_STRUCT_MEMBER);
  assert(exec_state->call_parent->call_parent != NULL && exec_state->call_parent->call_parent->node->flags == DDSTS_STRUCT);

  const char *decl_name = ((ddsts_definition_t*)exec_state->node)->name;

  ddsts_struct_t *struct_def = (ddsts_struct_t*)exec_state->call_parent->call_parent->node;

  ddsts_type_spec_t *member_type = ((ddsts_struct_member_t*)exec_state->call_parent->node)->member_type.type_spec;
  assert(member_type->node.flags == DDSTS_SEQUENCE);
  
  ddsts_type_spec_t *sequence_element_type = ((ddsts_sequence_t*)member_type)->element_type.type_spec;
  assert(sequence_element_type->node.flags == DDSTS_STRUCT || sequence_element_type->node.flags == DDSTS_FORWARD_STRUCT);

  const char *element_type_name = name_with_module_prefix((ddsts_definition_t*)sequence_element_type);
  if (element_type_name == NULL)
    return;

  write_header_seq_struct_with_name(struct_def, decl_name, element_type_name, ((output_context_t*)context)->ostream);

  os_free((void*)element_type_name);
}

static void write_header_sequence_sequence(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDSTS_DECLARATOR);
  assert(exec_state->call_parent != NULL && exec_state->call_parent->node->flags == DDSTS_STRUCT_MEMBER);
  assert(exec_state->call_parent->call_parent != NULL && exec_state->call_parent->call_parent->node->flags == DDSTS_STRUCT);

  const char *decl_name = ((ddsts_definition_t*)exec_state->node)->name;

  ddsts_struct_t *struct_def = (ddsts_struct_t*)exec_state->call_parent->call_parent->node;

#if (!defined(NDEBUG)) 
  ddsts_type_spec_t *member_type = ((ddsts_struct_member_t*)exec_state->call_parent->node)->member_type.type_spec;
  assert(member_type->node.flags == DDSTS_SEQUENCE);

  ddsts_type_spec_t *sequence_element_type = ((ddsts_sequence_t*)member_type)->element_type.type_spec;
  assert(sequence_element_type->node.flags == DDSTS_SEQUENCE);
#endif

  write_header_seq_struct_with_name(struct_def, decl_name, "dds_sequence_t", ((output_context_t*)context)->ostream);
}

static void write_header_open_struct(ddsts_walk_exec_state_t *exec_state, void *context)
{
  assert(exec_state->node->flags == DDSTS_STRUCT);

  struct_def_used_t *struct_def_used = find_struct_def_used((gen_header_context_t*)context, (ddsts_struct_t*)exec_state->node);
  if (struct_def_used == NULL) {
    return;
  }
  struct_def_used->as_definition = true;

  const char *full_name = name_with_module_prefix(&((ddsts_struct_t*)exec_state->node)->def);
  if (full_name == NULL)
    return;

  output(((output_context_t*)context)->ostream,
         "typedef struct $N\n"
         "{\n",
         'N', full_name,
         '\0');

  os_free((void*)full_name);
}

static void write_header_close_struct(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDSTS_STRUCT);
  ddsts_struct_t *struct_def = (ddsts_struct_t*)exec_state->node;

  const char *full_name = name_with_module_prefix(&struct_def->def);
  if (full_name == NULL)
    return;

  output(((output_context_t*)context)->ostream,
         "} $N;\n"
         "\n",
         'N', full_name,
         '\0');
  if (!struct_def->part_of) {
    output(((output_context_t*)context)->ostream,
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
  output(((output_context_t*)context)->ostream, "\n", '\0');

  os_free((void*)full_name);
}

static void write_header_array_size(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDSTS_ARRAY_SIZE);
  unsigned long long size = ((ddsts_array_size_t*)exec_state->node)->size; 
  char size_string[30];
  os_ulltostr(size, size_string, 30, NULL);
  output(((output_context_t*)context)->ostream, "[$S]", 'S', size_string, '\0');
}

static void write_header_struct_member(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(context);

  ddsts_ostream_puts(((output_context_t*)context)->ostream, "  ");

  assert(exec_state->node->flags == DDSTS_DECLARATOR);
  assert(exec_state->call_parent->node->flags == DDSTS_STRUCT_MEMBER);

  ddsts_definition_t* declarator = (ddsts_definition_t*)exec_state->node;
  const char *decl_name = declarator->name;
  ddsts_type_spec_t *type_spec = ((ddsts_struct_member_t*)exec_state->call_parent->node)->member_type.type_spec;
  
  switch (type_spec->node.flags) {
    case DDSTS_SHORT_TYPE:              output(((output_context_t*)context)->ostream, "int16_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_LONG_TYPE:               output(((output_context_t*)context)->ostream, "int32_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_LONG_LONG_TYPE:          output(((output_context_t*)context)->ostream, "int64_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_UNSIGNED_SHORT_TYPE:     output(((output_context_t*)context)->ostream, "uint16_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_UNSIGNED_LONG_TYPE:      output(((output_context_t*)context)->ostream, "uint32_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_UNSIGNED_LONG_LONG_TYPE: output(((output_context_t*)context)->ostream, "uint64_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_CHAR_TYPE:               output(((output_context_t*)context)->ostream, "char $D", 'D', decl_name, '\0'); break;
    case DDSTS_BOOLEAN_TYPE:            output(((output_context_t*)context)->ostream, "bool $D", 'D', decl_name, '\0'); break;
    case DDSTS_OCTET_TYPE:              output(((output_context_t*)context)->ostream, "uint8_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_INT8_TYPE:               output(((output_context_t*)context)->ostream, "int8_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_UINT8_TYPE:              output(((output_context_t*)context)->ostream, "uint8_t $D", 'D', decl_name, '\0'); break;
    case DDSTS_FLOAT_TYPE:              output(((output_context_t*)context)->ostream, "float $D", 'D', decl_name, '\0'); break;
    case DDSTS_DOUBLE_TYPE:             output(((output_context_t*)context)->ostream, "double $D", 'D', decl_name, '\0'); break;
    case DDSTS_STRING: {
      ddsts_string_t *string_type = (ddsts_string_t*)type_spec;
      if (string_type->bounded) {
        char size_string[30];
        os_ulltostr(string_type->max + 1, size_string, 30, NULL);
        output(((output_context_t*)context)->ostream, "char $D[$S]", 'D', decl_name, 'S', size_string, '\0');
      }
      else {
        output(((output_context_t*)context)->ostream, "char * $D", 'D', decl_name, '\0');
      }
      break;
    }
    case DDSTS_SEQUENCE: {
      ddsts_sequence_t *sequence_type = (ddsts_sequence_t*)type_spec;
      if (   sequence_type->element_type.type_spec->node.flags == DDSTS_STRUCT
          || sequence_type->element_type.type_spec->node.flags == DDSTS_FORWARD_STRUCT
          || sequence_type->element_type.type_spec->node.flags == DDSTS_SEQUENCE) {
        ddsts_node_t *struct_node = ddsts_node_get_parent(&declarator->type_spec.node, DDSTS_STRUCT);
        if (struct_node != NULL) {
          const char *full_name = name_with_module_prefix(&((ddsts_struct_t*)struct_node)->def);
          if (full_name != NULL) {
            output(((output_context_t*)context)->ostream, "$N_$D_seq $D", 'N', full_name, 'D', decl_name, '\0');
            os_free((void*)full_name);
          }
        }
      }
      else
        output(((output_context_t*)context)->ostream, "dds_sequence_t $D", 'D', decl_name, '\0');
      break;
    }
    case DDSTS_STRUCT: {
      ddsts_struct_t *struct_type = (ddsts_struct_t*)type_spec;
      const char *struct_name = name_with_module_prefix(&struct_type->def);
      if (struct_name != NULL) {
        output(((output_context_t*)context)->ostream, "$S $D", 'S', struct_name, 'D', decl_name, '\0');
        os_free((void*)struct_name);
      }
      break;
    }
    default:
      output(((output_context_t*)context)->ostream, "// type not supported: $D", 'D', decl_name, '\0');
      break;
  }

  ddsts_walk(exec_state, 0, DDSTS_ARRAY_SIZE, write_header_array_size, context);

  ddsts_ostream_puts(((output_context_t*)context)->ostream, ";\n");
}

static void write_header_struct(ddsts_walk_exec_state_t *exec_state, void *context);

static void write_header_struct_pre(ddsts_walk_exec_state_t *exec_state, void *context)
{
  switch (exec_state->node->flags) {
    case DDSTS_STRUCT:
      write_header_struct(exec_state, context);
      break;
    case DDSTS_STRUCT_MEMBER: {
      ddsts_struct_member_t *member = (ddsts_struct_member_t*)exec_state->node;
      if (member->member_type.type_spec->node.flags == DDSTS_SEQUENCE) {
        ddsts_sequence_t *seq_member_type = (ddsts_sequence_t*)member->member_type.type_spec;
        ddsts_type_spec_t *element_type = seq_member_type->element_type.type_spec;
        if (element_type->node.flags == DDSTS_STRUCT || element_type->node.flags == DDSTS_FORWARD_STRUCT) {
          ddsts_walk_exec_state_t elem_exec_state;
          elem_exec_state.call_parent = exec_state;
          elem_exec_state.node = &element_type->node;
          write_header_forward_struct(&elem_exec_state, context);
          ddsts_walk(exec_state, 0, DDSTS_DECLARATOR, write_header_sequence_struct, context);
        }
        else if (element_type->node.flags == DDSTS_SEQUENCE) {
          ddsts_walk(exec_state, 0, DDSTS_DECLARATOR, write_header_sequence_sequence, context);
        }
      }
      break;
    }
    default:
      assert(false);
      break;
   }
}

static void write_header_struct(ddsts_walk_exec_state_t *exec_state, void *context)
{
  ddsts_walk(exec_state, 0, DDSTS_STRUCT|DDSTS_STRUCT_MEMBER, write_header_struct_pre, context);
 
  write_header_open_struct(exec_state, context); 
  ddsts_walk(exec_state, DDSTS_STRUCT_MEMBER, DDSTS_DECLARATOR, write_header_struct_member, context);
  write_header_close_struct(exec_state, context);
}

static void write_header_modules(ddsts_walk_exec_state_t *exec_state, void *context)
{
  switch (exec_state->node->flags) {
    case DDSTS_STRUCT:
      write_header_struct(exec_state, context);
      break;
    case DDSTS_FORWARD_STRUCT:
      write_header_forward_struct(exec_state, context);
      break;
    default:
      assert(false);
      break;
  }
}

static void generate_header_file(const char* file_name, ddsts_node_t *root_node, ddsts_ostream_t *ostream)
{
  OS_UNUSED_ARG(root_node);
  const char *h_file_name = output_file_name(file_name, "h");
  if (h_file_name == NULL)
    return;
  if (!ddsts_ostream_open(ostream, h_file_name)) {
    fprintf(stderr, "Could not open file '%s' for writing\n", h_file_name);
    return;
  }

  write_copyright_header(ostream, h_file_name, file_name);
  write_header_intro(ostream, file_name);

  gen_header_context_t context;
  gen_header_context_init(&context, ostream);

  ddsts_walk_exec_state_t exec_state;
  exec_state.call_parent = NULL;
  exec_state.node = root_node;

  ddsts_walk(&exec_state, DDSTS_MODULE, DDSTS_STRUCT|DDSTS_FORWARD_STRUCT, write_header_modules, &context.output_context);

  write_header_close(ostream, file_name);

  free_gen_header_context(&context);

  ddsts_ostream_close(ostream);
  os_free((void*)h_file_name);
}

/* Generate source file */

/*   Functions for generating meta data */

typedef struct {
  output_context_t output_context;
  included_node_t *included_nodes;
} meta_data_context_t;

static void meta_data_context_init(meta_data_context_t *context, ddsts_ostream_t *ostream, included_node_t *included_nodes)
{
  context->output_context.ostream = ostream;
  context->included_nodes = included_nodes;
}

static void write_meta_data_open_array(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDSTS_ARRAY_SIZE);
  unsigned long long size = ((ddsts_array_size_t*)exec_state->node)->size; 
  char size_string[30];
  os_ulltostr(size, size_string, 30, NULL);
  output(((output_context_t*)context)->ostream, "<Array size=\\\"$S\\\">", 'S', size_string, '\0');
}

static void write_meta_data_close_array(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(exec_state);
  OS_UNUSED_ARG(context);

  ddsts_ostream_puts(((output_context_t*)context)->ostream, "</Array>");
}

static void write_meta_data_elem(ddsts_walk_exec_state_t *exec_state, void *context);

static void write_meta_data_type_spec(ddsts_walk_exec_state_t *exec_state, void *context)
{
  ddsts_type_spec_t *type_spec = (ddsts_type_spec_t*)exec_state->node;
  switch (type_spec->node.flags) {
    case DDSTS_SHORT_TYPE:              output(((output_context_t*)context)->ostream, "<Short/>", '\0'); break;
    case DDSTS_LONG_TYPE:               output(((output_context_t*)context)->ostream, "<Long/>", '\0'); break;
    case DDSTS_LONG_LONG_TYPE:          output(((output_context_t*)context)->ostream, "<LongLong/>", '\0'); break;
    case DDSTS_UNSIGNED_SHORT_TYPE:     output(((output_context_t*)context)->ostream, "<UShort/>", '\0'); break;
    case DDSTS_UNSIGNED_LONG_TYPE:      output(((output_context_t*)context)->ostream, "<ULong/>", '\0'); break;
    case DDSTS_UNSIGNED_LONG_LONG_TYPE: output(((output_context_t*)context)->ostream, "<ULongLong/>", '\0'); break;
    case DDSTS_CHAR_TYPE:               output(((output_context_t*)context)->ostream, "<Char/>", '\0'); break;
    case DDSTS_BOOLEAN_TYPE:            output(((output_context_t*)context)->ostream, "<Boolean/>", '\0'); break;
    case DDSTS_OCTET_TYPE:              output(((output_context_t*)context)->ostream, "<Octet/>", '\0'); break;
    case DDSTS_INT8_TYPE:               output(((output_context_t*)context)->ostream, "<Int8/>", '\0'); break;
    case DDSTS_UINT8_TYPE:              output(((output_context_t*)context)->ostream, "<UInt8/>", '\0'); break;
    case DDSTS_FLOAT_TYPE:              output(((output_context_t*)context)->ostream, "<Float/>", '\0'); break;
    case DDSTS_DOUBLE_TYPE:             output(((output_context_t*)context)->ostream, "<Double/>", '\0'); break;
    case DDSTS_LONG_DOUBLE_TYPE:        output(((output_context_t*)context)->ostream, "<LongDouble/>", '\0'); break;
    case DDSTS_STRING: {
      ddsts_string_t *string_type = (ddsts_string_t*)type_spec;
      if (string_type->bounded) {
        char size_string[30];
        os_ulltostr(string_type->max, size_string, 30, NULL);
        output(((output_context_t*)context)->ostream, "<String length=\\\"$S\\\"/>", 'S', size_string, '\0');
      }
      else {
        output(((output_context_t*)context)->ostream, "<String/>", '\0');
      }
      break;
    }
    case DDSTS_SEQUENCE: {
      ddsts_ostream_puts(((output_context_t*)context)->ostream, "<Sequence>");
      ddsts_walk_exec_state_t element_exec_state;
      element_exec_state.node = &((ddsts_sequence_t*)exec_state->node)->element_type.type_spec->node;
      element_exec_state.call_parent = exec_state;
      write_meta_data_type_spec(&element_exec_state, context);
      ddsts_ostream_puts(((output_context_t*)context)->ostream, "</Sequence>");
      break;
    }
    case DDSTS_STRUCT:
      if (type_spec->node.parent != NULL && type_spec->node.parent->flags == DDSTS_STRUCT) {
        write_meta_data_elem(exec_state, context);
      }
      else {
        output(((output_context_t*)context)->ostream, "<Type name=\\\"$N\\\"/>", 'N', ((ddsts_struct_t*)exec_state->node)->def.name, '\0');
      }
      break;
    case DDSTS_FORWARD_STRUCT:
      if (((ddsts_forward_declaration_t*)exec_state->node)->definition != NULL) {
        output(((output_context_t*)context)->ostream, "<Type name=\\\"$N\\\"/>", 'N', ((ddsts_forward_declaration_t*)exec_state->node)->definition->name, '\0');
      }
      break; 
    default:
      //assert(false);
      break;
  }
}

static void write_meta_data_elem(ddsts_walk_exec_state_t *exec_state, void *context)
{
  switch (exec_state->node->flags) {
    case DDSTS_MODULE:
      if (included_nodes_contains(((meta_data_context_t*)context)->included_nodes, exec_state->node)) {
        output(((output_context_t*)context)->ostream, "<Module name=\\\"$N\\\">", 'N', ((ddsts_definition_t*)exec_state->node)->name, '\0');
        ddsts_walk(exec_state, 0, DDSTS_MODULE|DDSTS_STRUCT, write_meta_data_elem, context);
        ddsts_ostream_puts(((output_context_t*)context)->ostream, "</Module>");
      }
      break;
    case DDSTS_STRUCT:
      if (included_nodes_contains(((meta_data_context_t*)context)->included_nodes, exec_state->node)) {
        output(((output_context_t*)context)->ostream, "<Struct name=\\\"$N\\\">", 'N', ((ddsts_definition_t*)exec_state->node)->name, '\0');
        ddsts_walk(exec_state, DDSTS_STRUCT_MEMBER, DDSTS_DECLARATOR, write_meta_data_elem, context);
        ddsts_ostream_puts(((output_context_t*)context)->ostream, "</Struct>");
      }
      break;
    case DDSTS_DECLARATOR: {
      output(((output_context_t*)context)->ostream, "<Member name=\\\"$N\\\">", 'N', ((ddsts_definition_t*)exec_state->node)->name, '\0');
      ddsts_walk(exec_state, 0, DDSTS_ARRAY_SIZE, write_meta_data_open_array, context);
      ddsts_struct_member_t *struct_member = (ddsts_struct_member_t*)exec_state->call_parent->node;
      ddsts_walk_exec_state_t type_spec_exec_state;
      type_spec_exec_state.node = &struct_member->member_type.type_spec->node;
      type_spec_exec_state.call_parent = exec_state->call_parent->call_parent;
      write_meta_data_type_spec(&type_spec_exec_state, context);
      ddsts_walk(exec_state, 0, DDSTS_ARRAY_SIZE, write_meta_data_close_array, context);
      ddsts_ostream_puts(((output_context_t*)context)->ostream, "</Member>");
      break;
    }
    default:
      assert(false);
      break;
  }
}
 
static void write_meta_data(ddsts_ostream_t *ostream, ddsts_struct_t *struct_def)
{
  /* Determine which structs should be include */
  included_node_t *included_nodes = NULL;
  find_used_structs(&included_nodes, struct_def);

  ddsts_node_t *root_node = &struct_def->def.type_spec.node;
  while (root_node->parent != NULL)
    root_node = root_node->parent;

  output(ostream, "<MetaData version=\\\"1.0.0\\\">", '\0');
 
  ddsts_walk_exec_state_t exec_state;
  exec_state.node = root_node;
  exec_state.call_parent = NULL;

  meta_data_context_t context;
  meta_data_context_init(&context, ostream, included_nodes);
  ddsts_walk(&exec_state, 0, DDSTS_MODULE|DDSTS_STRUCT, write_meta_data_elem, &context.output_context);

  output(ostream, "</MetaData>", '\0');

  included_nodes_free(included_nodes);
}

/*   Functions for generating the source file */

static void write_source_struct(ddsts_walk_exec_state_t *exec_state, void *context)
{
  OS_UNUSED_ARG(context);

  assert(exec_state->node->flags == DDSTS_STRUCT);
  ddsts_struct_t *struct_def = (ddsts_struct_t*)exec_state->node;
  if (struct_def->part_of)
    return;

  const char *full_name = name_with_module_prefix(&struct_def->def);
  if (full_name == NULL)
    return;

  output(((output_context_t*)context)->ostream,
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
  write_meta_data(((output_context_t*)context)->ostream, struct_def);
  output(((output_context_t*)context)->ostream,
         "\"\n"
         "};\n",
         '\0');

  os_free((void*)full_name);
}

static void generate_source_file(const char* file_name, ddsts_node_t *root_node, ddsts_ostream_t *ostream)
{
  OS_UNUSED_ARG(root_node);
  const char *c_file_name = output_file_name(file_name, "c");
  if (c_file_name == NULL)
    return;
  if (!ddsts_ostream_open(ostream, c_file_name)) {
    fprintf(stderr, "Could not open file '%s' for writing\n", c_file_name);
    return;
  }
  const char *h_file_name = output_file_name(file_name, "h");
  if (h_file_name == NULL)
    return;

  write_copyright_header(ostream, c_file_name, file_name);
  output(ostream, "#include \"$F\"\n\n\n\n", 'F', h_file_name, '\0');

  ddsts_walk_exec_state_t exec_state;
  exec_state.node = root_node;
  exec_state.call_parent = NULL;

  output_context_t output_context;
  output_context.ostream = ostream;
  ddsts_walk(&exec_state, DDSTS_MODULE, DDSTS_STRUCT, write_source_struct, &output_context);

  ddsts_ostream_close(ostream);
  os_free((void*)h_file_name);
  os_free((void*)c_file_name);
}

/* Function for generating C99 files */

static void ddsts_generate_C99_to_ostream(const char *file_name, ddsts_node_t *root_node, ddsts_ostream_t *ostream)
{
  generate_header_file(file_name, root_node, ostream);
  generate_source_file(file_name, root_node, ostream);
}

extern void ddsts_generate_C99(const char *file_name, ddsts_node_t *root_node)
{
  ddsts_ostream_t *ostream = NULL;
  ddsts_create_ostream_to_files(&ostream);
  if (ostream == NULL) {
    return;
  }
  ddsts_generate_C99_to_ostream(file_name, root_node, ostream);
  os_free((void*)ostream);
}

extern void ddsts_generate_C99_to_buffer(const char *file_name, ddsts_node_t *root_node, char *buffer, size_t buffer_len)
{
  ddsts_ostream_t *ostream = NULL;
  ddsts_create_ostream_to_buffer(buffer, buffer_len, &ostream);
  if (ostream == NULL) {
    return;
  }
  ddsts_generate_C99_to_ostream(file_name, root_node, ostream);
  os_free((void*)ostream);
}
