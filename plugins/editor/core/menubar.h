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

#ifndef __MENUBAR_H__
#define __MENUBAR_H__

#include <csutil/refarr.h>
#include <csutil/hash.h>

#include <wx/aui/aui.h>
#include <wx/menu.h>

#include <map>

#include "ieditor/menu.h"
#include "ieditor/operator.h"

namespace CS {
namespace EditorApp {
  
class MenuBar;

class MenuItem : public scfImplementation1<MenuItem,iMenuItem>, public wxEvtHandler
{
public:
  MenuItem (MenuBar* menuBar, wxMenu* menu, wxMenuItem* item);
  virtual ~MenuItem ();
  
  virtual wxMenuItem* GetwxMenuItem () const;
  
  virtual void AddListener (iMenuItemEventListener*);
  virtual void RemoveListener (iMenuItemEventListener*);
  
private:
  void OnToggle (wxCommandEvent& event);
  MenuBar* menuBar;
  wxMenu* menu;
  wxMenuItem* item;
  
  csRefArray<iMenuItemEventListener> listeners;
};

class Menu : public scfImplementation1<Menu,iMenu2>, public wxEvtHandler
{
public:
  Menu (MenuBar* menuBar, wxMenu* menu, const wxString& title);
  virtual ~Menu ();
  
  virtual wxMenu* GetwxMenu () const;
  
  virtual csPtr<iMenuItem> AppendItem (const char* item);
  virtual csPtr<iMenuItem> AppendSeparator ();
  virtual csPtr<iMenu2> AppendSubMenu (const char* item);

private:
  MenuBar* menuBar;
  wxMenu* menu;
  wxString title;
};


class MenuBar : public scfImplementation1<MenuBar,iMenuBar>, public wxEvtHandler
{
public:
  MenuBar (iObjectRegistry* obj_reg, wxMenuBar* menuBar);
  virtual ~MenuBar ();
  
  virtual wxMenuBar* GetwxMenuBar () const;
  
  virtual csPtr<iMenu2> Append (const char* item);

private:
  iObjectRegistry* object_reg;
  wxMenuBar* menuBar;
  friend class MenuOperatorItem;
};

} // namespace EditorApp
} // namespace CS

#endif
