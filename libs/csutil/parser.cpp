/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "cssysdef.h"
#include "csutil/parser.h"

// A string containing white spaces (' ', '\t', '\n', '\r')
static const char *kWhiteSpace = " \t\n\r";

csParser::csParser (bool allow_unknown_tokens)
{
  ResetParserLine ();
  last_offender[0] = 0;
  csParser::allow_unknown_tokens = allow_unknown_tokens;
}

void csParser::ResetParserLine ()
{
  parser_line = 1;
}

char* csParser::GetLastOffender () 
{ 
  return last_offender; 
}

int csParser::GetParserLine() 
{ 
  return parser_line; 
}

long csParser::GetObject (char **buf, csTokenVector * tokens, char **name,
  char **data)
{
  if (name) *name = NULL;
  if (data) *data = NULL;

  SkipCharacters (buf, kWhiteSpace);

  // skip comment lines.
  while (**buf == ';')
  {
    *buf = strchr (*buf, '\n');
    parser_line++;
    SkipCharacters (buf, kWhiteSpace);
  }

  if (!**buf)                   // at end of file
    return CS_PARSERR_EOF;

  // find the token.
  // for now just use brute force.  Improve later.
  int i;
  int toklen = SkipToken (*buf);
  bool found_token = false;
  for (i=0 ; i < tokens->Length ()-1 ; i++)
  {
    csTokenDesc* d = tokens->Get (i);
    if (d->len == toklen)
      if (!strncasecmp (d->token, *buf, toklen))
      {
        found_token = true;
        break;
      }
    else if (d->len > toklen)
    {
      // We can stop the search here because the token table is sorted
      // and we know there will be no more tokens with the same length
      // as the one we are trying to parse.
      break;
    }
  }

  if (found_token)
  {
    // Skip the token.
    *buf += toklen;
  }
  else
  {
    if (allow_unknown_tokens)
    {
      char old_c = (*buf)[toklen];
      (*buf)[toklen] = 0;
      strcpy (unknown_token, (*buf));
      (*buf)[toklen] = old_c;
      (*buf) += toklen;
    }
    else
    {
      char *p = strchr (*buf, '\n');
      if (p) *p = 0;
      strcpy (last_offender, *buf);
      return CS_PARSERR_TOKENNOTFOUND;
    }
  }

  SkipCharacters (buf, kWhiteSpace);

  // Get optional name.
  if (name)
    *name = GetSubText (buf, '\'', '\''); // single quotes
  SkipCharacters (buf, kWhiteSpace);

  // get optional data
  if (data)
  {
    if (**buf == '=') // An assignment rather than a command/object.
      *data = GetAssignmentText (buf);
    else
      *data = GetSubText (buf, '(', ')');
  }

  return found_token ? tokens->Get (i)->id : CS_PARSERR_TOKENNOTFOUND;
}

long csParser::GetCommand (char **buf, csTokenVector * tokens, char **params)
{
  return GetObject (buf, tokens, NULL, params);
}

int csParser::SkipToken (char *buf)
{
  char ch;
  int l = 0;
  while ((ch = *buf) != 0)
  {
    if (!((ch >= 'a' && ch <= 'z') ||
          (ch >= 'A' && ch <= 'Z') ||
          (ch >= '0' && ch <= '9') ||
	  ch == '_' || ch == '$'))
      return l;
    ++buf;                     // skip this character
    l++;
  }
  // we must be at the end of the buffer if we get here
  return l;
}

void csParser::SkipCharacters (char **buf, const char *toSkip)
{
  char ch;
  while ((ch = **buf) != 0)
  {
    if (ch == '\n') parser_line++;
    if (strchr (toSkip, ch) == NULL)
      return;
    ++*buf;                     // skip this character
  }
  // we must be at the end of the buffer if we get here
}

char *csParser::GetSubText (char **buf, char open, char close)
{
  if (**buf == 0 || **buf != open)
    return 0;
  ++*buf;                       // skip opening delimiter
  char *result = *buf;

  // now find the closing delimiter
  char theChar;
  long skip = 1;

  if (open == close)            // we cant nest identical delimiters
    open = 0;
  while ((theChar = **buf) != 0)
  {
    if (theChar == open)
      ++skip;
    if (theChar == close)
    {
      if (--skip == 0)
      {
        **buf = 0;              // null terminate the result string
        ++*buf;                 // move past the closing delimiter
        break;
      }
    }
    ++*buf;                     // try next character
  }
  return result;
}

char *csParser::GetAssignmentText (char **buf)
{
  ++*buf;                       // skip the '='
  SkipCharacters (buf, kWhiteSpace);
  char *result = *buf;

  // now, we need to find the next whitespace to end the data
  char theChar;

  while ((theChar = **buf) != 0)
  {
    if (theChar <= 0x20)        // space and control chars
      break;
    ++*buf;
  }
  **buf = 0;                    // null terminate it
  if (theChar)                  // skip the terminator
    ++*buf;
  return result;
}

