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

#ifndef CAL3DASSET_H__
#define CAL3DASSET_H__

#include "cssysdef.h"
#include "csutil/csstring.h"

#include "assetbase.h"

#ifdef CS_HAVE_CAL3D
#include <cal3d/cal3d.h>
#include <cal3d/animcallback.h>
#endif

struct iSpriteCal3DState;
struct iSpriteCal3DFactoryState;

class Cal3DAsset : public AssetBase
{
private:
  csRef<iSpriteCal3DState> cal3dstate;
  csRef<iSpriteCal3DFactoryState> cal3dsprite;

public:
  static bool Support(iMeshWrapper* mesh);

  Cal3DAsset(iObjectRegistry* obj_reg, iMeshWrapper* mesh);
  virtual ~Cal3DAsset();

  // Animations
  virtual bool SupportsAnimations();

  virtual csPtr<iStringArray> GetAnimations();

  virtual bool PlayAnimation(const char* animationName, bool cycle);

  virtual bool StopAnimation(const char* animationName);

  virtual bool GetReverseAction();

  virtual void SetReverseAction(bool value);


  // Sockets
  virtual bool SupportsSockets();

  virtual csPtr<iStringArray> GetSockets();

  virtual bool AttachMesh(const char* socketName, iMeshWrapper* mesh);

  virtual bool AddSocket(const char* socketName);

  virtual bool DeleteSocket(const char* socketName);

  virtual SocketDescriptor GetSocketTransform(const char* socketName);

  virtual bool SetSocketTransform(const char* socketName, const SocketDescriptor& desc);

  virtual bool DetachAll();


  // SubMeshes
  virtual bool SupportsSubMeshes();

  virtual csPtr<iStringArray> GetSubMeshes();

  virtual bool SetSubMeshRendering(const char* subMeshName, bool value);

  virtual bool SetSubMeshMaterial(const char* subMeshName, iMaterialWrapper* mat);


  // MorphTargets
  virtual bool SupportsMorphTargets();

  virtual csPtr<iStringArray> GetMorphTargets();

  virtual float GetMorphTargetWeight(const char* name);

  virtual bool SetMorphTargetWeight(const char* name, float value);

  // Particles
  virtual bool SupportsParticles() { return false; }
};

#endif // CAL3DASSET_H__
