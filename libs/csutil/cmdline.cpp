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
#include "csutil/syspath.h"
#include "csutil/cmdline.h"

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
  SCF_CONSTRUCT_IBASE (0);
  Initialize (argc, argv);
}

csCommandLineParser::~csCommandLineParser()
{
  SCF_DESTRUCT_IBASE ();
}

void csCommandLineParser::Initialize (int argc, const char* const argv[])
{
  resDir  = csInstallationPathsHelper::GetResourceDir (argv[0]);
  appDir  = csInstallationPathsHelper::GetAppDir      (argv[0]);
  appPath = csInstallationPathsHelper::GetAppPath     (argv[0]);

  for (int i = 1; i < argc; i++)
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
      Names.Push (opt);
  }
}

void csCommandLineParser::Reset()
{
  Options.DeleteAll ();
  Names.DeleteAll ();
}

csCommandLineOption*
csCommandLineParser::FindOption (const char *name, size_t iIndex) const
{
  size_t idx;
  for (idx = 0 ; idx < Options.Length () ; idx++)
  {
    csCommandLineOption* cl = Options[idx];
    if (!strcmp (cl->Name, name))
      break;
  }

  if (idx < Options.Length ())
  {
    while (iIndex)
    {
      idx++;
      if (idx >= Options.Length ())
        return 0;
      if (!strcmp (Options.Get (idx)->Name,  name))
        iIndex--;
    }
    return Options.Get (idx);
  }
  return 0;
}

bool csCommandLineParser::ReplaceOption (
  const char *iName, const char *iValue, size_t iIndex)
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

bool csCommandLineParser::ReplaceName (const char *iValue, size_t iIndex)
{
  if (iIndex < Names.Length ())
  {
    Names.Put (iIndex, iValue);
    return true;
  }
  else
    return false;
}

const char *csCommandLineParser::GetOption(const char *iName, size_t iIndex) const
{
  csCommandLineOption *clo = FindOption (iName, iIndex);
  return clo ? (clo->Value ? clo->Value : "") : 0;
}

const char *csCommandLineParser::GetName (size_t iIndex) const
{
  if (iIndex < Names.Length ())
    return Names[iIndex];
  return 0;
}

void csCommandLineParser::AddOption (const char *iName, const char *iValue)
{
  Options.Push (new csCommandLineOption (csStrNew (iName), csStrNew (iValue)));
}

void csCommandLineParser::AddName (const char *iName)
{
  Names.Push ((char*)iName);
}

bool csCommandLineParser::GetBoolOption(const char *iName, bool defaultValue)
{
  bool result = defaultValue;

  CS_ALLOC_STACK_ARRAY (char, negName, strlen(iName)+3);
  strcpy (negName, "no");
  strcpy (negName+2, iName);
  size_t idx;
  for (idx = Options.Length (); idx > 0; idx--)
  {
    csCommandLineOption* cl = Options[idx - 1];
    if (!strcmp (cl->Name, iName))
    {
      result = true;
      break;
    }
    if (!strcmp (cl->Name, negName))
    {
      result = false;
      break;
    }
  }

  return result;
}

const char* csCommandLineParser::GetResourceDir ()
{
  return resDir;
}

const char* csCommandLineParser::GetAppDir ()
{
  return appDir;
}

const char* csCommandLineParser::GetAppPath ()
{
  return appPath;
}
