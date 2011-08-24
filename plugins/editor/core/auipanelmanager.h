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
#include <csutil/hash.h>

#include <wx/aui/aui.h>
#include <wx/menu.h>

#include <map>

#include "ieditor/panelmanager.h"

#include "ieditor/menubar.h"

namespace CS {
namespace EditorApp {

class AUIPanelManager : public scfImplementation1<AUIPanelManager,iPanelManager>, public wxEvtHandler
{
public:
  AUIPanelManager (iObjectRegistry* obj_reg, wxWindow* parent);
  virtual ~AUIPanelManager ();
  
  virtual void Initialize ();
  virtual void Uninitialize ();

  virtual void AddPanel (iPanel* panel);
  virtual void RemovePanel (iPanel* panel);

  virtual void SetPanelVisible (iPanel* panel, bool visible);

public:
   void OnToggle (wxCommandEvent& event);
   void OnClose (wxAuiManagerEvent& event);
   
private:
  iObjectRegistry* object_reg;
  
  wxAuiManager mgr;
  
  csRefArray<iPanel> panels;
  csRefArray<iMenuCheckItem> items;
  
  csRef<iMenu> perspectivesMenu;
  csRef<iMenu> panelsMenu;
  
  csRef<iMenuItem> createPerspective;
  csRefArray<iMenuItem> perspectiveItems;
  csRefArray<iMenuItem> separatorItems;
  
  std::map<size_t, wxString> m_perspectives;

private:  
  //I WANT boost::bind DAMN IT!
  struct PanelListener : public scfImplementation1<PanelListener,iMenuItemEventListener>
   {
     AUIPanelManager* mgr;
     PanelListener(AUIPanelManager* mgr) : scfImplementationType (this), mgr(mgr) {}
     virtual void OnClick (iMenuItem* item);
   };
   friend struct PanelListener;
   csRef<PanelListener> panelListener;
   
   struct OnCreatePerspective : public scfImplementation1<OnCreatePerspective,iMenuItemEventListener>
   {
     AUIPanelManager* mgr;
     OnCreatePerspective(AUIPanelManager* mgr) : scfImplementationType (this), mgr(mgr) {}
     virtual void OnClick (iMenuItem* item);
   };
   friend struct OnCreatePerspective;
   csRef<OnCreatePerspective> onCreatePerspective;
   
   struct OnRestorePerspective : public scfImplementation1<OnRestorePerspective,iMenuItemEventListener>
   {
     AUIPanelManager* mgr;
     OnRestorePerspective(AUIPanelManager* mgr) : scfImplementationType (this), mgr(mgr) {}
     virtual void OnClick (iMenuItem* item);
   };
   friend struct OnRestorePerspective;
   csRef<OnRestorePerspective> onRestorePerspective;
};

} // namespace EditorApp
} // namespace CS

#endif
