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

#include <stdarg.h>

#include "csws/csws.h"
#include "csutil/stringarray.h"

struct iEngine;
struct iLoader;
struct iGraphics3D;
struct iEvent;
struct iMeshWrapper;
struct iKeyboardDriver;
struct iObjectRegistry;
struct iVirtualClock;
struct iSector;
struct iView;
struct iMeshObjectFactory;

class ViewMesh : public csApp
{
private:
  csRef<iEngine> engine;
  csRef<iLoader> loader;
  csRef<iGraphics3D> g3d;
  iSector* room;
  csRef<iView> view;
  csRef<iMeshWrapper> sprite;
  csVector3 spritepos;
  csRef<iMeshObjectFactory> imeshfact;
  float move_sprite_speed;
  float scale;
  bool is_cal3d;

  csMenu *menu,*activemenu;
  csWindow *dialog;
  csStringArray stateslist,actionlist,activelist,meshlist,morphanimationlist,socketlist;
  enum { movenormal, moveorigin, rotateorigin } cammode;

  void UpdateSpritePosition(csTicks elapsed);
  
  /**
   * Attach a mesh file to a socket on the main mesh that is loaded.
   * \param socketNumber  The number in the list of loaded sockets that we 
   *                       want to attach to.
   * \param fileName The VFS path to the mesh we want to attach.
   */
  bool AttachMeshToSocket( int socketNumber, char* fileName, float xrot, float yrot, float zrot );
  
  /**
   * Create the window for the rotation of a mesh attached to a slot.
   * \param socketNumber  The number in the list of loaded sockets that we 
   *                       want to attach to.
   * \param fileName The VFS path to the mesh we want to attach.
   */
  void CreateRotationWindow(int socketNumber, char* filename);
  
public:
  ViewMesh (iObjectRegistry *object_reg, csSkin &Skin);
  virtual ~ViewMesh ();

  static void Help ();
  bool LoadSprite (const char *filename,float scale);
  bool SaveSprite (const char *filename);
  void ConstructMenu();

  virtual bool Initialize ();
  virtual bool HandleEvent (iEvent&);
  virtual void Draw();
};

#endif // __VIEWMESH_H__

