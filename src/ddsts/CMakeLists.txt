#
# Copyright(c) 2006 to 2019 ADLINK Technology Limited and others
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
# v. 1.0 which is available at
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#

include(GenerateExportHeader)

set(headers
    include/dds/ddsts/typetree.h
    include/dds/ddsts/type_walk.h)

set(sources
    src/typetree.c
    src/type_walk.c)

add_library(ddsts ${headers} ${sources})
target_include_directories(
  ddsts
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
      "$<INSTALL_INTERFACE:include>"
)
target_link_libraries(ddsts PRIVATE OSAPI)


generate_export_header(
  ddsts EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/dds/ddsts/export.h")

