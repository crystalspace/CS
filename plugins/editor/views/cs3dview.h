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

#include "ieditor/view.h"
#include "ieditor/editor.h"

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "csutil/eventnames.h"
#include "cstool/collider.h"

#include <wx/event.h>
#include <wx/dnd.h>


class wxWindow;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

//#include "data/editor/images/meshIcon.xpm"

class CS3DView : public scfImplementation4<CS3DView,CS::EditorApp::iView,CS::EditorApp::iMapListener,iEventHandler,iComponent>
{
public:
  CS3DView (iBase* parent);
  virtual ~CS3DView();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

  // iView
  virtual wxWindow* GetWindow ();
  virtual const wxChar* GetCaption () const;
  virtual CS::EditorApp::ViewDockPosition GetDefaultDockPosition () const;
  

  // iMapListener
  virtual void OnMapLoaded (const char* path, const char* filename);
  virtual void OnLibraryLoaded (const char* path, const char* filename, iCollection* collection);


  /// Called by wxTimer Pump every X seconds
  void PushFrame ();

  void OnSize (wxSizeEvent& event);
  
  //void OnDrop (wxCoord x, wxCoord y, iEditorObject* obj);
  
private:
  iObjectRegistry* object_reg;

  wxWindow* window;
  
  // mouse pos of operation action start
  csVector2 opstartpos, mousepos;

  csRef<CS::EditorApp::iEditor> editor;
  csRef<CS::EditorApp::iViewManager> viewManager;
  
  csRef<iEngine> engine;
  csRef<iGraphics3D> g3d;
  csRef<iVirtualClock> vc;
  csRef<iKeyboardDriver> kbd;
  csRef<iCollideSystem> cdsys;
  csRef<iEventQueue> q;
  csRef< ::iView> view;
  
  /// Our collider used for gravity and CD.
  csColliderActor collider_actor;
  
  bool initializedColliderActor;

  void MoveCamera ();
  
  virtual bool HandleEvent (iEvent& ev);
  
  virtual void ProcessFrame ();
  virtual void FinishFrame ();
  

  CS_EVENTHANDLER_NAMES("crystalspace.editor.plugin.core.cs3dview")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
  
  class Pump : public wxTimer
  {
  public:
    Pump (CS3DView* p) : view (p) {}
    
    virtual void Notify ()
    { view->PushFrame (); }
  private:
    CS3DView* view;
  };

  Pump* pump;

  class View : public wxPanel
  {
    public:
      View(CS3DView* p, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize)
      : wxPanel (parent, id, pos, size), view(p)
      {}
    
      virtual void OnSize (wxSizeEvent& ev)
      { view->OnSize (ev); }
    private:
      CS3DView* view;
      
      DECLARE_EVENT_TABLE()
  };
  
  
  class MyDropTarget : public wxDropTarget
  {
  public:
    MyDropTarget (CS3DView* view) : view (view)
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
      //view->OnDrop (x, y, obj);
      
      return def;
    }
    
    virtual wxDragResult OnDragOver(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                    wxDragResult def)
    { return def; }
    
    private:
      csRef<CS3DView> view;
      //EditorDataObject* data;
  };
  
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
