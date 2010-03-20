/*
    Copyright (C) 2010 by Mike Gist

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

#ifndef PARTICLESTAB_H__
#define PARTICLESTAB_H__

#include "tabbase.h"

#include <ivaria/reporter.h>

class ParticlesTab : public TabBase
{
public:
  ParticlesTab(iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~ParticlesTab();

private:
  bool AddEmitter (const CEGUI::EventArgs& e);
  bool EditEmitter (const CEGUI::EventArgs& e);
  bool DelEmitter (const CEGUI::EventArgs& e);

  bool AddEffector (const CEGUI::EventArgs& e);
  bool EditEffector (const CEGUI::EventArgs& e);
  bool DelEffector (const CEGUI::EventArgs& e);

  bool AddSphere (const CEGUI::EventArgs& e);
  bool AddBox (const CEGUI::EventArgs& e);
  bool AddCone (const CEGUI::EventArgs& e);
  bool AddCylinder (const CEGUI::EventArgs& e);

  bool AddForce (const CEGUI::EventArgs& e);
  bool AddLinColor (const CEGUI::EventArgs& e);
  bool AddVelField (const CEGUI::EventArgs& e);
  bool AddLinear (const CEGUI::EventArgs& e);

  bool DoneEditing (const CEGUI::EventArgs& e);
};

//-------------------------------------------------------

ParticlesTab::ParticlesTab(iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass) 
{
  LoadLayout("particlestab.layout");
  AddToTabs();

  csRef<iStringArray> arr = asset->GetEmitters();
  UpdateList(arr, "Particles/EmitterList");

  arr = asset->GetEffectors();
  UpdateList(arr, "Particles/EffectorList");


  CEGUI::Window* window = 0;

  /* ------------------ Selection ------------------ */

  window = winMgr->getWindow("Particles/AddEmitter");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddEmitter, this));

  window = winMgr->getWindow("Particles/EditEmitter");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::EditEmitter, this));

  window = winMgr->getWindow("Particles/DelEmitter");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::DelEmitter, this));

  window = winMgr->getWindow("Particles/AddEffector");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddEffector, this));

  window = winMgr->getWindow("Particles/EditEffector");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::EditEffector, this));

  window = winMgr->getWindow("Particles/DelEffector");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::DelEffector, this));

  /* ------------------ Addition ------------------ */

  window = winMgr->getWindow("Particles/ChooseEmitter");
  window->hide();

  window = winMgr->getWindow("Particles/AddSphere");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddSphere, this));

  window = winMgr->getWindow("Particles/AddCone");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddCone, this));

  window = winMgr->getWindow("Particles/AddBox");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddBox, this));

  window = winMgr->getWindow("Particles/AddCylinder");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddCylinder, this));

  window = winMgr->getWindow("Particles/ChooseEffector");
  window->hide();

  window = winMgr->getWindow("Particles/AddForce");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddForce, this));

  window = winMgr->getWindow("Particles/AddLinColor");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddLinColor, this));

  window = winMgr->getWindow("Particles/AddVelField");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddVelField, this));

  window = winMgr->getWindow("Particles/AddLinear");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::AddLinear, this));

  /* ------------------ Editing ------------------ */
  window = winMgr->getWindow("Particles/Edit");
  window->hide();

  window = winMgr->getWindow("Particles/Edit/Done");
  window->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&ParticlesTab::DoneEditing, this));
}

ParticlesTab::~ParticlesTab() 
{
}

bool ParticlesTab::AddEmitter (const CEGUI::EventArgs& e)
{
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/main");
  window->hide();

  window = winMgr->getWindow("Particles/ChooseEmitter");
  window->show();

  return true;
}

bool ParticlesTab::EditEmitter (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/EmitterList", id))
  {
    // Update the properties list.
    csRef<iStringArray> arr = asset->GetEmitterProps(id);
    UpdateList(arr, "Particles/Edit/Properties");
  }

  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/main");
  window->hide();

  return true;
}

bool ParticlesTab::DelEmitter (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/EmitterList", id))
  {
    if(asset->DeleteEmitter(id))
    {
      csRef<iStringArray> arr = asset->GetEmitters();
      UpdateList(arr, "Particles/EmitterList");
    }
  }

  return true;
}

bool ParticlesTab::AddEffector (const CEGUI::EventArgs& e)
{
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/main");
  window->hide();

  window = winMgr->getWindow("Particles/ChooseEffector");
  window->show();

  return true;
}

bool ParticlesTab::EditEffector (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/EffectorList", id))
  {
    // Update the properties list.
    csRef<iStringArray> arr = asset->GetEffectorProps(id);
    UpdateList(arr, "Particles/Edit/Properties");
  }

  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/main");
  window->hide();

  return true;
}

bool ParticlesTab::DelEffector (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/EffectorList", id))
  {
    if(asset->DeleteEffector(id))
    {
      csRef<iStringArray> arr = asset->GetEffectors();
      UpdateList(arr, "Particles/EffectorList");
    }
  }

  return true;
}

bool ParticlesTab::AddSphere (const CEGUI::EventArgs& e)
{
  // Add a sphere emitter.
  iParticleEmitter* emitter = asset->AddEmitter(0);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEmitter");
  window->hide();

  return true;
}

bool ParticlesTab::AddCone (const CEGUI::EventArgs& e)
{
  // Add a cone emitter.
  iParticleEmitter* emitter = asset->AddEmitter(1);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEmitter");
  window->hide();

  return true;
}

bool ParticlesTab::AddBox (const CEGUI::EventArgs& e)
{
  // Add a box emitter.
  iParticleEmitter* emitter = asset->AddEmitter(2);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEmitter");
  window->hide();

  return true;
}

bool ParticlesTab::AddCylinder (const CEGUI::EventArgs& e)
{
  // Add a cylinder emitter.
  iParticleEmitter* emitter = asset->AddEmitter(3);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/ChooseEmitter");
  window->hide();

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  return true;
}

bool ParticlesTab::AddForce (const CEGUI::EventArgs& e)
{
  // Add a force effector.
  iParticleEffector* effector = asset->AddEffector(0);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEffector");
  window->hide();

  return true;
}

bool ParticlesTab::AddLinColor (const CEGUI::EventArgs& e)
{
  // Add a lincolor effector.
  iParticleEffector* effector = asset->AddEffector(1);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEffector");
  window->hide();

  return true;
}

bool ParticlesTab::AddVelField (const CEGUI::EventArgs& e)
{
  // Add a velocity field effector.
  iParticleEffector* effector = asset->AddEffector(2);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEffector");
  window->hide();

  return true;
}

bool ParticlesTab::AddLinear (const CEGUI::EventArgs& e)
{
  // Add a linear effector.
  iParticleEffector* effector = asset->AddEffector(3);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEffector");
  window->hide();

  return true;
}

bool ParticlesTab::DoneEditing (const CEGUI::EventArgs& e)
{
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/main");
  window->show();

  window = winMgr->getWindow("Particles/Edit");
  window->hide();

  return true;
}

#endif // PARTICLESTAB_H__
