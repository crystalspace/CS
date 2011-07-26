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

#ifndef __VIEWMANAGER_H__
#define __VIEWMANAGER_H__

#include <csutil/refarr.h>
#include <csutil/hash.h>

#include <wx/aui/aui.h>
#include <wx/menu.h>

#include <map>

#include "ieditor/view.h"

#include "ieditor/menubar.h"

namespace CS {
namespace EditorApp {

class AUIViewManager : public scfImplementation1<AUIViewManager,iViewManager>, public wxEvtHandler
{
public:
  AUIViewManager (iObjectRegistry* obj_reg, wxWindow* parent);
  virtual ~AUIViewManager ();
  
  virtual void Initialize ();
  virtual void Uninitialize ();

  virtual void AddView (iView* view);
  virtual void RemoveView (iView* view);

  virtual void SetViewVisible (iView* view, bool visible);

public:
   void OnToggle (wxCommandEvent& event);
   void OnClose (wxAuiManagerEvent& event);
   
private:
  iObjectRegistry* object_reg;
  
  wxAuiManager mgr;
  
  csRefArray<iView> views;
  csRefArray<iMenuCheckItem> items;
  
  csRef<iMenu> perspectivesMenu;
  csRef<iMenu> viewsMenu;
  
  csRef<iMenuItem> createPerspective;
  csRefArray<iMenuItem> perspectiveItems;
  csRefArray<iMenuItem> separatorItems;
  
  std::map<size_t, wxString> m_perspectives;
  
  
  csRef<iMenuItem> testOperator;

private:  
  //I WANT boost::bind DAMN IT!
  struct ViewListener : public scfImplementation1<ViewListener,iMenuItemEventListener>
   {
     AUIViewManager* mgr;
     ViewListener(AUIViewManager* mgr) : scfImplementationType (this), mgr(mgr) {}
     virtual void OnClick (iMenuItem* item);
   };
   friend struct ViewListener;
   csRef<ViewListener> viewListener;
   
   struct OnCreatePerspective : public scfImplementation1<OnCreatePerspective,iMenuItemEventListener>
   {
     AUIViewManager* mgr;
     OnCreatePerspective(AUIViewManager* mgr) : scfImplementationType (this), mgr(mgr) {}
     virtual void OnClick (iMenuItem* item);
   };
   friend struct OnCreatePerspective;
   csRef<OnCreatePerspective> onCreatePerspective;
   
   struct OnRestorePerspective : public scfImplementation1<OnRestorePerspective,iMenuItemEventListener>
   {
     AUIViewManager* mgr;
     OnRestorePerspective(AUIViewManager* mgr) : scfImplementationType (this), mgr(mgr) {}
     virtual void OnClick (iMenuItem* item);
   };
   friend struct OnRestorePerspective;
   csRef<OnRestorePerspective> onRestorePerspective;
};

} // namespace EditorApp
} // namespace CS

#endif
