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

#include "cssysdef.h"

#include "expparser.h"
#include "tokenhelper.h"

const char* csExpressionToken::TypeDescription (csExpressionTokenType type)
{
  switch (type)
  {
    case tokenOperator:
      return "operator";
    case tokenBraces:
      return "braces";
    case tokenIdentifier:
      return "identifier";
    case tokenNumber:
      return "number";
  }
  CS_ASSERT (false);
  return "unknown";
}

//---------------------------------------------------------------------------

const char* csExpressionTokenizer::SetLastError (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  lastError.FormatV (msg, args);
  va_end (args);
  return lastError;
}

enum TokenizerState
{
  tSkipWhitespace,
  tNewToken,
  tOperator,
  tIdentifier,
  tNumber,
  tQuotation
};

const char* csExpressionTokenizer::Tokenize (const char* string, size_t len,
					     csExpressionTokenList& tokens)
{
  const char* operatorChars = "=&|<>";

  CS_ASSERT (string != 0);
  const char* end = string + len;
  TokenizerState state = tSkipWhitespace;
  csExpressionToken currentToken;

  currentToken.tokenLen = 0;

  while ((*string != 0) && (string < end))
  {
    switch (state)
    {
      case tSkipWhitespace:
	if (*string == ' ')
	  string++;
	else
	  state = tNewToken;
	break;
      case tNewToken:
	if ((*string == '!') || (strchr (operatorChars, *string) != 0))
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  currentToken.tokenStart = string++;
	  currentToken.tokenLen = 1;
	  currentToken.type = tokenOperator;
	  state = tOperator;
	}
	else if ((*string == '(') || (*string == ')'))
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  currentToken.tokenStart = string++;
	  currentToken.tokenLen = 1;
	  currentToken.type = tokenBraces;
	  state = tSkipWhitespace;
	}
	else if (*string == '.')
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  currentToken.tokenStart = string++;
	  currentToken.tokenLen = 1;
	  currentToken.type = tokenOperator;
	  state = tSkipWhitespace;
	}
	else if (isalpha (*string) || (*string  == '_'))
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  currentToken.tokenStart = string++;
	  currentToken.tokenLen = 1;
	  currentToken.type = tokenIdentifier;
	  state = tIdentifier;
	}
	else if (isdigit (*string)/* || (*string == '.')*/ || (*string == '-'))
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  currentToken.tokenStart = string++;
	  currentToken.tokenLen = 1;
	  currentToken.type = tokenNumber;
	  state = tNumber;
	}
	else if (*string == '"')
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  currentToken.tokenStart = ++string;
	  currentToken.tokenLen = 0;
	  currentToken.type = tokenIdentifier;
	  state = tQuotation;
	}
	else
	{
	  return SetLastError ("Unexpected character '%c' (%" PRId8 ")",
	    *string, *string);
	}
	break;
      case tOperator:
	if (*string == '!')
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  currentToken.tokenStart = string++;
	  currentToken.tokenLen = 1;
	  currentToken.type = tokenOperator;
	  state = tOperator;
	}
	else if (strchr (operatorChars, *string) != 0)
	{
	  currentToken.tokenLen++;
	  string++;
	}
	else
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  state = tSkipWhitespace;
	  currentToken.tokenLen = 0;
	}
	break;
      case tIdentifier:
	if (isalnum (*string) || (*string == '_'))
	{
	  currentToken.tokenLen++;
	  string++;
	}
	else
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  state = tSkipWhitespace;
	  currentToken.tokenLen = 0;
	}
	break;
      case tNumber:
	if (isalnum (*string) || (*string == '.') || (*string == 'e') || 
	  (*string == 'E'))
	{
	  currentToken.tokenLen++;
	  string++;
	}
	else
	{
	  if (currentToken.tokenLen != 0) tokens.Push (currentToken);
	  state = tSkipWhitespace;
	  currentToken.tokenLen = 0;
	}
	break;
      case tQuotation:
	if (*string == '\\')
	{
	  currentToken.tokenLen += 2;
	  string += 2;
	}
	else if (*string == '"')
	{
	  //if (currentToken.tokenLen != 0) 
	  tokens.Push (currentToken);
	  state = tSkipWhitespace;
	  currentToken.tokenLen = 0;
	  string++;
	}
	else
	{
	  currentToken.tokenLen++;
	  string++;
	}
    }
  }
  
  if (state == tQuotation)
    return "Unterminated quotation";

  if (currentToken.tokenLen != 0) tokens.Push (currentToken);

  return 0;
}

//---------------------------------------------------------------------------

const char* csExpressionParser::SetLastError (const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  lastError.FormatV (msg, args);
  va_end (args);
  return lastError;
}

struct Operator
{
  const char* opToken;
  int precedence;
};

enum
{
  opDereference = 0,
  opNot = 1,
  opGreaterEq = 2,
  opLesserEq = 2,
  opGreater = 2,
  opLesser = 2,
  opNEqual = 3,
  opEqual = 3,
  opAnd = 4,
  opOr = 5
};

static const Operator opPrecedence[] = {
  {"||", opOr},
  {"&&", opAnd},
  {"==", opEqual},
  {"!=", opNEqual},
  {"<",  opLesser},
  {">",  opGreater},
  {"<=", opLesserEq},
  {">=", opGreaterEq},
  {"!",	 opNot},
  {".",	 opDereference},
  {0, 0}
};

static void SkipOuterBraces (const csExpressionTokenList& tokens,
			     size_t& offset, size_t& num)
{
  if ((num >= 2) && (tokens[offset].type == tokenBraces)
    && (TokenEquals (tokens[offset].tokenStart, tokens[offset].tokenLen, "("))
    && (tokens[offset + num - 1].type == tokenBraces)
    && (TokenEquals (tokens[offset + num - 1].tokenStart, 
    tokens[offset + num - 1].tokenLen, ")")))
  {
    offset++;
    num -= 2;
  }
}

const char* csExpressionParser::Parse (const csExpressionTokenList& tokens,
				       csExpression*& result, size_t offset, 
				       size_t num)
{
  result = 0;
  const size_t end = offset + num;
  size_t pos = offset;
  const char* err;

  size_t opPos = (size_t)~0;
  int highestPrecedence = -1;
  bool hasBraces = false;

  /// Look for operator with highest precendence
  while (pos < end)
  {
    if (tokens[pos].type == tokenBraces)
    {
      size_t skip;
      err = MatchBrace (tokens, pos, num + offset - pos, skip);
      if (err) return err;
      pos += skip + 1;
      hasBraces = true;
    }
    else if (tokens[pos].type == tokenOperator)
    {
      const Operator* op = opPrecedence;
      while (op->opToken != 0)
      {
	if (TokenEquals (tokens[pos].tokenStart, tokens[pos].tokenLen, 
	  op->opToken))
	{
	  if (op->precedence > highestPrecedence)
	  {
	    highestPrecedence = op->precedence;
	    opPos = pos;
	  }
	  break;
	}
	op++;
      }
      if (op->opToken == 0)
      {
	return SetLastError ("Unknown operator '%s'", 
	  csExpressionToken::Extractor (tokens[pos]).Get ());
      }
    }
    pos++;
  }

  if (highestPrecedence == opNot)
  {
    // special case: !
    if (opPos != offset)
    {
      return "Misplaced '!'";
    }
    csExpression* rightExp;
    err = Parse (tokens, rightExp, offset + 1, num - 1);
    if (err) return err;
    result = new csExpression (0, tokens[opPos], rightExp);
  }
  else if (opPos == (size_t)~0)
  {
    if (hasBraces)
    {
      SkipOuterBraces (tokens, offset, num);
      return Parse (tokens, result, offset, num);
    }
    else
    {
      size_t opEnd;
      err = ParseOperand (tokens, result, offset, num, opEnd);
      if (err) return err;
      if (opEnd != end - 1)
      {
	CS_ASSERT (opEnd < end);
	delete result; result = 0;
	return SetLastError ("Excess tokens beyond '%s'",
	  csExpressionToken::Extractor (tokens[opEnd]).Get ());
      }
    }
  }
  else
  {
    csExpression* leftExp;
    csExpression* rightExp;
    err = Parse (tokens, leftExp, offset, opPos - offset);
    if (err) return err;
    err = Parse (tokens, rightExp, opPos + 1, num + offset - opPos - 1);
    if (err) return err;
    result = new csExpression (leftExp, tokens[opPos], rightExp);
  }

  return 0;
}

const char* csExpressionParser::ParseOperand (const csExpressionTokenList& tokens,
					      csExpression*& result, size_t offset, 
					      size_t num, size_t& opEnd)
{
  result = 0;
  if (num == 0)
  {
    return "Unexpected end of expression";
  }

  if (tokens[offset].type == tokenOperator)
  {
    if (TokenEquals (tokens[offset].tokenStart, tokens[offset].tokenLen, "!"))
    {
      csExpression* lit;
      const char* err = ParseOperand (tokens, lit, offset + 1, num - 1, opEnd);
      if (err) return err;
      result = new csExpression (0, tokens[offset], lit);
      return 0;
    }
    else
    {
      return SetLastError ("Unexpected operator '%s'", 
	csExpressionToken::Extractor (tokens[offset]).Get ());
    }
  }
  else if ((tokens[offset].type == tokenIdentifier) ||
    (tokens[offset].type == tokenNumber))
  {
    result = new csExpression (tokens[offset]);
    opEnd = offset;
    return 0;
  }
  else if (tokens[offset].type == tokenBraces)
  {
    if (TokenEquals (tokens[offset].tokenStart, tokens[offset].tokenLen, "("))
    {
      size_t subexpNum;
      const char* err = MatchBrace (tokens, offset, num, subexpNum);
      if (err) return err;
      err = Parse (tokens, result, offset + 1, subexpNum);
      opEnd = offset + subexpNum + 2;
      return err;
    }
    else
      return SetLastError ("Unexpected token '%s'", 
	csExpressionToken::Extractor (tokens[offset]).Get ());
  }
  else
  {
    return SetLastError ("Unexpected token ('%s') of type '%s'",
      csExpressionToken::Extractor (tokens[offset]).Get (), 
      csExpressionToken::TypeDescription (tokens[offset].type));
  }
}

const char* csExpressionParser::MatchBrace (
  const csExpressionTokenList& tokens, size_t offset, size_t num, 
  size_t& skipCount)
{
  if (num == 0)
  {
    return "Unexpected end of expression";
  }

  skipCount = 0;

  if (!TokenEquals (tokens[offset].tokenStart, tokens[offset].tokenLen, "("))
  {
    return SetLastError ("'(' expected, '%s' found",
      csExpressionToken::Extractor (tokens[offset]).Get ());
  }

  size_t pos = offset + 1;
  const size_t end = offset + num;

  size_t braceLevel = 1;
  while (pos < end)
  {
    if (tokens[pos].type == tokenBraces)
    {
      if (TokenEquals (tokens[pos].tokenStart, tokens[pos].tokenLen, "("))
      {
	braceLevel++;
      }
      else if (TokenEquals (tokens[pos].tokenStart, tokens[pos].tokenLen, ")"))
      {
	braceLevel--;
      }
      else
      {
	return SetLastError ("'(' or ')' expected, '%s' found",
	  csExpressionToken::Extractor (tokens[pos]).Get ());
      }
    }
    if (braceLevel == 0) break;
    skipCount++;
    pos++;
  }
  if (braceLevel != 0)
    return "Unbalanced number of braces";
  return 0;
}
