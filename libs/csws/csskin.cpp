/*
    Crystal Space Windowing System: Base skin support
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#include <string.h>

#include "cssysdef.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"
#include "csws/csskin.h"

//--//--//--//--//--//--//--//--//--//--//--// The skin repository class -//--//

csSkin::~csSkin ()
{
  DeleteAll ();
}

bool csSkin::FreeItem (csSome Item)
{
  delete (csSkinSlice *)Item;
  return true;
}

int csSkin::CompareKey (csSome Item, csConstSome Key, int) const
{
  return strcmp (((csSkinSlice *)Item)->GetName (), (char *)Key);
}

int csSkin::Compare (csSome Item1, csSome Item2, int) const
{
  return strcmp (((csSkinSlice *)Item1)->GetName (), ((csSkinSlice *)Item2)->GetName ());
}

void csSkin::Apply (csComponent *iComp)
{
  iComp->SendBroadcast (cscmdSkinChanged, this);
}

void csSkin::Initialize (csApp *iApp)
{
  for (int i = 0; i < count; i++)
    Get (i)->Initialize (iApp, this);
}

void csSkin::Deinitialize ()
{
  for (int i = 0; i < count; i++)
    Get (i)->Deinitialize ();
}

//--//--//--//--//--//--//--//--//- Basic functionality for skin slices --//--//

void csSkinSlice::Apply (csComponent &This)
{
  if (This.skinslice)
    This.skinslice->Reset (This);
  This.skinslice = this;
  if (This.GetState (CSS_VISIBLE))
    This.SetRect (This.bound);
}

void csSkinSlice::Reset (csComponent &This)
{
  This.ResetPalette ();
  if (This.GetState (CSS_VISIBLE))
    This.SetRect (This.bound);
}
