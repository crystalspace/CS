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

#ifndef __CORE_CS3DPANEL_H__
#define __CORE_CS3DPANEL_H__

#include "ieditor/space.h"
#include "ieditor/editor.h"

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "csutil/eventnames.h"
#include "cstool/collider.h"
#include <csutil/weakref.h>

#include <wx/event.h>
#include <wx/dnd.h>


class wxWindow;

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

class CS3DSpace : public scfImplementation1<CS3DSpace,iSpace>
{
public:
  CS3DSpace (iBase* parent);
  virtual ~CS3DSpace();

  // iSpace
  virtual bool Initialize (iObjectRegistry* obj_reg, iSpaceFactory* fact, wxWindow* parent);
  virtual iSpaceFactory* GetFactory () const { return factory; }
  virtual wxWindow* GetWindow ();
  virtual void DisableUpdates (bool val) { disabled = val; }
  bool disabled;


  // iMapListener
  virtual void OnMapLoaded (const char* path, const char* filename);
  virtual void OnLibraryLoaded (const char* path, const char* filename, iCollection* collection);


  /// Called by wxTimer Pump every X seconds
  void PushFrame ();

  void OnSize (wxSizeEvent& event);
  
  //void OnDrop (wxCoord x, wxCoord y, iEditorObject* obj);
  
private:
  iObjectRegistry* object_reg;
  csRef<iSpaceFactory> factory;

  wxWindow* window;
  
  // mouse pos of operation action start
  csVector2 opstartpos, mousepos;

  csRef<iEditor> editor;
  
  csRef<iEngine> engine;
  csRef<iGraphics3D> g3d;
  csRef<iVirtualClock> vc;
  csRef<iKeyboardDriver> kbd;
  csRef<iCollideSystem> cdsys;
  csRef<iEventQueue> q;
  csRef<iWxWindow> wxwin;
  csRef<iView> view;
  
  /// Our collider used for gravity and CD.
  csColliderActor collider_actor;
  
  bool initializedColliderActor;

  void MoveCamera ();
  
  virtual bool HandleEvent (iEvent& ev);
  
  virtual void ProcessFrame ();
  virtual void FinishFrame ();
  

  struct EventHandler : public scfImplementation1<EventHandler,iEventHandler>
  {
    CS3DSpace* space;
    EventHandler(CS3DSpace* space) : scfImplementationType (this), space(space) {}
    virtual bool HandleEvent (iEvent& ev) { return space->HandleEvent(ev); }
    CS_EVENTHANDLER_NAMES("crystalspace.editor.plugin.core.cs3dspace")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  friend struct EventHandler;
  csRef<EventHandler> eventHandler;
  
  struct MapListener : public scfImplementation1<MapListener,iMapListener>
  {
    CS3DSpace* space;
    MapListener(CS3DSpace* space) : scfImplementationType (this), space(space) {}
    virtual void OnMapLoaded (const char* path, const char* filename) { space->OnMapLoaded (path, filename);  }
    virtual void OnLibraryLoaded (const char* path, const char* filename, iCollection* collection) { space->OnLibraryLoaded (path, filename, collection);  }
    CS_EVENTHANDLER_NAMES("crystalspace.editor.plugin.core.cs3dspace")
    CS_EVENTHANDLER_NIL_CONSTRAINTS
  };
  friend struct MapListener;
  csRef<MapListener> mapListener;
  
  
  
  class Pump : public wxTimer
  {
  public:
    Pump (CS3DSpace* p) : space (p) {}
    
    virtual void Notify ()
    { space->PushFrame (); }
  private:
    CS3DSpace* space;
  };

  Pump* pump;

  class Space : public wxPanel
  {
    public:
      Space(CS3DSpace* p, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize)
      : wxPanel (parent, id, pos, size), space(p)
      {}
    
      virtual void OnSize (wxSizeEvent& ev)
      { if (space) space->OnSize (ev); }
    private:
      CS3DSpace* space;
      
      DECLARE_EVENT_TABLE()
  };
  
  
  class MyDropTarget : public wxDropTarget
  {
  public:
    MyDropTarget (CS3DSpace* space) : space (space)
    {
      //data = new EditorDataObject ();
      //SetDataObject (data);
    }
    
    // wxDropTarget
    virtual bool OnDrop(wxCoord x, wxCoord y)
    {
      printf("OnDrop %d %d\n", x, y);
      return true;
    }

    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def)
    {
      printf("OnData\n");
      if (!GetData ())
        return wxDragNone;
      //iEditorObject* obj = data->GetEditorObject ();
      //printf ("You dropped a %s\n", obj->GetName ());
      //space->OnDrop (x, y, obj);
      
      return def;
    }
    
    virtual wxDragResult OnDragOver(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                    wxDragResult def)
    { return def; }
    
    private:
      csWeakRef<CS3DSpace> space;
      //EditorDataObject* data;
  };
  
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
