/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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

#include "cssysdef.h"
#include "csutil/regexp.h"
#include "csutil/util.h"

bool csRegExpMatcher::Compile (int needFlags)
{
  if ((regex == 0) || 
    ((needFlags & ~REG_NOSUB) != (compiledFlags & ~REG_NOSUB)) ||
    ((needFlags & REG_NOSUB) && !(compiledFlags & REG_NOSUB)))
  {
    if (regex != 0)
      regfree (regex);
    else
      regex = new regex_t;

    int res = regcomp (regex, pattern, 
      needFlags | (extendedRE ? REG_EXTENDED : 0));

    switch (res)
    {
      case 0:		  compileError = NoError;	  break;
      case REG_BADBR:	  compileError = BadBraces;	  break;
      case REG_BADPAT:	  compileError = BadPattern;	  break;
      case REG_BADRPT:	  compileError = BadRepetition;	  break;
      case REG_ECOLLATE:  compileError = ErrCollate;	  break;
      case REG_ECTYPE:	  compileError = ErrCharType;	  break;
      case REG_EESCAPE:	  compileError = ErrEscape;	  break;
      case REG_ESUBREG:	  compileError = ErrSubReg;	  break;
      case REG_EBRACK:	  compileError = ErrBrackets;	  break;
      case REG_EPAREN:	  compileError = ErrParentheses;  break;
      case REG_EBRACE:	  compileError = ErrBraces;	  break;
      case REG_ERANGE:	  compileError = ErrRange;	  break;
      case REG_ESPACE:	  compileError = ErrSpace;	  break;
    }
  }
  return (compileError == NoError);
}

csRegExpMatcher::csRegExpMatcher (const char* pattern, bool extendedRE) : 
  regex(0)
{
  csRegExpMatcher::pattern = csStrNew (pattern);
  csRegExpMatcher::extendedRE = extendedRE;
}

csRegExpMatcher::~csRegExpMatcher ()
{
  if (regex)
  {
    regfree (regex);
    delete regex;
  }
  delete[] pattern;
}

csRegExpMatchError csRegExpMatcher::Match (const char* string, int flags)
{
  int compileFlags = REG_NOSUB;
  if (flags & IgnoreCase) compileFlags |= REG_ICASE;
  if (flags & NewLine) compileFlags |= REG_NEWLINE;

  if (!Compile (compileFlags)) return compileError;

  int execflags = 0;
  if (flags & NotBOL) execflags |= REG_NOTBOL;
  if (flags & NotEOL) execflags |= REG_NOTEOL;

  return (regexec (regex, string, 0, 0, execflags) == 0) ? NoError : NoMatch;
}

csRegExpMatchError csRegExpMatcher::Match (const char* string, 
					   csArray<csRegExpMatch>& matches, 
					   int flags)
{
  int compileFlags = 0;
  if (flags & IgnoreCase) compileFlags |= REG_ICASE;
  if (flags & NewLine) compileFlags |= REG_NEWLINE;

  if (!Compile (compileFlags)) return compileError;

  int execflags = 0;
  if (flags & NotBOL) execflags |= REG_NOTBOL;
  if (flags & NotEOL) execflags |= REG_NOTEOL;

  CS_ALLOC_STACK_ARRAY(regmatch_t, re_matches, regex->re_nsub);
  if (regexec (regex, string, regex->re_nsub, re_matches, execflags) != 0)
    return NoMatch;

  for (size_t i = 0; i < regex->re_nsub; i++)
  {
    csRegExpMatch match;
    match.startOffset = re_matches[i].rm_so;
    match.endOffset = re_matches[i].rm_eo;
    matches.Push (match);
  }

  return NoError;
}

