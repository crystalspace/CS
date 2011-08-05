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

#ifndef __DAMN_DAMNVIEW_H__
#define __DAMN_DAMNVIEW_H__

#include "ieditor/space.h"
#include "ieditor/actionmanager.h"
#include "ieditor/editor.h"

#include <csutil/scf_implementation.h>
#include <iutil/comp.h>
#include <csutil/hash.h>
#include <iengine/engine.h>
#include <iengine/mesh.h>

#include <wx/event.h>
#include <wx/panel.h>


#include <map>


using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
class DAMNSpace : public scfImplementation2<DAMNSpace,CS::EditorApp::iSpace, iResourceListener>,
  public wxPanel
{
public:
  DAMNSpace (iBase* parent);
  virtual ~DAMNSpace ();

  // iSpace
  virtual bool Initialize (iObjectRegistry* obj_reg, iSpaceFactory* fact, wxWindow* parent);
  virtual iSpaceFactory* GetFactory () const { return factory; }
  virtual wxWindow* GetWindow ();
  virtual void DisableUpdates (bool val) { }
  
  // iResourceListener
  virtual void OnLoaded (iLoadingResource* resource);
  typedef std::map<csRef<iLoadingResource>, std::string> SearchResults;
  SearchResults searchResults;
  size_t current;
  csRef<iProgressMeter> meter;

private:
  iObjectRegistry* object_reg;
  csRef<iSpaceFactory> factory;
  
  wxGridSizer* sizer;
  wxSearchCtrl* srchCtrl;
  wxScrolledWindow* prespaceList;

  csRef<iEditor> editor;
  csRef<iActionManager> actionManager;
  
  csRef<iPluginManager> plugmgr;
  csRef<CS::Network::HTTP::iHTTPConnectionFactory> http;
  csRef<iResourceManager> damn;

  void OnSize (wxSizeEvent& event);
  
  void OnSearchButton (wxCommandEvent& event);
  void OnCancelButton (wxCommandEvent& event);
  
  DECLARE_EVENT_TABLE()
};

class Preview : public wxPanel
{
  wxImage wximage;
  wxBitmapButton* bitmapBtn;
  wxStaticText* text;
  wxBoxSizer* sizer;
public:
  Preview(wxWindow* parent, csRef<iImage> image, const std::string& label);
  void OnClicked (wxCommandEvent& event);
  void OnSize (wxSizeEvent& event);
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
