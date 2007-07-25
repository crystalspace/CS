/*
    Copyright (C) 2004 by Frank Richter

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
#include "regex_wrapper.h"

static int exec_flags(int flags)
{
  int execflags = 0;
  if (flags & csrxNotBOL) execflags |= REG_NOTBOL;
  if (flags & csrxNotEOL) execflags |= REG_NOTEOL;
  return execflags;
}

bool csRegExpMatcher::Compile (int flags, bool nosub)
{
  int needFlags = 0;
  if (extendedRE) needFlags |= REG_EXTENDED;
  if (nosub) needFlags |= REG_NOSUB;
  if (flags & csrxIgnoreCase) needFlags |= REG_ICASE;
  if (flags & csrxNewLine) needFlags |= REG_NEWLINE;

  if ((regex == 0) || 
    ((needFlags & ~REG_NOSUB) != (compiledFlags & ~REG_NOSUB)) ||
    ((needFlags & REG_NOSUB) && !(compiledFlags & REG_NOSUB)))
  {
    if (regex != 0)
      regfree ((regex_t*)regex);
    else
      regex = new regex_t;

    int res = regcomp ((regex_t*)regex, pattern, needFlags);

    switch (res)
    {
      case 0:		  compileError = csrxNoError;        break;
      case REG_BADBR:	  compileError = csrxBadBraces;      break;
      case REG_BADPAT:	  compileError = csrxBadPattern;     break;
      case REG_BADRPT:	  compileError = csrxBadRepetition;  break;
      case REG_ECOLLATE:  compileError = csrxErrCollate;     break;
      case REG_ECTYPE:	  compileError = csrxErrCharType;    break;
      case REG_EESCAPE:	  compileError = csrxErrEscape;	     break;
      case REG_ESUBREG:	  compileError = csrxErrSubReg;      break;
      case REG_EBRACK:	  compileError = csrxErrBrackets;    break;
      case REG_EPAREN:	  compileError = csrxErrParentheses; break;
      case REG_EBRACE:	  compileError = csrxErrBraces;      break;
      case REG_ERANGE:	  compileError = csrxErrRange;       break;
      case REG_ESPACE:	  compileError = csrxErrSpace;       break;
      default:            compileError = csrxErrUnknown;     break;
    }
  }
  return (compileError == csrxNoError);
}

csRegExpMatcher::csRegExpMatcher (const char* pattern, bool extendedRE) : 
  regex(0)
{
  csRegExpMatcher::pattern = csStrNew (pattern);
  csRegExpMatcher::extendedRE = extendedRE;
}

csRegExpMatcher::csRegExpMatcher (const csRegExpMatcher& other) :
  regex (0)
{
  pattern = csStrNew (other.pattern);
  extendedRE = other.extendedRE;
}

csRegExpMatcher::~csRegExpMatcher ()
{
  if (regex)
  {
    regfree ((regex_t*)regex);
    delete (regex_t*)regex;
  }
  delete[] pattern;
}

csRegExpMatcher& csRegExpMatcher::operator= (const csRegExpMatcher &other)
{
  if (regex)
  {
    regfree ((regex_t*)regex);
    delete (regex_t*)regex;
    regex = 0;
  }
  delete[] pattern;
  
  pattern = csStrNew (other.pattern);
  extendedRE = other.extendedRE;

  return *this;
}

csRegExpMatchError csRegExpMatcher::Match (const char* string, int flags)
{
  if (!Compile (flags, true))
    return compileError;
  return (regexec ((regex_t*)regex, string, 0, 0, exec_flags(flags)) == 0) ?
    csrxNoError : csrxNoMatch;
}

csRegExpMatchError csRegExpMatcher::Match (const char* string, 
					   csArray<csRegExpMatch>& matches, 
					   int flags)
{
  matches.Empty();
  if (!Compile (flags, false))
    return compileError;

  CS_ALLOC_STACK_ARRAY(regmatch_t, re_matches, ((regex_t*)regex)->re_nsub);
  if (regexec ((regex_t*)regex, string, ((regex_t*)regex)->re_nsub,
      re_matches, exec_flags(flags)) != 0)
    return csrxNoMatch;

  for (size_t i = 0; i < ((regex_t*)regex)->re_nsub; i++)
  {
    csRegExpMatch match;
    match.startOffset = re_matches[i].rm_so;
    match.endOffset = re_matches[i].rm_eo;
    matches.Push (match);
  }
  return csrxNoError;
}

