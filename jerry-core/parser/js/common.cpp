/* Copyright 2015 Samsung Electronics Co., Ltd.
 * Copyright 2015 University of Szeged.
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

#include "common.h"
#include "ecma-helpers.h"

/**
 * Checks whether the next UTF8 character is a valid identifier start.
 *
 * @return non-zero if it is.
 */
int
util_is_identifier_start (const uint8_t *src_p) /* pointer to a vaild UTF8 character */
{
  if (*src_p > 127)
  {
    return 0;
  }

  return util_is_identifier_start_character (*src_p);
} /* util_is_identifier_start */

/**
 * Checks whether the next UTF8 character is a valid identifier part.
 *
 * @return non-zero if it is.
 */
int
util_is_identifier_part (const uint8_t *src_p) /* pointer to a vaild UTF8 character */
{
  if (*src_p > 127)
  {
    return 0;
  }

  return util_is_identifier_part_character (*src_p);
} /* util_is_identifier_part */

/**
 * Checks whether the character is a valid identifier start.
 *
 * @return non-zero if it is.
 */
int
util_is_identifier_start_character (uint16_t chr) /**< EcmaScript character */
{
  return (((chr | 0x20) >= 'a' && (chr | 0x20) <= 'z')
          || chr == '$' || chr == '_');
} /* util_is_identifier_start_character */

/**
 * Checks whether the character is a valid identifier part.
 *
 * @return non-zero if it is.
 */
int
util_is_identifier_part_character (uint16_t chr) /**< EcmaScript character */
{
  return (((chr | 0x20) >= 'a' && (chr | 0x20) <= 'z')
          || (chr >= '0' && chr <= '9')
          || chr == '$' || chr == '_');
} /* util_is_identifier_part_character */

/**
 * Converts a character to UTF8 bytes.
 *
 * @return length of the UTF8 representation.
 */
size_t
util_to_utf8_bytes (uint8_t *dst_p, /**< destination buffer */
                    uint16_t chr) /**< EcmaScript character */
{
  if (!(chr & ~0x007f))
  {
    /* 00000000 0xxxxxxx -> 0xxxxxxx */
    *dst_p = (uint8_t) chr;
    return 1;
  }

  if (!(chr & ~0x07ff))
  {
    /* 00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx */
    *(dst_p++) = (uint8_t) (0xc0 | ((chr >> 6) & 0x1f));
    *dst_p = (uint8_t) (0x80 | (chr & 0x3f));
    return 2;
  }

  /* zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx */
  *(dst_p++) = (uint8_t) (0xe0 | ((chr >> 12) & 0x0f));
  *(dst_p++) = (uint8_t) (0x80 | ((chr >> 6) & 0x3f));
  *dst_p = (uint8_t) (0x80 | (chr & 0x3f));
  return 3;
} /* util_to_utf8_bytes */

/**
 * Returns the length of the UTF8 representation of a character.
 *
 * @return length of the UTF8 representation.
 */
size_t
util_get_utf8_length (uint16_t chr) /**< EcmaScript character */
{
  if (!(chr & ~0x007f))
  {
    /* 00000000 0xxxxxxx */
    return 1;
  }

  if (!(chr & ~0x07ff))
  {
    /* 00000yyy yyxxxxxx */
    return 2;
  }

  /* zzzzyyyy yyxxxxxx */
  return 3;
} /* util_get_utf8_length */

/**
 * Initializes a function literal from the argument.
 */
void
util_set_function_literal (lexer_literal_t *literal_p, /**< literal */
                           void *function_p) /* function */
{
  literal_p->u.bytecode_p = function_p;
} /* util_set_function_literal */

/**
 * Free literal.
 */
void
util_free_literal (lexer_literal_t *literal_p) /**< literal */
{
  if (literal_p->type == LEXER_IDENT_LITERAL
      || literal_p->type == LEXER_STRING_LITERAL)
  {
    if (!(literal_p->status_flags & LEXER_FLAG_SOURCE_PTR))
    {
      PARSER_FREE ((uint8_t *) literal_p->u.char_p);
    }
  }
  else if ((literal_p->type == LEXER_FUNCTION_LITERAL)
           || (literal_p->type == LEXER_REGEXP_LITERAL))
  {
    ecma_bytecode_deref (literal_p->u.bytecode_p);
  }
} /* util_free_literal */

#ifdef PARSER_DUMP_BYTE_CODE

/**
 * Debug utility to print a character sequence.
 */
static void
util_print_chars (const uint8_t *char_p, /**< character pointer */
                  size_t size) /**< size */
{
  while (size > 0)
  {
    printf ("%c", *char_p++);
    size--;
  }
}

/**
 * Debug utility to print a number.
 */
static void
util_print_number (ecma_number_t num_p)
{
  lit_utf8_byte_t str_buf[ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER];
  lit_utf8_size_t str_size = ecma_number_to_utf8_string (num_p, str_buf, sizeof (str_buf));
  str_buf[str_size] = 0;
  printf ("%s", str_buf);
}

/**
 * Print literal.
 */
void
util_print_literal (lexer_literal_t *literal_p) /**< literal */
{
  if (literal_p->type == LEXER_IDENT_LITERAL)
  {
    if (literal_p->status_flags & LEXER_FLAG_VAR)
    {
      printf ("var_ident(");
    }
    else
    {
      printf ("ident(");
    }
    util_print_chars (literal_p->u.char_p, literal_p->prop.length);
  }
  else if (literal_p->type == LEXER_FUNCTION_LITERAL)
  {
    printf ("function");
    return;
  }
  else if (literal_p->type == LEXER_STRING_LITERAL)
  {
    printf ("string(");
    util_print_chars (literal_p->u.char_p, literal_p->prop.length);
  }
  else if (literal_p->type == LEXER_NUMBER_LITERAL)
  {
    lit_literal_t literal = lit_get_literal_by_cp (literal_p->u.value);
    printf ("number(");
    util_print_number (lit_number_literal_get_number (literal));
  }
  else if (literal_p->type == LEXER_REGEXP_LITERAL)
  {
    printf ("regexp");
    return;
  }
  else
  {
    printf ("unknown");
    return;
  }

  printf (")");
} /* util_print_literal */

#endif /* PARSER_DUMP_BYTE_CODE */
