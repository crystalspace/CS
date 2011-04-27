/*
    Copyright (C) 2009 by Jelle Hellemans

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

#ifndef SOCKETTAB_H__
#define SOCKETTAB_H__

#include "tabbase.h"

#include <ivaria/reporter.h>

class SocketTab : public TabBase
{
private:
  bool AttachButton (const CEGUI::EventArgs& e);
  bool DetachButton (const CEGUI::EventArgs& e);
  bool AddSocket (const CEGUI::EventArgs& e);
  bool DelSocket (const CEGUI::EventArgs& e);
  bool SelSocket (const CEGUI::EventArgs& e);
  bool SaveSocket (const CEGUI::EventArgs& e);

  bool CreateProp(size_t i, const char* name, const char* type, const char* value);
  bool CreatePropString(size_t i, const char* name, const char* value);
  bool CreatePropVector3(size_t i, const char* name, const char* value);

  void UpdateSocketInfo(const SocketDescriptor& trans);
  SocketDescriptor GetSocketInfo();

public:
  SocketTab(iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~SocketTab();
};

//-------------------------------------------------------

SocketTab::SocketTab(iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass) 
{
  LoadLayout("sockettab.layout");
  AddToTabs();

  csRef<iStringArray> arr = asset->GetSockets();
  UpdateList(arr, "Sockets/List");
  if (arr->GetSize()) asset->SetSelectedSocket(arr->Get(0));
  UpdateSocketInfo(asset->GetSocketTransform(asset->GetSelectedSocket()));

  CEGUI::Window* btn = 0;

  btn = winMgr->getWindow("Sockets/AttachButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SocketTab::AttachButton, this));

  btn = winMgr->getWindow("Sockets/DetachButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SocketTab::DetachButton, this));

  btn = winMgr->getWindow("Sockets/AddSocket");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SocketTab::AddSocket, this));

  btn = winMgr->getWindow("Sockets/DelSocket");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SocketTab::DelSocket, this));

  btn = winMgr->getWindow("Sockets/List");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&SocketTab::SelSocket, this));

  btn = winMgr->getWindow("Sockets/EditSocket/SaveButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SocketTab::SaveSocket, this));
}

SocketTab::~SocketTab() 
{
}

bool SocketTab::CreateProp(size_t i, const char* name, const char* type, const char* value)
{
  if (!strcmp(type, "String"))
    return CreatePropString(i, name, value);
  else if (!strcmp(type, "Vector3"))
    return CreatePropVector3(i, name, value);
  else
    return false;
}

bool SocketTab::CreatePropString(size_t i, const char* name, const char* value)
{
  using namespace CEGUI;
  Window* edit = winMgr->getWindow("Sockets/EditSocket");

  Window* n = winMgr->createWindow("ice/StaticText");
  n->setText(name);
  n->setProperty("BackgroundEnabled", "False");
  n->setProperty("FrameEnabled", "False");
  n->setPosition( UVector2(UDim(0.0f, 5.0f),UDim(0.0f, 5.0f+(i*30.0f))) );
  n->setSize( UVector2(UDim(0.0f, 60.0f),UDim(0.0f, 20.0f)) );
  edit->addChildWindow(n);

  Window* v = winMgr->createWindow("ice/Editbox");
  v->setText(value);
  v->setPosition( UVector2(UDim(0.0f, 65.0f),UDim(0.0f, 5.0f+(i*30.0f))) );
  v->setSize( UVector2(UDim(1.0f, -75.0f),UDim(0.0f, 28.0f)) );
  v->setUserString("Name", name);
  v->setUserString("Type", "String");
  edit->addChildWindow(v);

  return true;
}

bool SocketTab::CreatePropVector3(size_t i, const char* name, const char* value)
{
  using namespace CEGUI;
  Window* edit = winMgr->getWindow("Sockets/EditSocket");

  Window* n = winMgr->createWindow("ice/StaticText");
  n->setText(name);
  n->setProperty("BackgroundEnabled", "False");
  n->setProperty("FrameEnabled", "False");
  n->setPosition( UVector2(UDim(0.0f, 5.0f),UDim(0.0f, 5.0f+(i*30.0f))) );
  n->setSize( UVector2(UDim(0.0f, 60.0f),UDim(0.0f, 20.0f)) );
  edit->addChildWindow(n);

  Window* v = winMgr->createWindow("ice/Editbox");
  v->setText(value);
  v->setPosition( UVector2(UDim(0.0f, 65.0f),UDim(0.0f, 5.0f+(i*30.0f))) );
  v->setSize( UVector2(UDim(1.0f, -75.0f),UDim(0.0f, 28.0f)) );
  v->setUserString("Name", name);
  v->setUserString("Type", "Vector3");
  edit->addChildWindow(v);

  return true;
}

void SocketTab::UpdateSocketInfo(const SocketDescriptor& trans)
{
  CEGUI::Window* parent = winMgr->getWindow("Sockets/EditSocket");

  // Cleanup old windows.
  CEGUI::Window* save = winMgr->getWindow("Sockets/EditSocket/SaveButton");
  parent->removeChildWindow(save);
  while (parent->getChildCount())
    winMgr->destroyWindow(parent->getChildAtIdx(0));
  parent->addChildWindow(save);

  // Create new.
  SocketDescriptor::const_iterator it = trans.begin();
  for (size_t i = 0; it != trans.end(); it++, i++)
  {
    CreateProp(i, it->first.c_str(), it->second.first.c_str(), it->second.second.c_str());
  }
}

SocketDescriptor SocketTab::GetSocketInfo()
{
  SocketDescriptor desc;

  CEGUI::Window* parent = winMgr->getWindow("Sockets/EditSocket");
  for (size_t i = 0; i < parent->getChildCount(); i++)
  {
    CEGUI::Window* child = parent->getChildAtIdx(i);
    if (child->getName() == "Sockets/EditSocket/SaveButton") continue;
    
    TypeValue typeValue;
    try
    {
      TypeValue typeValue(child->getUserString("Type").c_str(), child->getText().c_str());
      desc[child->getUserString("Name").c_str()] = typeValue;
    }
    catch (CEGUI::UnknownObjectException&)
    {
      continue;
    }
  }

  return desc;
}

bool SocketTab::AttachButton (const CEGUI::EventArgs& e)
{
  CEGUI::Window* form = winMgr->getWindow("Form");
  CEGUI::Window* stddlg = winMgr->getWindow("StdDlg");

  form->hide();
  stddlg->show();
  stddlg->setUserString("Purpose", "Attach");
  return true;
}

bool SocketTab::DetachButton (const CEGUI::EventArgs& e)
{
  asset->AttachMesh(asset->GetSelectedSocket(), 0);

  return true;
}

bool SocketTab::AddSocket (const CEGUI::EventArgs& e)
{
  csReporterHelper::Report(object_reg, CS_REPORTER_SEVERITY_WARNING,"", "Adding sockets is not yet implemented");

  asset->AddSocket("");

  csRef<iStringArray> arr = asset->GetSockets();
  UpdateList(arr, "Sockets/List");
  return true;
}

bool SocketTab::DelSocket (const CEGUI::EventArgs& e)
{
  csReporterHelper::Report(object_reg, CS_REPORTER_SEVERITY_WARNING,"", "Deleting sockets is not yet implemented");

  asset->DeleteSocket("");

  csRef<iStringArray> arr = asset->GetSockets();
  UpdateList(arr, "Sockets/List");
  return true;
}

bool SocketTab::SelSocket (const CEGUI::EventArgs& e)
{
  csString str;
  if (GetSelectedItemText("Sockets/List", str))
  {
    asset->SetSelectedSocket(str.GetData());
    UpdateSocketInfo(asset->GetSocketTransform(asset->GetSelectedSocket()));
  }
 
  return true;
}

bool SocketTab::SaveSocket (const CEGUI::EventArgs& e)
{
  if (strcmp (asset->GetSelectedSocket(), "") == 0) return true;
  asset->SetSocketTransform(asset->GetSelectedSocket(), GetSocketInfo());

  csRef<iStringArray> arr = asset->GetSockets();
  UpdateList(arr, "Sockets/List");
  UpdateSocketInfo(asset->GetSocketTransform(asset->GetSelectedSocket()));

  return true;
}

#endif // SOCKETTAB_H__
