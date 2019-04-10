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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dds/ddsts/typetree.h"
#include "parser.h"
#include "templ_script.h"

static void
usage(const char *prog)
{
  fprintf(stderr, "Usage: %s FILE [--ts FILE [--debug]]\n", prog);
}

void report_error(int line, int column, const char *msg)
{
  fprintf(stderr, "ERROR %d.%d: %s\n", line, column, msg);
}

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }
  const char *input_file_name = argv[1];
  bool ts_backend = false;
  const char *ts_file_name = NULL;
  bool ts_debug = false;

  for (int i = 2; i < argc;) {
    if (strcmp(argv[i], "--ts") == 0) {
      ts_backend = true;
      i++;
      if (i >= argc) {
        usage(argv[0]);
        fprintf(stderr, "Error: Missing file name after --ts option.\n");
        return EXIT_FAILURE;
      }
      ts_file_name = argv[i];
      i++;
      if (i < argc && strcmp(argv[i], "--debug") == 0) {
        ts_debug = true;
        i++;
      }
    }
    else {
      usage(argv[0]);
      fprintf(stderr, "Error: Unexpected argument '%s'\n", argv[i]);
      return EXIT_FAILURE;
    }
  }

  ddsts_type_t *root_type = NULL;
  if (dds_idl_parse_file(argv[1], report_error, &root_type) != DDS_RETCODE_OK) {
    return EXIT_FAILURE;
  }

  if (ts_backend) {
    FILE *tsf = fopen(ts_file_name, "rt");
    if (tsf == NULL) {
      usage(argv[0]);
      fprintf(stderr, "Error: cannot open file '%s'\n", ts_file_name);
      return EXIT_FAILURE;
    }
    dds_exec_templ_script(input_file_name, root_type, tsf, ts_debug);
    fclose(tsf);
  }

  ddsts_free_type(root_type);

  return EXIT_SUCCESS;
}
