/*
  Copyright (C) 2010 Christian Van Brussel, Communications and Remote
      Sensing Laboratory of the School of Engineering at the 
      Universite catholique de Louvain, Belgium
      http://www.tele.ucl.ac.be

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __KRYSTAL_H__
#define __KRYSTAL_H__

#include "avatartest.h"

class KrystalScene : public AvatarScene
{
 public:
  KrystalScene (AvatarTest* avatarTest);
  ~KrystalScene ();

  // Camera related
  csVector3 GetCameraStart ();
  csVector3 GetCameraTarget ();

  // From csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  // Creation of objects
  bool CreateAvatar ();

  // User interaction with the scene
  void ResetScene ();

  // Display of comments 
  void DisplayKeys ();

 private:
  AvatarTest* avatarTest;

  // Animesh
  csRef<iAnimatedMeshFactory> animeshFactory;
  csRef<iAnimatedMesh> animesh;
};

#endif // __KRYSTAL_H__
