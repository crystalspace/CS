/*
    Copyright (C) 2006 by Seth Yastrov

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
#include "cstool/saverref.h"

//---------------------------------------------------------------------------


csPluginReference::csPluginReference (const char* name, const char* id) :
  scfImplementationType (this),
  name (name), id (id)
{
}

csPluginReference::~csPluginReference ()
{
}

const char* csPluginReference::GetName () const
{
  return name;
}

const char* csPluginReference::GetClassID () const
{
  return id;
}

//---------------------------------------------------------------------------


csLibraryReference::csLibraryReference (const char* file, const char* path,
    bool checkDupes) :
  scfImplementationType (this),
  file (file), path (path), checkDupes (checkDupes)
{
}

csLibraryReference::~csLibraryReference ()
{
}

const char* csLibraryReference::GetFile () const
{
  return file;
}

const char* csLibraryReference::GetPath () const
{
  return path;
}

bool csLibraryReference::GetCheckDupes () const
{
  return checkDupes;
}

//---------------------------------------------------------------------------

csAddonReference::csAddonReference (const char* plugin, const char* paramsfile,
    iBase* addonobj)
  : scfImplementationType (this),
  plugin (plugin), paramsfile (paramsfile), addonobj (addonobj)
{
}

csAddonReference::~csAddonReference ()
{
}

const char* csAddonReference::GetPlugin () const
{
  return plugin;
}

const char* csAddonReference::GetParamsFile () const
{
  return paramsfile;
}

iBase* csAddonReference::GetAddonObject () const
{
  return addonobj;
}

