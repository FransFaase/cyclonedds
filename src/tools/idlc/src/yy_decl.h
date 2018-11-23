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
#ifndef DDS_TT_YY_DECL_H
#define DDS_TT_YY_DECL_H

#define YY_DECL int dds_tt_parser_lex \
  (YYSTYPE * yylval_param, \
   YYLTYPE * yylloc_param, \
   yyscan_t yyscanner, \
   dds_tt_context_t *context)

extern YY_DECL;

#endif /* DDS_TT_YY_DECL_H */

