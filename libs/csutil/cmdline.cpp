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

#include "cssysdef.h"
#include "csutil/cmdline.h"

struct csCommandLineOption
{
  /// Option name
  char *Name;
  /// Option value
  char *Value;
  /// Name and Value should be already allocated
  csCommandLineOption (char *iName, char *iValue)
  {
    Name = iName;
    Value = iValue;
  }
  /// Destructor
  ~csCommandLineOption ()
  { delete [] Name; delete [] Value; }
};

CS_IMPLEMENT_TYPED_VECTOR_DELETE (
  csCommandLineParser::csCommandLineOptionVector_Helper, csCommandLineOption);

int csCommandLineParser::csCommandLineOptionVector::CompareKey
  (csSome Item, csConstSome Key, int /*Mode*/) const
{
  return strcmp (((csCommandLineOption *)Item)->Name, (const char*)Key);
}

SCF_IMPLEMENT_IBASE (csCommandLineParser)
  SCF_IMPLEMENTS_INTERFACE (iCommandLineParser)
SCF_IMPLEMENT_IBASE_END

csCommandLineParser::csCommandLineParser (iBase *Parent) : Names (16, 16)
{
  SCF_CONSTRUCT_IBASE (Parent);
}

csCommandLineParser::csCommandLineParser (int argc, const char* const argv[]) :
  Names (16, 16)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Initialize (argc, argv);
}

void csCommandLineParser::Initialize (int argc, const char* const argv[])
{
  int i;
  for (i = 1; i < argc; i++)
  {
    char *opt = (char *)argv [i];
    if (*opt == '-')
    {
      while (*opt == '-') opt++;
      char *arg = strchr (opt, '=');
      if (arg)
      {
        int n = arg - opt;
        char *newopt = new char [n + 1];
        memcpy (newopt, opt, n);
        (opt = newopt) [n] = 0;
        arg = csStrNew (arg + 1);
      }
      else
        opt = csStrNew (opt);
      Options.Push (new csCommandLineOption (opt, arg));
    }
    else
      Names.Push (csStrNew (opt));
  }
}

void csCommandLineParser::Reset()
{
  Options.DeleteAll ();
  Names.DeleteAll ();
}

csCommandLineOption*
csCommandLineParser::FindOption (const char *iName, int iIndex) const
{
  int idx = Options.FindKey (iName);
  if (idx >= 0)
  {
    while (iIndex)
    {
      idx++; 
      if (idx >= Options.Length ())
        return NULL;
      if (Options.CompareKey (Options.Get (idx), iName, 0) == 0)
        iIndex--;
    }
    return Options.Get (idx);
  }
  return NULL;
}

bool csCommandLineParser::ReplaceOption (
  const char *iName, const char *iValue, int iIndex)
{
  csCommandLineOption *clo = FindOption (iName, iIndex);
  if (clo)
  {
    delete [] clo->Value;
    clo->Value = csStrNew (iValue);
    return true;
  }
  else
    return false;
}

bool csCommandLineParser::ReplaceName (const char *iValue, int iIndex)
{
  if ((iIndex >= 0) && (iIndex < Names.Length ()))
  {
    Names.Replace (iIndex, csStrNew (iValue));
    return true;
  }
  else
    return false;
}

const char *csCommandLineParser::GetOption(const char *iName, int iIndex) const
{
  csCommandLineOption *clo = FindOption (iName, iIndex);
  return clo ? (clo->Value ? clo->Value : "") : NULL;
}

const char *csCommandLineParser::GetName (int iIndex) const
{
  if ((iIndex >= 0) && (iIndex < Names.Length ()))
    return (const char *)Names.Get (iIndex);
  return NULL;
}

void csCommandLineParser::AddOption (const char *iName, const char *iValue)
{
  Options.Push (new csCommandLineOption (csStrNew (iName), csStrNew (iValue)));
}

void csCommandLineParser::AddName (const char *iName)
{
  Names.Push (csStrNew (iName));
}


