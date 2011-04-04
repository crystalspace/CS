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

#ifndef GENERALTAB_H__
#define GENERALTAB_H__

#include "tabbase.h"

#include <ivaria/reporter.h>

class GeneralTab : public TabBase
{
private:
  ViewMesh* viewmesh;

private:
  bool CameraModeRotate (const CEGUI::EventArgs& e);
  bool CameraModeMoveOrigin (const CEGUI::EventArgs& e);
  bool CameraModeMoveNormal (const CEGUI::EventArgs& e);
  bool LightThreePoint (const CEGUI::EventArgs& e);
  bool LightFrontBackTop (const CEGUI::EventArgs& e);
  bool LightUnlit (const CEGUI::EventArgs& e);
  bool LoadButton (const CEGUI::EventArgs& e);
  bool LoadLibButton (const CEGUI::EventArgs& e);
  bool ResetCameraButton (const CEGUI::EventArgs& e);
  bool ReloadButton (const CEGUI::EventArgs& e);
  bool SaveButton (const CEGUI::EventArgs& e);
  bool SaveBinaryButton (const CEGUI::EventArgs& e);
  bool SetScaleSprite (const CEGUI::EventArgs& e);


public:
  GeneralTab(ViewMesh* viewmesh, iObjectRegistry* obj_reg, AssetBase* ass);
  virtual ~GeneralTab();
};

//-------------------------------------------------------

GeneralTab::GeneralTab(ViewMesh* viewmesh, iObjectRegistry* obj_reg, AssetBase* ass) 
  : TabBase(obj_reg, ass), viewmesh(viewmesh) 
{
  LoadLayout("generaltab.layout");
  AddToTabs();

  CEGUI::Window* btn = 0;

  btn = winMgr->getWindow("General/SaveButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&GeneralTab::SaveButton, this));

  btn = winMgr->getWindow("General/LoadButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&GeneralTab::LoadButton, this));

  btn = winMgr->getWindow("General/SaveBinaryButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&GeneralTab::SaveBinaryButton, this));

  btn = winMgr->getWindow("General/LoadLibButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&GeneralTab::LoadLibButton, this));

  btn = winMgr->getWindow("General/ReloadButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&GeneralTab::ReloadButton, this));

  btn = winMgr->getWindow("General/ResetCameraButton");
  btn->subscribeEvent(CEGUI::PushButton::EventClicked,
    CEGUI::Event::Subscriber(&GeneralTab::ResetCameraButton, this));

  btn = winMgr->getWindow("General/NormalMovementRadio");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&GeneralTab::CameraModeMoveNormal, this));
  CEGUI::RadioButton* radio = static_cast<CEGUI::RadioButton*> (btn);
  radio->setGroupID (1);
  radio->setID (101);
  radio->setSelected (false);

  btn = winMgr->getWindow("General/LooktooriginRadio");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&GeneralTab::CameraModeMoveOrigin, this));
  radio = static_cast<CEGUI::RadioButton*> (btn);
  radio->setGroupID (1);
  radio->setID (102);
  radio->setSelected (false);

  btn = winMgr->getWindow("General/RotateRadio");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&GeneralTab::CameraModeRotate, this));
  radio = static_cast<CEGUI::RadioButton*> (btn);
  radio->setGroupID (1);
  radio->setID (103);
  radio->setSelected (true);

  btn = winMgr->getWindow("General/ThreePointLighting");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&GeneralTab::LightThreePoint, this));
  radio = static_cast<CEGUI::RadioButton*> (btn);
  radio->setGroupID (2);
  radio->setID (201);
  radio->setSelected (true);

  btn = winMgr->getWindow("General/FrontBackTopLighting");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&GeneralTab::LightFrontBackTop, this));
  radio = static_cast<CEGUI::RadioButton*> (btn);
  radio->setGroupID (2);
  radio->setID (202);
  radio->setSelected (false);

  btn = winMgr->getWindow("General/UnlitLighting");
  btn->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged,
    CEGUI::Event::Subscriber(&GeneralTab::LightUnlit, this));
  radio = static_cast<CEGUI::RadioButton*> (btn);
  radio->setGroupID (2);
  radio->setID (203);
  radio->setSelected (false);

  btn = winMgr->getWindow("General/ScaleSprite");
  btn->subscribeEvent(CEGUI::Editbox::EventTextAccepted,
    CEGUI::Event::Subscriber(&GeneralTab::SetScaleSprite, this));
 
}

GeneralTab::~GeneralTab() 
{
}

bool GeneralTab::CameraModeRotate (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("General/RotateRadio");

  if (radio->getSelectedButtonInGroup () == radio)
    viewmesh->cameraManager.SetCameraMode (CS::Demo::CAMERA_ROTATE);
  return true;
}

bool GeneralTab::CameraModeMoveOrigin (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("General/LooktooriginRadio");

  if (radio->getSelectedButtonInGroup () == radio)
    viewmesh->cameraManager.SetCameraMode (CS::Demo::CAMERA_MOVE_LOOKAT);
  return true;
}

bool GeneralTab::CameraModeMoveNormal (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("General/NormalMovementRadio");

  if (radio->getSelectedButtonInGroup () == radio)
    viewmesh->cameraManager.SetCameraMode (CS::Demo::CAMERA_MOVE_FREE);
  return true;
}

bool GeneralTab::LightThreePoint (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("General/ThreePointLighting");

  if (radio->getSelectedButtonInGroup () == radio)
    viewmesh->SetLightMode (THREE_POINT);

  return true;
}

bool GeneralTab::LightFrontBackTop (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("General/FrontBackTopLighting");

  if (radio->getSelectedButtonInGroup () == radio)
    viewmesh->SetLightMode (FRONT_BACK_TOP);

  return true;
}

bool GeneralTab::LightUnlit (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::RadioButton* radio = 
    (CEGUI::RadioButton*) winMgr->getWindow("General/UnlitLighting");

  if (radio->getSelectedButtonInGroup () == radio)
    viewmesh->SetLightMode (UNLIT);

  return true;
}

bool GeneralTab::LoadButton (const CEGUI::EventArgs& e)
{
  viewmesh->form->hide();
  viewmesh->stddlg->show();
  viewmesh->stddlg->setUserString("Purpose", "Load");
  return true;
}

bool GeneralTab::LoadLibButton (const CEGUI::EventArgs& e)
{
  viewmesh->form->hide();
  viewmesh->stddlg->show();
  viewmesh->stddlg->setUserString("Purpose", "LoadLib");
  return true;
}

bool GeneralTab::SaveButton (const CEGUI::EventArgs& e)
{
  viewmesh->form->hide();
  viewmesh->stddlg->show();
  viewmesh->stddlg->setUserString("Purpose", "Save");
  return true;
}

bool GeneralTab::SaveBinaryButton (const CEGUI::EventArgs& e)
{
  viewmesh->form->hide();
  viewmesh->stddlg->show();
  viewmesh->stddlg->setUserString("Purpose", "SaveBinary");
  return true;
}

bool GeneralTab::SetScaleSprite (const CEGUI::EventArgs& e)
{
  CEGUI::WindowManager* winMgr = cegui->GetWindowManagerPtr ();

  CEGUI::Editbox* component = (CEGUI::Editbox*)winMgr->getWindow("General/ScaleSprite");
  const CEGUI::String& text = component->getText();

  if (text.empty()) return true;

  float f;
  if (csScanStr(text.c_str(),"%f", &f) != 1) return true;

  viewmesh->ScaleSprite(f);
  return true;
}

//---------------------------------------------------------------------------


bool GeneralTab::ResetCameraButton (const CEGUI::EventArgs& e)
{
  viewmesh->cameraManager.ResetCamera ();
  return true;
}

bool GeneralTab::ReloadButton (const CEGUI::EventArgs& e)
{
  if (viewmesh->reloadFilename == "")
      return true;

  viewmesh->collection->ReleaseAllObjects();

  size_t reloadLibraryCount = viewmesh->reloadLibraryFilenames.GetSize();
  for(size_t i=0; i < reloadLibraryCount; ++i)
  {
    viewmesh->LoadLibrary(viewmesh->reloadLibraryFilenames[i], false);
  }

  viewmesh->LoadSprite(viewmesh->reloadFilename, viewmesh->reloadFilePath);

  return true;
}


#endif // GENERALTAB_H__
