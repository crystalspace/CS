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

#ifndef __SETINGSMANAGER_H__
#define __SETINGSMANAGER_H__

#include <csutil/refarr.h>
#include <csutil/hash.h>

#include <wx/aui/aui.h>
#include <wx/menu.h>
#include <wx/dialog.h>

#include <wx/listctrl.h>

#include <map>

#include "ieditor/settingsmanager.h"

#include "ieditor/menubar.h"

struct wxStaticText;

namespace CS {
namespace EditorApp {
	
typedef std::string Binding;
typedef std::map<long, std::map<long, Binding> > Bindings;

class SettingsManager : public scfImplementation1<SettingsManager,iSettingsManager>, public wxDialog
{
public:
  SettingsManager (iObjectRegistry* obj_reg, wxWindow* parent);
  virtual ~SettingsManager ();
  
  virtual void Initialize ();
  virtual void Uninitialize ();

private:
  iObjectRegistry* object_reg;
  wxWindow* parent;
  csRef<iMenu> settingsMenu;
  csRef<iMenuItem> preferences;

private:  
  //I WANT boost::bind DAMN IT!
  struct PreferencesListener : public scfImplementation1<PreferencesListener,iMenuItemEventListener>
   {
     SettingsManager* mgr;
     PreferencesListener(SettingsManager* mgr) : scfImplementationType (this), mgr(mgr) {}
     virtual void OnClick (iMenuItem* item);
   };
   friend struct PreferencesListener;
   csRef<PreferencesListener> preferencesListener;
};

class KeyBindingsList: public wxListCtrl
{
public:
	KeyBindingsList(Bindings& bindings, 
						 wxWindow *parent,
						 const wxWindowID id,
						 const wxPoint& pos,
						 const wxSize& size,
						 long style)
			: wxListCtrl(parent, id, pos, size, style), bindings(bindings)
			{
			}
private:
	virtual wxString OnGetItemText(long item, long column) const;
	Bindings& bindings;
};

class SettingsDialog : public wxDialog
{
public:
  SettingsDialog(wxWindow *parent, const wxChar *title);
private:
	Bindings bindings;
	void OnEdit(wxCommandEvent& event);
	KeyBindingsList* m_listCtrl;
};

class KeyBindingDialog : public wxDialog
{
public:
  KeyBindingDialog(wxWindow *parent, KeyBindingsList* m_listCtrl, const std::string& action, std::string& shortcut);
private:
	void OnKeyDown(wxKeyEvent& e);
	KeyBindingsList* m_listCtrl;
	wxStaticText* binding;
	std::string& shortcut;
};



} // namespace EditorApp
} // namespace CS

#endif
