/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein and Steve Israelson

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

#ifndef __CS_CS2XML_PARSER_H__
#define __CS_CS2XML_PARSER_H__

#include "csutil/csvector.h"
#include "csutil/util.h"

/**
 * Structure to describe a single token in a token table that is passed
 * to most token-parsing functions.
 */
class csTokenDesc
{
public:
  /// value returned when token is matched
  long	id;
  /// token to match
  char *token;
  /// Store length for optimization.
  int len;

  csTokenDesc () { id=0; token=NULL;  }
  ~csTokenDesc () { delete [] token; }
};

/**
 * Token table that is passed to most token-parsing functions.
 * Created using CS_TOKEN_DEF_START, CS_TOKEN_DEF, CS_TOKEN_DEF_END.<p>
 * Usage example:
 * <pre>
 * CS_TOKEN_DEF_START
 *   CS_TOKEN_DEF (ORIG)
 *   CS_TOKEN_DEF (FIRST_LEN)
 *   CS_TOKEN_DEF (FIRST)
 *   CS_TOKEN_DEF (SECOND_LEN)
 *   CS_TOKEN_DEF (SECOND)
 *   CS_TOKEN_DEF (MATRIX)
 *   CS_TOKEN_DEF (V)
 * CS_TOKEN_DEF_END
 * ...
 * void some_func ()
 * {
 *   CS_TOKEN_TABLE_START (tokens)
 *     CS_TOKEN_TABLE (ORIG)
 *     CS_TOKEN_TABLE (FIRST_LEN)
 *     CS_TOKEN_TABLE (FIRST)
 *     CS_TOKEN_TABLE (SECOND_LEN)
 *     CS_TOKEN_TABLE (SECOND)
 *     CS_TOKEN_TABLE (MATRIX)
 *     CS_TOKEN_TABLE (V)
 *   CS_TOKEN_TABLE_END
 *   ...
 *   while ((cmd = csGetObject (&buf, tokens, &name, &params)) > 0)
 *   {
 *     switch (cmd)
 *     {
 *       case CS_TOKEN_ORIG:
 *         ...
 *       case CS_TOKEN_FIRST_LEN:
 *         ...
 *       case CS_TOKEN_FIRST:
 *         ...
 *     }
 *   }
 * ...
 * }
 * </pre>
 */
class csTokenVector : public csVector
{
public:
  virtual ~csTokenVector () {DeleteAll ();}
  virtual bool FreeItem (csSome Item)
  {
    delete (csTokenDesc*)Item;
    return true;
  }
  csTokenDesc* Get (int idx) const
  {
    return (csTokenDesc*)csVector::Get (idx);
  }
  virtual int Compare (csSome Item1, csSome Item2, int Mode=0) const
  {
    (void)Mode;
    csTokenDesc *td1 = (csTokenDesc*)Item1;
    csTokenDesc *td2 = (csTokenDesc*)Item2;
    int l1 = td1->len;
    int l2 = td2->len;
    return l1 > l2 ? -1 : l1 < l2 ? 1 : 0;
  }
  virtual int CompareKey (csSome Item1, csConstSome Key, int Mode=0) const
  {
    (void)Mode;
    csTokenDesc *td1 = (csTokenDesc*)Item1;
    csTokenDesc *td2 = (csTokenDesc*)Key;
    int l1 = td1->len;
    int l2 = td2->len;
    return l1 > l2 ? -1 : l1 < l2 ? 1 : 0;
  }
  csTokenVector *Push (int id, const char *name)
  {
    csTokenDesc *td = new csTokenDesc;
    td->id = id;
    td->token = (name ? csStrNew (name) : 0);
    td->len = name ? strlen (name) : 0;
    InsertSorted (td);
    return this;
  }
};

/**
 * A set of macros for easier building of token tables.
 */
#define CS_TOKEN_DEF_START		\
  enum					\
  {					\
    CS_TOKEN_EMPTY = 0,
#define CS_TOKEN_DEF(name)		\
    CS_TOKEN_ ## name,
#define CS_TOKEN_DEF_END		\
    CS_TOKEN_TOTAL_COUNT		\
  };
#define CS_TOKEN_TABLE_START(name)	\
  csTokenVector name ## _hlp, *name;    \
  name = &name ## _hlp;			\
  name->
#define CS_TOKEN_TABLE(name)		\
    Push (CS_TOKEN_ ## name, #name )->
#define CS_TOKEN_TABLE_END		\
    Push (0, NULL);

#define CS_PARSERR_EOF			-2
#define CS_PARSERR_TOKENNOTFOUND	-1

/**
 * Provides a set of functions to parse CS ASCII data (like worlds,
 * sprites etc.)
 */
class csParser
{
private:
  char last_offender[255];

  int parser_line;

  // If the following flag is true then we allow unknown tokens
  // (i.e. they will not generate an error). The token name will
  // be copied to 'unknown_token'. By default this is false.
  bool allow_unknown_tokens;

  // Name of an unknown token (if allow_unknown_tokens == true).
  char unknown_token[255];

public:
  /**
   * Initialize the parser. If 'allow_unknown_tokens' is true then unknown
   * tokens are allowed. It will still return CS_PARSERR_TOKENNOTFOUND
   * in such case but the parsing will continue and the unknown token
   * name can be retrieved with 'GetUnknownToken()'.
   */
  csParser (bool allow_unknown_tokens = false);

  /**
   * Reset the line number of the line being parsed to line one.  This does not
   * rewind a file pointer or any other such operation.  It merely resets the
   * internal line number counter which is reported by csGetParserLine().
   */
  void ResetParserLine ();

  /**
   * Get the current line number of the file being parsed.
   */
  int GetParserLine ();

  /**
   * Get pointer to last offending token.
   */
  char* GetLastOffender ();

  /**
   * Get pointer to last unknown token. This is only valid if
   * the parser was constructed with the allow_unknown_tokens flag to true.
   */
  const char* GetUnknownToken () { return unknown_token; }

  /**
   * Get next token and his parameters.
   * Pass in a pointer to a buffer of text. This pointer is modified to point
   * to the text after the object description. The token id for the object is
   * returned. The name pointer will point to the optional name string in the
   * buffer. The data pointer will point to the optional data for the object.
   * NULL can be passed for name/data if this information is not wanted.
   * Otherwise the variables pointed to are ALWAYS modified; if the optional
   * name/data is not present or an error occured retrieving the token the
   * variables pointed to by name/data are set to NULL.
   * <i>The text buffer will get modified so BEWARE.</i>
   * <p><b>eg text</b>:
   * <pre>
   *   ROOM 'test room' ( 1, 2, 3 )
   * </pre>
   * <p>returns CS_PARSERR_TOKENNOTFOUND on error (if the parser was
   *    created with allow_unknown_tokens == true then this is not actually
   *    an error).
   * <p>returns CS_PARSERR_EOF on EOF.
   */
  long GetObject (char **buf, csTokenVector *tokens, char **name, char **data);

  /**
   * Pass in a pointer to a buffer of text. This pointer is modified to point
   * to the text after the command description. The token id for the command is
   * returned. The params pointer will point to the optional data for the
   * object.
   * <i>The text buffer will get modified so BEWARE.</i>
   * <p><b>eg text</b>:
   * <code>
   *   TEXTURE( 1, 2, 3 )
   * </code>
   * <p>returning CS_PARSERR_TOKENNOTFOUND on error.
   * <p>returning CS_PARSERR_EOF on EOF.
   * <p><b>NOTE</b>: Should be modified to accept assignments like:
   * <code>
   *   LIGHT=1
   * </code>
   */
  long GetCommand (char **buf, csTokenVector *tokens, char **params);

  /**
   * Returns the string of text between the open and close characters.
   * Modifies the buffer. Moves the buffer pointer to after the last delimiter.
   * Can return NULL; buffer MUST already be at the opening delimiter.
   * Skips nested delimiters too.
   * <p><b>NOTE</b>: Should skip quoted text, does not at this time.
   */
  char *GetSubText (char **buf, char open, char close);

  /**
   * Skips any characters in the toSkip string.
   * Changes the buf pointer.
   */
  void SkipCharacters (char **buf, const char *toSkip);

  /**
   * Returns the length of the next token in the buffer.
   */
  int SkipToken (char *buf);

  /**
   * Returns the string of text after a = up to the next
   * whitespace. Terminates the string and moves the buf pointer.
   */
  char *GetAssignmentText (char **buf);
};

#endif // __CS_CS2XML_PARSER_H__

