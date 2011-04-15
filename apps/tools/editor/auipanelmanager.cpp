/*
    Copyright (C) 2007 by Seth Yastrov

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
#include "csutil/scf.h"

#include <csutil/objreg.h>

#include "auipanelmanager.h"

namespace CSE
{

AUIPanelManager::AUIPanelManager (iObjectRegistry* obj_reg)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iPanelManager");
}

AUIPanelManager::~AUIPanelManager ()
{
  object_reg->Unregister (this, "iPanelManager");
}

void AUIPanelManager::Uninitialize ()
{
  mgr.UnInit ();
}

void AUIPanelManager::SetManagedWindow (wxWindow* managedWindow)
{
  mgr.SetManagedWindow (managedWindow);
}

wxWindow* AUIPanelManager::GetManagedWindow ()
{
  return mgr.GetManagedWindow ();
}

void AUIPanelManager::AddPanel (iPanel* panel)
{
  wxAuiPaneInfo info;
  info.Direction(panel->GetDefaultDockPosition());
  info.Caption(panel->GetCaption ());
  
  mgr.AddPane (panel->GetWindow (), info);

  mgr.Update ();

  panels.Push (panel);
}

void AUIPanelManager::RemovePanel (iPanel* panel)
{
  mgr.DetachPane (panel->GetWindow ());
  mgr.Update ();

  panels.Delete (panel);
}

void AUIPanelManager::SetPanelVisible (iPanel* panel, bool visible)
{
  wxAuiPaneInfo info = mgr.GetPane (panel->GetWindow ());

  if (info.IsOk ())
  {
    if (visible)
      info.Show ();
    else
      info.Hide ();
  }
  
  mgr.Update ();
}

}
