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
#ifndef DDSI_ETH_H
#define DDSI_ETH_H

#include "ddsi/ddsi_tran.h"
#include "os/os.h"

int ddsi_eth_enumerate_interfaces(ddsi_tran_factory_t fact, os_ifaddrs_t **ifs);

#endif /* DDSI_ETH_H */
