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

  bool UpdateBool (const CEGUI::EventArgs& e);
  bool UpdateX (const CEGUI::EventArgs& e);
  bool UpdateY (const CEGUI::EventArgs& e);
  bool UpdateZ (const CEGUI::EventArgs& e);
  bool UpdateA (const CEGUI::EventArgs& e);

  bool HandleEditing (const CEGUI::EventArgs& e);
  bool DoneEditing (const CEGUI::EventArgs& e);

  template<bool Emitter>
  void ShowPropControls(PropType type, uint id);

  // Active emitter/effector.
  iParticleEmitter* emitter;
  iParticleEffector* effector;
};

//-------------------------------------------------------

ParticlesTab::ParticlesTab(iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass), emitter(0), effector(0)
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

  window = winMgr->getWindow("Particles/Edit/Properties");
  window->subscribeEvent(CEGUI::Listbox::EventSelectionChanged,
    CEGUI::Event::Subscriber(&ParticlesTab::HandleEditing, this));

  CEGUI::Combobox* combobox = (CEGUI::Combobox*)winMgr->getWindow("Particles/Edit/Bool");
  CEGUI::ListboxTextItem* item = new CEGUI::ListboxTextItem("True", 0);
  item->setTextColours(CEGUI::colour(0.0f, 0.0f, 0.0f));
  combobox->addItem(item);
  item = new CEGUI::ListboxTextItem("False", 1);
  item->setTextColours(CEGUI::colour(0.0f, 0.0f, 0.0f));
  combobox->addItem(item);
  combobox->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted,
    CEGUI::Event::Subscriber(&ParticlesTab::UpdateBool, this));
  combobox->hide();

  CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
  editbox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ParticlesTab::UpdateX, this));
  editbox->hide();

  editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
  editbox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ParticlesTab::UpdateY, this));
  editbox->hide();

  editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
  editbox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ParticlesTab::UpdateZ, this));
  editbox->hide();

  editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/A");
  editbox->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&ParticlesTab::UpdateA, this));
  editbox->hide();

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
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/main");
  window->hide();

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  if (GetSelectedItemID("Particles/EmitterList", id))
  {
    emitter = asset->GetEmitter(id);

    // Update the properties list.
    csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
    UpdateList(arr, "Particles/Edit/Properties");

    ShowPropControls<true>(asset->GetEmitterPropType(emitter, 0), 0);
  }

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
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/main");
  window->hide();

  window = winMgr->getWindow("Particles/Edit");
  window->show();


  if (GetSelectedItemID("Particles/EffectorList", id))
  {
    effector = asset->GetEffector(id);

    // Update the properties list.
    csRef<iStringArray> arr = asset->GetEffectorProps(effector);
    UpdateList(arr, "Particles/Edit/Properties");

    ShowPropControls<false>(asset->GetEffectorPropType(effector, 0), 0);
  }

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
  emitter = asset->AddEmitter(0);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<true>(asset->GetEmitterPropType(emitter, 0), 0);

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
  emitter = asset->AddEmitter(1);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<true>(asset->GetEmitterPropType(emitter, 0), 0);

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
  emitter = asset->AddEmitter(2);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<true>(asset->GetEmitterPropType(emitter, 0), 0);

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
  emitter = asset->AddEmitter(3);
  if(emitter)
  {
    csRef<iStringArray> arr = asset->GetEmitters();
    UpdateList(arr, "Particles/EmitterList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEmitterProps(emitter);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<true>(asset->GetEmitterPropType(emitter, 0), 0);

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
  effector = asset->AddEffector(0);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<false>(asset->GetEffectorPropType(effector, 0), 0);

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
  effector = asset->AddEffector(1);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<false>(asset->GetEffectorPropType(effector, 0), 0);

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
  effector = asset->AddEffector(2);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<false>(asset->GetEffectorPropType(effector, 0), 0);

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
  effector = asset->AddEffector(3);
  if(effector)
  {
    csRef<iStringArray> arr = asset->GetEffectors();
    UpdateList(arr, "Particles/EffectorList");
  }

  // Update the properties list.
  csRef<iStringArray> arr = asset->GetEffectorProps(effector);
  UpdateList(arr, "Particles/Edit/Properties");
  ShowPropControls<false>(asset->GetEffectorPropType(effector, 0), 0);

  // Show the editing window.
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/Edit");
  window->show();

  window = winMgr->getWindow("Particles/ChooseEffector");
  window->hide();

  return true;
}

bool ParticlesTab::HandleEditing (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/Edit/Properties", id))
  {
    if(emitter)
    {
      ShowPropControls<true>(asset->GetEmitterPropType(emitter, id), id);
    }
    else if(effector)
    {
      ShowPropControls<false>(asset->GetEffectorPropType(effector, id), id);
    }
  }

  return true;
}

bool ParticlesTab::DoneEditing (const CEGUI::EventArgs& e)
{
  CEGUI::Window* window = 0;

  window = winMgr->getWindow("Particles/main");
  window->show();

  window = winMgr->getWindow("Particles/Edit");
  window->hide();

  emitter = 0;
  effector = 0;

  return true;
}

template<bool Emitter>
void ParticlesTab::ShowPropControls(PropType type, uint id)
{
  CEGUI::Combobox* boolWindow = (CEGUI::Combobox*)winMgr->getWindow("Particles/Edit/Bool");
  CEGUI::Editbox* xWindow = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
  CEGUI::Editbox* yWindow = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
  CEGUI::Editbox* zWindow = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
  CEGUI::Editbox* aWindow = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/A");

  boolWindow->hide();
  xWindow->hide();
  yWindow->hide();
  zWindow->hide();
  aWindow->hide();

  switch(type)
  {
  case Bool:
    {
      bool val;

      if(Emitter)
      {
        if(!asset->GetEmitterPropValue(emitter, id, val))
          break;
      }
      else
      {
        if(!asset->GetEffectorPropValue(effector, id, val))
          break;
      }

      boolWindow->setItemSelectState((size_t)0, val);
      boolWindow->setItemSelectState((size_t)1, !val);
      boolWindow->show();
      break;
    }
  case Float:
    {
      float val;

      if(Emitter)
      {
        if(!asset->GetEmitterPropValue(emitter, id, val))
          break;
      }
      else
      {
        if(!asset->GetEffectorPropValue(effector, id, val))
          break;
      }

      csString desc;
      desc.Format("%g", val);
      xWindow->setText(desc.GetData());      
      xWindow->show();
      break;
    }
  case Vector2:
    {
      csVector2 val;

      if(Emitter)
      {
        if(!asset->GetEmitterPropValue(emitter, id, val))
          break;
      }
      else
      {
        if(!asset->GetEffectorPropValue(effector, id, val))
          break;
      }

      csString desc;
      desc.Format("%g", val.x);
      xWindow->setText(desc.GetData()); 
      xWindow->show();

      desc.Format("%g", val.y);
      yWindow->setText(desc.GetData());
      yWindow->show();
      break;
    }
  case Vector3:
    {
      csVector3 val;

      if(Emitter)
      {
        if(!asset->GetEmitterPropValue(emitter, id, val))
          break;
      }
      else
      {
        if(!asset->GetEffectorPropValue(effector, id, val))
          break;
      }

      csString desc;
      desc.Format("%g", val.x);
      xWindow->setText(desc.GetData()); 
      xWindow->show();

      desc.Format("%g", val.y);
      yWindow->setText(desc.GetData());
      yWindow->show();

      desc.Format("%g", val.z);
      zWindow->setText(desc.GetData());
      zWindow->show();
      break;
    }
  case Color4:
    {
      csColor4 val;

      if(Emitter)
      {
        if(!asset->GetEmitterPropValue(emitter, id, val))
          break;
      }
      else
      {
        if(!asset->GetEffectorPropValue(effector, id, val))
          break;
      }

      csString desc;
      desc.Format("%g", val.red);
      xWindow->setText(desc.GetData()); 
      xWindow->show();

      desc.Format("%g", val.green);
      yWindow->setText(desc.GetData());
      yWindow->show();

      desc.Format("%g", val.blue);
      zWindow->setText(desc.GetData());
      zWindow->show();

      desc.Format("%g", val.alpha);
      aWindow->setText(desc.GetData());
      aWindow->show();
      break;
    }
  case Enum:
    {
      printf("enum!\n");
      break;
    }
  case Unknown:
  default:
    {
      printf("unknown!\n");
      break;
    }
  }
}

bool ParticlesTab::UpdateBool (const CEGUI::EventArgs& e)
{
  CEGUI::Combobox* combobox = (CEGUI::Combobox*)winMgr->getWindow("Particles/Edit/Bool");
  bool val = combobox->getSelectedItem()->getID() == 0;

  uint id;
  if (GetSelectedItemID("Particles/Edit/Properties", id))
  {
    CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Particles/Edit/Properties");
    float scrollPos = list->getVertScrollbar()->getScrollPosition();

    if(emitter)
    {
      asset->SetEmitterPropValue(emitter, id, val);
      UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
    }
    else if(effector)
    {
      asset->SetEffectorPropValue(effector, id, val);
      UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
    }

    SetSelectedItemByID("Particles/Edit/Properties", id);
    list->getVertScrollbar()->setScrollPosition(scrollPos);
  }

  return true;
}

bool ParticlesTab::UpdateX (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/Edit/Properties", id))
  {
    PropType type;
    CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Particles/Edit/Properties");
    float scrollPos = list->getVertScrollbar()->getScrollPosition();

    if(emitter)
    {
      type = asset->GetEmitterPropType(emitter, id);
    }
    else if(effector)
    {
      type = asset->GetEffectorPropType(effector, id);
    }

    switch(type)
    {
    case Float:
      {
        float val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    case Vector2:
      {
        csVector2 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.x);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.y);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    case Vector3:
      {
        csVector3 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.x);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.y);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
        sscanf(editbox->getText().c_str(), "%f", &val.z);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    case Color4:
      {
        csColor4 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.red);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.green);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
        sscanf(editbox->getText().c_str(), "%f", &val.blue);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/A");
        sscanf(editbox->getText().c_str(), "%f", &val.alpha);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    default:
      {
        break;
      }
    }

    SetSelectedItemByID("Particles/Edit/Properties", id);
    list->getVertScrollbar()->setScrollPosition(scrollPos);
  }

  return true;
}

bool ParticlesTab::UpdateY (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/Edit/Properties", id))
  {
    PropType type;
    CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Particles/Edit/Properties");
    float scrollPos = list->getVertScrollbar()->getScrollPosition();

    if(emitter)
    {
      type = asset->GetEmitterPropType(emitter, id);
    }
    else if(effector)
    {
      type = asset->GetEffectorPropType(effector, id);
    }

    switch(type)
    {
    case Vector2:
      {
        csVector2 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.x);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.y);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    case Vector3:
      {
        csVector3 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.x);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.y);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
        sscanf(editbox->getText().c_str(), "%f", &val.z);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    case Color4:
      {
        csColor4 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.red);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.green);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
        sscanf(editbox->getText().c_str(), "%f", &val.blue);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/A");
        sscanf(editbox->getText().c_str(), "%f", &val.alpha);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    default:
      {
        break;
      }
    }

    SetSelectedItemByID("Particles/Edit/Properties", id);
    list->getVertScrollbar()->setScrollPosition(scrollPos);
  }

  return true;
}

bool ParticlesTab::UpdateZ (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/Edit/Properties", id))
  {
    PropType type;
    CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Particles/Edit/Properties");
    float scrollPos = list->getVertScrollbar()->getScrollPosition();

    if(emitter)
    {
      type = asset->GetEmitterPropType(emitter, id);
    }
    else if(effector)
    {
      type = asset->GetEffectorPropType(effector, id);
    }

    switch(type)
    {
    case Vector3:
      {
        csVector3 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.x);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.y);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
        sscanf(editbox->getText().c_str(), "%f", &val.z);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    case Color4:
      {
        csColor4 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.red);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.green);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
        sscanf(editbox->getText().c_str(), "%f", &val.blue);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/A");
        sscanf(editbox->getText().c_str(), "%f", &val.alpha);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    default:
      {
        break;
      }
    }

    SetSelectedItemByID("Particles/Edit/Properties", id);
    list->getVertScrollbar()->setScrollPosition(scrollPos);
  }

  return true;
}

bool ParticlesTab::UpdateA (const CEGUI::EventArgs& e)
{
  uint id;
  if (GetSelectedItemID("Particles/Edit/Properties", id))
  {
    PropType type;
    CEGUI::Listbox* list = (CEGUI::Listbox*)winMgr->getWindow("Particles/Edit/Properties");
    float scrollPos = list->getVertScrollbar()->getScrollPosition();

    if(emitter)
    {
      type = asset->GetEmitterPropType(emitter, id);
    }
    else if(effector)
    {
      type = asset->GetEffectorPropType(effector, id);
    }

    switch(type)
    {
    case Color4:
      {
        csColor4 val;
        CEGUI::Editbox* editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/X");
        sscanf(editbox->getText().c_str(), "%f", &val.red);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Y");
        sscanf(editbox->getText().c_str(), "%f", &val.green);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/Z");
        sscanf(editbox->getText().c_str(), "%f", &val.blue);
        editbox = (CEGUI::Editbox*)winMgr->getWindow("Particles/Edit/A");
        sscanf(editbox->getText().c_str(), "%f", &val.alpha);

        if(emitter)
        {
          asset->SetEmitterPropValue(emitter, id, val);
          UpdateList(asset->GetEmitterProps(emitter), "Particles/Edit/Properties");
        }
        else if(effector)
        {
          asset->SetEffectorPropValue(effector, id, val);
          UpdateList(asset->GetEffectorProps(effector), "Particles/Edit/Properties");
        }

        break;
      }
    default:
      {
        break;
      }
    }

    SetSelectedItemByID("Particles/Edit/Properties", id);
    list->getVertScrollbar()->setScrollPosition(scrollPos);
  }

  return true;
}

#endif // PARTICLESTAB_H__
