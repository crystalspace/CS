/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_WIREFRM_H__
#define __CS_WIREFRM_H__

#include "csutil/scf.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"

struct iGraphics3D;
struct iTextureManager;
class csCamera;

#define PLANE_VERTEX_RADIUS .1

#define WF_ORTHO_PERSP -1
#define WF_ORTHO_X 0
#define WF_ORTHO_Y 1
#define WF_ORTHO_Z 2

/**
 * A color for an object in a WireFrame.
 * This color object is special in that it provides support
 * for depth based dimming.
 */
class csWfColor
{
public:
  csWfColor* next;	// Next color.
  int r, g, b;		// Base color.
  int col_idx[16];	// 16-levels based on distance.

public:
  ///
  csWfColor (iTextureManager* txtmgr, int r, int g, int b);
  /// Get a color based on depth.
  int GetColor (float z);
};

/**
 * Object in a WireFrame.
 */
class csWfObject
{
private:
  csWfObject* next, * prev;

protected:
  csWfColor* color;

public:
  ///
  csWfObject ();
  ///
  virtual ~csWfObject ();

  /// Set the prefered color.
  void SetColor (csWfColor* col) { color = col; }
  ///
  csWfColor* GetColor () { return color; }

  ///
  csWfObject* GetNext () { return next; }
  ///
  csWfObject* GetPrev () { return prev; }
  ///
  void SetNext (csWfObject* ob) { next = ob; }
  ///
  void SetPrev (csWfObject* ob) { prev = ob; }

  /// Draw this object given a camera.
  virtual void Draw (iGraphics3D* g, csCamera* c, int ortho = WF_ORTHO_PERSP) = 0;

  /// Do the perspective transform.
  bool Perspective (csCamera* c, csVector3& v, csVector2& persp, float radius, float& pradius);
  //
  /// Do an orthographic transform.
  bool Orthographic (csCamera* c, int ortho, csVector3& v, csVector2& persp);
};

/**
 * Vertex in a WireFrame.
 */
class csWfVertex : public csWfObject
{
private:
  csVector3 loc;

public:
  ///
  csVector3& GetLocation () { return loc; }

  ///
  void SetLocation (const csVector3& v) { loc = v; }
  ///
  void SetLocation (float x, float y, float z) { loc.x = x; loc.y = y; loc.z = z; }

  ///
  virtual void Draw (iGraphics3D* g, csCamera* c, int ortho = WF_ORTHO_PERSP);
};

/**
 * Line in a WireFrame.
 */
class csWfLine : public csWfObject
{
private:
  csVector3 v1, v2;

public:
  ///
  csVector3& GetVertex1 () { return v1; }
  ///
  csVector3& GetVertex2 () { return v2; }
  ///
  void SetLine (csVector3& vv1, csVector3& vv2) { v1 = vv1; v2 = vv2; }

  ///
  virtual void Draw (iGraphics3D* g, csCamera* c, int ortho = WF_ORTHO_PERSP);
};

/**
 * A polygon in a WireFrame.
 */
class csWfPolygon : public csWfObject
{
private:
  int num_vertices;
  csVector3* vertices;
  csVector3 center;		// Calculated center of polygon.
  float A, B, C, D;		// Plane equation of polygon.
  csWfColor* vcolor;		// Visibility color.

public:
  ///
  csWfPolygon ();
  ///
  virtual ~csWfPolygon ();

  ///
  void SetVertexCount (int n);
  ///
  int GetVertexCount () { return num_vertices; }
  ///
  void SetVertex (int i, csVector3& v);

  /// Set the prefered color for the visibility lines.
  void SetVisColor (csWfColor* vcol) { vcolor = vcol; }

  /// This function should be called after all vertices have been set.
  void Prepare ();

  /// Return true if object is visible from a given camera.
  bool IsVisible (csCamera* camera);

  ///
  virtual void Draw (iGraphics3D* g, csCamera* c, int ortho = WF_ORTHO_PERSP);
};

/**
 * A WireFrame is a set of lines, vertices and polygons in 3D.
 * You can draw it given a camera.
 */
class csWireFrame
{
private:
  csWfObject* objects;
  iTextureManager* txtmgr;
  csWfColor* colors;
  csWfColor* white;
  csWfColor* red;
  csWfColor* green;
  csWfColor* blue;
  csWfColor* yellow;

  size_t numObjects;

public:
  ///
  csWireFrame (iTextureManager* txtmgr);
  ///
  virtual ~csWireFrame ();

  ///
  void Clear ();

  ///
  size_t Entries () { return numObjects; }

  /// Get the default white color.
  csWfColor* GetWhite () { return white; }

  /// Get the default red color.
  csWfColor* GetRed () { return red; }

  /// Get the default blue color.
  csWfColor* GetBlue () { return blue; }

  /// Get the default green color.
  csWfColor* GetGreen () { return green; }

  /// Get the default yellow color.
  csWfColor* GetYellow () { return yellow; }

  /**
   * Find a registered color.
   */
  csWfColor* FindColor (int r, int g, int b);

  /**
   * Register a color so that it can be used by objects in
   * the wireframe. This is needed because colors are
   * represented by an array (darker colors for objects in the
   * distance).<p>
   * The returned value is the one that should be given to the
   * csWfObject::SetColor function.
   */
  csWfColor* RegisterColor (int r, int g, int b);

  ///
  csWfVertex* AddVertex (const csVector3& v);
  ///
  csWfLine* AddLine (csVector3& v1, csVector3& v2);
  ///
  csWfPolygon* AddPolygon ();

  ///
  void Draw (iGraphics3D* g, csCamera* c, int ortho = WF_ORTHO_PERSP);

  /// Apply a function to all objects contained in the WireFrame
  void Apply (void (*func)( csWfObject*, void*), void*);
};

/**
 * A WireFrame associated with a camera.
 */
class csWireFrameCam
{
private:
  csWireFrame* wf;
  csCamera* c;

public:
  ///
  csWireFrameCam (iTextureManager* txtmgr);
  ///
  virtual ~csWireFrameCam ();

  ///
  csWireFrame* GetWireframe () { return wf; }
  ///
  csCamera* GetCamera () { return c; }

  ///
  void KeyUp (float speed, bool slow, bool fast);
  ///
  void KeyDown (float speed, bool slow, bool fast);
  ///
  void KeyLeft (float speed, bool slow, bool fast);
  ///
  void KeyRight (float speed, bool slow, bool fast);
  ///
  void KeyLeftStrafe (float speed, bool slow, bool fast);
  ///
  void KeyRightStrafe (float speed, bool slow, bool fast);
  ///
  void KeyPgDn (float speed, bool slow, bool fast);
  ///
  void KeyPgUp (float speed, bool slow, bool fast);
};

#endif // __CS_WIREFRM_H__

