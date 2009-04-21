/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

#ifndef __CS_POLYTEXT_H__
#define __CS_POLYTEXT_H__

#include "csgeom/math3d.h"
#include "csutil/array.h"
#include "csutil/cscolor.h"
#include "iengine/light.h"
#include "iengine/movable.h"
#include "ivideo/txtmgr.h"

#include "lghtmap.h"
#include "polyrender.h"

struct iMaterialHandle;
struct iPolygon3D;
struct iLight;
struct csRGBpixel;
class csMatrix3;
class csVector3;
class csVector2;
class csColor;
class csFrustumContext;

class csPolygon3D;
class csPolygon3DStatic;
class csPolyTexture;
class csLightMap;
class csLightPatch;



/**
 * This class represents a lighted texture for a polygon.
 */
class csPolyTexture
{
  friend class csPolygon3D;

private:
  /// LightMap.
  csLightMap* lm;

  /**
   * Compared against csThing version to know whether lightmap needs updating.
   */
  uint32 light_version;

public:
  /**
   * Option variable: control how much the angle of the light with the polygon
   * it hits affects the final light value. Values ranges from -1 to 1.
   * With -1 the polygons will get no light at all. With 0 it will be perfect
   * cosine rule. With 1 the cosine is ignored and it will be like Crystal Space
   * was in the past. Note that changing this value at runtime only has an
   * effect on dynamic lights.
   */
  static float cfg_cosinus_factor;

  /// Constructor.
  csPolyTexture ();
  /// Destructor.
  ~csPolyTexture ();

  void SetTextureMapping (csPolyTextureMapping* mapping);

  /**
   * Set the lightmap for this polytexture . Can also be used to clear
   * the reference to the lightmap if 'lightmap' is 0.
   */
  void SetLightMap (csLightMap* lightmap);

  /// Get the cslightmap, for engine internal use.
  csLightMap *GetLightMap() { return lm; }

  /**
   * Initialize the lightmaps.
   */
  void InitLightMaps ();

#if 0
  /**
   * Update the lightmap for the given light.
   * 'vis' will be false if the polygon is totally shadowed. In this
   * case we should use 'subpoly' to see where the shadow must go and
   * not the base polygon which this csPolyTexture points too.
   * This function returns true if the light actually affected the
   * polygon. False otherwise.
   */
  bool FillLightMap (iFrustumView* lview, csLightingPolyTexQueue* lptq,
  	bool vis, csPolygon3D* subpoly,
	const csMatrix3& m_world2tex,
	const csVector3& v_world2tex,
	const csPlane3& subpoly_plane,
	csPolygon3DStatic* spoly);
#endif

  /**
   * Update the real lightmap for a given csLightPatch
   * (used for a dynamic light).
   */
  void ShineDynLightMap (csLightPatch* lp,
	const csMatrix3& m_world2tex,
	const csVector3& v_world2tex,
	csPolygon3D* polygon,
	const csPlane3& polygon_world_plane,
	csLightingScratchBuffer& finalLM);
  // Assuming cosfact == 0 this will do one scanline of a lightmap.
  void ShineDynLightMapHoriz (
	int du, csRGBcolor* map_uv,
	csVector3& v2,
	float dv2x_x, float dv2x_y, float dv2x_z,
	iLight* light, const csColor& color, float infradius_sq,
	const csPlane3& polygon_world_plane);
  // Same but using cosfact now.
  void ShineDynLightMapHorizCosfact (
	int du, csRGBcolor* map_uv,
	csVector3& v2,
	float dv2x_x, float dv2x_y, float dv2x_z,
	iLight* light, const csColor& color, float infradius_sq,
	float cosfact,
	const csPlane3& polygon_world_plane);

  /**
   * Transform this plane from object space to world space using
   * the given transform.
   */
  void ObjectToWorld (const csMatrix3& m_obj2tex, const csVector3& v_obj2tex,
  	const csReversibleTransform& obj,
	csMatrix3& m_world2tex,
	csVector3& v_world2tex);

  //iMaterialHandle *GetMaterialHandle ();

  /// Get light version.
  uint32 GetLightVersion () const { return light_version; }

  /**
   * Recalculate all pseudo and real dynamic lights if the
   * texture is dirty. The function returns true if there
   * was a recalculation (then the texture needs to be removed
   * from the texture cache).
   */
  bool RecalculateDynamicLights (
	const csMatrix3& m_world2tex,
	const csVector3& v_world2tex,
	csPolygon3D* polygon,
	const csPlane3& polygon_world_plane,
	const csColor& dynambient,
	csLightingScratchBuffer& finalLM);

  /// Query the size of one light cell
  int GetLightCellSize ();
  /// Query log2 (cell size)
  int GetLightCellShift ();
};

#endif // __CS_POLYTEXT_H__

