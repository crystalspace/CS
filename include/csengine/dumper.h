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

#ifndef DUMPER_H
#define DUMPER_H

class csMatrix3;
class csVector3;
class csVector2;
class csPlane;
class csBox;
class csCamera;
class csPolyTxtPlane;
class csPolygon2D;
class csPolygon3D;
class csPolygonSet;
class csPolyTexture;
class csSector;
class csWorld;
class csSpriteTemplate;
class csSprite3D;
class csBspTree;
class csBspNode;
class csPolygonClipper;
class csFrustrum;
class csPoly2DPool;
class csLightPatchPool;
class csQuadcube;
class csQuadtree;
class csQuadtreeNode;

/**
 * This class knows how to dump debug information about several
 * other classes. It is put here in order to minimize dependencies.
 * Other classes only need to make this class a friend if it has
 * to access private or protected stuff.
 */
class Dumper
{
private:
  static void dump (csBspTree* tree, csBspNode* node, int indent);
  static void dump (csQuadtreeNode* node, char* buf, int bufdim,
  	int depth, int x1, int y1, int x2, int y2);

public:
  static void dump (csMatrix3* m, char* name);
  static void dump (csVector3* v, char* name);
  static void dump (csVector2* v, char* name);
  static void dump (csPlane* p);
  static void dump (csBox* b);
  static void dump (csCamera* c);
  static void dump (csPolyTxtPlane* p);
  static void dump (csPolygon2D* p, char* name);
  static void dump (csPolygon3D* p);
  static void dump (csPolygonSet* p);
  static void dump (csPolyTexture* p, char* name);
  static void dump (csSector* s);
  static void dump (csWorld* w);
  static void dump (csSpriteTemplate* s);
  static void dump (csSprite3D* s);
  static void dump (csBspTree* tree);
  static void dump (csPolygonClipper* clipper, char* name);
  static void dump (csFrustrum* frustrum, char* name);
  static void dump (csPoly2DPool* pool, char* name);
  static void dump (csLightPatchPool* pool, char* name);
  static void dump (csQuadcube* cube);
  static void dump (csQuadtree* tree);
};

#endif
