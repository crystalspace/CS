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

#ifndef __MAINFRAME_H__
#define __MAINFRAME_H__

#include "ieditor/editor.h"
#include "ieditor/panelmanager.h"
#include "ieditor/menubar.h"
#include "ieditor/actionmanager.h"

#include "iutil/vfs.h"
#include "ivaria/pmeter.h"
#include "csutil/csstring.h"

#include <wx/frame.h>
#include <wx/statusbr.h>
#include <wx/timer.h>

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

class Editor;
class StatusBar;

enum
{
  ID_Quit = wxID_EXIT,
  ID_Open = wxID_OPEN,
  ID_Save = wxID_SAVE,
  ID_Undo = wxID_UNDO,
  ID_Redo = wxID_REDO,
  ID_MoveTool = wxID_HIGHEST + 2000,
  ID_RotateTool,
  ID_ScaleTool,
  ID_ToolBar,
  ID_ImportLibrary
};

class MainFrame : public wxFrame
{
public:
  MainFrame (iObjectRegistry* object_reg, Editor* editor,
	     const wxString& title, const wxPoint& pos, const wxSize& size);
  virtual ~MainFrame ();

  bool Initialize ();

  csPtr<iProgressMeter> GetProgressMeter ();

  void Update ();

  void PushMapFile (const char* path, const char* filename, bool clearEngine);
  void PushLibraryFile (const char* path, const char* filename);

  void OnOpen (wxCommandEvent& event);
  void OnSave (wxCommandEvent& event);
  void OnImportLibrary (wxCommandEvent& event);
  void OnQuit (wxCommandEvent& event);

  void OnUndo (wxCommandEvent& event);
  void OnRedo (wxCommandEvent& event);

  void OnMoveTool (wxCommandEvent& event);
  void OnRotateTool (wxCommandEvent& event);
  void OnScaleTool (wxCommandEvent& event);

  void UpdateEditMenu ();

private:
  void CreateHelper(csArray<csSimpleRenderMesh> &helpers, const char* factname, const char* color);

  iObjectRegistry* object_reg;

  Editor* editor;
  csRef<iVFS> vfs;
  csRef<iActionManager> actionManager;
  csRef<iActionListener> actionListener;

  StatusBar* statusBar;

  class ActionListener : public scfImplementation1<ActionListener, iActionListener>
  {
  public:
    ActionListener (MainFrame* mainFrame)
      : scfImplementationType (this), mainFrame (mainFrame)
    {}

    virtual ~ActionListener () {}

    virtual void OnActionDone (iAction* action)
    { mainFrame->UpdateEditMenu (); }

  private:
    MainFrame* mainFrame;
  };

  struct ResourceData
  {
    csString path;
    csString file;
    bool clearEngine;
    bool isLibrary;
  };
  csArray<ResourceData> resourceData;
  ResourceData* loadingResource;
  csRef<iThreadReturn> loadingReturn;

  class Pump : public wxTimer
  {
  public:
    Pump (MainFrame* frame) : frame (frame) {}
    
    virtual void Notify ()
    { frame->Update (); }
  private:
    MainFrame* frame;
  };
  Pump* pump;

  DECLARE_EVENT_TABLE()
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
