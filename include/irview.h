/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __IRVIEW_H__
#define __IRVIEW_H__

#include "csutil/scf.h"
#include "csgeom/plane3.h"
#include "csgeom/transfrm.h"
#include "igraph3d.h"

struct iEngine;
struct iClipper2D;
struct iGraphics2D;
struct iGraphics3D;
struct iCamera;
struct csFog;

/**
 * Information for vertex based fog. There is an instance of this
 * structure in iRenderView for every fogged sector that
 * we encounter. It contains information which allows us to calculate
 * the thickness of the fog for any given ray through the incoming
 * and outgoing portals of the sector.
 */
class csFogInfo
{
public:
  /// Next in list (back in recursion time).
  csFogInfo* next;

  /// The incoming plane (plane of the portal).
  csPlane3 incoming_plane;
  /// The outgoing plane (also of a portal).
  csPlane3 outgoing_plane;
  /**
   * If this is false then there is no incoming plane (the current sector has
   * fog and is not being drawn through a portal).
   */
  bool has_incoming_plane;

  /**
   * If this is false there is no outgoing plane.  The 'outgoing plane
   * distance' is then calculated by straight distance to a vertex instead of
   * projecting throught the outgoing plane
   */
  bool has_outgoing_plane;

  /// The structure describing the fog.
  csFog* fog;
};

SCF_VERSION (iRenderView, 0, 0, 3);

/**
 * This interface represents all information needed to render
 * some object in a current draw context.
 */
struct iRenderView : public iBase
{
  /// Get the engine.
  virtual iEngine* GetEngine () = 0;
  /// Get the 2D clipper for this view.
  virtual iClipper2D* GetClipper () = 0;
  /**
   * If true then we have to clip all objects to the portal frustum
   * (returned with GetClipper()). Normally this is not needed but
   * some portals require this. If GetClipPlane() returns true then the
   * value of this function is also implied to be true.
   */
  virtual bool IsClipperRequired () = 0;
  /// Get the 2D graphics subsystem.
  virtual iGraphics2D* GetGraphics2D () = 0;
  /// Get the 3D graphics subsystem.
  virtual iGraphics3D* GetGraphics3D () = 0;
  /// Get the view frustum defined at z=1.
  virtual void GetViewFrustum (float& leftx, float& rightx, float& topy, float& boty) = 0;
  /**
   * Get the 3D clip plane that should be used to clip all geometry.
   * If this function returns false then this plane is invalid and should
   * not be used. Otherwise it must be used to clip the object before
   * drawing.
   */
  virtual bool GetClipPlane (csPlane3& pl) = 0;
  /**
   * Every fogged sector we encountered results in an extra structure in the
   * following list. This is only used if we are doing vertex based fog.
   * This function will return the first csFogInfo instance.
   */
  virtual csFogInfo* GetFirstFogInfo () = 0;
  /**
   * Get the current camera.
   */
  virtual iCamera* GetCamera () = 0;
  /**
   * Calculate the fog information in the given G3DPolygonDP structure.
   */
  virtual void CalculateFogPolygon (G3DPolygonDP& poly) = 0;
  /**
   * Calculate the fog information in the given G3DPolygonDPFX structure.
   */
  virtual void CalculateFogPolygon (G3DPolygonDPFX& poly) = 0;
  /**
   * Calculate the fog information in the given G3DTriangleMesh
   * structure. This function assumes the fog array is already preallocated
   * and the rest of the structure should be filled in.
   * This function will take care of correctly enabling/disabling fog.
   */
  virtual void CalculateFogMesh (const csTransform& tr_o2c, G3DTriangleMesh& mesh) = 0;
};

#endif

