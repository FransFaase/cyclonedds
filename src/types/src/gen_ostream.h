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
#ifndef DDS_TS_GEN_OSTREAM_H
#define DDS_TS_GEN_OSTREAM_H

typedef struct dds_ts_ostream_t dds_ts_ostream_t;
struct dds_ts_ostream_t {
  bool (*open)(dds_ts_ostream_t *ostream, const char *name);
  void (*close)(dds_ts_ostream_t *ostream);
  void (*put)(dds_ts_ostream_t *ostream, char ch);
};

bool dds_ts_ostream_open(dds_ts_ostream_t *ostream, const char *name);
void dds_ts_ostream_close(dds_ts_ostream_t *ostream);
void dds_ts_ostream_put(dds_ts_ostream_t *ostream, char ch);
void dds_ts_ostream_puts(dds_ts_ostream_t *ostream, const char *str);

void dds_ts_create_ostream_to_buffer(char *buffer, size_t len, dds_ts_ostream_t **ostream);

#endif /* DDS_TS_GEN_OSTREAM_H */
