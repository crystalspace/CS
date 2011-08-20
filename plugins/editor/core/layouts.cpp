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
#include <iutil/plugin.h>


#include "layouts.h"

#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/button.h>
#include <wx/artprov.h>
#include <wx/stattext.h>

namespace CS {
namespace EditorApp {

BaseLayout::BaseLayout(iObjectRegistry* obj_reg)
  : scfImplementationType (this), object_reg (obj_reg)
{
}

void BaseLayout::OnOperator (wxCommandEvent& event)
{
  printf("HeaderLayout::OnOperator %d\n", event.GetId());
  csRef<iOperator> op = operators.Get(event.GetId(), csRef<iOperator>());
  if (op)
  {
    csRef<iOperatorManager> operatorManager = csQueryRegistry<iOperatorManager> (object_reg);
    operatorManager->Execute(op);
  }
}

void BaseLayout::OnMenu (wxCommandEvent& event)
{
  printf("HeaderLayout::OnMenu %d\n", event.GetId());
  
  
  csRef<iMenu> menu = menus.Get(event.GetId(), csRef<iMenu>());
  if (menu)
  {
    csRef<iContext> context = csQueryRegistry<iContext> (object_reg);
    if (menu->Poll(context))
    {
      wxMenu wx;
      csRef<iLayout> layout;
      layout.AttachNew(new MenuLayout(object_reg, GetWindow(), &wx));
      menu->Draw(context, layout);
      GetWindow()->PopupMenu(&wx);
    }
  }
}

iOperator* BaseLayout::GetOperator (const char* id)
{
  csRef<iOperatorManager> operatorManager = csQueryRegistry<iOperatorManager> (object_reg);
  csRef<iOperator> op = operatorManager->Create(id);
  return op;
}

iMenu* BaseLayout::GetMenu (const char* id)
{
  csRef<iPluginManager> plugmgr = csQueryRegistry<iPluginManager> (object_reg);
  csRef<iComponent> comp = plugmgr->QueryPluginInstance(id);
  if (!comp)
  {
    comp = plugmgr->LoadPluginInstance (id, iPluginManager::lpiInitialize | iPluginManager::lpiReportErrors| iPluginManager::lpiLoadDependencies);
  }
  csRef<iMenu> menu = scfQueryInterfaceSafe<iMenu> (comp);
  if (!menu)
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR, "crystalspace.managers.space", "Not of type iMenu '%s'", id);
    return 0;
  }
  return menu;
}

//----------------------------------------------------------------------
HeaderLayout::HeaderLayout(iObjectRegistry* obj_reg, wxWindow* parent)
  : BaseLayout (obj_reg)
{
  box = new wxBoxSizer(wxVERTICAL);
  tb = new wxToolBar(parent, wxID_ANY);
  tb->Realize();
  box->Add(tb, 1, wxEXPAND);
  parent->SetSizer(box, true);
  box->SetSizeHints(parent);
}

HeaderLayout::~HeaderLayout()
{
  //tb->Destroy();
  box->Clear(true);
}

iOperator* HeaderLayout::AppendOperator(const char* id, const char* label, const char* icon) 
{ 
  wxString l(label, wxConvUTF8);
  wxToolBarToolBase* item = tb->AddTool(wxID_ANY, l, wxArtProvider::GetBitmap(wxART_ERROR));
  tb->Realize();
  csRef<iOperator> op = GetOperator(id);
  operators.PutUnique(item->GetId(), op);
  tb->Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(HeaderLayout::OnOperator), 0, this);
  return op; 
}

iMenu* HeaderLayout::AppendMenu(const char* id, const char* label)
{
  wxString l(label, wxConvUTF8);
  wxButton* entry = new wxButton(tb, wxID_ANY, l, wxDefaultPosition, wxDefaultSize, wxNO_BORDER /*| wxBU_EXACTFIT*/);
  tb->AddControl(entry);
  tb->Realize();

  csRef<iMenu> menu = GetMenu(id);

  menus.PutUnique(entry->GetId(), menu);
  tb->Connect(entry->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(HeaderLayout::OnMenu), 0, this);
  return menu;
}

iProperty* HeaderLayout::AppendProperty(iResource*, const char* id) { return 0; }
void HeaderLayout::AppendLabel(const char* label) {}
void HeaderLayout::AppendSeperator() 
{
  tb->AddSeparator();
  tb->Realize();
}
iLayout* HeaderLayout::Row() { return 0; }
iLayout* HeaderLayout::Column() { return 0; }


wxWindow* HeaderLayout::GetWindow() 
{
  return tb;
}

//----------------------------------------------------------------------

MenuLayout::MenuLayout(iObjectRegistry* obj_reg, wxWindow* parent, wxMenu* menu)
  : BaseLayout (obj_reg),  parent(parent), menu(menu)
{
}

MenuLayout::~MenuLayout()
{
}

iOperator* MenuLayout::AppendOperator(const char* id, const char* label, const char* icon) 
{ 
  wxString l(label, wxConvUTF8);
  wxMenuItem* item = menu->Append(wxID_ANY, l);
  csRef<iOperator> op = GetOperator(id);
  parent->Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MenuLayout::OnOperator), 0, this);
  operators.PutUnique(item->GetId(), op);
  return op; 
}

iMenu* MenuLayout::AppendMenu(const char* id, const char* label)
{
  return 0;
}

iProperty* MenuLayout::AppendProperty(iResource*, const char* id) { return 0; }
void MenuLayout::AppendLabel(const char* label) {}
void MenuLayout::AppendSeperator() 
{
  menu->AppendSeparator();
}
iLayout* MenuLayout::Row() { return 0; }
iLayout* MenuLayout::Column() { return 0; }

wxWindow* MenuLayout::GetWindow() 
{
  return parent;
}

//----------------------------------------------------------------------

PanelLayout::PanelLayout(iObjectRegistry* obj_reg, wxWindow* parent)
  : BaseLayout (obj_reg),  parent(parent)
{
  paneSz = new wxBoxSizer(wxVERTICAL);

  parent->SetSizer(paneSz);
  paneSz->SetSizeHints(parent);
}

PanelLayout::~PanelLayout()
{
  paneSz->Clear(true);
}

iOperator* PanelLayout::AppendOperator(const char* id, const char* label, const char* icon) 
{ 
  wxString l(label, wxConvUTF8);
  wxButton* item = new wxButton(parent, wxID_ANY, l, wxDefaultPosition, wxDefaultSize/*, wxNO_BORDER | wxBU_EXACTFIT*/);
  csRef<iOperator> op = GetOperator(id);
  parent->Connect(item->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MenuLayout::OnOperator), 0, this);
  operators.PutUnique(item->GetId(), op);
  
  paneSz->Add(item, 0, wxEXPAND, 0);
  
  return op; 
}

iMenu* PanelLayout::AppendMenu(const char* id, const char* label)
{
  return 0;
}

iProperty* PanelLayout::AppendProperty(iResource*, const char* id) { return 0; }
void PanelLayout::AppendLabel(const char* label) 
{
  wxString l(label, wxConvUTF8);
  paneSz->Add(new wxStaticText(parent, wxID_ANY, l), 1, wxEXPAND, 0);
}

void PanelLayout::AppendSeperator() {}
iLayout* PanelLayout::Row() { return 0; }
iLayout* PanelLayout::Column() { return 0; }

wxWindow* PanelLayout::GetWindow() 
{
  return parent;
}
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(CollapsiblePane, wxCollapsiblePane)
  EVT_SIZE(CollapsiblePane::OnSize)
  EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY, CollapsiblePane::OnChanged)
END_EVENT_TABLE()

CollapsiblePane::CollapsiblePane (iObjectRegistry* obj_reg, wxWindow* parent, const char* label)
  : wxCollapsiblePane(parent, wxID_ANY, wxString(label, wxConvUTF8), wxDefaultPosition, wxDefaultSize, wxCP_NO_TLW_RESIZE),
  object_reg(obj_reg)
{

}

CollapsiblePane::~CollapsiblePane()
{
}

void CollapsiblePane::OnSize (wxSizeEvent& event)
{
  printf("CollapsiblePane::OnSize %d - %d\n", event.GetSize().GetWidth(), event.GetSize().GetHeight());
  //SetSize (event.GetSize());
  Layout();
  event.Skip();
}

void CollapsiblePane::OnChanged (wxCollapsiblePaneEvent& event)
{
  printf("CollapsiblePane::OnChanged\n");
  GetParent()->Layout();
  event.Skip();
}

//----------------------------------------------------------------------

} // namespace EditorApp
} // namespace CS
