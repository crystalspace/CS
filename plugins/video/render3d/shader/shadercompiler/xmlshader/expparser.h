/*
  Copyright (C) 2004 by Frank Richter
	    (C) 2004 by Jorrit Tyberghein

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __EXPPARSER_H__
#define __EXPPARSER_H__

#include "csutil/array.h"
#include "csutil/csstring.h"

/**
 * Possible token types. Stored to simplify the parser's work.
 */
enum csExpressionTokenType
{
  tokenOperator,
  tokenBraces,
  tokenIdentifier,
  tokenNumber
};

/**
 * A single token of an expression.
 */
struct csExpressionToken
{
  const char* tokenStart;
  size_t tokenLen;
  csExpressionTokenType type;

  /// Return a name for a type for error reporting purposes.
  static const char* TypeDescription (csExpressionTokenType type);

  class Extractor
  {
    csString scratch;
  public:
    Extractor (const csExpressionToken& t)
    {
      scratch.Append (t.tokenStart, t.tokenLen);
    }
    const char* Get ()
    {
      return scratch.GetDataSafe ();
    }
  };
};

/// List of tokens.
typedef csArray<csExpressionToken> csExpressionTokenList;

/**
 * Tokenizes strings so the array of tokens can later be parsed.
 */
class csExpressionTokenizer
{
  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);
public:
  const char* Tokenize (const char* string, size_t len, 
    csExpressionTokenList& tokens);
};

/**
 * A node in the expression tree. 
 */
struct csExpression
{
  enum
  {
    Expression,
    Value
  } type;

  union
  {
    struct
    {
      csExpression* left;
      csExpressionToken op;
      csExpression* right;
    } expressionValue;
    csExpressionToken valueValue;
  };

  csExpression (csExpression* left, const csExpressionToken& op, 
    csExpression* right)
  {
    type = Expression;
    expressionValue.left = left;
    expressionValue.op = op;
    expressionValue.right = right;
  }

  csExpression (const csExpressionToken& value)
  {
    type = Value;
    valueValue = value;
  }

  ~csExpression ()
  {
    if (type == Expression)
    {
      delete expressionValue.left;
      delete expressionValue.right;
    }
  }
};

/**
 * Turns a list of tokens into an expression tree.
 */
class csExpressionParser
{
  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);

  const char* Parse (const csExpressionTokenList& tokens,
    csExpression*& result, size_t offset, size_t num);
  const char* ParseOperand (const csExpressionTokenList& tokens,
    csExpression*& result, size_t offset, size_t num, size_t& opEnd);
  /**
   * Count how many tokens are between a pair of braces, not including
   * the braces.
   */
  const char* MatchBrace (const csExpressionTokenList& tokens,
    size_t offset, size_t num, size_t& skipCount);
public:
  const char* Parse (const csExpressionTokenList& tokens,
    csExpression*& result)
  {
    return Parse (tokens, result, 0, tokens.Length ());
  }
};

#endif // __EXPPARSER_H__
