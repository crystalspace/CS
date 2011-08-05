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
#include "ieditor/space.h"

/* Fun fact: should occur after csutil/event.h, otherwise, gcc may report
 * missing csMouseEventHelper symbols. */
#include "csutil/custom_new_disable.h"
#include <wx/wx.h>
#include "csutil/custom_new_enable.h"

#include <wx/popupwin.h>
#include <wx/srchctrl.h>

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
  virtual int FilterEvent(wxEvent& event);
};


IMPLEMENT_APP(EditorApp)


int EditorApp::FilterEvent(wxEvent& event)
{
  if (event.GetEventType()==wxEVT_KEY_DOWN && ((wxKeyEvent&)event).GetKeyCode()==WXK_SPACE)
  {
    printf("EditorApp::FilterEvent %s\n", (wxGetActiveWindow()?"has":"no"));
    wxTextCtrl* hastextfocus = dynamic_cast<wxTextCtrl*>(wxGetActiveWindow());
    if (hastextfocus) return -1;
    
    wxPopupTransientWindow* p = new wxPopupTransientWindow( GetTopWindow());
    wxSearchCtrl* srchCtrl = new wxSearchCtrl(p, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(200, -1), 0);
    wxScrolledWindow* m_panel = new wxScrolledWindow(p, wxID_ANY);
    m_panel->SetScrollbars(0, 1, 0, 0);
    
    wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );
    wxStaticText* text = new wxStaticText( m_panel, wxID_ANY,
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("Operator Something\n")
                          wxT("And more..."));
    
    
    sizer->Add(text, 0, wxALL, 0);
    
    topSizer->Add(srchCtrl, 0, wxTOP|wxEXPAND, 0);
    topSizer->Add(m_panel, 0, wxALL, 0);
    
    m_panel->SetMaxSize(wxSize(200, 250));
     
    m_panel->SetSizer(sizer);
    sizer->SetSizeHints(m_panel);
    p->SetSizer(topSizer);
    topSizer->SetSizeHints(p);
    
    topSizer->Layout();
    
    /*
    srchCtrl->SetBackgroundColour(wxColour(wxT("#1c1c1c")));
    srchCtrl->SetForegroundColour(wxColour(wxT("#2b2b2b")));
    m_panel->SetBackgroundColour(wxColour(wxT("#1c1c1c")));
    text->SetForegroundColour(*wxWHITE);
    */
    p->Position(wxGetMousePosition(), wxSize(0, 0) );
    p->Popup();
    return true;
  }

  return -1;
}

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
  //if (!editor->LoadPlugin ("crystalspace.editor.plugin.core.cs3dview")) return false;
  //if (!editor->LoadPlugin ("crystalspace.editor.plugin.damn.damnview")) return false;
  
  if (!editor->LoadPlugin ("crystalspace.editor.plugin.core.cs3dheader")) return false;
  
  csRef<CS::EditorApp::iSpaceManager> spacemgr = csQueryRegistry<CS::EditorApp::iSpaceManager> (object_reg);
  if (!spacemgr->Register ("crystalspace.editor.plugin.core.cs3dspace")) return false;
  if (!spacemgr->Register ("crystalspace.editor.plugin.damn.damnspace")) return false;
  
  // Start the application
  if (!editor->StartApplication ()) return false;

  return true;
}

int EditorApp::OnExit ()
{
  csInitializer::DestroyApplication (object_reg);
  return 0;
}
