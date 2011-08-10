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

#ifndef __CORE_PROPERTIESSPACE_H__
#define __CORE_PROPERTIESSPACE_H__

#include "ieditor/space.h"

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include <csutil/weakref.h>
#include <iutil/objreg.h>

#include <wx/event.h>
#include <wx/panel.h>

class wxWindow;

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

class PropertiesSpace : public scfImplementation1<PropertiesSpace,iSpace>
{
public:
  PropertiesSpace (iBase* parent);
  virtual ~PropertiesSpace();

  // iSpace
  virtual bool Initialize (iObjectRegistry* obj_reg, iSpaceFactory* fact, wxWindow* parent);
  virtual iSpaceFactory* GetFactory () const { return factory; }
  virtual wxWindow* GetWindow ();
  virtual void DisableUpdates (bool val) { }

  void OnSize (wxSizeEvent& event);

  
private:
  iObjectRegistry* object_reg;
  csRef<iSpaceFactory> factory;

  wxWindow* window;
  
  class Space : public wxPanel
  {
    public:
      Space(PropertiesSpace* p, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize)
      : wxPanel (parent, id, pos, size), space(p)
      {}
    
      virtual void OnSize (wxSizeEvent& ev)
      { if (space) space->OnSize (ev); }
    private:
      PropertiesSpace* space;
      
      DECLARE_EVENT_TABLE()
  };
  
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
