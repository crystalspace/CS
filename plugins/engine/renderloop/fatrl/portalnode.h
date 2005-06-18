/*
    Copyright (C) 2005 by Jorrit Tyberghein
	      (C) 2005 by Frank Richter

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

#ifndef __CS_PORTALNODE_H__
#define __CS_PORTALNODE_H__

#include "iutil/objreg.h"
#include "ivideo/shader/shader.h"

#include "csutil/dirtyaccessarray.h"
#include "csutil/weakref.h"

#include "rendernode.h"

class csPortalRenderNodeFactory;

class csPortalRenderNode : public csRenderNode
{
  friend class csPortalRenderNodeFactory;

  csPortalRenderNodeFactory* factory;

  csDirtyAccessArray<csVector3> camera_vertices;
  csPoly2D poly;
  csPlane3 camera_plane;
  iPortal* portal;
  csReversibleTransform movtrans;

  int old_cliptype;
  csPlane3 old_near_plane;
  bool old_do_near_plane;
  csRef<iClipper2D> old_clipper;

  float oldFogDensity;
  csVector3 oldFogColor;
  csVector4 oldFogPlane;

  csShaderVariableContext& shadervars;

  csPortalRenderNode (iPortal* portal, iRenderView* rview,
    const csReversibleTransform& movtrans, csPortalRenderNodeFactory* factory, 
    csShaderVariableContext& shadervars);
  void PrepareView (iRenderView* rview, iSector* sector);
  void DoWarp (iRenderView* rview);
  void UnprepareView ();
public:
  virtual bool Preprocess (iRenderView* rview);
  virtual void Postprocess (iRenderView* rview);

  bool PreMeshCollect (iRenderView* rview);
  void PostMeshCollect (iRenderView* rview);
};

class csPortalRenderNodeFactory
{
  friend class csPortalRenderNode;

  csWeakRef<iShaderManager> shaderManager;
  csWeakRef<iShader> fog_shader;
  csStringID fogplane_name, fogdensity_name, fogcolor_name;

  bool ClipToPlane (iPortal* portal, csPlane3 *portal_plane, 
    const csVector3 &v_w2c, csVector3 * &pverts, int &num_verts,
    const csVector3* camera_vertices);
  bool DoPerspective (csVector3 *source, int num_verts, csPoly2D *dest, 
    bool mirror, int fov, float shift_x, float shift_y, 
    const csPlane3& plane_cam);
public:
  csPortalRenderNodeFactory (iObjectRegistry* object_reg);

  csPortalRenderNode* CreatePortalNode (iPortal* portal, iRenderView* rview,
    const csReversibleTransform& movtrans, bool doClip, 
    csShaderVariableContext& shadervars);
};

#endif // __CS_PORTALNODE_H__
