/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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
#include "csutil/prfxcfg.h"
#include "csutil/util.h"

csPrefixConfig::csPrefixConfig(const char *fName, iVFS *vfs, const char *prf,
  const char *AliasName)
{
  size_t len = strlen(prf);
  Prefix = new char[len + 2];
  memcpy(Prefix, prf, len);
  Prefix[len] = '.';
  Prefix[len + 1] = 0;
  PrefixLength = len + 1;

  if (AliasName) Alias = csStrNew (AliasName);
  else Alias = 0;

  Load(fName, vfs);
}

csPrefixConfig::~csPrefixConfig()
{
  delete[] Prefix;
  delete[] Alias;
}

bool csPrefixConfig::LoadNow(const char *Filename, iVFS *vfs, bool overwrite)
{
  csConfigFile cfg;

  // load the raw configuration
  if (!cfg.Load(Filename, vfs))
    return false;

  // copy all options for the current user
  csRef<iConfigIterator> it (cfg.Enumerate(Prefix));
  while (it->Next())
    if (overwrite || !KeyExists(it->GetKey(true)))
      SetStr(it->GetKey(true), it->GetStr());

  // copy the EOF comment
  SetEOFComment(cfg.GetEOFComment());

  return true;
}

bool csPrefixConfig::SaveNow(const char *Filename, iVFS *vfs) const
{
  csRef<iConfigIterator> it;
  csConfigFile cfg;

  // first load the existing config file to preserve the user
  // configuration of other applications
  cfg.Load(Filename, vfs);

  // Delete all keys for this application. In fact we only have to delete
  // those keys that exist in 'cfg' but not in this object, but I don't
  // know a fast way to test for this. Anyway, with this approach the
  // keys in the prefix config file are always grouped by application,
  // which isn't too bad after all.
  it = cfg.Enumerate(Prefix);
  while (it->Next())
    cfg.DeleteKey(it->GetKey());

  // copy all options for the current user
  it = ((iConfigFile*)this)->Enumerate();
  while (it->Next())
  {
    char tmp[1024];
    memcpy(tmp, Prefix, PrefixLength);
    strcpy(tmp + PrefixLength, it->GetKey());
    cfg.SetStr(tmp, it->GetStr());
  }

  // copy EOF comment
  cfg.SetEOFComment(GetEOFComment());

  // write config file
  return cfg.Save();
}

const char *csPrefixConfig::GetFileName () const
{
  return Alias ? Alias : csConfigFile::GetFileName ();
}
