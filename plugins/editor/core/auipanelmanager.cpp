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

#include "cssysdef.h"
#include "csutil/scf.h"

#include <csutil/objreg.h>

#include "auipanelmanager.h"

#include "mainframe.h"

#include <wx/colordlg.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/textdlg.h>


namespace CS {
namespace EditorApp {

  AUIPanelManager::AUIPanelManager (iObjectRegistry* obj_reg, wxWindow* parent)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iPanelManager");
  panelListener.AttachNew (new PanelListener(this));
  onCreatePerspective.AttachNew (new OnCreatePerspective(this));
  onRestorePerspective.AttachNew (new OnRestorePerspective(this));

  mgr.SetManagedWindow (parent);
  
  csRef<iMenuBar> menuBar = csQueryRegistry<iMenuBar> (object_reg);

  perspectivesMenu = menuBar->Append("&View");
  panelsMenu = perspectivesMenu->AppendSubMenu("&Panels");
  csRef<iMenuItem> separator = perspectivesMenu->AppendSeparator();
  separatorItems.Push (separator);
}

AUIPanelManager::~AUIPanelManager ()
{
  object_reg->Unregister (this, "iPanelManager");
}

void AUIPanelManager::Uninitialize ()
{
  mgr.UnInit ();
}

void AUIPanelManager::Initialize ()
{
  createPerspective = perspectivesMenu->AppendItem("&Create Perspective");
  createPerspective->AddListener(onCreatePerspective);

  if (m_perspectives.size() == 0)
  {
    csRef<iMenuItem> separator = perspectivesMenu->AppendSeparator();
    separatorItems.Push (separator);
  }

  {//Create default perspective, all panels in their default state at this point.
    csRef<iMenuItem> item = perspectivesMenu->AppendItem("&Default");
    item->AddListener(onRestorePerspective);
    perspectiveItems.Push(item);
    m_perspectives[item->GetwxMenuItem ()->GetId()] = mgr.SavePerspective();
  }
  
  {// Let's hide the Scene and Asset panels to auto create the Terrain perspective.
    for (size_t i = 0; i < panels.GetSize(); i++)
    {
      if (wxString(panels.Get(i)->GetCaption ()) == wxT("Scene Browser") || wxString(panels.Get(i)->GetCaption ()) == wxT("Asset Browser"))
      {
        wxAuiPaneInfo& info = mgr.GetPane (panels.Get(i)->GetWindow ());
        info.Hide();
      }
    }
    mgr.Update ();
    
    csRef<iMenuItem> item = perspectivesMenu->AppendItem("&Terrain");
    item->AddListener(onRestorePerspective);
    perspectiveItems.Push(item);
    m_perspectives[item->GetwxMenuItem ()->GetId()] = mgr.SavePerspective();
    
    for (size_t i = 0; i < panels.GetSize(); i++)
    {
      wxAuiPaneInfo& info = mgr.GetPane (panels.Get(i)->GetWindow ());
      info.Show();
    }
    mgr.Update ();
  }
}

void AUIPanelManager::AddPanel (iPanel* panel)
{
  wxAuiPaneInfo info;
  info.Name(panel->GetCaption ());
  info.Direction(panel->GetDefaultDockPosition());
  info.Caption(panel->GetCaption ());
  
  mgr.AddPane (panel->GetWindow (), info);

  mgr.Update ();
  
  csRef<iMenuCheckItem> item = panelsMenu->AppendCheckItem(wxString(panel->GetCaption ()).mb_str());
  item->Check (true);
  item->AddListener(panelListener);

  mgr.GetManagedWindow()->Connect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(AUIPanelManager::OnClose), 0, this);

  panels.Push (panel);
  items.Push (item);
}

void AUIPanelManager::RemovePanel (iPanel* panel)
{
  mgr.DetachPane (panel->GetWindow ());
  mgr.Update ();

  panels.Delete (panel);
  size_t found = panels.Find(panel);
  if (found != csArrayItemNotFound)
  {
      panels.DeleteIndex (found);
      items.DeleteIndex (found);
  }
}

void AUIPanelManager::SetPanelVisible (iPanel* panel, bool visible)
{
  wxAuiPaneInfo& info = mgr.GetPane (panel->GetWindow ());
 
  if (info.IsOk ())
  { 
    if (visible)
      info.Show ();
    else
      info.Hide ();
  }
  
  size_t found = panels.Find(panel);
  if (found == csArrayItemNotFound) return;
  
  iMenuCheckItem* m = items.Get(found);
  if (m) m->Check (visible);
  
  mgr.Update ();
}

void AUIPanelManager::OnClose (wxAuiManagerEvent& event)
{
  for (size_t i = 0; i < panels.GetSize(); i++)
  {
    if (panels.Get(i)->GetWindow () == event.GetPane()->window)
      SetPanelVisible(panels.Get(i), false);
  }
}

void AUIPanelManager::PanelListener::OnClick (iMenuItem* item)
{
  iMenuCheckItem* i = static_cast<iMenuCheckItem*>(item);
  size_t found = mgr->items.Find(i);
  if (found != csArrayItemNotFound)
  {
      mgr->SetPanelVisible(mgr->panels.Get(found), (static_cast<iMenuCheckItem*>(item))->IsChecked());
  }
}

void AUIPanelManager::OnCreatePerspective::OnClick(iMenuItem*)
{
    wxTextEntryDialog dlg(mgr->mgr.GetManagedWindow (), wxT("Enter a name for the new perspective:"), wxT("wxAUI Test"));

    dlg.SetValue(wxString::Format(wxT("Perspective %u"), unsigned(mgr->m_perspectives.size() + 1)));
    if (dlg.ShowModal() != wxID_OK)
        return;

    if (mgr->m_perspectives.size() == 0)
    {
	csRef<iMenuItem> separator = mgr->perspectivesMenu->AppendSeparator();
	mgr->separatorItems.Push (separator);
    }
    
    csRef<iMenuItem> item = mgr->perspectivesMenu->AppendItem(dlg.GetValue().mb_str());
    item->AddListener(mgr->onRestorePerspective);
    mgr->perspectiveItems.Push(item);
    mgr->m_perspectives[item->GetwxMenuItem ()->GetId()] = mgr->mgr.SavePerspective();
}

void AUIPanelManager::OnRestorePerspective::OnClick(iMenuItem* item)
{
    mgr->mgr.LoadPerspective(mgr->m_perspectives.find(item->GetwxMenuItem ()->GetId())->second);
    
    for (size_t i = 0; i < mgr->panels.GetSize(); i++)
    {
      const wxAuiPaneInfo& info = mgr->mgr.GetPane (mgr->panels.Get(i)->GetWindow ());
      iMenuCheckItem* m = mgr->items.Get(i);
      if (m) m->Check (info.IsShown());
    }
}

} // namespace EditorApp
} // namespace CS
