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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parser.h"

static void
usage(const char *prog)
{
  fprintf(stderr, "Usage: %s FILE\n", prog);
}

int
main(int argc, char *argv[])
{
  int ret = EXIT_FAILURE;

  if (argc != 2) {
    usage(argv[0]);
  } else if (dds_ts_parse_file(argv[1]) == 0) {
    ret = EXIT_SUCCESS;
  }

  return ret;
}

#if TESTING_SOME_TESTS
#include <string.h>

bool test_parse_stringify(const char *input, const char *output)
{
  char buffer[1000];
  buffer[0] = '\0';
  dds_ts_parse_string_stringify(input, buffer, 1000);
  if (strcmp(buffer, output) == 0) {
    return true;
  }
  fprintf(stderr, "Parsing: '%s'\n", input);
  fprintf(stderr, "Expect:  '%s'\n", output);
  fprintf(stderr, "Result:  '");
  for (char *s = buffer; *s != '\0'; s++)
    if (*s < ' ')
      fprintf(stderr, "<%02x>", *s);
    else
      fprintf(stderr, "%c", *s);
  fprintf(stderr, "'\n");
  fprintf(stderr, "%s\n", strcmp(buffer, output) == 0 ? "OK" : "ERROR");
  return false;
}

#define CU_ASSERT(X) X
void run_test()
{
}
#endif
