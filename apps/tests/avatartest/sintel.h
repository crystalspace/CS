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
#ifndef __SINTEL_H__
#define __SINTEL_H__

#include "avatartest.h"

class SintelScene : public AvatarScene
{
 public:
  SintelScene (AvatarTest* avatarTest);
  ~SintelScene ();

  // Camera related
  csVector3 GetCameraStart ();
  float GetCameraMinimumDistance ();
  csVector3 GetCameraTarget ();

  // Dynamic simuation related
  float GetSimulationSpeed ();

  // From csBaseEventHandler
  void Frame ();
  bool OnKeyboard (iEvent &event);
  bool OnMouseDown (iEvent &event);

  // Creation of objects
  bool CreateAvatar ();

  // User interaction with the scene
  void ResetScene ();

  // Display of information on the state of the scene
  void UpdateStateDescription ();

 private:
  AvatarTest* avatarTest;

  csRef<iMeshWrapper> hairsMesh;
  csRef<iMeshWrapper> eyesMesh;

  // Management of facial expressions 
  struct ActiveMorphComponent
  {
    uint morphTarget;
    float currentWeight;
    float startWeight;
    float targetWeight;
    csTicks targetTime;
  };

  struct MorphComponent
  {
    uint morphTarget;
    float weight;
  };

  struct FacialExpression
  {
    csArray<MorphComponent> morphComponents;
  };

  csArray<FacialExpression> facialExpressions;
  FacialExpression* activeFacialExpression;
  int currentExpressionIndex;
  csArray<ActiveMorphComponent> activeMorphComponents;
  bool activeFacialTransition;
};

#endif // __SINTEL_H__
