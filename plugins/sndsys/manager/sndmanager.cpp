/*
    Copyright (C) 2005 by Jorrit Tyberghein

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
#include "csqint.h"
#include "csqsqrt.h"
#include "csver.h"

#include <string.h>
#include <ctype.h>

#include "sndmanager.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_FACTORY (csSndSysManager)

bool csSndSysManager::Initialize (iObjectRegistry *object_reg)
{
  csSndSysManager::object_reg = object_reg;
  return true;
}

iSndSysWrapper* csSndSysManager::CreateSound (const char* name)
{
  csSndSysWrapper* wrap = new csSndSysWrapper (name);
  sounds.Push (wrap);
  wrap->DecRef ();
  return wrap;
}

void csSndSysManager::RemoveSound (iSndSysWrapper* snd)
{
  csSndSysWrapper* wrap = (csSndSysWrapper*)snd;
  sounds.Delete (wrap);
}

void csSndSysManager::RemoveSound (size_t idx)
{
  sounds.DeleteIndex (idx);
}

void csSndSysManager::RemoveSounds ()
{
  sounds.DeleteAll ();
}

iSndSysWrapper* csSndSysManager::GetSound (size_t idx)
{
  return sounds[idx];
}

iSndSysWrapper* csSndSysManager::FindSoundByName (const char* name)
{
  // @@@ Optimize with hash.
  size_t i;
  for (i = 0 ; i < sounds.Length () ; i++)
    if (!strcmp (name, sounds[i]->GetName ()))
      return sounds[i];
  return 0;
}

//---------------------------------------------------------------------------

