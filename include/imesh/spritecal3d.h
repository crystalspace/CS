/*
    Copyright (C) 2003 by Keith Fulton

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

#ifndef __CS_IMESH_SPRITECAL3D_H__
#define __CS_IMESH_SPRITECAL3D_H__

#include "csutil/scf.h"
#include "csutil/garray.h"
#include "csutil/cscolor.h"
#include "ivideo/graph3d.h"

class csColor;
struct iMaterialWrapper;
struct iSkeleton;
struct iSkeletonState;
struct iMeshObject;
struct iMeshWrapper;
struct iMeshObjectFactory;
struct iRenderView;
struct iRenderView;



SCF_VERSION (iSpriteCal3DFactoryState, 0, 0, 3);

/**
 * This interface describes the API for the 3D sprite factory mesh object.
 */
struct iSpriteCal3DFactoryState : public iBase
{
  /// Initialize internal Cal3d data structures.
  virtual bool Create(const char *name) = 0;

  /** This prints the message if any cal3d function is unsuccessful.
   * There is no way I can see to retrieve the string and use cs report 
   * with it.
   */
  virtual void ReportLastError () = 0;

  /**
   * This sets the path to which other filenames will be appended before
   * loading.
   */
  virtual void SetBasePath(const char *path) = 0;

  /**
   * This sets the scale factor of the sprite. 1 = as-is size
   */
  virtual void SetRenderScale(float scale) = 0;

  /**
   * This loads the supplied file as the skeleton data for the sprite.
   */
  virtual bool LoadCoreSkeleton(const char *filename) = 0;

  /**
   * This loads the supplied file as one animation action for the sprite.
   */
  virtual int  LoadCoreAnimation(const char *filename,
				 const char *name,
				 int type,
				 float base_velocity,
				 float min_velocity,
				 float max_velocity) = 0;

  /**
   * This loads a submesh which will attach to this skeleton.
   */
  virtual bool LoadCoreMesh(const char *filename) = 0;

  /**
   * This jams a CS material into a cal3d material struct.
   * Don't try this at home!
   */
  virtual bool AddCoreMaterial(iMaterialWrapper *mat) = 0;

  /**
   * Cal3d requires extra initialization once all materials are loaded.
   * The loader calls this at the appropriate time automatically.
   */
  virtual void BindMaterials() = 0;

};

SCF_VERSION (iSpriteCal3DState, 0, 0, 1);

/**
 * This interface describes the API for the 3D sprite mesh object.
 */
struct iSpriteCal3DState : public iBase
{
    virtual int GetAnimCount() = 0;
    virtual const char *GetAnimName(int idx) = 0;
    virtual void ClearAllAnims() = 0;
    virtual bool SetAnimCycle(const char *name, float pct) = 0;
    virtual bool AddAnimCycle(const char *name, float pct, float delay) = 0;
    virtual bool ClearAnimCycle(const char *name, float delay) = 0;
    virtual bool SetAnimAction(const char *name, float delayIn, float delayOut) = 0;
    virtual bool SetVelocity(float vel) = 0;
    virtual void SetLOD(float lod) = 0;
};

#endif // __CS_IMESH_SPRITE3D_H__

