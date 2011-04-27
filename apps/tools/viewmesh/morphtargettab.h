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

#ifndef MORPHTARGETTAB_H__
#define MORPHTARGETTAB_H__

#include "tabbase.h"

#include "ivaria/reporter.h"

class MorphTargetTab : public TabBase
{
private:
  bool SelMorph (const CEGUI::EventArgs& e);
  bool BlendButton (const CEGUI::EventArgs& e);
  bool ClearButton (const CEGUI::EventArgs& e);
  bool ResetCameraButton (const CEGUI::EventArgs& e);

public:
  MorphTargetTab(iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~MorphTargetTab();
};

//-------------------------------------------------------

MorphTargetTab::MorphTargetTab(iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass) 
{
  LoadLayout("morphtargettab.layout");
  AddToTabs();

  csRef<iStringArray> arr = asset->GetMorphTargets();
  UpdateList(arr, "MorphTargets/List");
  if (arr->GetSize()) asset->SetSelectedMorphTarget(arr->Get(0));

  CEGUI::Window* btn = 0;

  btn = winMgr->getWindow("MorphTargets/List");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&MorphTargetTab::SelMorph, this));

  btn = winMgr->getWindow("MorphTargets/BlendButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MorphTargetTab::BlendButton, this));

  btn = winMgr->getWindow("MorphTargets/ClearButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&MorphTargetTab::ClearButton, this));

  CEGUI::Window* component = winMgr->getWindow("MorphTargets/WeightInput");
  component->setProperty("Text", "1.0");
}

MorphTargetTab::~MorphTargetTab() 
{
}

bool MorphTargetTab::SelMorph (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("MorphTargets/List");

  CEGUI::ListboxItem* item = list->getFirstSelectedItem();
  if (!item) return true;

  const CEGUI::String& text = item->getText();
  if (text.empty()) return true;

  asset->SetSelectedMorphTarget(text.c_str());
  return true;
}

bool MorphTargetTab::BlendButton (const CEGUI::EventArgs& e)
{
  float weight=1, delay=1;

  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Window* component = winMgr->getWindow("MorphTargets/WeightInput");
  CEGUI::String Sweight = component->getProperty("Text");

  if (! Sweight.empty())
  {
    if(csScanStr(Sweight.c_str(), "%f", &weight) != 1) weight = 1;
  }

  component = winMgr->getWindow("MorphTargets/DelayInput");
  CEGUI::String Sdelay = component->getProperty("Text");
  if (! Sdelay.empty())
  {
    if(csScanStr(Sdelay.c_str(), "%f", &delay) != 1) delay = 1;
  }

  asset->SetMorphTargetWeight(asset->GetSelectedMorphTarget(), weight);

  return true;
}

bool MorphTargetTab::ClearButton (const CEGUI::EventArgs& e)
{
  asset->SetMorphTargetWeight(asset->GetSelectedMorphTarget(), 0.0f);

  return true;
}


#endif // MORPHTARGETTAB_H__
