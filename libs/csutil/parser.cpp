/*
    Copyright (C) 1998 by Jorrit Tyberghein

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
#include "sysdef.h"
#include "csutil/parser.h"

const char *kWhiteSpace = " \t\n\r";
static char last_offender[255];
int parser_line;

char* csGetLastOffender ()
{
  return last_offender;
}

long csGetObject (char **buf, csTokenDesc * tokens, char **name, char **data)
{
  SkipCharacters (buf, kWhiteSpace);

  // skip comment lines.
  while (**buf == ';')
  {
    *buf = strchr (*buf, '\n');
    parser_line++;
    SkipCharacters (buf, kWhiteSpace);
  }

  if (!**buf)                   // at end of file
    return PARSERR_EOF;

  // find the token.
  // for now just use brute force.  Improve later.
  while (tokens->id != 0)
  {
    if (!strncasecmp (tokens->token, *buf, strlen (tokens->token)))
    {
      // here we could be matching PART of a string
      // we could detect this here or the token list
      // could just have the longer tokens first in the list
      break;
    }
    ++tokens;
  }
  if (tokens->id == 0)
  {
    char *p = strchr (*buf, '\n');
    if (p) *p = 0;
    strcpy (last_offender, *buf);
    return PARSERR_TOKENNOTFOUND;
  }
  // skip the token
  *buf += strlen (tokens->token);
  SkipCharacters (buf, kWhiteSpace);

  // get optional name
  *name = GetSubText (buf, 0x27, 0x27); // single quotes
  SkipCharacters (buf, kWhiteSpace);

  // get optional data
  if (**buf == '=')             // hmm, this is an assignment and not a command/object
    *data = GetAssignmentText (buf);
  else
    *data = GetSubText (buf, '(', ')');

  return tokens->id;
}

long csGetCommand (char **buf, csTokenDesc * tokens, char **params)
{
  char *name;
  return csGetObject (buf, tokens, &name, params);
}

void SkipCharacters (char **buf, const char *toSkip)
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

char *GetSubText (char **buf, char open, char close)
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

char *GetAssignmentText (char **buf)
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
