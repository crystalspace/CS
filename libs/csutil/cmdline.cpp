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

csCommandLineParser::csCommandLineParser (iBase *Parent) 
  : scfImplementationType (this, Parent), Names (16, 16)
{
}

csCommandLineParser::csCommandLineParser (int argc, const char* const argv[]) 
  : scfImplementationType (this),  Names (16, 16)
{
  Initialize (argc, argv);
}

csCommandLineParser::~csCommandLineParser()
{
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
  for (idx = 0 ; idx < Options.GetSize () ; idx++)
  {
    csCommandLineOption* cl = Options[idx];
    if (!strcmp (cl->Name, name))
      break;
  }

  if (idx < Options.GetSize ())
  {
    while (iIndex)
    {
      idx++;
      if (idx >= Options.GetSize ())
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
  if (iIndex < Names.GetSize ())
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
  if (iIndex < Names.GetSize ())
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

  csString negName;
  negName << "no" << iName;
  size_t idx;
  for (idx = Options.GetSize (); idx > 0; idx--)
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

const char* csCommandLineParser::GetOptionName (size_t iIndex) const
{
  if (iIndex >= Options.GetSize()) return 0;
  return Options[iIndex]->Name;
}

const char* csCommandLineParser::GetOption (size_t iIndex) const
{
  if (iIndex >= Options.GetSize()) return 0;
  return Options[iIndex]->Value;
}
