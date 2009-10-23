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

#ifndef ANIMATIONTAB_H__
#define ANIMATIONTAB_H__

#include "tabbase.h"

#include <ivaria/reporter.h>

class AnimationTab : public TabBase
{
private:
  bool ReversAnimation (const CEGUI::EventArgs& e);
  bool StopAnimation (const CEGUI::EventArgs& e);
  bool SlowerAnimation (const CEGUI::EventArgs& e);
  bool AddAnimation (const CEGUI::EventArgs& e);
  bool FasterAnimation (const CEGUI::EventArgs& e);
  bool SetAnimation (const CEGUI::EventArgs& e);
  bool RemoveAnimation (const CEGUI::EventArgs& e);
  bool ClearAnimation (const CEGUI::EventArgs& e);
  bool SelAnimation (const CEGUI::EventArgs& e);

public:
  AnimationTab(iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~AnimationTab();
};

//-------------------------------------------------------

AnimationTab::AnimationTab(iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass) 
{
  LoadLayout("animationtab.layout");
  AddToTabs();

  csRef<iStringArray> arr = asset->GetAnimations();
  UpdateList(arr, "Animations/List");
  if (arr->GetSize()) asset->SetSelectedAnimation(arr->Get(0));

  CEGUI::Window* btn = 0;

  btn = winMgr->getWindow("Animations/ReverseAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::ReversAnimation, this));

  btn = winMgr->getWindow("Animations/StopAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::StopAnimation, this));

  btn = winMgr->getWindow("Animations/SlowerAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::SlowerAnimation, this));

  btn = winMgr->getWindow("Animations/AddAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::AddAnimation, this));

  btn = winMgr->getWindow("Animations/FasterAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::FasterAnimation, this));

  btn = winMgr->getWindow("Animations/SetAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::SetAnimation, this));

  btn = winMgr->getWindow("Animations/RemoveAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::RemoveAnimation, this));

  btn = winMgr->getWindow("Animations/ClearAnimation");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&AnimationTab::ClearAnimation, this));

  btn = winMgr->getWindow("Animations/List");
  btn->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&AnimationTab::SelAnimation, this)); 
}

AnimationTab::~AnimationTab() 
{
}

bool AnimationTab::ReversAnimation (const CEGUI::EventArgs& e)
{
  asset->SetReverseAction(!asset->GetReverseAction());
  return true;
}

bool AnimationTab::StopAnimation (const CEGUI::EventArgs& e)
{
  //move_sprite_speed = 0;
  return true;
}

bool AnimationTab::SlowerAnimation (const CEGUI::EventArgs& e)
{
  //move_sprite_speed -= 0.5f;
  return true;
}

bool AnimationTab::AddAnimation (const CEGUI::EventArgs& e)
{
  asset->PlayAnimation(asset->GetSelectedAnimation(), true);

  return true;
}

bool AnimationTab::FasterAnimation (const CEGUI::EventArgs& e)
{
  //move_sprite_speed += 0.5f;
  return true;
}

bool AnimationTab::SetAnimation (const CEGUI::EventArgs& e)
{
  asset->PlayAnimation(asset->GetSelectedAnimation(), true);

  return true;
}

bool AnimationTab::RemoveAnimation (const CEGUI::EventArgs& e)
{
  //TODO: Implement it.

  csReporterHelper::Report(object_reg, CS_REPORTER_SEVERITY_WARNING,"", "Removal of Animation is not yet implemented");
  return true;
}

bool AnimationTab::ClearAnimation (const CEGUI::EventArgs& e)
{
  asset->StopAnimation(asset->GetSelectedAnimation());

  return true;
}

bool AnimationTab::SelAnimation (const CEGUI::EventArgs& e)
{
  csString str;
  if (GetSelectedItemText("Animations/List", str))
    asset->SetSelectedAnimation(str.GetData());

  return true;
}


#endif // ANIMATIONTAB_H__
