/*
    Copyright (C) 2011 by Jorrit Tyberghein, Jelle Hellemans, Christian Van Brussel

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

#define CS_IMPLEMENT_PLATFORM_APPLICATION
/* This is needed due the WX headers using free() inline, but the opposing
 * malloc() is in the WX libs. */
#define CS_NO_MALLOC_OVERRIDE

#include "cssysdef.h"

#include "cstool/initapp.h"
#include "iutil/objreg.h"

#include "ieditor/editor.h"

/* Fun fact: should occur after csutil/event.h, otherwise, gcc may report
 * missing csMouseEventHelper symbols. */
#include "csutil/custom_new_disable.h"
#include <wx/wx.h>
#include "csutil/custom_new_enable.h"

CS_IMPLEMENT_APPLICATION

#if defined(CS_PLATFORM_WIN32)

#ifndef SW_SHOWNORMAL
#define SW_SHOWNORMAL 1
#endif

/*
  WX provides WinMain(), but not main(), which is required for console apps.
 */
int main (int argc, const char* const argv[])
{
  return WinMain (GetModuleHandle (0), 0, GetCommandLineA (), SW_SHOWNORMAL);
}

#endif


// Define a new application type
class EditorApp : public wxApp
{
public:
  iObjectRegistry* object_reg;
  csRef<CS::EditorApp::iEditor> editor;

  virtual bool OnInit (void);
  virtual int OnExit (void);
};


IMPLEMENT_APP(EditorApp)

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
bool EditorApp::OnInit (void)
{
  wxInitAllImageHandlers ();

#if defined(wxUSE_UNICODE) && wxUSE_UNICODE
  char** csargv;
  csargv = (char**)cs_malloc(sizeof(char*) * argc);
  for(int i = 0; i < argc; i++) 
  {
    csargv[i] = strdup (wxString(argv[i]).mb_str().data());
  }
  object_reg = csInitializer::CreateEnvironment (argc, csargv);
  //cs_free(csargv);
#else
  object_reg = csInitializer::CreateEnvironment (argc, argv);
#endif

  // Load the iEditor plugin
  csRef<iPluginManager> plugmgr = 
    csQueryRegistry<iPluginManager> (object_reg);
  if (!plugmgr) return false;

  editor = csLoadPlugin<CS::EditorApp::iEditor>
    (plugmgr, "crystalspace.editor.plugin.core.gui");
  if (!editor) return false;

  // The main frame of the editor is now opened, we can setup some parameters such as
  // the title of the frame
  (dynamic_cast<wxFrame*> (editor->GetWindow ()))->SetTitle (wxT ("Crystal Space Editor"));

  // Start the engine
  if (!editor->StartEngine ()) return false;

  // Load the specific plugins for the Crystal Space editor
  if (!editor->LoadPlugin ("crystalspace.editor.plugin.core.cs3dpanel")) return false;
  if (!editor->LoadPlugin ("crystalspace.editor.plugin.core.scenebrowserpanel")) return false;
  if (!editor->LoadPlugin ("crystalspace.editor.plugin.core.assetbrowserpanel")) return false;
  if (!editor->LoadPlugin ("crystalspace.editor.plugin.core.csobjectmaplistener")) return false;
  if (!editor->LoadPlugin ("crystalspace.editor.plugin.core.csinterfacewrappers")) return false;

  // Start the application
  if (!editor->StartApplication ()) return false;

  return true;
}

int EditorApp::OnExit ()
{
  csInitializer::DestroyApplication (object_reg);
  return 0;
}
