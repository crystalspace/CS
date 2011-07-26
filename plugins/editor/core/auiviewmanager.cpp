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

#include "auiviewmanager.h"

#include "mainframe.h"

#include <wx/colordlg.h>
#include <wx/msgdlg.h>
#include <wx/button.h>
#include <wx/textdlg.h>


namespace CS {
namespace EditorApp {

  AUIViewManager::AUIViewManager (iObjectRegistry* obj_reg, wxWindow* parent)
  : scfImplementationType (this), object_reg (obj_reg)
{
  object_reg->Register (this, "iViewManager");
  viewListener.AttachNew (new ViewListener(this));
  onCreatePerspective.AttachNew (new OnCreatePerspective(this));
  onRestorePerspective.AttachNew (new OnRestorePerspective(this));

  mgr.SetManagedWindow (parent);
  
  csRef<iMenuBar> menuBar = csQueryRegistry<iMenuBar> (object_reg);

  perspectivesMenu = menuBar->Append("&View");
  viewsMenu = perspectivesMenu->AppendSubMenu("&Views");
  csRef<iMenuItem> separator = perspectivesMenu->AppendSeparator();
  separatorItems.Push (separator);
}

AUIViewManager::~AUIViewManager ()
{
  object_reg->Unregister (this, "iViewManager");
}

void AUIViewManager::Uninitialize ()
{
  mgr.UnInit ();
}

void AUIViewManager::Initialize ()
{
  testOperator = perspectivesMenu->AppendOperator("cs.editor.operator.test");


  createPerspective = perspectivesMenu->AppendItem("&Create Perspective");
  createPerspective->AddListener(onCreatePerspective);

  if (m_perspectives.size() == 0)
  {
    csRef<iMenuItem> separator = perspectivesMenu->AppendSeparator();
    separatorItems.Push (separator);
  }

  {//Create default perspective, all views in their default state at this point.
    csRef<iMenuItem> item = perspectivesMenu->AppendItem("&Default");
    item->AddListener(onRestorePerspective);
    perspectiveItems.Push(item);
    m_perspectives[item->GetwxMenuItem ()->GetId()] = mgr.SavePerspective();
  }
  
  {// Let's hide the Scene and Asset views to auto create the Terrain perspective.
    for (size_t i = 0; i < views.GetSize(); i++)
    {
      if (wxString(views.Get(i)->GetCaption ()) == wxT("Scene Browser") || wxString(views.Get(i)->GetCaption ()) == wxT("Asset Browser"))
      {
        wxAuiPaneInfo& info = mgr.GetPane (views.Get(i)->GetWindow ());
        info.Hide();
      }
    }
    mgr.Update ();
    
    csRef<iMenuItem> item = perspectivesMenu->AppendItem("&Terrain");
    item->AddListener(onRestorePerspective);
    perspectiveItems.Push(item);
    m_perspectives[item->GetwxMenuItem ()->GetId()] = mgr.SavePerspective();
    
    for (size_t i = 0; i < views.GetSize(); i++)
    {
      wxAuiPaneInfo& info = mgr.GetPane (views.Get(i)->GetWindow ());
      info.Show();
    }
    mgr.Update ();
  }
}

void AUIViewManager::AddView (iView* view)
{
  wxAuiPaneInfo info;
  info.Name(view->GetCaption ());
  info.Direction(view->GetDefaultDockPosition());
  info.Caption(view->GetCaption ());
  
  mgr.AddPane (view->GetWindow (), info);

  mgr.Update ();
  
  csRef<iMenuCheckItem> item = viewsMenu->AppendCheckItem(wxString(view->GetCaption ()).mb_str());
  item->Check (true);
  item->AddListener(viewListener);

  mgr.GetManagedWindow()->Connect(wxEVT_AUI_PANE_CLOSE, wxAuiManagerEventHandler(AUIViewManager::OnClose), 0, this);

  views.Push (view);
  items.Push (item);
}

void AUIViewManager::RemoveView (iView* view)
{
  mgr.DetachPane (view->GetWindow ());
  mgr.Update ();

  views.Delete (view);
  size_t found = views.Find(view);
  if (found != csArrayItemNotFound)
  {
    views.DeleteIndex (found);
    items.DeleteIndex (found);
  }
}

void AUIViewManager::SetViewVisible (iView* view, bool visible)
{
  wxAuiPaneInfo& info = mgr.GetPane (view->GetWindow ());
 
  if (info.IsOk ())
  { 
    if (visible)
      info.Show ();
    else
      info.Hide ();
  }
  
  size_t found = views.Find(view);
  if (found == csArrayItemNotFound) return;
  
  iMenuCheckItem* m = items.Get(found);
  if (m) m->Check (visible);
  
  mgr.Update ();
}

void AUIViewManager::OnClose (wxAuiManagerEvent& event)
{
  for (size_t i = 0; i < views.GetSize(); i++)
  {
    if (views.Get(i)->GetWindow () == event.GetPane()->window)
      SetViewVisible(views.Get(i), false);
  }
}

void AUIViewManager::ViewListener::OnClick (iMenuItem* item)
{
  iMenuCheckItem* i = static_cast<iMenuCheckItem*>(item);
  size_t found = mgr->items.Find(i);
  if (found != csArrayItemNotFound)
  {
      mgr->SetViewVisible(mgr->views.Get(found), (static_cast<iMenuCheckItem*>(item))->IsChecked());
  }
}

void AUIViewManager::OnCreatePerspective::OnClick(iMenuItem*)
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

void AUIViewManager::OnRestorePerspective::OnClick(iMenuItem* item)
{
  mgr->mgr.LoadPerspective(mgr->m_perspectives.find(item->GetwxMenuItem ()->GetId())->second);
  
  for (size_t i = 0; i < mgr->views.GetSize(); i++)
  {
    const wxAuiPaneInfo& info = mgr->mgr.GetPane (mgr->views.Get(i)->GetWindow ());
    iMenuCheckItem* m = mgr->items.Get(i);
    if (m) m->Check (info.IsShown());
  }
}

} // namespace EditorApp
} // namespace CS
