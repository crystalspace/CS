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
#include "cstool/csdemoapplication.h"
#include "cstool/csview.h"
#include "cstool/meshobjtmpl.h"
#include "csutil/cmdhelp.h"
#include "csutil/cmdline.h"
#include "csutil/common_handlers.h"
#include "csutil/csbaseeventh.h"
#include "csutil/evoutlet.h"
#include "csutil/parray.h"
#include "csutil/plugmgr.h"
#include "csutil/virtclk.h"
#include "csutil/xmltiny.h"

#include "iengine/camera.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/sector.h"
#include "imap/loader.h"
#include "imap/writer.h"
#include "imesh/sprite3d.h"
#include "imesh/spritecal3d.h"
#include "ivaria/icegui.h"

#include "assetbase.h"
#include "tabbase.h"

class GeneralTab;
class MaterialTab;

enum LightMode
{
  THREE_POINT = 0,
  FRONT_BACK_TOP,
  UNLIT
};

class ViewMesh : public csApplicationFramework, public csBaseEventHandler
{
 private:

  csRef<iCollection> collection;

  csStringArray reloadLibraryFilenames;

  csString reloadFilename;
  csString reloadFilePath;

  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iThreadedLoader> tloader;
  csRef<iSaver> saver;
  csRef<iGraphics3D> g3d;
  csRef<iKeyboardDriver> kbd;
  csRef<iVirtualClock> vc;
  csRef<iVFS> vfs;
  csRef<iView> view;
  csRef<iCEGUI> cegui;
  csRef<iThreadReturn> loading;
  iSector* room;
  csRef<FramePrinter> printer;

  CS::Demo::CameraManager cameraManager;

  LightMode lightMode;
  void SetLightMode (LightMode lightMode);

  CEGUI::Window* form;
  CEGUI::Window* stddlg;

  float scale;

  csRef<AssetBase> asset;

  int lod_level;
  int max_lod_level;
  bool auto_lod;

  bool OnKeyboard (iEvent&);

  void Frame ();

  static void Help ();
  void HandleCommandLine();

  bool CreateRoom ();
  bool CreateGui ();
  void LoadLibrary(const char* file, bool record = true);
  void LoadTexture(const char* file, const char* name);
  void LoadSprite (const char* file, const char* path = 0);
  void SaveSprite (const char* file, bool binary);
  void AttachMesh (const char* file);
  void ScaleSprite (float newScale);
  void MoveLights (const csVector3 &a, const csVector3 &b, const csVector3 &c);

  // Tabs
  csRef<GeneralTab> generalTab;
  friend class GeneralTab;
  csRef<MaterialTab> materialTab;

  csRefArray<TabBase> tabs;

  template<class T>
  void RegisterTab ()
  {
    csRef<TabBase> t;
    t.AttachNew(new T(GetObjectRegistry(), asset));
    tabs.Push(t);
  }

  void UnRegisterTabs ();
  

 public:

  ViewMesh ();
  ~ViewMesh ();

  void OnExit ();
  bool OnInitialize (int argc, char* argv[]);

  bool Application ();

private:

  void StdDlgUpdateLists(const char* filename);

  bool StdDlgOkButton (const CEGUI::EventArgs& e);
  bool StdDlgCancleButton (const CEGUI::EventArgs& e);
  bool StdDlgFileSelect (const CEGUI::EventArgs& e);
  bool StdDlgDirSelect (const CEGUI::EventArgs& e);
  bool StdDlgDirChange (const CEGUI::EventArgs& e);

  CS_EVENTHANDLER_NAMES ("crystalspace.viewmesh")
  
  virtual const csHandlerID * GenericPrec (csRef<iEventHandlerRegistry> &r1, 
    csRef<iEventNameRegistry> &r2, csEventID event) const 
  {
    static csHandlerID precConstraint[2];
    
    precConstraint[0] = r1->GetGenericID("crystalspace.cegui");
    precConstraint[1] = CS_HANDLERLIST_END;
    return precConstraint;
  }

  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
};

#endif // __VIEWMESH_H__
