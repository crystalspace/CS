/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __VIEWMESH_H__
#define __VIEWMESH_H__

#include "cssysdef.h"
#include "csgfx/shadervarcontext.h"
#include "cstool/csapplicationframework.h"
#include "cstool/csview.h"
#include "cstool/meshobjtmpl.h"
#include "csutil/cmdhelp.h"
#include "csutil/cmdline.h"
#include "csutil/csbaseeventh.h"
#include "csutil/evoutlet.h"
#include "csutil/plugmgr.h"
#include "csutil/virtclk.h"
#include "csutil/xmltiny.h"
#include "iaws/awscnvs.h"
#include "iaws/awsecomp.h"
#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/region.h"
#include "iengine/sector.h"
#include "imap/loader.h"
#include "imap/writer.h"
#include "imesh/sprite3d.h"
#include "imesh/spritecal3d.h"
#include "imesh/thing.h"

struct vmAnimCallback;

class ViewMesh : public csApplicationFramework, public csBaseEventHandler
{
 private:

  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iSaver> saver;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iVFS> vfs;
  csRef<iView> view;
  iSector* room;
  int x,y;

  csRef<iAws> aws;
  iAwsPrefManager* awsprefs;
  csRef<iAwsCanvas> awsCanvas;
  iAwsComponent* form;
  iAwsComponent* stddlg;
  enum { load, loadlib, save, savebinary, attach } stddlgPurpose;

  enum { movenormal, moveorigin, rotateorigin } camMode;
  float rotX, rotY;
  float roomsize, scale;
  float move_sprite_speed;

  csRef<iMeshWrapper> spritewrapper;
  csRef<iSprite3DFactoryState> sprite;
  csRef<iSpriteCal3DFactoryState> cal3dsprite;
  csRef<iSprite3DState> state;
  csRef<iSpriteCal3DState> cal3dstate;
  iSpriteSocket* selectedSocket;
  iSpriteCal3DSocket* selectedCal3dSocket;
  const char* selectedAnimation;
  const char* selectedMorphTarget;
  float meshTx, meshTy, meshTz;

  vmAnimCallback* callback;

  bool OnKeyboard (iEvent&);
  bool HandleEvent (iEvent &);

  void ProcessFrame ();
  void FinishFrame ();

  static void Help ();
  void HandleCommandLine();

  void CreateRoom ();
  void CreateGui ();
  void LoadLibrary(const char* file);
  void LoadTexture(const char* file, const char* name);
  void LoadSprite (const char* file);
  void SaveSprite (const char* file, bool binary);
  void AttachMesh (const char* file);
  void SelectSocket (const char* newsocket);
  void ScaleSprite (float newScale);
  void UpdateSocketList ();
  void UpdateMorphList ();
  void UpdateAnimationList ();
  void UpdateSocket ();

  //SETTING
  static void CameraMode (unsigned long, intptr_t awst, iAwsSource *s);
  static void LoadButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void LoadLibButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void SaveButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void SaveBinaryButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void SetScaleSprite (unsigned long, intptr_t awst, iAwsSource *s);
  //ANIMATION
  static void ReversAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void StopAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SlowerAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void AddAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void FasterAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SetAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void RemoveAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void ClearAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SelAnimation (unsigned long, intptr_t awstut, iAwsSource *source);
  //SOCKET
  static void SetMesh (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SetSubMesh (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SetTriangle (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SetRotX (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SetRotY (unsigned long, intptr_t awstut, iAwsSource *source);
  static void SetRotZ (unsigned long, intptr_t awstut, iAwsSource *source);
  static void AttachButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void DetachButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void AddSocket (unsigned long, intptr_t awst, iAwsSource *s);
  static void DelSocket (unsigned long, intptr_t awst, iAwsSource *s);
  static void SelSocket (unsigned long, intptr_t awst, iAwsSource *s);
  static void RenameSocket (unsigned long, intptr_t awst, iAwsSource *s);
  //MORPH
  static void SelMorph (unsigned long, intptr_t awst, iAwsSource *s);
  static void BlendButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void ClearButton (unsigned long, intptr_t awst, iAwsSource *s);

 public:

  ViewMesh ();
  ~ViewMesh ();

  void OnExit ();
  bool OnInitialize (int argc, char* argv[]);

  bool Application ();

private:

  void StdDlgUpdateLists(const char* filename);

  static void StdDlgOkButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void StdDlgCancleButton (unsigned long, intptr_t awst, iAwsSource *s);
  static void StdDlgFileSelect (unsigned long, intptr_t awst, iAwsSource *s);
  static void StdDlgDirSelect (unsigned long, intptr_t awst, iAwsSource *s);
  bool StdDlgDirChange (const CEGUI::EventArgs& e);

  CS_EVENTHANDLER_NAMES ("crystalspace.viewmesh")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
};

#endif // __VIEWMESH_H__
