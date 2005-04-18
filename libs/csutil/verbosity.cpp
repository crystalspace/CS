/*
    Copyright (C) 2005 by Eric Sunshine <sunshine@sunshineco.com>

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
#include "csutil/verbosity.h"
#include "csutil/stringarray.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include <ctype.h>

static bool word_p(char c, size_t n) { return c == '_' || isalnum(c); }
static bool name_p(char c, size_t n)
{ return (n == 0 && (c == '-' || c == '+')) || c == '.' || word_p(c,n); }

csVerbosityParser csParseVerbosity (int argc, const char* const argv[])
{
  csVerbosityParser v;
  for (int i = 1; i < argc; ++i)
  {
    char const* s = argv[i];
    if (*s == '-')
    {
      do { ++s; } while (*s == '-');
      const char* eq = strchr(s, '=');
      if (csStrNCaseCmp(s, "verbose", eq ? (eq - s) : strlen (s)) == 0)
	v.Parse(eq != 0 ? eq + 1 : "");
    } // Allow multiple `--verbose' options, so do not break out of loop.
  }
  return v;
}

bool csCheckVerbosity (int argc, const char* const argv[],
		       const char* flags, bool fuzzy)
{
  csVerbosityParser v(csParseVerbosity(argc, argv));
  return v.Enabled(flags, fuzzy);
}

csVerbosityParser::csVerbosityParser (const char* s)
{
  Parse(s);
}

// Emit a parse-related error, highlighting the problem location.
bool csVerbosityParser::Error(char const* msg, char const* s, size_t pos)
{
  csPrintfErr("ERROR: Verbosity parser: %s: %-*.*s<<ERROR>>%s\n",
	      msg, pos, pos, s, s + pos);
  return false;
}

bool csVerbosityParser::Split(char const* s, char delim, SplitPredicate pred,
			      bool empty_okay, csStringArray& tokens)
{
  bool ok = true;
  tokens.Empty();
  if (s == 0) s = "";
  char const* t = s;
  if (*s != '\0')
  {
    while (ok)
    {
      Str token;
      char const* const s0 = s;
      while (pred(*s, s - s0))
	token << *s++;
      if (token.IsEmpty())
	ok = Error("malformed input", t, s - t);
      else
      {
	tokens.Push(token);
	if (*s == delim && s[1] == '\0')
	  ok = Error("orphaned delimiter", t, s - t);
	else if (*s == delim)
	  s++;
	else if (*s == '\0')
	  break;
	else // (*s != '\0')
	  ok = Error("unexpected token", t, s - t);
      }
    }
  }
  if (ok && !empty_okay && tokens.IsEmpty())
    ok = Error("missing input", t, s - t);
  return ok;
}

csVerbosityParser::Str csVerbosityParser::Join(
  csStringArray const& tokens, Str delim)
{
  Str name;
  for (size_t i = 0, n = tokens.Length(); i < n; i++)
  {
    if (i != 0) name << delim;
    name << tokens[i];
  }
  return name;
}

// Parse enable/disable flag; default is `enable': ([+-]?)
bool csVerbosityParser::ParseToggle(char const*& s)
{
  bool enable = true;
  char const c = *s;
  if (c == '+' || c == '-')
  {
    enable = (c == '+');
    s++;
  }
  return enable;
}

// Parse verbosity class: (symbol(\.symbol)*) where symbol => ([[:alnum:]_]+)
bool csVerbosityParser::ParseFlag(char const* s, csStringArray& tokens,
				  bool empty_okay)
{
  return Split(s, '.', word_p, empty_okay, tokens);
}

// Parse classes: (class(,class)*)
void csVerbosityParser::Parse(char const* s)
{
  if (s != 0)
  {
    csStringArray names;
    if (Split(s, ',', name_p, true, names))
    {
      for (size_t i = 0, n = names.Length(); i < n; i++)
      {
	csStringArray tokens;
	char const* flag = names[i];
	bool const enable = ParseToggle(flag);
	if (ParseFlag(flag, tokens, false))
	{
	  if (flags.IsEmpty())
	    flags.Register("", !enable); // Fallback case.
	  Str name(Join(tokens,"."));
	  flags.Register(name, enable);
	}
      }
      if (flags.IsEmpty()) // Bare `--verbose'; enable full verbosity.
	flags.Register("", true);
    }
  }
}

bool csVerbosityParser::TestFlag(Str name, bool& enable) const
{
  csStringID const b = flags.Request(name.GetDataSafe());
  bool const ok = (b != csInvalidStringID);
  if (ok)
    enable = (bool)b;
  return ok;
}

bool csVerbosityParser::Enabled(char const* flag, bool fuzzy) const
{
  bool enable = false;
  if (!fuzzy)
    TestFlag(flag, enable);
  else
  {
    csStringArray tokens;
    if (ParseFlag(flag, tokens, true))
    {
      for (size_t i = 0, n = tokens.Length(); i <= n; i++)
      {
	if (TestFlag(Join(tokens,"."), enable))
	  break;
	else if (i < n)
	  tokens.Truncate(n - i - 1);
      }
    }
  }
  return enable;
}

SCF_IMPLEMENT_IBASE(csVerbosityManager)
  SCF_IMPLEMENTS_INTERFACE(iVerbosityManager)
SCF_IMPLEMENT_IBASE_END
