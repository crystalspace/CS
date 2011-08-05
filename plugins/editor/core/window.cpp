/*
    Copyright (C) 2011 by Jelle Hellemans

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

#include <csutil/objreg.h>
#include <ieditor/space.h>

#include "window.h"

#include <wx/sizer.h>


#include "wx/dc.h"
#include "wx/dcmemory.h"
#include "wx/artprov.h"

namespace CS {
namespace EditorApp {
  
BEGIN_EVENT_TABLE(Window, wxSplitterWindow)
  EVT_SPLITTER_DCLICK(wxID_ANY, Window::OnDClick)
  EVT_SPLITTER_UNSPLIT(wxID_ANY, Window::OnUnsplitEvent)
END_EVENT_TABLE()

Window::Window (iObjectRegistry* obj_reg, wxWindow* parent, bool hor)
  : wxSplitterWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN), 
  object_reg (obj_reg), horizontal(hor)
{
  ViewControl* control = new ViewControl(object_reg, this);
  Initialize(control);
}

Window::Window (iObjectRegistry* obj_reg, wxWindow* parent, ViewControl* control, bool hor)
  : wxSplitterWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,wxSP_3D | wxSP_LIVE_UPDATE | wxCLIP_CHILDREN), 
  object_reg (obj_reg), horizontal(hor)
{
  control->Reparent(this);
  Initialize(control);
}

Window::~Window ()
{
  printf("Window::~Window\n");
}

bool Window::Split()
{
  if (IsSplit()) return false;
  ViewControl* c = (ViewControl*)GetWindow1();
  Window* w1 = new Window(object_reg, this, c, !horizontal);
  Window* w2 = new Window(object_reg, this, !horizontal);
  
  printf("Window::Split 1\n");
  if (horizontal) SplitHorizontally(w1, w2);
  else SplitVertically(w1, w2);
  printf("Window::Split 2\n");

  return true;
}

void Window::OnDClick(wxSplitterEvent& event)
{
  printf("Window::OnDClick\n");
  event.Veto();
  wxWindow* w1 = GetWindow1();
  wxWindow* w2 = GetWindow2();
  Unsplit();
  if (GetSplitMode() == wxSPLIT_VERTICAL)
    SplitHorizontally(w1, w2);
  else
    SplitVertically(w1, w2);
}

void Window::OnUnsplitEvent(wxSplitterEvent& event)
{
  printf("Window::OnUnsplitEvent\n");
  wxWindow* w = event.GetWindowBeingRemoved();
  if (w) w->Destroy();
}

// ----------------------------------------------------------------------------

#include "data/editor/images/sceneIcon.xpm"

ViewControl::ViewControl (iObjectRegistry* obj_reg, wxWindow* parent)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), object_reg (obj_reg)
{
  box = new wxBoxSizer(wxVERTICAL);
  
  wxPanel* menuBar = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 20));
  {
    wxBoxSizer* bar = new wxBoxSizer(wxHORIZONTAL);
    {
      wxAuiToolBar* tb1 = new wxAuiToolBar(menuBar, wxID_ANY);
      
      BitmapComboBox* m_combobox = new BitmapComboBox(obj_reg, tb1, this);
      
      if (space) box->Add(space->GetWindow(""), 1, wxEXPAND);   
      
      static MenuEntry* entry1 = new MenuEntry(obj_reg, tb1, wxT("View")); 
      tb1->AddControl(entry1);    
      
      MenuEntry* entry2 = new MenuEntry(obj_reg, tb1, wxT("Test")); 
      tb1->AddControl(entry2);            
      
      tb1->AddControl(m_combobox);
      /*tb1->AddTool(1, wxT("Test"), wxArtProvider::GetBitmap(wxART_ERROR));
      tb1->AddSeparator();
      tb1->AddTool(2, wxT("Test"), wxArtProvider::GetBitmap(wxART_QUESTION));
      tb1->AddTool(3, wxT("Test"), wxArtProvider::GetBitmap(wxART_INFORMATION));
      tb1->AddTool(4, wxT("Test"), wxArtProvider::GetBitmap(wxART_WARNING));
      tb1->AddTool(5, wxT("Test"), wxArtProvider::GetBitmap(wxART_MISSING_IMAGE));*/
      tb1->Realize();
      bar->Add(tb1, 1, wxEXPAND | wxALL, 0);
    }
    {
      wxAuiToolBar* tb1 = new wxAuiToolBar(menuBar, wxID_ANY);
      tb1->AddTool(1, wxT("Split"), wxArtProvider::GetBitmap(wxART_MISSING_IMAGE));
      tb1->Connect(1, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ViewControl::OnClicked), 0, this);
      tb1->AddTool(2, wxT("Duplicate"), wxArtProvider::GetBitmap(wxART_MISSING_IMAGE));
      tb1->Connect(2, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ViewControl::OnClicked), 0, this);
      tb1->Realize();
      bar->Add(tb1, 0, wxEXPAND | wxALIGN_RIGHT, 0);
    }
    menuBar->SetSizer(bar);
  }
  box->Add(menuBar, 0, wxEXPAND);
  SetSizer(box);
  box->SetSizeHints(this);
}

ViewControl::~ViewControl ()
{
  printf("ViewControl::~ViewControl\n");
}

void ViewControl::OnClicked (wxCommandEvent& event)
{
  printf("ViewControl::OnClicked %d\n", event.GetId());
  if (event.GetId() == 1)
  {
    space->DisableUpdates(true);
    Window* window = (Window*)this->GetParent();
    window->Split();
  }
  else
  {
    wxFrame* frame = new wxFrame(this, wxID_ANY, wxT ("3D View"), wxDefaultPosition, GetSize());
    /*Window* m_splitter =*/ new Window(object_reg, frame);
    frame->Show(true);
  }
}

// ----------------------------------------------------------------------------

BitmapComboBox::BitmapComboBox (iObjectRegistry* obj_reg, wxWindow* parent, ViewControl* ctrl)
  : wxBitmapComboBox(parent, wxID_ANY, wxEmptyString,wxDefaultPosition, wxSize(45, 20),0, NULL, wxCB_READONLY),
  object_reg(obj_reg), control(ctrl)
{
  csRef<iSpaceManager> mgr = csQueryRegistry<iSpaceManager> (object_reg);
  bool instanced = false;
  csHash<csRef<iSpaceFactory>, csString>::ConstGlobalIterator spaces = mgr->GetAll().GetIterator ();
  size_t i = 0;
  while (spaces.HasNext())
  {
    i++;
    iSpaceFactory* f = spaces.Next();
    wxString label(f->GetLabel(), wxConvUTF8);
    Append(label, f->GetIcon());
    if (instanced) continue;
    if (f->AllowMultiple() || f->GetCount() == 0)
    {
      printf("BitmapComboBox::BitmapComboBox Creating %s\n", f->GetLabel());
      ctrl->space = f->Create(control);
      SetSelection(i-1);
      instanced = true;
    }
  }
  Connect(GetId(), wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(BitmapComboBox::OnSelected), 0, this);
  
}

BitmapComboBox::~BitmapComboBox ()
{
}

void BitmapComboBox::OnSelected (wxCommandEvent& event)
{
  printf("BitmapComboBox::OnSelected %s\n", (const char*)GetValue().mb_str(wxConvUTF8));
  csRef<iSpaceManager> mgr = csQueryRegistry<iSpaceManager> (object_reg);
  csHash<csRef<iSpaceFactory>, csString>::ConstGlobalIterator spaces = mgr->GetAll().GetIterator ();
  size_t i = 0;
  while (spaces.HasNext())
  {
    i++;
    iSpaceFactory* f = spaces.Next();
    wxString label(f->GetLabel(), wxConvUTF8);
    if (GetValue() == label)
    {
      if (f->AllowMultiple() || f->GetCount() == 0)
      {
        printf("BitmapComboBox::BitmapComboBox Creating %s\n", f->GetLabel());
        control->box->Remove(control->space->GetWindow(""));
        control->space = f->Create(control);
        control->box->Insert(0, control->space->GetWindow(""), 1, wxEXPAND);
        control->box->Layout();
        SetSelection(i-1);
      }
      else
      {
        printf("BitmapComboBox::OnSelected FAILED\n");
      }
      break;
    }
  }
}

// ----------------------------------------------------------------------------

BEGIN_EVENT_TABLE(MenuEntry, wxButton)
  EVT_BUTTON(wxID_ANY, MenuEntry::OnClicked)
END_EVENT_TABLE()

MenuEntry::MenuEntry (iObjectRegistry* obj_reg, wxWindow* parent, const wxString& label)
  : wxButton(parent, wxID_ANY, label, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxBU_EXACTFIT), 
  object_reg (obj_reg)
{
}

MenuEntry::~MenuEntry ()
{
}

void MenuEntry::OnClicked (wxCommandEvent& event)
{
  printf("MenuEntry::OnClicked %d\n", event.GetId());
  
  wxMenu* editMenu = new wxMenu ();
  editMenu->Append (wxID_ANY, wxT("&Undo\tCtrl+Z"));
  editMenu->Append (wxID_ANY, wxT("&Redo\tCtrl+Y"));
  
  PopupMenu(editMenu);
}

// ----------------------------------------------------------------------------

} // namespace EditorApp
} // namespace CS
