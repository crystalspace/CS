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
static char last_offender[255];
static int parser_line;

char* csGetLastOffender () { return last_offender; }

// XXX: Here we have a problem: the parser_line variable is static and
// therefor not shared across plugins, because the real fix (moving everything
// into a class) takes time, we'll just always return -1 now...
//int   csGetParserLine   () { return parser_line;   }
int csGetParserLine() { return -1; }

void  csResetParserLine () { parser_line = 1;      }

long csGetObject (char **buf, csTokenVector * tokens, char **name, char **data)
{
  csSkipCharacters (buf, kWhiteSpace);

  // skip comment lines.
  while (**buf == ';')
  {
    *buf = strchr (*buf, '\n');
    parser_line++;
    csSkipCharacters (buf, kWhiteSpace);
  }

  if (!**buf)                   // at end of file
    return CS_PARSERR_EOF;

  // find the token.
  // for now just use brute force.  Improve later.
  int i=0;
  for (i=0; i < tokens->Length ()-1; i++)
  {
    if (!strncasecmp (tokens->Get (i)->token, *buf, strlen (tokens->Get (i)->token)))
    {
      break;
    }
  }

  if (i+1 == tokens->Length ())
  {
    char *p = strchr (*buf, '\n');
    if (p) *p = 0;
    strcpy (last_offender, *buf);
    return CS_PARSERR_TOKENNOTFOUND;
  }
  // skip the token
  *buf += strlen (tokens->Get (i)->token);
  csSkipCharacters (buf, kWhiteSpace);

  // get optional name
  *name = csGetSubText (buf, '\'', '\''); // single quotes
  csSkipCharacters (buf, kWhiteSpace);

  // get optional data
  if (**buf == '=') // An assignment rather than a command/object.
    *data = csGetAssignmentText (buf);
  else
    *data = csGetSubText (buf, '(', ')');

  return tokens->Get (i)->id;
}

long csGetCommand (char **buf, csTokenVector * tokens, char **params)
{
  char *name;
  return csGetObject (buf, tokens, &name, params);
}

void csSkipCharacters (char **buf, const char *toSkip)
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

char *csGetSubText (char **buf, char open, char close)
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

char *csGetAssignmentText (char **buf)
{
  ++*buf;                       // skip the '='
  csSkipCharacters (buf, kWhiteSpace);
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
