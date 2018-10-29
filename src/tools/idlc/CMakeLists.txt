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
project(idlpp C)
cmake_minimum_required(VERSION 3.6)

# Find Bison and Flex
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

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

add_executable(idlpp "src/main.c" "src/parser.c" "src/tools.c" "${BISON_parser_OUTPUTS}" "${FLEX_lexer_OUTPUTS}")
target_include_directories(idlpp PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories(idlpp PRIVATE "${CMAKE_SOURCE_DIR}/os/include")
target_include_directories(idlpp PRIVATE "include")
target_include_directories(idlpp PRIVATE "src")
target_include_directories(idlpp PRIVATE "${GENERATED_SOURCE_DIR}")
add_executable("idlpptest" "src/maintest.c" "src/parser.c" "src/tools.c" "${BISON_parser_OUTPUTS}" "${FLEX_lexer_OUTPUTS}")
target_include_directories("idlpptest" PRIVATE "${CMAKE_SOURCE_DIR}")
target_include_directories("idlpptest" PRIVATE "${CMAKE_SOURCE_DIR}/os/include")
target_include_directories("idlpptest" PRIVATE "include")
target_include_directories("idlpptest" PRIVATE "src")
target_include_directories("idlpptest" PRIVATE "${GENERATED_SOURCE_DIR}")

