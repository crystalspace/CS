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

#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "ieditor/editor.h"
#include "ieditor/interfacewrappermanager.h"
#include "ieditor/actionmanager.h"

#include "iengine/collection.h"
#include "iutil/comp.h"
#include "csutil/refarr.h"

#include "auipanelmanager.h"
#include "mainframe.h"
#include "menubar.h"

struct iObjectRegistry;
struct iSaver;
struct iVFS;
struct csSimpleRenderMesh;

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

class Editor : public scfImplementation2<Editor, iEditor, iComponent>
{
public:
  Editor (iBase* parent);
  virtual ~Editor ();
  
  // iComponent
  virtual bool Initialize (iObjectRegistry* reg);

  virtual bool StartEngine ();
  virtual bool StartApplication ();
  virtual bool LoadPlugin (const char* name);

  inline virtual wxWindow* GetWindow ()
  { return static_cast<wxWindow*> (mainFrame); }

  inline virtual iPanelManager* GetPanelManager () const
  { return panelManager; }

  inline virtual iMenuBar* GetMenuBar () const
  { return menuBar; }

  virtual csPtr<iProgressMeter> GetProgressMeter ();

  virtual iThreadReturn* LoadMapFile (const char* path, const char* filename, bool clearEngine);

  virtual void SaveMapFile (const char* path, const char* filename);
  
  virtual iThreadReturn* LoadLibraryFile (const char* path, const char* filename);
  
  virtual void AddMapListener (iMapListener* listener);

  virtual void RemoveMapListener (iMapListener* listener);

  virtual csPtr<iEditorObject> CreateEditorObject (iBase* object, wxBitmap* icon);
  
  virtual iObjectList* GetSelection ();

  virtual iObjectList* GetObjects ();
  
  virtual void SetHelperMeshes (csArray<csSimpleRenderMesh>* helpers);
  virtual csArray<csSimpleRenderMesh>* GetHelperMeshes ();

  virtual void SetTransformStatus (TransformStatus status);
  virtual TransformStatus GetTransformStatus ();

  void FireMapLoaded (const char* path, const char* file);
  void FireLibraryLoaded (const char* path, const char* file);

private:
  void Help ();

  iObjectRegistry* object_reg;

  csArray<csSimpleRenderMesh>* helper_meshes;
  TransformStatus transstatus;

  MainFrame* mainFrame;

  csRef<iEngine> engine;
  csRef<iVFS> vfs;
  csRef<iThreadedLoader> loader;
  csRef<iSaver> saver;

  csRef<iCollection> mainCollection;

  csRef<AUIPanelManager> panelManager;
  csRef<MenuBar> menuBar;
  csRef<iInterfaceWrapperManager> interfaceManager;
  csRef<iActionManager> actionManager;

  /*
  csRef<iActionManager> actionManager;
  csRef<iToolManager> toolManager;
  csRef<iPluginManager> pluginManager;
  */

  csRefArray<iMapListener> mapListeners;

  csRef<iObjectList> selection;
  csRef<iObjectList> objects;

};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
