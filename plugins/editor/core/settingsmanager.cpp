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

#include "settingsmanager.h"

#include "mainframe.h"

#include <wx/colordlg.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/textdlg.h>
#include <wx/stattext.h>


namespace CS {
namespace EditorApp {

  SettingsManager::SettingsManager (iObjectRegistry* obj_reg, wxWindow* parent)
  : scfImplementationType (this), object_reg (obj_reg), parent(parent)
{
  object_reg->Register (this, "iSettingsManager");
  preferencesListener.AttachNew (new PreferencesListener(this));
  
  csRef<iMenuBar> menuBar = csQueryRegistry<iMenuBar> (object_reg);

  settingsMenu = menuBar->Append("&Settings");
  preferences = settingsMenu->AppendItem("&Preferences");
  preferences->AddListener(preferencesListener);
}

SettingsManager::~SettingsManager ()
{
  object_reg->Unregister (this, "iPanelManager");
}

void SettingsManager::Uninitialize ()
{
}

void SettingsManager::Initialize ()
{
}

const wxChar* ITEMS[][2] =
{
    { _T("Cat"), _T("meow") },
    { _T("Cow"), _T("moo") },
    { _T("Crow"), _T("caw") },
    { _T("Dog"), _T("woof") },
    { _T("Duck"), _T("quack") },
    { _T("Mouse"), _T("squeak") },
    { _T("Owl"), _T("hoo") },
    { _T("Pig"), _T("oink") },
    { _T("Pigeon"), _T("coo") },
    { _T("Sheep"), _T("baaah") },
};



SettingsDialog::SettingsDialog(wxWindow *parent, const wxChar *title)
             : wxDialog(parent, wxID_ANY, wxString(title)), m_listCtrl(0)
{
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxNotebook *notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBK_LEFT);
    topsizer->Add( notebook, 1, wxGROW );

    wxButton *button = new wxButton( this, wxID_OK, _T("OK") );
    topsizer->Add( button, 0, wxALIGN_RIGHT | wxALL, 10 );

    wxPanel *panel = new wxPanel( notebook, wxID_ANY );
    notebook->AddPage( panel, _T("Keybindings") );
    
    wxPanel *panel2 = new wxPanel( notebook, wxID_ANY );
    notebook->AddPage( panel2, _T("Proxy") );

    wxSizer *panelsizer = new wxBoxSizer( wxVERTICAL );
    
    bindings[0][0] = "Select all";
    bindings[0][1] = "<ctrl>-A";
    
    
		m_listCtrl = new KeyBindingsList(bindings, panel, wxID_ANY, wxDefaultPosition, wxSize(450, 550),
									wxLC_REPORT | wxLC_VIRTUAL |wxSUNKEN_BORDER | wxLC_EDIT_LABELS | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
		panelsizer->Add( m_listCtrl, 1, wxGROW|wxALL, 0 );

		m_listCtrl->InsertColumn(0, _T("Action"));
		m_listCtrl->InsertColumn(1, _T("Shortcut"));
		m_listCtrl->SetItemCount(bindings.size());
		
		wxButton *change = new wxButton( panel, wxID_ANY, _T("Change") );
    panelsizer->Add(change, 0, wxALIGN_RIGHT | wxALL, 10 );
		
		panel->Connect(change->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SettingsDialog::OnEdit), 0, this);


		panel->SetAutoLayout( true );
		panel->SetSizer( panelsizer );
		SetSizer( topsizer );
		topsizer->SetSizeHints( this );
 
}

void SettingsDialog::OnEdit(wxCommandEvent& WXUNUSED(event))
{
	long itemCur = m_listCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
	if ( itemCur != -1 )
	{
		//m_listCtrl->EditLabel(itemCur);
		KeyBindingDialog dialog(this, m_listCtrl, bindings[itemCur][0], bindings[itemCur][1]);
		dialog.ShowModal();
	}
}

wxString KeyBindingsList::OnGetItemText(long item, long column) const
{
    if ( GetItemCount() == bindings.size())
    {
        return wxString(bindings[item][column].c_str(), wxConvUTF8);
    }
    else
    {
        return wxString::Format(_T("Column %ld of item %ld"), column, item);
    }
}

KeyBindingDialog::KeyBindingDialog(wxWindow *parent, KeyBindingsList* m_listCtrl, const std::string& action, std::string& shortcut)
             : wxDialog(parent, wxID_ANY, _T("Grab key")), m_listCtrl(m_listCtrl), binding(0), shortcut(shortcut)
{
   wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );   
   wxString string = wxString::Format(_T("Press the combination of keys you which to use for '%s'"), wxString(action.c_str(), wxConvUTF8).c_str());
   
   wxStaticText* text = new wxStaticText(this, wxID_ANY, string);
   topsizer->Add(text, 1, wxEXPAND | wxALL, 10 );
   
   binding = new wxStaticText(this, wxID_ANY, wxEmptyString);
   topsizer->Add(binding, 1, wxALIGN_CENTER | wxALL, 10 );
   
   wxButton *button = new wxButton( this, wxID_OK, _T("OK") );
   topsizer->Add( button, 0, wxALIGN_RIGHT | wxALL, 10 );
    
   SetSizer( topsizer );
	 topsizer->SetSizeHints( this );
	 topsizer->SetMinSize(text->GetBestSize());
	 
	 this->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(KeyBindingDialog::OnKeyDown), 0, this);
	 text->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(KeyBindingDialog::OnKeyDown), 0, this);
	 binding->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(KeyBindingDialog::OnKeyDown), 0, this);
	 button->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(KeyBindingDialog::OnKeyDown), 0, this);
	 this->SetFocus();
}

void KeyBindingDialog::OnKeyDown(wxKeyEvent& event) 
{
	std::string string;
	if(event.ControlDown())
	{
		string += "<ctrl>-";
	}
	if(event.ShiftDown())
	{
		string += "<shift>-";
	}
	string += toupper(event.GetKeyCode());
	shortcut = string;
	binding->SetLabel(wxString(string.c_str(), wxConvUTF8));
	m_listCtrl->Refresh();
}


void SettingsManager::PreferencesListener::OnClick (iMenuItem* item)
{
	SettingsDialog dialog(mgr->parent, _T("Notebook Sizer Test Dialog") );
	dialog.ShowModal();
}


} // namespace EditorApp
} // namespace CS
