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



#include "menubar.h"

#include <wx/menu.h>


namespace CS {
namespace EditorApp {


MenuItem::MenuItem (MenuBar* menuBar, wxMenu* menu, wxMenuItem* item)
  : scfImplementationType (this), menuBar(menuBar), menu(menu), item(item)
{
  menuBar->GetwxMenuBar()->GetParent()->Connect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MenuItem::OnToggle), 0, this);
}

MenuItem::~MenuItem ()
{
  menuBar->GetwxMenuBar()->GetParent()->Disconnect(item->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MenuItem::OnToggle), 0, this); 
  menu->Remove(item);
  delete item;
}

void MenuItem::OnToggle (wxCommandEvent& event)
{
  for (size_t i = 0; i < listeners.GetSize(); i++)
    listeners.Get(i)->OnClick(this);
}

wxMenuItem* MenuItem::GetwxMenuItem () const
{
  return item;
}

void MenuItem::AddListener (iMenuItemEventListener* l)
{
  listeners.Push(l);
}

void MenuItem::RemoveListener (iMenuItemEventListener* l)
{
  listeners.Delete(l);
}

//---------------------------------------------------------------

Menu::Menu (MenuBar* menuBar, wxMenu* menu, const wxString& title)
  : scfImplementationType (this), menuBar(menuBar), menu(menu), title(title)
{
}

Menu::~Menu ()
{
  int pos = menuBar->GetwxMenuBar()->FindMenu(title);
  if (pos != wxNOT_FOUND) menuBar->GetwxMenuBar()->Remove(pos);
  delete menu;
}

wxMenu* Menu::GetwxMenu () const
{
  return menu;
}

csPtr<iMenuItem> Menu::AppendItem (const char* item)
{
  wxString str(item, wxConvUTF8);
  wxMenuItem* i = menu->Append(wxID_ANY, str);

  csRef<iMenuItem> ref;
  ref.AttachNew (new MenuItem (menuBar, menu, i));

  return csPtr<iMenuItem> (ref);
}

csPtr<iMenuItem> Menu::AppendSeparator ()
{
  wxMenuItem* i = menu->AppendSeparator();

  csRef<iMenuItem> ref;
  ref.AttachNew (new MenuItem (menuBar, menu, i));

  return csPtr<iMenuItem> (ref);
}

csPtr<iMenu2> Menu::AppendSubMenu (const char* item)
{
  wxMenu* m = new wxMenu();
  wxString str(item, wxConvUTF8);
  menu->AppendSubMenu(m, str);

  csRef<iMenu2> ref;
  ref.AttachNew (new Menu (menuBar, m, str));

  return csPtr<iMenu2> (ref);
}

//---------------------------------------------------------------

MenuBar::MenuBar (iObjectRegistry* obj_reg, wxMenuBar* menuBar)
  : scfImplementationType (this), object_reg (obj_reg), menuBar (menuBar)
{
  object_reg->Register (this, "iMenuBar");
}

MenuBar::~MenuBar ()
{
  object_reg->Unregister (this, "iMenuBar");
}

wxMenuBar* MenuBar::GetwxMenuBar () const
{
  return menuBar;
}
  
csPtr<iMenu2> MenuBar::Append (const char* item)
{
  assert(menuBar);
  wxMenu* menu = new wxMenu();
  wxString str(item, wxConvUTF8);
  menuBar->Append(menu, str);

  csRef<iMenu2> ref;
  ref.AttachNew (new Menu (this, menu, str));

  return csPtr<iMenu2> (ref);
}

} // namespace EditorApp
} // namespace CS
