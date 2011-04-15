/*  -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
    Copyright (C) 2004 by Peter Amstutz, Jorrit Tyberghein

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

#include <cssysdef.h>

#include <cstool/initapp.h>
#include <iutil/objreg.h>

#include "editor.h"

/* Fun fact: should occur after csutil/event.h, otherwise, gcc may report
 * missing csMouseEventHelper symbols. */
#include <wx/wx.h>

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
class EditorApp: public wxApp
{
public:
  iObjectRegistry* object_reg;
  CSE::Editor* editor;

  virtual bool OnInit(void);
  virtual int OnExit(void);
};


IMPLEMENT_APP(EditorApp)

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
bool EditorApp::OnInit(void)
{
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

  editor = new CSE::Editor ();
  if (!editor->Initialize (object_reg))
    return false;

  return true;
}

int EditorApp::OnExit()
{
  delete editor;
  csInitializer::DestroyApplication (object_reg);
  return 0;
}
