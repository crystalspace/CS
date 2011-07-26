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
#include "ieditor/actionmanager.h"
#include "ieditor/settingsmanager.h"
#include "ieditor/operator.h"
#include "ieditor/context.h"

#include "iengine/collection.h"
#include "iutil/comp.h"
#include "csutil/refarr.h"

#include "auiviewmanager.h"
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

  inline virtual iViewManager* GetViewManager () const
  { return viewManager; }

  inline virtual iMenuBar* GetMenuBar () const
  { return menuBar; }

  virtual csPtr<iProgressMeter> GetProgressMeter ();

  virtual iThreadReturn* LoadMapFile (const char* path, const char* filename, bool clearEngine);

  virtual void SaveMapFile (const char* path, const char* filename);
  
  virtual iThreadReturn* LoadLibraryFile (const char* path, const char* filename);
  
  virtual void AddMapListener (iMapListener* listener);

  virtual void RemoveMapListener (iMapListener* listener);
 
  virtual void SetHelperMeshes (csArray<csSimpleRenderMesh>* helpers);
  virtual csArray<csSimpleRenderMesh>* GetHelperMeshes ();

  void FireMapLoaded (const char* path, const char* file);
  void FireLibraryLoaded (const char* path, const char* file);

private:
  void Help ();

  iObjectRegistry* object_reg;

  csArray<csSimpleRenderMesh>* helper_meshes;

  MainFrame* mainFrame;

  csRef<iEngine> engine;
  csRef<iVFS> vfs;
  csRef<iThreadedLoader> loader;
  csRef<iSaver> saver;

  csRef<iCollection> mainCollection;

  csRef<iContext> context;
  
  csRef<AUIViewManager> viewManager;
  csRef<MenuBar> menuBar;
  csRef<iSettingsManager> settingsManager;
  csRef<iActionManager> actionManager;
  
  csRef<iOperatorManager> operatorManager;

  csRefArray<iMapListener> mapListeners;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
