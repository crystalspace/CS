/*
    Copyright (C) 2011 by Jelle Hellemans

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

#ifndef __DAMN_DAMNPANEL_H__
#define __DAMN_DAMNPANEL_H__

#include "ieditor/objectlist.h"
#include "ieditor/panelmanager.h"
#include "ieditor/actionmanager.h"
#include "ieditor/editor.h"

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>
#include <csutil/hash.h>
#include <iengine/engine.h>
#include <iengine/mesh.h>

#include <wx/event.h>
#include <wx/panel.h>
#include <wx/treectrl.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>

#include <map>


using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
class DAMNPanel : public scfImplementation3<DAMNPanel,iPanel,iComponent, iResourceListener>,
  public wxPanel
{
public:
  DAMNPanel (iBase* parent);
  virtual ~DAMNPanel ();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

  // iPanel
  virtual wxWindow* GetWindow ();
  virtual const wxChar* GetCaption () const;
  virtual PanelDockPosition GetDefaultDockPosition () const;
  
  // iResourceListener
  virtual void OnLoaded (iLoadingResource* resource);
  typedef std::map<csRef<iLoadingResource>, std::string> SearchResults;
  SearchResults searchResults;
  size_t current;
  csRef<iProgressMeter> meter;

private:
  iObjectRegistry* object_reg;
  
  wxSizer* sizer;
  wxListCtrl* listCtrl;
  wxSearchCtrl* srchCtrl;
  
  wxImageList* imageListNormal;

  csRef<iEditor> editor;
  csRef<iPanelManager> panelManager;
  csRef<iActionManager> actionManager;
  
  csRef<iPluginManager> plugmgr;
  csRef<CS::Network::HTTP::iHTTPConnectionFactory> http;
  csRef<iResourceManager> damn;

  csRef<iObjectList> objects;

  void OnSize (wxSizeEvent& event);
  
  void OnSearchButton (wxCommandEvent& event);
  void OnCancelButton (wxCommandEvent& event);
  
  DECLARE_EVENT_TABLE()
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
