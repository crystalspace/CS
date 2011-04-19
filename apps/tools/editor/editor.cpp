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

#include "cssysdef.h"
#include "csutil/scf.h"

#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "imap/saverfile.h"
#include "iutil/stringarray.h"
#include "ivideo/graph2d.h"
#include "ivaria/collider.h"

#include <wx/textctrl.h>

#include "ieditor/panelmanager.h"
#include "objectlist.h"
#include "auipanelmanager.h"
#include "interfacewrappermanager.h"
#include "actionmanager.h"
#include "editorobject.h"
#include "mainframe.h"

#include "editor.h"

namespace CSE
{

Editor::Editor ()
  : scfImplementationType (this), helper_meshes (0), transstatus (NOTHING)
{
}

Editor::~Editor ()
{
  delete helper_meshes;
  // Remove ourself from object registry
  object_reg->Unregister (this, "iEditor");
}

bool Editor::Initialize (iObjectRegistry* reg)
{
  object_reg = reg;

  // Check for commandline help.
  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    csCommandLineHelper commandLineHelper;

    // Printing help
    commandLineHelper.PrintApplicationHelp
      (object_reg, "cseditor", "cseditor <OPTIONS>",
       "The Crystal Space editor");
    return true;
  }

  selection.AttachNew (new ObjectList ());
  objects.AttachNew (new ObjectList ());

  object_reg->Register (this, "iEditor");
  
  panelManager.AttachNew (new AUIPanelManager (object_reg));
  interfaceManager.AttachNew (new InterfaceWrapperManager (object_reg));
  actionManager.AttachNew (new ActionManager (object_reg));
  
  // Create the main frame
  mainFrame = new MainFrame (wxT ("Crystal Space Editor"), wxDefaultPosition, wxSize (800, 600));
  mainFrame->Initialize (object_reg);

  mainFrame->Show ();
  
  // Initialize CS and load plugins
  if (!InitCS ())
    return false;

  mainFrame->SecondInitialize (object_reg);

  return true;
}

bool Editor::InitCS ()
{
  // Request every standard plugin except for OpenGL/WXGL canvas
  if (!csInitializer::RequestPlugins (object_reg,
        CS_REQUEST_VFS,
	CS_REQUEST_PLUGIN ("crystalspace.graphics2d.wxgl", iGraphics2D),
        CS_REQUEST_OPENGL3D,
        CS_REQUEST_ENGINE,
        CS_REQUEST_FONTSERVER,
        CS_REQUEST_IMAGELOADER,
        CS_REQUEST_LEVELLOADER,
        CS_REQUEST_LEVELSAVER,
        CS_REQUEST_REPORTER,
        CS_REQUEST_REPORTERLISTENER,
        CS_REQUEST_PLUGIN ("crystalspace.collisiondetection.opcode",
                           iCollideSystem),
        CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Can't initialize plugins!");
    return false;
  }

  // Load plugins
  LoadPlugins ();
  
  // Open the main system. This will open all the previously loaded plug-ins.
  if (!csInitializer::OpenApplication (object_reg))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Error opening system!");
    return false;
  }

  engine = csQueryRegistry<iEngine> (object_reg);
  if (engine == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "No iEngine plugin!");
    return false;
  }

  engine->SetSaveableFlag (true);
  
  vfs = csQueryRegistry<iVFS> (object_reg);
  if (vfs == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "No iVFS plugin!");
    return false;
  }
  
  loader = csQueryRegistry<iLoader> (object_reg);
  if (loader == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "No iLoader plugin!");
    return false;
  }

  saver = csQueryRegistry<iSaver> (object_reg);
  if (saver == 0)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "No iSaver plugin!");
    return false;
  }

  //mainCollection = engine->CreateCollection ("Main collection");

  return true;
}

void Editor::LoadPlugins ()
{
  // TODO: Add additional plugin directories to scan, through settings system?
  csRef<iStringArray> pluginClasses =
    iSCF::SCF->QueryClassList ("crystalspace.editor.plugin.");
  if (pluginClasses.IsValid())
  {
    csRef<iPluginManager> plugmgr =
      csQueryRegistry<iPluginManager> (object_reg);
    for (size_t i = 0; i < pluginClasses->GetSize (); i++)
    {
      const char* className = pluginClasses->Get (i);
      iBase* b = plugmgr->LoadPlugin (className);

      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.application.editor", "Attempt to load plugin '%s' %s",
        className, (b != 0) ? "successful" : "failed");
      if (b != 0) b->DecRef ();
    }
  };
}

bool Editor::LoadMapFile (const char* path, const char* filename, iProgressMeter* meter,
			  bool clearEngine)
{
  vfs->ChDir (path);

  if (!loader->LoadMapFile (filename, clearEngine, mainCollection, false))
    return false;

  // TODO: Remove me. I'm only here to test the relighting progress gauge.
  //engine->SetLightingCacheMode (0);
  
  engine->Prepare (meter);
  
  // Notify map listeners
  csRefArray<iMapListener>::Iterator it = mapListeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnMapLoaded (path, filename);
  }

  return true;
}

bool Editor::LoadLibraryFile (const char* path, const char* filename)
{
  vfs->ChDir (path);

  iCollection* collection = engine->CreateCollection ("loading_collection");

  if (!loader->LoadLibraryFile (filename, collection, false))
    return false;
  
  // Notify map listeners
  csRefArray<iMapListener>::Iterator it = mapListeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnLibraryLoaded (path, filename, collection);
  }

  return true;
}

void Editor::SaveMapFile (const char* path, const char* filename)
{
  vfs->ChDir (path);
  
  saver->SaveCollectionFile (mainCollection, filename, CS_SAVER_FILE_WORLD);
}

void Editor::AddMapListener (iMapListener* listener)
{
  mapListeners.Push (listener);
}

void Editor::RemoveMapListener (iMapListener* listener)
{
  mapListeners.Delete (listener);
}

csPtr<iEditorObject> Editor::CreateEditorObject (iBase* object, wxBitmap* icon)
{
  return csPtr<iEditorObject> (new EditorObject (object_reg, object, icon));
}

iObjectList* Editor::GetSelection ()
{
  return selection;
}

iObjectList* Editor::GetObjects ()
{
  return objects;
}

void Editor::SetHelperMeshes (csArray<csSimpleRenderMesh>* helpers)
{
  delete helper_meshes;
  helper_meshes = helpers;
}
csArray<csSimpleRenderMesh>* Editor::GetHelperMeshes ()
{
  return helper_meshes;
}

void Editor::SetTransformStatus (TransformStatus status)
{
  transstatus = status;
}
Editor::TransformStatus Editor::GetTransformStatus ()
{
  return transstatus;
}

}
