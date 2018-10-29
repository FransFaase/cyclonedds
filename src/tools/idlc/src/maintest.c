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

#include "parser.h"

static void
usage(const char *prog)
{
  fprintf(stderr, "Usage: %s\n", prog);
}

void test_parse(const char *str, int okay, int *nr_errors)
{
  int result = idl_parse_string(str, /*ignore_yyerror=*/okay == 0);
  if (okay == 1 && result != 0) {
    fprintf(stderr, "Error: parsing '%s' failed with %d\n", str, result);
    (*nr_errors)++;
  } else if (okay == 0 && result == 0) {
    fprintf(stderr, "Error: parsing '%s' should have failed\n", str);
    (*nr_errors)++;
  }
}

int
main(int argc, char *argv[])
{
  if (argc != 1) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  int nr_errors = 0;

  test_parse("", 0, &nr_errors);
  test_parse("??", 0, &nr_errors);
  test_parse("const short een = 1;", 1, &nr_errors);

  if (nr_errors > 0) {
    fprintf(stderr, "Nummer failed tests is %d\n", nr_errors);
  } else {
    fprintf(stderr, "All test passed\n");
  }

  return EXIT_SUCCESS;
}
