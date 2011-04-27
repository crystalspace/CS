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

#ifndef SUBMESHTAB_H__
#define SUBMESHTAB_H__

#include "tabbase.h"

#include <ivaria/reporter.h>

class SubMeshTab : public TabBase
{
private:
  bool SelSubmesh (const CEGUI::EventArgs& e);
  bool AttachSMButton (const CEGUI::EventArgs& e);
  bool DetachSMButton (const CEGUI::EventArgs& e);
  bool SelectMatButton (const CEGUI::EventArgs& e);

public:
  SubMeshTab(iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~SubMeshTab();
};

//-------------------------------------------------------

SubMeshTab::SubMeshTab(iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass)
{
  LoadLayout("submeshtab.layout");
  AddToTabs();

  csRef<iStringArray> arr = asset->GetSubMeshes();
  UpdateList(arr, "SubMeshes/List");
  if (arr->GetSize()) asset->SetSelectedSubMesh(arr->Get(0));

  CEGUI::Window* btn = 0;

  btn = winMgr->getWindow("SubMeshes/List");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
      CEGUI::Event::Subscriber(&SubMeshTab::SelSubmesh, this));

  btn = winMgr->getWindow("SubMeshes/AttachSMButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&SubMeshTab::AttachSMButton, this));

  btn = winMgr->getWindow("SubMeshes/DetachSMButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
      CEGUI::Event::Subscriber(&SubMeshTab::DetachSMButton, this));

  btn = winMgr->getWindow("SubMeshes/SelectMatButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&SubMeshTab::SelectMatButton, this));
  
}

SubMeshTab::~SubMeshTab() 
{
}

bool SubMeshTab::SelSubmesh (const CEGUI::EventArgs& e)
{
  csString str;
  if (GetSelectedItemText("SubMeshes/List", str))
    asset->SetSelectedSubMesh(str.GetData());

  return true;
}

bool SubMeshTab::AttachSMButton (const CEGUI::EventArgs& e)
{
  if (strcmp (asset->GetSelectedSubMesh(), "") == 0)
    return true;

  asset->SetSubMeshRendering(asset->GetSelectedSubMesh(), true);

  return true;
}

bool SubMeshTab::DetachSMButton (const CEGUI::EventArgs& e)
{
  if (strcmp (asset->GetSelectedSubMesh(), "") == 0)
    return true;

  asset->SetSubMeshRendering(asset->GetSelectedSubMesh(), false);

  return true;
}

bool SubMeshTab::SelectMatButton (const CEGUI::EventArgs& e)
{
  csString selectedMaterial;
  if (asset->GetSelectedSubMesh() &&
     (strcmp (asset->GetSelectedSubMesh(), "") != 0)
    && GetSelectedItemText("SubMeshes/MatList", selectedMaterial))
  {
    csRef<iEngine> engine = csQueryRegistry<iEngine> (object_reg);
    iMaterialWrapper* mat = engine->GetMaterialList()->FindByName(selectedMaterial);
    if (mat)
      asset->SetSubMeshMaterial(asset->GetSelectedSubMesh(), mat);
  }

  return true;
}


#endif // SUBMESHTAB_H__
