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
#include "imap/parser.h"
#include "imap/writer.h"
#include "imesh/sprite3d.h"
#include "imesh/spritecal3d.h"
#include "imesh/thing.h"

class vmAnimCallback;

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
  void LoadLibrary(const char* path, const char* file);
  void LoadTexture(const char* path, const char* file, const char* name);
  void LoadSprite (const char* path, const char* file);
  void SaveSprite (const char* path, const char* file, bool binary);
  void AttachMesh (const char* path, const char* file);
  void SelectSocket (const char* newsocket);
  void ScaleSprite (float newScale);
  void UpdateSocketList ();
  void UpdateMorphList ();
  void UpdateAnimationList ();
  void UpdateSocket ();

  //SETTING
  static void CameraMode (intptr_t awst, iAwsSource *s);
  static void LoadButton (intptr_t awst, iAwsSource *s);
  static void LoadLibButton (intptr_t awst, iAwsSource *s);
  static void SaveButton (intptr_t awst, iAwsSource *s);
  static void SaveBinaryButton (intptr_t awst, iAwsSource *s);
  static void SetScaleSprite (intptr_t awst, iAwsSource *s);
  //ANIMATION
  static void ReversAnimation (intptr_t awstut, iAwsSource *source);
  static void StopAnimation (intptr_t awstut, iAwsSource *source);
  static void SlowerAnimation (intptr_t awstut, iAwsSource *source);
  static void AddAnimation (intptr_t awstut, iAwsSource *source);
  static void FasterAnimation (intptr_t awstut, iAwsSource *source);
  static void SetAnimation (intptr_t awstut, iAwsSource *source);
  static void RemoveAnimation (intptr_t awstut, iAwsSource *source);
  static void ClearAnimation (intptr_t awstut, iAwsSource *source);
  static void SelAnimation (intptr_t awstut, iAwsSource *source);
  //SOCKET
  static void SetMesh (intptr_t awstut, iAwsSource *source);
  static void SetSubMesh (intptr_t awstut, iAwsSource *source);
  static void SetTriangle (intptr_t awstut, iAwsSource *source);
  static void SetRotX (intptr_t awstut, iAwsSource *source);
  static void SetRotY (intptr_t awstut, iAwsSource *source);
  static void SetRotZ (intptr_t awstut, iAwsSource *source);
  static void AttachButton (intptr_t awst, iAwsSource *s);
  static void DetachButton (intptr_t awst, iAwsSource *s);
  static void AddSocket (intptr_t awst, iAwsSource *s);
  static void DelSocket (intptr_t awst, iAwsSource *s);
  static void SelSocket (intptr_t awst, iAwsSource *s);
  static void RenameSocket (intptr_t awst, iAwsSource *s);
  //MORPH
  static void SelMorph (intptr_t awst, iAwsSource *s);
  static void BlendButton (intptr_t awst, iAwsSource *s);
  static void ClearButton (intptr_t awst, iAwsSource *s);

public:

  ViewMesh ();
  ~ViewMesh ();

  void OnExit ();
  bool OnInitialize (int argc, char* argv[]);

  bool Application ();

private:

  bool ParseDir(const char* filename);
  void StdDlgUpdateLists(const char* filename);

  static void StdDlgOkButton (intptr_t awst, iAwsSource *s);
  static void StdDlgCancleButton (intptr_t awst, iAwsSource *s);
  static void StdDlgFileSelect (intptr_t awst, iAwsSource *s);
  static void StdDlgDirSelect (intptr_t awst, iAwsSource *s);
};

#endif // __VIEWMESH_H__
