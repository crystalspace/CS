/*
    Copyright (C) 1998 by Jorrit Tyberghein and Steve Israelson
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __PARSER_H__
#define __PARSER_H__

/**
 * Structure to describe a single token
 */
typedef struct 
{
  /// value returned when token is matched
  long	id;
  /// token to match
  char	*token;
} csTokenDesc;

/**
 * A set of macros for easier building of token tables. Usage example:
 * <code>
 * TOKEN_DEF_START
 *   TOKEN_DEF (ORIG)
 *   TOKEN_DEF (FIRST_LEN)
 *   TOKEN_DEF (FIRST)
 *   TOKEN_DEF (SECOND_LEN)
 *   TOKEN_DEF (SECOND)
 *   TOKEN_DEF (MATRIX)
 *   TOKEN_DEF (V)
 * TOKEN_DEF_END
 * ...
 * void some_func ()
 * {
 *   TOKEN_TABLE_START (tokens)
 *     TOKEN_TABLE (ORIG)
 *     TOKEN_TABLE (ORIG)
 *     TOKEN_TABLE (FIRST_LEN)
 *     TOKEN_TABLE (FIRST)
 *     TOKEN_TABLE (SECOND_LEN)
 *     TOKEN_TABLE (SECOND)
 *     TOKEN_TABLE (MATRIX)
 *     TOKEN_TABLE (V)
 *   TOKEN_TABLE_END
 *   ...
 *   while ((cmd = csGetObject (&buf, tokens, &name, &params)) > 0)
 *   {
 *     switch (cmd)
 *     {
 *       case TOKEN_ORIG:
 *         ...
 *       case TOKEN_FIRST_LEN:
 *         ...
 *       case TOKEN_FIRST:
 *         ...
 *     }
 *   }
 * ...
 * </code>
 */
/// Start a csTokenDesc array
#define TOKEN_DEF_START			\
  enum					\
  {					\
    TOKEN_NONE = 0,
#define TOKEN_DEF(name)			\
    TOKEN_ ## name,
#define TOKEN_DEF_END			\
    TOKEN_TOTAL_COUNT			\
  };
#define TOKEN_TABLE_START(name)		\
  static csTokenDesc name [] =		\
  {
#define TOKEN_TABLE(name)		\
    { TOKEN_ ## name, #name },
#define TOKEN_TABLE_END			\
    { 0, 0 }				\
  };

/// A string containing white spaces (' ', '\t', '\n', '\r')
extern const char *kWhiteSpace;

#define PARSERR_EOF -2
#define PARSERR_TOKENNOTFOUND -1

/**
 * Get pointer to last offending token.
 */
char* csGetLastOffender ();

/**
 * Pass in a pointer to a buffer of text. This pointer is modified to point
 * to the text after the object description. The token id for the object is
 * returned. The name pointer will point to the optional name string in the
 * buffer. The data pointer will point to the optional data for the object.
 * <i>The text buffer will get modified so BEWARE.</i>
 * <p><b>eg text</b>:
 * <code>
 *   ROOM "test room" ( 1, 2, 3 )
 * </code>
 * <p>returning PARSERR_TOKENNOTFOUND on error.
 * <p>returning PARSERR_EOF on EOF.
 */
long csGetObject(char **buf, csTokenDesc *tokens, char **name, char **data);
/**
 * Pass in a pointer to a buffer of text. This pointer is modified to point
 * to the text after the command description. The token id for the command is
 * returned. The params pointer will point to the optional data for the object.
 * <i>The text buffer will get modified so BEWARE.</i>
 * <p><b>eg text</b>:
 * <code>
 *   TEXTURE( 1, 2, 3 )
 * </code>
 * <p>returning -1 is an error.
 * <p>returning -2 is EOF.
 * <p><b>NOTE</b>: Should be modified to accept assignments like:
 * <code>
 *   LIGHT=1
 * </code>
 */
long csGetCommand(char **buf, csTokenDesc *tokens, char **params);
/**
 * Returns the string of text between the open and close characters.
 * Modifies the buffer. Moves the buffer pointer to after the last delimiter.
 * Can return NULL; buffer MUST already be at the opening delimiter.
 * Skips nested delimiters too.
 * <p><b>NOTE</b>: Should skip quoted text, does not at this time.
 */
char *GetSubText(char **buf, char open, char close);
/**
 * Skips any characters in the toSkip string.
 * Changes the buf pointer.
 */
void SkipCharacters(char **buf, const char *toSkip);
/**
 * Returns the string of text after a = up to the next
 * whitespace. Terminates the string and moves the buf pointer.
 */
char *GetAssignmentText(char **buf);

#endif // __PARSER_H__
