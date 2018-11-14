#
# Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
# v. 1.0 which is available at
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#
#project(idlpp C)
#cmake_minimum_required(VERSION 3.6)
add_subdirectory(tests)

# Find Bison and Flex
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Abstraction REQUIRED)

# Create directory where Bison and Flex generated sources will be placed.
set(GENERATED_SOURCE_DIR "${CMAKE_BINARY_DIR}/src/tools/idlc/src")
file(MAKE_DIRECTORY "${GENERATED_SOURCE_DIR}")

set(PARSER_BASENAME "idl.parser.c")
set(PARSER_PATH "${GENERATED_SOURCE_DIR}/${PARSER_BASENAME}")
set(LEXER_BASENAME "idl.lexer.c")
set(LEXER_PATH "${GENERATED_SOURCE_DIR}/${LEXER_BASENAME}")

bison_target(parser "src/idl.y" "${PARSER_PATH}")
flex_target(
  lexer "src/idl.l" "${LEXER_PATH}"
  COMPILE_FLAGS
    "--header-file=${GENERATED_SOURCE_DIR}/idl.lexer.h")

add_library(idlpplib "src/parser.c" "src/tools.c" "src/type.c" "src/type_api.c" "src/type_walker.c" "src/stringify.c" "${BISON_parser_OUTPUTS}" "${FLEX_lexer_OUTPUTS}")
target_include_directories(idlpplib PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(idlpplib PRIVATE "${CMAKE_SOURCE_DIR}/os/include")
target_include_directories(idlpplib PRIVATE "include")
target_include_directories(idlpplib PRIVATE "src")
target_include_directories(idlpplib PRIVATE "${GENERATED_SOURCE_DIR}")

target_link_libraries(idlpplib PRIVATE OSAPI)

get_target_property(os_api_src_dir OSAPI SOURCE_DIR)
# We need to expose some of the OS headers as well.
target_include_directories(idlpplib PUBLIC "$<BUILD_INTERFACE:${os_api_src_dir}/include>")

add_executable(idlpp "src/main.c")
target_link_libraries(idlpp PRIVATE OSAPI idlpplib)

