/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef CS_RADIOSTY_H
#define CS_RADIOSTY_H

#include "csobject/csobject.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csengine/polygon.h"
#include "csengine/lghtmap.h"

class csWorld;
class csPolygon3D;
class csLightMap;
class csRGBLightMap;
class csProgressMeter;

/**
 *  A radiosity polygon, containing lightmap patches, the lumels.
 *  Radiosity rendering specific info is kept here.
 */
class csRadPoly : csObject {
private:
  csPolygon3D* polygon;
  float area;
  float total_unshot_light; // diffuse * area * avg_delta_level
  csLightMap *csmap;
  int width, height, size;
  /// ptr to static lightmap of polygon
  csRGBLightMap *lightmap;
  /// the change to this lightmap, unshot light.
  csRGBLightMap *deltamap;
  /// the lumels covered by this polygon
  float *lumel_coverage_map;
  /// to convert lumels to world coords.
  csVector3 lumel_origin, lumel_x_axis, lumel_y_axis;
  /// the area of one lumel of the polygon
  float one_lumel_area;

public:
  csRadPoly(csPolygon3D *original);
  ~csRadPoly();

  /// for queueing of polys to shoot their light
  inline float GetPriority() { return total_unshot_light; }
  /// get area size of polygon, precomputed
  inline float GetArea() const { return area; }
  /// get normal vector for polygon 
  const csVector3& GetNormal() const {return polygon->GetPolyPlane()->Normal();}
  /// Get diffuse reflection value for polygon. 0.55-0.75 is nice.
  inline float GetDiffuse() const { return 0.7; }

  /// get original csPolgyon3D for this radpoly
  inline csPolygon3D *GetPolygon3D() const { return polygon; }
  /// get width of lightmap and deltamap
  inline int GetWidth() const{ return width; }
  /// get height of lightmap and deltamap
  inline int GetHeight() const { return height; }
  /// get size of lightmap and deltamap
  inline int GetSize() const { return size; }
  /// check if a delta lumel is zero
  inline bool DeltaIsZero(int suv)
  { return !(deltamap->GetRed()[suv] || deltamap->GetGreen()[suv] || 
      deltamap->GetBlue()[suv] ); }
  /// check if a lumel is not used by this polygon
  inline bool LumelNotCovered(int suv)
  { return lumel_coverage_map[suv] == 0.0; }
  /// get lumel coverage for a lumel
  inline float GetLumelCoverage(int suv)
  { return lumel_coverage_map[suv]; }

  /// Get world coordinates for a lumel -- slow method
  void GetLumelWorldCoords(csVector3& res, int x, int y);
  /// Setup fast lumel to world coords - uses GetLumelWorldCoords
  void SetupQuickLumel2World();
  /// Quick getting lumel to world coords;
  inline void QuickLumel2World(csVector3& res, int x, int y)
  { res = lumel_origin + x* lumel_x_axis + y * lumel_y_axis; }
  /// get area of one lumel in world space
  inline float GetOneLumelArea() const { return one_lumel_area; }
  /** 
   * computes new priority value, summing the light unshot.
   * Note: Do no touch the variable total_unshot_light without 
   * doing a list->Delete(this), beforehand.
   */
  void ComputePriority();
  /** 
   * Add a fraction to delta map of lumel from given source RadPoly.
   * suv is index in src, ruv is index in maps of this poly.
   */
  void AddDelta(csRadPoly *src, int suv, int ruv, float fraction, 
    const csColor& filtercolor);

  /** 
   * light has been shot, copy delta to lightmap, clear delta, 
   * recompute priority. (this radpoly may not be in list as
   * the total_unshot_light value changes).
   */
  void CopyAndClearDelta();

  /**
   * Get the sums of red.green.blue in the delta map.
   */
  void GetDeltaSums(int &red, int &green, int &blue);

  /** 
   *  Add ambient value to lightmap 
   */
  void ApplyAmbient(int red, int green, int blue);

  /**
   *  Compute the lumel coverage array. 1.0 = lumel fully visible.
   *  0.0 lumel not covered by polygon. Returns array of size 'size'.
   *  you have to delete[] it.
   */
  float *ComputeLumelCoverage();

  /**
   *  Compute the texture map at the size of the lumel map.
   *  Gets the texture, and scales down. You must delete the map.
   */
  csRGBLightMap *ComputeTextureLumelSized();
  
  /// Get a RadPoly when you have a csPolygon3D (only type we attach to)
  static csRadPoly* GetRadPoly(csPolygon3D &object);

  CSOBJTYPE;
};


/**
 *  csRadTree is a binary tree, used to store RadPolys by csRadList
 */
class csRadTree{
private:
  csRadPoly *element;
  csRadTree *left, *right;
  /// deletes this node which must have non-null left and right subtrees.
  void DelNode();
  /// Returns Leftmost node in this tree, parent is also stored or NULL
  csRadTree* FindLeftMost(csRadTree*& parent);
  /// Returns rightost node in this tree, parent is also stored or NULL
  csRadTree* FindRightMost(csRadTree*& parent);
public:
  /// create a new node, with values
  inline csRadTree(csRadPoly *n, csRadTree *l, csRadTree *r)
  {element = n; left=l; right=r;}
  /// delete the tree, does not delete the elements.
  inline ~csRadTree() {if (left) delete left; if (right) delete right;}

  /// Insert RadPoly into tree;
  void Insert(csRadPoly *p);
  /// Delete RadPoly from tree; returns new tree. does not delete element.
  csRadTree* Delete(csRadPoly *p);
  /// get element with highest priority. It is deleted, returns new tree.
  csRadTree* PopHighest(csRadPoly*& p);
  /// get node priority
  inline float GetPriority() { return element->GetPriority(); }
  /// traverse tree in in-order (from low to high), calling func(element).
  void TraverseInOrder( void (*func)( csRadPoly * ) );
};


/**
 *  All csRadPolys, keeps them sorted on priority.
 */
class csRadList {
private:
  /// does the actual work
  csRadTree *tree;
  /// number of elements
  int num;

public:
  /// create
  csRadList();
  /// delete, also deletes all elements!
  ~csRadList();

  /// Insert an element
  void InsertElement(csRadPoly *p);
  /// Delete an element
  void DeleteElement(csRadPoly *p);
  /// get element with highest priority. It is also deleted.
  csRadPoly *PopHighest();
  /// print list on output
  void Print();
  /// traverse in some order.
  void Traverse( void (*func)( csRadPoly * ) ) 
  { if(tree) tree->TraverseInOrder(func); }
  /// get the number of elements in the list
  inline int GetNumElements() { return num; }
};

/**
 *  calculates radiosity
 */
class csRadiosity {
private:
  /// world being radiosity rendered
  csWorld *world;
  /// list of all radiosity polygon info
  csRadList *list;

  /// progress meter of the work
  csProgressMeter *meter;
  /// the probably highest priority, starting priority.
  float start_priority;
  /// the number of iterations done.
  int iterations;

  /// Shooting info
  /// lightness factor, built up to form_factor * diffuse * area.
  float factor;
  /// polys that are shooting / shot at.
  csRadPoly *shoot_src, *shoot_dest;
  /// lumels in worlds coords
  csVector3 src_lumel, dest_lumel;
  /// normals pointing towards shooting.
  csVector3 src_normal, dest_normal;
  /// size of source patches in source polygon
  float source_poly_lumel_area;
  /// area of source patch visible for lighting
  float source_patch_area;
  /// index into source maps
  int src_uv; 
  /// texture map of source polygon - reasonably quick to compute, saves
  /// a lot of space.
  csRGBLightMap *texturemap;
  /// color of source lumel for multiplying delta's with.
  csColor src_lumel_color;

public:
  /// create all radiosity data.
  csRadiosity(csWorld *current_world);
  /// get rid of radiosity data.
  ~csRadiosity();
  /// Does the whole radiosity thing. This is the one to call.
  void DoRadiosity();

  /// get next best poly to shoot, or NULL if we should stop.
  csRadPoly* FetchNext();
  /// Shoot light from one polygon to another
  void ShootRadiosityToPolygon(csRadPoly* dest);
  /// Prepare to shoot from source poly
  void PrepareShootSource(csRadPoly*src);
  /// Prepare to shoot from source to dest 
  void PrepareShootDest(csRadPoly*dest);
  /// Prepare to shoot from a lumel
  void PrepareShootSourceLumel(int sx, int sy, int suv);
  /// Shoot it, dest lumel given.
  void ShootPatch(int rx, int ry, int ruv);


  /** 
   *  Compute visibility of one lumel to another lumel.
   *  positions in world coords now.
   */
  float GetVisibility(csRadPoly *srcpoly, const csVector3& src, 
    csRadPoly *destpoly, const csVector3& dest);
  /// Apply all deltamaps if stopped early, and add ambient light
  void ApplyDeltaAndAmbient();
  /// Remove old ambient, for now.
  void RemoveAmbient();
  /// check if dest poly is visible to source poly
  bool VisiblePoly(csRadPoly *src, csRadPoly *dest);

};


#endif //CS_RADIOSTY_H
