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

#ifndef __IENGINE_THING_H__
#define __IENGINE_THING_H__

#include "csutil/scf.h"

class csVector3;
class csMatrix3;
class csThing;
struct iSector;
struct iMovable;
struct iPolygon3D;

SCF_VERSION (iThing, 0, 0, 4);

/**
 * This is the generalized interface to Things.<p>
 * A thing is a 3D model, in most cases non-moveable, which is mostly used
 * for details and also for walls of a sector.
 */
struct iThing : public iBase
{
  /// @@@ Used by the engine to retrieve internal thing object (ugly)
  virtual csThing *GetPrivateObject () = 0;

  /// Get polygon set name
  virtual const char *GetName () const = 0;
  /// Set polygon set name
  virtual void SetName (const char *iName) = 0;

  /**
   * Compress the vertex table so that all nearly identical vertices
   * are compressed. The polygons in the set are automatically adapted.
   * This function can be called at any time in the creation of the object
   * and it can be called multiple time but it normally only makes sense
   * to call this function after you have finished adding all polygons
   * and all vertices.<p>
   * Note that calling this function will make the camera vertex array
   * invalid.
   */
  virtual void CompressVertices () = 0;

  /// Query number of polygons in set
  virtual int GetPolygonCount () = 0;
  /// Get a polygon from set by his index
  virtual iPolygon3D *GetPolygon (int idx) = 0;
  /// Create a new polygon and return a pointer to it
  virtual iPolygon3D *CreatePolygon (const char *iName) = 0;

  /// Query number of vertices in set
  virtual int GetVertexCount () = 0;
  /// Get the given vertex coordinates in object space
  virtual csVector3 &GetVertex (int idx) = 0;
  /// Get the given vertex coordinates in world space
  virtual csVector3 &GetVertexW (int idx) = 0;
  /// Get the given vertex coordinates in camera space
  virtual csVector3 &GetVertexC (int idx) = 0;
  /// Create a vertex given his object-space coords and return his index
  virtual int CreateVertex (csVector3 &iVertex) = 0;

  /// Create a key/value pair object
  virtual bool CreateKey (const char *iName, const char *iValue) = 0;

  /// Get the movable for this thing.
  virtual iMovable* GetMovable () = 0;
};

#endif
