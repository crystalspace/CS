/*
    Copyright (C)1998-2000 by Jorrit Tyberghein

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

#ifndef __CS_DUMPER_H__
#define __CS_DUMPER_H__

class csMatrix3;
class csVector3;
class csVector2;
class csPlane3;
class csBox2;
class csCamera;
class csTextureList;
class csPolyTxtPlane;
class csPolyTreeBBox;
class csPolygonStub;
class csPolygon2D;
class csPolygon3D;
class csCurve;
class csThing;
class csPolyTexture;
class csSector;
class csEngine;
class csOctree;
class csOctreeNode;
class csBspTree;
class csBspTree2D;
class csBspNode;
class csBspNode2D;
class csOctree;
class csOctreeNode;
class csPolygonClipper;
class csFrustum;
class csPoly2DPool;
class csLightPatchPool;
class csQuadcube;
class csQuadtree;
class csQuadtreeNode;
class csHashSet;
struct iPolygon3D;

/**
 * This class knows how to dump debug information about several
 * other classes. It is put here in order to minimize dependencies.
 * Other classes only need to make this class a friend if it has
 * to access private or protected stuff.
 */
class Dumper
{
private:
  static void dump (csBspTree*, csBspNode*, int indent);
  static void dump (csOctree*, csOctreeNode*, int indent);
  static void dump (csBspTree2D*, csBspNode2D*, int indent);
  //static void dump (csQuadtreeNode*, char* buf, int bufdim,
  //	int depth, int x1, int y1, int x2, int y2);
  static void dump_stubs_node (csPolygonStub*, char const* name, int level);
  static void dump_stubs_obj (csPolygonStub*, char const* name, int level);
  static void dump_stubs (csBspNode*, char const* name, int level);
  static void dump_stubs (csOctreeNode*, char const* name, int level);
  static bool check_stubs (csOctreeNode*);
  static bool check_stubs (csBspNode*);

public:
  static void dump (csMatrix3*, char const* name);
  static void dump (csVector3*, char const* name);
  static void dump (csVector2*, char const* name);
  static void dump (csPlane3*);
  static void dump (csBox2*);
  static void dump (csCamera*);
  static void dump (csPolyTxtPlane*);
  static void dump (csPolygon2D*, char const* name);
  static void dump (csPolygon3D*);
  static void dump (iPolygon3D*);
  static void dump (csThing*);
  static void dump (csPolyTexture*, char const* name);
  static void dump (csSector*);
  static void dump (csEngine*);
  static void dump (csBspTree*);
  static void dump (csBspTree2D*);
  static void dump (csOctree*);
  static void dump (csPolygonClipper*, char const* name);
  static void dump (csFrustum*, char const* name);
  static void dump (csPoly2DPool*, char const* name);
  static void dump (csLightPatchPool*, char const* name);
  //static void dump (csQuadcube*);
  static void dump (csQuadtree*);
  static void dump_stubs (csOctree*);
  static void dump_stubs (csPolyTreeBBox*);
};

#endif // __CS_DUMPER_H__
