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

#include "ieditor/objectlist.h"
#include "ieditor/panelmanager.h"
#include "ieditor/editor.h"

#include "csutil/scf_implementation.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "csutil/eventnames.h"
#include "cstool/collider.h"

#include <wx/event.h>
#include <wx/dnd.h>

#include "dataobject.h"

class wxWindow;

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

#include "data/editor/images/meshIcon.xpm"

class CS3DPanel : public scfImplementation5<CS3DPanel,iPanel,iMapListener,iObjectListListener,iEventHandler,iComponent>
{
public:
  CS3DPanel (iBase* parent);
  virtual ~CS3DPanel();

  // iComponent
  virtual bool Initialize (iObjectRegistry* obj_reg);

  // iPanel
  virtual wxWindow* GetWindow ();
  virtual const wxChar* GetCaption () const;
  virtual PanelDockPosition GetDefaultDockPosition () const;

  // iMapListener
  virtual void OnMapLoaded (const char* path, const char* filename);
  virtual void OnLibraryLoaded (const char* path, const char* filename, iCollection* collection);

  // iObjectListListener
  virtual void OnObjectAdded (iObjectList* list, iEditorObject* obj);
  virtual void OnObjectRemoved (iObjectList* list, iEditorObject* obj);
  virtual void OnObjectsCleared (iObjectList* list);
  
  /// Called by wxTimer Pump every X seconds
  void PushFrame ();

  void OnSize (wxSizeEvent& event);
  
  void OnDrop (wxCoord x, wxCoord y, iEditorObject* obj);
  
private:
  iObjectRegistry* object_reg;

  wxWindow* window;
  
  // mouse pos of operation action start
  csVector2 opstartpos, mousepos;

  csRef<iEditor> editor;
  csRef<iPanelManager> panelManager;
  
  csRef<iObjectList> selection, objects;
  
  csRef<iEngine> engine;
  csRef<iGraphics3D> g3d;
  csRef<iVirtualClock> vc;
  csRef<iKeyboardDriver> kbd;
  csRef<iCollideSystem> cdsys;
  csRef<iEventQueue> q;
  csRef<iView> view;
  
  /// Our collider used for gravity and CD.
  csColliderActor collider_actor;
  
  bool initializedColliderActor;

  void MoveCamera ();
  
  virtual bool HandleEvent (iEvent& ev);
  
  virtual void ProcessFrame ();
  virtual void FinishFrame ();
  
  
  /// Updates the selection bounding box
  void UpdateSelection ();
  

  CS_EVENTHANDLER_NAMES("crystalspace.editor.plugin.core.cs3dpanel")
  CS_EVENTHANDLER_NIL_CONSTRAINTS

  CS_DECLARE_EVENT_SHORTCUTS;
  
  class Pump : public wxTimer
  {
  public:
    Pump (CS3DPanel* p) : panel (p) {}
    
    virtual void Notify ()
    { panel->PushFrame (); }
  private:
    CS3DPanel* panel;
  };

  Pump* pump;

  class Panel : public wxPanel
  {
    public:
      Panel(CS3DPanel* p, wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize)
      : wxPanel (parent, id, pos, size), panel(p)
      {}
    
      virtual void OnSize (wxSizeEvent& ev)
      { panel->OnSize (ev); }
    private:
      CS3DPanel* panel;
      
      DECLARE_EVENT_TABLE()
  };
  
  
  class MyDropTarget : public wxDropTarget
  {
  public:
    MyDropTarget (CS3DPanel* panel) : panel (panel)
    {
      data = new EditorDataObject ();
      SetDataObject (data);
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
      iEditorObject* obj = data->GetEditorObject ();
      printf ("You dropped a %s\n", obj->GetName ());
      panel->OnDrop (x, y, obj);
      
      return def;
    }
    
    virtual wxDragResult OnDragOver(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                    wxDragResult def)
    { return def; }
    
    private:
      csRef<CS3DPanel> panel;
      EditorDataObject* data;
  };
  
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
