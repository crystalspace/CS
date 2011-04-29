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
#include "iutil/cmdline.h"
#include "iutil/stringarray.h"
#include "ivaria/collider.h"
#include "ivideo/graph2d.h"

#include <wx/textctrl.h>

#include "ieditor/panelmanager.h"
#include "objectlist.h"
#include "menubar.h"
#include "auipanelmanager.h"
#include "interfacewrappermanager.h"
#include "actionmanager.h"
#include "editorobject.h"
#include "mainframe.h"

#include "editor.h"

namespace CS {
namespace EditorApp {

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
    Help ();
    return true;
  }

  selection.AttachNew (new ObjectList ());
  objects.AttachNew (new ObjectList ());

  object_reg->Register (this, "iEditor");
  
  panelManager.AttachNew (new AUIPanelManager (object_reg));
  menuBar.AttachNew (new MenuBar (object_reg));
  interfaceManager.AttachNew (new InterfaceWrapperManager (object_reg));
  actionManager.AttachNew (new ActionManager (object_reg));
  
  // Create the main frame
  mainFrame = new MainFrame (wxT ("Crystal Space Editor"), wxDefaultPosition, wxSize (1024, 768));
  mainFrame->Initialize (object_reg, this);

  mainFrame->Show ();
  
  // Initialize CS and load plugins
  if (!InitCS ())
    return false;

  mainFrame->SecondInitialize (object_reg);
  
  panelManager->SecondInitialize (object_reg);

  return true;
}

void Editor::Help ()
{
  csCommandLineHelper commandLineHelper;

  // Usage examples
  commandLineHelper.AddCommandLineExample ("cseditor data/castel/world");
  commandLineHelper.AddCommandLineExample ("cseditor -R=data/kwartz.zip kwartz.lib");
  commandLineHelper.AddCommandLineExample ("cseditor -R=data/seymour.zip Seymour.dae");

  // Command line options
  commandLineHelper.AddCommandLineOption
    ("R", "Real path to be mounted in VFS", csVariant (""));
  commandLineHelper.AddCommandLineOption
    ("C", "VFS directory where to find the files", csVariant ("/"));
  commandLineHelper.AddCommandLineOption
    ("L", "Load a library file (for textures/materials)", csVariant (""));

  // Printing help
  commandLineHelper.PrintApplicationHelp
    (object_reg, "cseditor", "cseditor <OPTIONS> [filename]",
     "The Crystal Space editor\n\n"
     "If provided, it will load the given file from the specified VFS directory."
     " If no VFS directory is provided then it will assume the one of the file. "
     "An additional real path can be provided to be mounted before loading the file."
     " This is useful for example to mount an archive in VFS before accessing the"
     " files in it.");
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

  engine = csQueryRegistry<iEngine> (object_reg);
  if (!engine)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Failed to locate 3D engine!");
    return false;
  }
  engine->SetSaveableFlag(true);

  if (!csInitializer::RequestPlugins(object_reg,
    CS_REQUEST_LEVELSAVER,
    CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Failed to initialize iSaver plugin!");
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

  vfs = csQueryRegistry<iVFS> (object_reg);
  if (!vfs)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Failed to locate iVFS plugin!");
    return false;
  }
  
  loader = csQueryRegistry<iThreadedLoader> (object_reg);
  if (!loader)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Failed to locate iThreadedLoader plugin!");
    return false;
  }

  saver = csQueryRegistry<iSaver> (object_reg);
  if (!saver)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
              "crystalspace.application.editor",
              "Failed to locate iSaver plugin!");
    return false;
  }

  mainCollection = engine->CreateCollection ("Main collection");

  // Analyze the command line arguments
  csRef<iCommandLineParser> cmdline =
    csQueryRegistry<iCommandLineParser> (object_reg);

  const char* libname;
  for (int i = 0; (libname = cmdline->GetOption ("L", i)); i++)
    mainFrame->PushLibraryFile ("", libname);

  const char* realPath = cmdline->GetOption ("R");
  if (realPath)
  {
    vfs->Mount ("/tmp/viewmesh", realPath);
    //vfs->ChDir ("/tmp/viewmesh");
  }

  csString filename = cmdline->GetName (0);
  csString vfsDir = cmdline->GetOption ("C");
  if (vfsDir.IsEmpty ())
    vfsDir = realPath;

  if (vfsDir.IsEmpty () && filename)
  {
    size_t index = filename.FindLast ('/');
    if (index != (size_t) -1)
    {
      vfsDir = filename.Slice (0, index);
      filename = filename.Slice (index + 1);
    }
  }

  if (filename)
    mainFrame->PushMapFile (vfsDir, filename, false);

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
      csRef<iComponent> c (plugmgr->LoadPluginInstance (className,
        iPluginManager::lpiInitialize | iPluginManager::lpiReportErrors
          | iPluginManager::lpiLoadDependencies));
      csRef<iBase> b = scfQueryInterface<iBase> (c);

      csReport (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
        "crystalspace.application.editor", "Attempt to load plugin '%s' %s",
        className, b ? "successful" : "failed");
      if (b) b->DecRef ();
    }
  };
}

csPtr<iProgressMeter> Editor::GetProgressMeter ()
{
  return mainFrame->GetProgressMeter ();
}

iThreadReturn* Editor::LoadMapFile (const char* path, const char* filename, bool clearEngine)
{
  vfs->ChDir (path);

  if (clearEngine)
  {
    engine->RemoveCollection (mainCollection);
    mainCollection = engine->CreateCollection ("Main collection");
  }

  csRef<iThreadReturn> loadingResult =
    loader->LoadMapFile (vfs->GetCwd (), filename, clearEngine, mainCollection);
  return loadingResult;
}

iThreadReturn* Editor::LoadLibraryFile (const char* path, const char* filename)
{
  vfs->ChDir (path);

  csRef<iThreadReturn> loadingResult =
    loader->LoadLibraryFile (vfs->GetCwd (), filename, mainCollection);
  return loadingResult;
}

void Editor::SaveMapFile (const char* path, const char* filename)
{
  vfs->ChDir (path);
  
  saver->SaveCollectionFile (mainCollection, filename, CS_SAVER_FILE_WORLD);
}

void Editor::FireMapLoaded (const char* path, const char* filename)
{
  csRef<iProgressMeter> progressMeter = GetProgressMeter ();
  engine->Prepare (progressMeter);

  // TODO: Remove me. I'm only here to test the relighting progress gauge.
  //engine->SetLightingCacheMode (0);

  // Notify map listeners
  csRefArray<iMapListener>::Iterator it = mapListeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnMapLoaded (path, filename);
  }
}

void Editor::FireLibraryLoaded (const char* path, const char* filename)
{
  // Notify map listeners
  csRefArray<iMapListener>::Iterator it = mapListeners.GetIterator ();
  while (it.HasNext ())
  {
    it.Next ()->OnLibraryLoaded (path, filename, mainCollection);
  }
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

} // namespace EditorApp
} // namespace CS
