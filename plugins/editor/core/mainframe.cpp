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
#include "csutil/eventnames.h"
#include "csutil/scf.h"

#include "iutil/objreg.h"
#include "iengine/engine.h"
#include "iengine/mesh.h"
#include "iengine/texture.h"
#include "imap/loader.h"
#include "imesh/genmesh.h"
#include "imesh/object.h"
#include "iutil/event.h"
#include "iutil/eventq.h"
#include "ivideo/texture.h"
#include "ivideo/graph3d.h"

#include <wx/menu.h>
#include <wx/toolbar.h>

#include "editor.h"
#include "mainframe.h"
#include "statusbar.h"
#include "vfsfiledialog.h"

#include "ieditor/action.h"

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

BEGIN_EVENT_TABLE (MainFrame, wxFrame)
  EVT_MENU (ID_Open, MainFrame::OnOpen)
  EVT_MENU (ID_Save, MainFrame::OnSave)
  EVT_MENU (ID_ImportLibrary, MainFrame::OnImportLibrary)
  EVT_MENU (ID_Quit, MainFrame::OnQuit)
  
  EVT_MENU (ID_Undo, MainFrame::OnUndo)
  EVT_MENU (ID_Redo, MainFrame::OnRedo)
END_EVENT_TABLE ()

MainFrame::MainFrame (iObjectRegistry* object_reg, Editor* editor,
		      const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame (NULL, -1, title, pos, size), object_reg (object_reg), editor (editor),
  loadingResource (nullptr), pump (nullptr)
{
  wxMenu* fileMenu = new wxMenu ();

  fileMenu->Append (ID_Open, wxT("&Open world file\tCtrl+O"));
  fileMenu->Append (ID_ImportLibrary, wxT("&Import library file\tCtrl+I"));
  fileMenu->Append (ID_Save, wxT("&Save world file\tCtrl+S"));
  fileMenu->AppendSeparator ();
  fileMenu->Append (ID_Quit, wxT("&Quit\tCtrl+Q"));

  wxMenu* editMenu = new wxMenu ();
  editMenu->Append (ID_Undo, wxT("&Undo\tCtrl+Z"));
  editMenu->Append (ID_Redo, wxT("&Redo\tCtrl+Y"));
  
  wxMenuBar* menuBar = new wxMenuBar ();
  menuBar->Append (fileMenu, wxT("&File"));
  menuBar->Append (editMenu, wxT("&Edit"));

  SetMenuBar (menuBar);
  menuBar->Reparent (this);

  statusBar = new StatusBar (this);
  SetStatusBar (statusBar);
  
  SetStatusText (wxT("Ready"));

  PositionStatusBar ();
  statusBar->Show ();

  actionManager = csQueryRegistry<iActionManager> (object_reg);
  if (actionManager)
  {
    actionListener.AttachNew (new MainFrame::ActionListener (this));
    actionManager->AddListener (actionListener);

    UpdateEditMenu ();
  }
}

MainFrame::~MainFrame ()
{
  delete statusBar;
  delete pump;
  delete loadingResource;
}

bool MainFrame::Initialize ()
{
  pump = new Pump(this);
  pump->Start (20);

  return true;
}

csPtr<iProgressMeter> MainFrame::GetProgressMeter ()
{
  csRef<iProgressMeter> meter;
  meter.AttachNew (new StatusBarProgressMeter (statusBar));
  return csPtr<iProgressMeter> (meter);
}

void MainFrame::Update ()
{
  // Check the status of any resource currently loaded
  if (loadingResource && loadingReturn->IsFinished ())
  {
    if (loadingReturn->WasSuccessful ())
    {
      csString text = loadingResource->isLibrary ? "Library \"" : "Map \"";
      text += loadingResource->file + "\" was loaded successfully";
      SetStatusText (wxString::FromAscii (text.GetData ()));

      if (loadingResource->isLibrary)
	editor->FireLibraryLoaded (loadingResource->path, loadingResource->file);
      else
	editor->FireMapLoaded (loadingResource->path, loadingResource->file);
    }

    else
    {
      csString text = "Failed to load ";
      text += loadingResource->isLibrary ? "library \"" : "map \"";
      text += loadingResource->file + "\"";
      SetStatusText (wxString::FromAscii (text.GetData ()));
    }

    delete loadingResource;
    loadingResource = nullptr;
    loadingReturn = nullptr;
  }

  // If there are no loading active then push any pending one
  if (!loadingResource && resourceData.GetSize ())
  {
    loadingResource = new ResourceData ();
    *loadingResource = resourceData.Top ();
    resourceData.DeleteIndex (0);

    csString text = "Loading ";
    text += loadingResource->isLibrary ? "library \"" : "map \"";
    text += loadingResource->file + "\"...";
    SetStatusText (wxString::FromAscii (text.GetData ()));

    if (loadingResource->isLibrary)
      loadingReturn = editor->LoadLibraryFile (loadingResource->path, loadingResource->file);
    else
      loadingReturn = editor->LoadMapFile (loadingResource->path, loadingResource->file, loadingResource->clearEngine);
  }
}

void MainFrame::PushMapFile (const char* path, const char* filename, bool clearEngine)
{
  ResourceData resource;
  resource.path = path;
  resource.file = filename;
  resource.clearEngine = clearEngine;
  resource.isLibrary = false;
  resourceData.Push (resource);
}

void MainFrame::PushLibraryFile (const char* path, const char* filename)
{
  ResourceData resource;
  resource.path = path;
  resource.file = filename;
  resource.clearEngine = false;
  resource.isLibrary = true;
  resourceData.Push (resource);
}

void MainFrame::OnOpen (wxCommandEvent& event)
{
  if (!vfs)
    vfs = csQueryRegistry<iVFS> (object_reg);

  csString path = vfs->GetCwd ();
  if (path == "/")
    path = "/lev/";

  cssVFSFileDlg dialog (this, -1, _("Open world file"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, vfs, path, VFS_OPEN);
  if (!dialog.ShowModal ())
    return;
  PushMapFile (dialog.GetPath (), dialog.GetFilename (), true);
}

void MainFrame::OnSave (wxCommandEvent& event)
{
  if (!vfs)
    vfs = csQueryRegistry<iVFS> (object_reg);

  csString path = vfs->GetCwd ();
  if (path == "/")
    path = "/lev/";

  cssVFSFileDlg dialog (this, -1, _("Save world file"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, vfs, path, VFS_SAVE);
  if (!dialog.ShowModal ())
    return;

  SetStatusText (wxT("Saving map..."));

  editor->SaveMapFile (dialog.GetPath (), dialog.GetFilename ());

  SetStatusText (wxT("Ready"));
}

void MainFrame::OnImportLibrary(wxCommandEvent& event)
{
  if (!vfs)
    vfs = csQueryRegistry<iVFS> (object_reg);

  csString path = vfs->GetCwd ();
  if (path == "/")
    path = "/lib/";

  cssVFSFileDlg dialog (this, -1, _("Import library file"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, vfs, path, VFS_OPEN);
  if (!dialog.ShowModal ())
    return;
  PushLibraryFile (dialog.GetPath (), dialog.GetFilename ());
}

void MainFrame::OnQuit (wxCommandEvent& event)
{
  // Tell CS we're going down
  csRef<iEventQueue> q (csQueryRegistry<iEventQueue> (object_reg));
  if (q) q->GetEventOutlet ()->Broadcast (csevQuit (object_reg));

  Destroy ();
}

void MainFrame::OnUndo (wxCommandEvent& event)
{
  actionManager->Undo ();
}

void MainFrame::OnRedo (wxCommandEvent& event)
{
  actionManager->Redo ();
}

void MainFrame::CreateHelper(csArray<csSimpleRenderMesh> &helpers, const char* factname, const char* color)
{
  csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
  csRef<iMeshFactoryWrapper> fact = engine->GetMeshFactories()->FindByName(factname);
  
  csReversibleTransform trf (csXScaleMatrix3 (8) * csYScaleMatrix3 (8) * csZScaleMatrix3 (8), csVector3(0,0,0));
  fact->HardTransform(trf);
  
  csRef<iTextureWrapper> tex = engine->GetTextureList ()->FindByName (color);
  if (!(fact && tex))
    return;
  csRef<iGeneralFactoryState> fact_state = scfQueryInterface<iGeneralFactoryState> (fact->GetMeshObjectFactory ());
  if (!fact_state)
    return;

  csSimpleRenderMesh mesh;
  mesh.vertices = fact_state->GetVertices ();
  mesh.vertexCount = fact_state->GetVertexCount ();
  mesh.texture = tex->GetTextureHandle ();
  mesh.indices = (uint *)fact_state->GetTriangles();
  mesh.indexCount = fact_state->GetTriangleCount()*3;
  mesh.texcoords = fact_state->GetTexels();
  helpers.Push (mesh);
}

void MainFrame::UpdateEditMenu ()
{
  printf("MainFrame::UpdateEditMenu\n");
  const iAction* undo = actionManager->PeekUndo ();
  const iAction* redo = actionManager->PeekRedo ();

  wxMenuBar* menuBar = GetMenuBar ();
  
  menuBar->Enable(ID_Undo, undo != 0);
  menuBar->Enable(ID_Redo, redo != 0);

  if (undo)
    menuBar->SetLabel(ID_Undo, wxT("Undo \"")
        + wxString(undo->GetDescription (), wxConvUTF8) + wxT("\"\tCtrl+Z"));
  
  if (redo)
    menuBar->SetLabel(ID_Redo, wxT("Redo \"")
        + wxString(redo->GetDescription (), wxConvUTF8) + wxT("\"\tCtrl+Y"));
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
