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

#ifndef __IPORTAL_H__
#define __IPORTAL_H__

#include "csutil/scf.h"
#include "csutil/flags.h"

/**
 * If this flag is set then this portal will clip all geometry in
 * the destination sector. This must be used for portals which arrive
 * in the middle of a sector.
 */
#define CS_PORTAL_CLIPDEST 0x00000001

/**
 * If this flag is set then this portal will do a Z-fill after
 * rendering the contents. This is mainly useful for floating portals
 * where it is possible that there is geometry in the same sector
 * that will be rendered behind the portal (and does could accidently
 * get written in the portal sector because the Z-buffer cannot
 * be trusted).
 */
#define CS_PORTAL_ZFILL 0x00000002

/**
 * If this flag is set then this portal will do space warping.
 * You can use this to implement mirrors or other weird portal effects.
 * Don't set this flag directly. Use SetWarp() instead. It is safe
 * to disable and query this flag though.
 */
#define CS_PORTAL_WARP 0x00000004

/**
 * If this flag is set then this portal mirrors space (changes order
 * of the vertices of polygons). Don't set this flag directly. It will
 * be automatically set if a mirroring space warp is used with SetWarp().
 */
#define CS_PORTAL_MIRROR 0x00000008

/**
 * A flag which indicates if the destination of this portal should not be
 * transformed from object to world space. For mirrors you should
 * disable this flag because you want the destination to move with the
 * source.
 */
#define CS_PORTAL_STATICDEST 0x00000010

class csMatrix3;
class csVector3;
struct iSector;
struct iPolygon3D;

SCF_VERSION (iPortal, 0, 0, 2);

/**
 * This is the interface to the Portal objects. Polygons that are
 * really `openings' to different areas have a portal associated
 * with them. The portal object defines where the opening leads,
 * how exactly the geometry behind portal is to be handled and
 * so on.
 */
struct iPortal : public iBase
{
  /// Set portal flags (see CS_PORTAL_XXX values above)
  virtual csFlags& GetFlags () = 0;

  /// Get the sector that the portal points to
  virtual iSector *GetPortal () = 0;
  /// Set portal to point to specified sector
  virtual void SetPortal (iSector *iDest) = 0;

  /**
   * Set the warping transformation for this portal in object space and world
   * space.
   */
  virtual void SetWarp (const csMatrix3 &m_w, const csVector3 &v_w_before,
    const csVector3 &v_w_after) = 0;
  /// Set warping transformation to mirror around given polygon
  virtual void SetMirror (iPolygon3D *iPoly) = 0;
};

#endif // __IPORTAL_H__
