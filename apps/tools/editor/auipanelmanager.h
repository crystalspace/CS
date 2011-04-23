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

#ifndef __PANELMANAGER_H__
#define __PANELMANAGER_H__

#include <csutil/refarr.h>

#include "wx/aui/aui.h"

#include "ieditor/panelmanager.h"

namespace CS {
namespace EditorApp {

class AUIPanelManager : public scfImplementation1<AUIPanelManager,iPanelManager>
{
public:
  AUIPanelManager (iObjectRegistry* obj_reg);
  virtual ~AUIPanelManager ();

  virtual void Uninitialize ();
  
  virtual void SetManagedWindow (wxWindow* managedWindow);

  virtual wxWindow* GetManagedWindow ();

  virtual void AddPanel (iPanel* panel);

  virtual void RemovePanel (iPanel* panel);

  virtual void SetPanelVisible (iPanel* panel, bool visible);

private:
  iObjectRegistry* object_reg;
  
  wxAuiManager mgr;
  csRefArray<iPanel> panels;
  
};

} // namespace EditorApp
} // namespace CS

#endif
