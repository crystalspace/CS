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

#ifndef __CS_RADIOSTY_H__
#define __CS_RADIOSTY_H__

#if 0
// TOTALLY DISABLED FOR NOW!

#include "csutil/csobject.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "iengine/lview.h"

class csPolygon3D;
class csLightMap;
class csRGBMap;
class csProgressPulse;
class csShadowFrustum;
struct iProgressMeter;

/// computes x ** (2 ** y), using only y multiplies.
float FastPow2(float x, const int y);

/**
 * A small class encapsulating an RGB lightmap.
 * The float version of csRGBMap
 * Helps radiosity.
 */
class csRGBFloatLightMap
{
private:
  int max_sizeRGB;      // Max size for every map.
  float* map;

public:
  ///
  void Clear ()
  {
    delete [] map; map = 0;
    max_sizeRGB = 0;
  }

  ///
  csRGBFloatLightMap () : max_sizeRGB (0), map (0) { }
  ///
  ~csRGBFloatLightMap () { Clear (); }

  ///
  int GetMaxSize () { return max_sizeRGB; }
  ///
  void SetMaxSize (int s) { max_sizeRGB = s; }

  /// Get data.
  float* GetMap () const { return map; }
  /// Get red map.
  float* GetRed () const { return map; }
  /// Get green map.
  float* GetGreen () const { return map+max_sizeRGB; }
  /// Get blue map.
  float* GetBlue () const { return map+(max_sizeRGB<<1); }

  /// Set color maps.
  void SetMap (float* m) { map = m; }

  ///
  void Alloc (int size)
  {
    max_sizeRGB = size;
    delete [] map;
    map = new float [size*3];
  }

  /// copy from floatmap
  void Copy (csRGBFloatLightMap& other, int size)
  {
    Clear ();
    if (other.map) { Alloc (size); memcpy (map, other.map,
      size * 3 * sizeof(float)); }
  }

  /// copy from bytemap
  void Copy (csRGBMap& other, int size)
  {
    Clear ();
    if (other.GetArray()) {
      Alloc (size);
      csRGBpixel* m = other.GetArray ();
	  int i;
      for(i=0; i<size; i++)
      {
        GetRed()[i] = m[i].red;
        GetGreen()[i] = m[i].green;
        GetBlue()[i] = m[i].blue;
      }
    }
  }

  /// copy to bytemap
  void CopyTo (csRGBMap& other, int size)
  {
    other.Clear ();
    if (GetMap()) {
      other.Alloc (size);
      csRGBpixel* m = other.GetArray ();
	  int i;
      for(i=0; i<size; i++)
      {
        m[i].red = (unsigned char)GetRed()[i];
        m[i].green = (unsigned char)GetGreen()[i];
        m[i].blue = (unsigned char)GetBlue()[i];
      }
    }
  }
};

/**
 *  A radiosity Element, containing lightmap patches, the lumels.
 *  Radiosity rendering specific info is kept here.
 */
class csRadElement : public csObject {
protected:
  float area;

  float total_unshot_light; // diffuse * area * avg_delta_level

public://@@@
  csLightMap *csmap;

protected:
  int width, height, size;

  /// ptr to static lightmap of polygon
  csRGBMap *lightmap;

  /// the change to this lightmap, unshot light.
  csRGBFloatLightMap *deltamap;

  /**
   * if we are debugging radiosity then this is a copy of the
   * lightmap that we store in order to restore the static lightmap
   * values later.
   */
  csRGBMap* copy_lightmap;

  /// the area of one lumel of the polygon
  float one_lumel_area;

  /// values for loop detection, last shooting priority and repeats
  float last_shoot_priority;
  int num_repeats;

protected:

  /// return the material handle
  virtual iMaterialWrapper* GetMaterialWrapper () = 0;

  /// return the flat color value
  virtual csColor GetFlatColor() const = 0;

  // setup some necessary values
  virtual void Setup() {};



public:
  csRadElement();
  ~csRadElement();

  /// for queueing of elements to shoot their light
  inline float GetPriority() { return total_unshot_light; }

  /// get area size of element, precomputed
  inline float GetArea() const { return area; }

  /// Get diffuse reflection value for element. 0.55-0.75 is nice.
  inline float GetDiffuse() const { return 0.7f; }

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

  /// return the sector of this element
  virtual csSector* GetSector () const = 0;

  /// return the normal at the point x, y
  virtual const csVector3& GetNormal(int x, int y) const = 0;

  /// return the average normal for this element
  csVector3 GetAvgNormal() const;

  /// check if a patch has zero delta
  bool DeltaIsZero(int suv, int w, int h);

  /// Get avg texture colour for a patch.
  void GetTextureColour(int suv, int w, int h, csColor &avg,
    csRGBMap *texturemap);

  /// Cap every delta in patch to value.
  void CapDelta(int suv, int w, int h, float max);

  /// Get the summed delta of a patch
  void GetSummedDelta(int suv, int w, int h, csColor& sum);

  /// get the delta map
  inline csRGBFloatLightMap* GetDeltaMap() { return deltamap; }

  /**
   * For debugging: keep a pointer of the static lightmap
   * and temporarily copy the deltamap to the static lightmap again.
   */
  void ShowDeltaMap ();

  /**
   * For debugging: restore the state of the static lightmaps
   * and free the copy of the static lightmap.
   */
  void RestoreStaticMap ();

  /// Get last shooting priority of this radpoly
  inline float GetLastShootingPriority() { return last_shoot_priority;}

  /// Set last shooting priority
  inline void SetLastShootingPriority(float val) {last_shoot_priority=val;}

  /// Get number of repeats shooting at the same priority for this poly.
  inline int GetRepeatCount() { return num_repeats; }

  /// Increment the number of repeats of shooting at same priority by one.
  inline void IncNumRepeats() {num_repeats++;}

  /// Get world coordinates for a lumel
  virtual void Lumel2World(csVector3& res, int x, int y) = 0;

  /// get area of one lumel in world space
  inline float GetOneLumelArea() const { return one_lumel_area; }


  /// Populates the shadow coverage Matrix for this element
  virtual void GetCoverageMatrix(csFrustumView* lview,
                                 csCoverageMatrix* shadow_matrix) = 0;

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
  void AddDelta(csRadElement *src, int suv, int ruv, float fraction,
    const csColor& filtercolor);

  /**
   * Add a value *to* the deltamap.
   */
  inline void AddToDelta(int ruv, const csColor& value)
  { deltamap->GetRed()[ruv] += value.red;
    deltamap->GetGreen()[ruv] += value.green;
    deltamap->GetBlue()[ruv] += value.blue;
  }

  /**
   * light has been shot, copy delta to lightmap, clear delta,
   * recompute priority. (this radpoly may not be in list as
   * the total_unshot_light value changes).
   */
  void CopyAndClearDelta();

  /**
   * Get the sums of red.green.blue in the delta map.
   */
  void GetDeltaSums(float &red, float &green, float &blue);

  /**
   *  Add ambient value to deltamap
   */
  void ApplyAmbient(int red, int green, int blue);

  /**
   *  Compute the texture map at the size of the lumel map.
   *  Gets the texture, and scales down. You must delete the map.
   */
  csRGBMap *ComputeTextureLumelSized();

  /// Get a RadElement when you have a csPolygon3D or a csCurve
  static csRadElement* GetRadElement(csPolygon3D &object);

  /// Get a RadElement
  static csRadElement* GetRadElement(csCurve &object);
};


SCF_VERSION (csRadPoly, 0, 0, 1);

/**
 *  A radiosity polygon, containing lightmap patches, the lumels.
 *  Radiosity rendering specific info is kept here.
 */
class csRadPoly : public csRadElement
{
private:
  csPolygon3D* polygon;
  csSector* sector;
  csVector3 lumel_origin, lumel_x_axis, lumel_y_axis;

protected:
  /// return the material handle for this polygon
  virtual iMaterialWrapper * GetMaterialWrapper ()
  { return polygon->GetMaterialWrapper (); }

  /// return the flat color for the polygons texture
  virtual csColor GetFlatColor() const;

  // setup some necessary values
  virtual void Setup();

public:
  csRadPoly(csPolygon3D *original, csSector* sector);
  ~csRadPoly();

  /// get normal vector for polygon
  const csVector3& GetNormal(int x, int y) const
  { (void)x; (void)y; return polygon->GetPolyPlane()->Normal();}

  /// get original csPolgyon3D for this radpoly
  inline csPolygon3D *GetPolygon3D() const { return polygon; }

  /// Get world coordinates for a lumel -- slow method
  void CalcLumel2World(csVector3& res, int x, int y);

  /// Get world coordinates for a lumel
  virtual void Lumel2World(csVector3& res, int x, int y);

  csSector* GetSector () const { return sector; }

  /// Populates the shadow coverage Matrix for this element
  virtual void GetCoverageMatrix(csFrustumView* lview,
                                 csCoverageMatrix* shadow_matrix)
  {
      (void)lview;
      (void)shadow_matrix;
#if 0
// @@@@@@@@@@@ REWRITE
    GetPolygon3D()->GetLightMapInfo()->GetPolyTex()->
                    GetCoverageMatrix(*lview, *shadow_matrix);
#endif
  }
  SCF_DECLARE_IBASE_EXT (csRadElement);
};


SCF_VERSION (csRadCurve, 0, 0, 1);

/**
 *  A radiosity curve, containing lightmap patches, the lumels.
 *  Radiosity rendering specific info is kept here.
 */
class csRadCurve : public csRadElement {
private:
  csCurve* curve;
  csSector* sector;

protected:
  /// return the texture handle for this curve
  virtual iMaterialWrapper * GetMaterialWrapper ()
  { return curve->Material; }

  /// return the flat color for the polygons texture
  virtual csColor GetFlatColor() const
  {
    /// @@@ I'm not sure why curves don't have a flat color, so for now
    /// @@@ just return a default color of mid-gray
    return csColor(0.5, 0.5, 0.5);
  }

  virtual void Setup();

public:
  csRadCurve (csCurve* curve, csSector* sector);
  ~csRadCurve();

  /// get normal vector for the Curve
  virtual const csVector3& GetNormal(int x, int y) const;

  /// Get world coordinates for a lumel
  virtual void Lumel2World(csVector3& res, int x, int y);

  csSector* GetSector () const { return sector; }

  /// Populates the shadow coverage Matrix for this element
  virtual void GetCoverageMatrix(csFrustumView* /*(lview*/,
                                 csCoverageMatrix* /*shadow_matrix*/)
  { /* @@@ curve->GetCoverageMatrix(*lview, *shadow_matrix); */ }

  SCF_DECLARE_IBASE_EXT (csRadElement);
};


/**
 *  csRadTree is a binary tree, used to store RadPolys by csRadList
 */
class csRadTree{
private:
  csRadElement *element;

  csRadTree *left, *right;

  /// deletes this node which must have non-null left and right subtrees.
  void DelNode();

  /// Returns Leftmost node in this tree, parent is also stored or 0
  csRadTree* FindLeftMost(csRadTree*& parent);

  /// Returns rightost node in this tree, parent is also stored or 0
  csRadTree* FindRightMost(csRadTree*& parent);

public:
  /// create a new node, with values
  inline csRadTree(csRadElement *n, csRadTree *l, csRadTree *r)
  {element = n; left=l; right=r;}

  /// delete the tree, does not delete the elements.
  inline ~csRadTree() {delete left; delete right;}

  /// Insert RadElement into tree;
  void Insert(csRadElement *e);

  /// Delete RadElement from tree; returns new tree. does not delete element.
  csRadTree* Delete(csRadElement *e);

  /// get element with highest priority. It is deleted, returns new tree.
  csRadTree* PopHighest(csRadElement*& e);

  /// get node priority
  inline float GetPriority() { return element->GetPriority(); }

  /// traverse tree in in-order (from low to high), calling func(element).
  void TraverseInOrder( void (*func)( csRadElement * ) );
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
  void InsertElement(csRadElement *e);

  /// Delete an element
  void DeleteElement(csRadElement *e);

  /// get element with highest priority. It is also deleted.
  csRadElement *PopHighest();

  /// print list on output
  void Print();

  /// traverse in some order.
  void Traverse( void (*func)( csRadElement * ) )
  { if(tree) tree->TraverseInOrder(func); }

  /// get the number of elements in the list
  inline int GetElementCount() { return num; }
};

/**
 *  calculates radiosity
 */
class csRadiosity {
public:
  /**
   * Configuration information
   *
   * when true static specular gloss is applied, giving extra light.
   */
  static bool do_static_specular;
  /// amount of specular to add 0..1 is a sane setting.
  static float static_specular_amount;
  /**
   *  Bigger value gives smaller highlight. Try between 0..10.
   *  The value is the 2log(n), where n is the usual highlightsize used
   *  with specular highlighting in other programs.
   */
  static int static_specular_tightness;

  /** Multiplier for amount of texture colour used.
   *  regular value is 1.0, smaller will be darker, larger will make
   *  surroundings blend eachothers colours more.
   *  0.0 means not to use texture colouring.
   */
  static float colour_bleed;

  /** when priority gets below this value calculation will stop.
   *  > 0.0. Set higher for shorter calculation time, and less quality.
   */
  static float stop_priority;
  /** The improvement factor in priority when calculation can stop.
   *  Must be > 0. Try 1000. Larger values, better quality, more time needed.
   */
  static float stop_improvement;
  /// max number of iterations, after that amount of polygons processed stop.
  static int stop_iterations;

  /**
   * light will be shot from n by n lumels.
   */
  static int source_patch_size;

private:
  /// world being radiosity rendered
  iEngine *engine;
  /// list of all radiosity polygon info
  csRadList *list;

  /**
   * For debugging: if true we are showing the delta maps in the
   * static map.
   */
  bool showing_deltamaps;

  /// progress meter of the work
  iProgressMeter *meter;
  /// pulse to see polygon shootings
  csProgressPulse *pulse;
  /// the probably highest priority, starting priority.
  float start_priority;
  /// the number of iterations done.
  int iterations;

  /// Shooting info
  /// lightness factor, built up to form_factor * diffuse * area.
  float factor;
  /// polys that are shooting / shot at.
  csRadElement *shoot_src, *shoot_dest;
  /// lumels in worlds coords
  csVector3 src_lumel, dest_lumel;
  /// normals pointing towards shooting.
  csVector3 src_normal, dest_normal;
  /// size of source patches in source polygon, i.e. the size of the lumels
  float source_poly_patch_area;
  /// size of the source patch, width, height
  int srcp_width, srcp_height;
  /// area of source patch visible for lighting
  float source_patch_area;
  /// index into source maps
  int src_uv;
  /// x, y of source lumel
  int src_x, src_y;
  /// texture map of source polygon - reasonably quick to compute, saves
  /// a lot of space.
  csRGBMap *texturemap;
  /// the shadows lying on the dest polygon, 1=full visible, 0=all shadow
  csCoverageMatrix *shadow_matrix;
  /// color from passing portals between source and dest polygon.
  csColor trajectory_color;
  /// color of source lumel for multiplying delta's with.
  csColor src_lumel_color;
  /// delta to add to destination, the summed delta  * src_lumel_colour
  csColor delta_color;

public:
  /// create all radiosity data.
  csRadiosity(iEngine *current_engine, iProgressMeter* meter);
  /// get rid of radiosity data.
  ~csRadiosity();
  /// Does the whole radiosity thing. This is the one to call.
  void DoRadiosity();

  /**
   * Do the radiosity a few steps at a time. This is useful for debugging.
   * Returns false on stop criterium.
   */
  bool DoRadiosityStep (int steps);
  /// For step-by-step radiosity: return next polygon to process.
  csPolygon3D* GetNextPolygon ();
  /**
   * For debugging: temporarily show all delta maps.
   * Calling this again will restore the situation.
   */
  void ToggleShowDeltaMaps ();
  /**
   * Restore the static maps after doing ToggleShowDeltaMaps().
   * Calling DoRadiosityStep() will also do this.
   */
  void RestoreStaticMaps ();

  /// get next best poly to shoot, or 0 if we should stop.
  csRadElement* FetchNext();
  /// Start a sector frustum to shoot from the source. callback is used.
  void StartFrustum();
  /// found a destination polygon, test and process it
  void ProcessDest(csRadElement *dest, csFrustumView *lview);
  /// Shoot light from one polygon to another
  void ShootRadiosityToElement(csRadElement* dest);
  /// Prepare to shoot from source poly
  void PrepareShootSource(csRadElement* src);
  /// Prepare to shoot from source to dest, if false skip dest.
  bool PrepareShootDest(csRadElement* dest, csFrustumView *lview);
  /// Prepare to shoot from a lumel
  void PrepareShootSourceLumel(int sx, int sy, int suv);
  /// Shoot it, dest lumel given.
  void ShootPatch(int rx, int ry, int ruv);


  /// Apply all deltamaps if stopped early, and add ambient light
  void ApplyDeltaAndAmbient();
  /// Remove old ambient, for now.
  void RemoveAmbient();

};

#endif

#endif // __CS_RADIOSTY_H__
