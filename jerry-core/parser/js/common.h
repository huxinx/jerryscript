/* Copyright 2015-2016 Samsung Electronics Co., Ltd.
 * Copyright 2015-2016 University of Szeged.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <setjmp.h>

#ifndef JERRY_NDEBUG
#define PARSER_DEBUG
#endif

#ifndef JERRY_NDEBUG
/* Note: This flag is independent from debug mode. */
#define PARSER_DUMP_BYTE_CODE
#endif

#include "ecma-globals.h"
#include "ecma-regexp-object.h"
#include "lit-literal.h"
#include "mem-heap.h"

/* The utilites here are just for compiling purposes, JS
 * engines should have an optimized version for them. */

/* Malloc functions. */

#define PARSER_MALLOC(size) mem_heap_alloc_block (size, MEM_HEAP_ALLOC_LONG_TERM)
#define PARSER_FREE(ptr) mem_heap_free_block ((void *) ptr)

#define PARSER_MALLOC_LOCAL(size) mem_heap_alloc_block (size, MEM_HEAP_ALLOC_SHORT_TERM)
#define PARSER_FREE_LOCAL(ptr) mem_heap_free_block (ptr)

/* UTF character management. Only ASCII characters are
 * supported for simplicity. */

int util_is_identifier_start (const uint8_t *);
int util_is_identifier_part (const uint8_t *);
int util_is_identifier_start_character (uint16_t);
int util_is_identifier_part_character (uint16_t);
size_t util_to_utf8_bytes (uint8_t *, uint16_t);
size_t util_get_utf8_length (uint16_t);

/* Immediate management. */

/**
 * Literal types.
 *
 * The LEXER_UNUSED_LITERAL type is internal and
 * used for various purposes.
 */
typedef enum
{
  LEXER_IDENT_LITERAL = 0,          /**< identifier literal */
  LEXER_STRING_LITERAL = 1,         /**< string literal */
  LEXER_NUMBER_LITERAL = 2,         /**< number literal */
  LEXER_FUNCTION_LITERAL = 3,       /**< function literal */
  LEXER_REGEXP_LITERAL = 4,         /**< regexp literal */
  LEXER_UNUSED_LITERAL = 5,         /**< unused literal, can only be
                                         used by the byte code generator. */
} lexer_literal_type_t;

/* Flags for status_flags. */

/* Local identifier (var, function arg). */
#define LEXER_FLAG_VAR 0x01
/* This local identifier cannot be stored in register. */
#define LEXER_FLAG_NO_REG_STORE 0x02
/* This local identifier is initialized with a value. */
#define LEXER_FLAG_INITIALIZED 0x04
/* This local identifier has a reference to the function itself. */
#define LEXER_FLAG_FUNCTION_NAME 0x08
/* This local identifier is a function argument. */
#define LEXER_FLAG_FUNCTION_ARGUMENT 0x10
/* No space is allocated for this character literal. */
#define LEXER_FLAG_SOURCE_PTR 0x20
/* Initialize this variable after the byte code is freed. */
#define LEXER_FLAG_LATE_INIT 0x40

/**
 * Literal data.
 */
typedef struct
{
  union
  {
    lit_cpointer_t value;                /**< literal value (not processed by the parser) */
    const uint8_t *char_p;               /**< character value */
    ecma_compiled_code_t *bytecode_p;    /**< compiled function or regexp pointer */
    uint32_t source_data;                /**< encoded source literal */
  } u;

#ifdef PARSER_DUMP_BYTE_CODE
  struct
#else
  union
#endif
  {
    uint16_t length;                     /**< length of ident / string literal */
    uint16_t index;                      /**< real index during post processing */
  } prop;

  uint8_t type;                          /**< type of the literal */
  uint8_t status_flags;                  /**< status flags */
} lexer_literal_t;

void util_free_literal (lexer_literal_t *);

#ifdef PARSER_DUMP_BYTE_CODE
void util_print_literal (lexer_literal_t *);
#endif /* PARSER_DUMP_BYTE_CODE */

/* TRY/CATCH block */

#define PARSER_TRY_CONTEXT(context_name) \
  jmp_buf context_name

#define PARSER_THROW(context_name) \
  longjmp (context_name, 1);

#define PARSER_TRY(context_name) \
  { \
    if (!setjmp (context_name)) \
    { \

#define PARSER_CATCH \
    } \
    else \
    {

#define PARSER_TRY_END \
    } \
  }

/* Other */

#define PARSER_INLINE inline
#define PARSER_NOINLINE __attribute__ ((noinline))

#endif /* COMMON_H */
