/*
    Copyright (C) 1998,2000 by Jorrit Tyberghein

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

#ifndef CSSPRITE_H
#define CSSPRITE_H

#include "csutil/cscolor.h"
#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/poly2d.h"
#include "csgeom/poly3d.h"
#include "csgeom/box.h"
#include "csobject/nobjvec.h"
#include "csengine/polyint.h"
#include "csengine/bspbbox.h"
#include "csengine/rview.h"
#include "csengine/texture.h"
#include "csengine/tranman.h"
#include "csengine/triangle.h"
#include "igraph3d.h"

class Dumper;
class csTextureList;
class csTextureHandle;
class csLightHitsSprite;
class csSkeleton;
class csSkeletonState;
class csSprite3D;
class csBspContainer;
struct iTextureHandle;

/**
 * A frame for 3D sprite animation.
 */
class csFrame : public csBase
{
private:
  int animation_index;
  int texturing_index;
  char* name;

  /// Bounding box in object space for this frame.
  csBox3 box;

public:
  ///
  csFrame (int anm_idx, int tex_idx);
  ///
  virtual ~csFrame ();

  ///
  int GetAnmIndex () { return animation_index; }
  ///
  int GetTexIndex () { return texturing_index; }
  ///
  void SetTexIndex (int tex_idx) { texturing_index = tex_idx; }

  ///
  void SetName (char * n);
  ///
  char* GetName () { return name; }

  /**
   * Compute the object space bounding box for this frame.
   * This has to be called after setting up the frame and before
   * using it.
   */
  // void ComputeBoundingBox (int num_vertices);
  void SetBoundingBox (csBox3& b) { box = b; }

  /**
   * Get the bounding box in object space.
   */
  void GetBoundingBox (csBox3& b) { b = box; }
};

/**
 * A Action frameset for a 3D sprite animation.
 */
class csSpriteAction : public csBase
{
public:
  /// Initialize a action object
  csSpriteAction ();
  /// Destroy this action object
  virtual ~csSpriteAction ();

  /// Add a frame to this action
  void AddFrame (csFrame * frame, int delay);
  /// Set action name
  void SetName (char *n);
  /// Get action name
  char * GetName ()
  { return name; }
  /// Get total number of frames in this action
  int GetNumFrames ()
  { return frames.Length (); }
  /// Query the frame number f
  csFrame* GetFrame (int f)
  { return (f < frames.Length ()) ? (csFrame *)frames [f] : (csFrame*)NULL; }
  /// Get delay for frame number f
  int GetFrameDelay (int f)
  { return (int)delays [f]; }

private:
  char *name;
  csVector frames;
  csVector delays;
};

/**
 * A 3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 * This class represents a template from which a csSprite3D
 * class can be made.
 */
class csSpriteTemplate : public csObject
{
  friend class Dumper;

private:
  friend class csSprite3D;
  friend class csCollider;

  /// Texture handle as returned by iTextureManager.
  csTextureHandle* cstxt;

  /// An optional skeleton.
  csSkeleton* skeleton;

  /**
   * The order in which to introduce levels in order to get to a higher LOD.
   * The index of this array is the vertex number which is introduced.
   * The vertices of this template were reordered (by GenerateLOD()) so that
   * the first vertices are used in low-detail. The contents of this array
   * is the vertex number to emerge from.
   */
  int* emerge_from;

  /// The frames
  csNamedObjVector frames;
  /// The actions (a vector of csSpriteAction objects)
  csNamedObjVector actions;

  /// The base mesh is also the texture alignment mesh.
  csTriangleMesh* texel_mesh;
  csVector texels;

  /// This mesh is optimized for skeletal and vertex animation.
  csTriangleMesh* vertex_mesh;
  csVector vertices;

  /// The normal mesh is used for smooth shading control.
  csTriangleMesh* normal_mesh;
  csVector normals;

  /// Array that maps Texels to Normals
  int* texel_to_normal;
  /// Array that maps Texels to Vertices
  int* texel_to_vertex;

public:

  /// Create the sprite template
  csSpriteTemplate ();
  /// Destroy the template
  virtual ~csSpriteTemplate ();

  /**
   * Create a new sprite for this template.
   * The 'default' action will be made default. If there is
   * no default action the first action will be made default.
   * The sprite will also be initialized (csSprite3D::InitSprite()).
   */
  csSprite3D* NewSprite ();

  /// Set the skeleton for this sprite template.
  void SetSkeleton (csSkeleton* sk);

  /// Get the skeleton for this sprite template.
  csSkeleton* GetSkeleton () { return skeleton; }

  /// Get the 'emerge_from' array from which you can construct triangles.
  int* GetEmergeFrom () { return emerge_from; }

  /**
   * Generate the collapse order.
   * This function will also reorder all the vertices in the template.
   * So be careful!
   */
  void GenerateLOD ();

  /**
   * Compute the object space bounding box for all frames in this
   * template. This has to be called after setting up the template and before
   * using it.
   */
  void ComputeBoundingBox ();

  ///
  csTriangleMesh* GetTexelMesh () {return texel_mesh;}

  /// Add some vertices, normals, and texels
  void AddVertices (int num);
  /// Add a vertex, normal, and texel
  void AddVertex () { AddVertices (1); }

  /// Query the number of texels.
  int GetNumTexels () { return ((csPoly2D*)(texels.Get(0)))->GetNumVertices (); }
  ///
  csVector2& GetTexel (int frame, int vertex)
    { return (*((csPoly2D*)texels.Get(frame)))[vertex]; }

  /// Query the number of vertices.
  int GetNumVertices () { return ((csPoly3D*)(vertices.Get(0)))->GetNumVertices (); }
  ///
  csVector3& GetVertex (int frame, int vertex)
    { return (*((csPoly3D*)vertices.Get(frame)))[texel_to_vertex[vertex]]; }

  /// Query the number of normals.
  int GetNumNormals () { return ((csPoly3D*)(normals.Get(0)))->GetNumVertices (); }
  ///
  csVector3& GetNormal (int frame, int vertex)
    { return (*((csPoly3D*)normals.Get(frame)))[texel_to_normal[vertex]]; }

  /**
   * Add a triangle to the normal, texel, and vertex meshes
   * a, b and c are indices to texel vertices
   */
  void AddTriangle (int a, int b, int c);
  /// returns the texel indices for triangle 'x'  
  csTriangle GetTriangle (int x) { return texel_mesh->GetTriangle(x); }
  /// returns the triangles of the texel_mesh
  csTriangle* GetTriangles ()    { return texel_mesh->GetTriangles(); }
  /// returns the number of triangles in the sprite
  int GetNumTriangles ()         { return texel_mesh->GetNumTriangles(); }

  /// Create and add a new frame to the sprite.
  csFrame* AddFrame ();
  /// find a named frame into the sprite.
  csFrame* FindFrame (char * name);
  /// Query the number of frames
  int GetNumFrames () { return frames.Length (); }
  /// Query the frame number f
  csFrame* GetFrame (int f)
  { return (f < frames.Length ()) ? (csFrame *)frames [f] : (csFrame*)NULL; }

  /// Create and add a new action frameset to the sprite.
  csSpriteAction* AddAction ();
  /// find a named action into the sprite.
  csSpriteAction* FindAction (const char * name);
  /// Get the first action.
  csSpriteAction* GetFirstAction ()
  { return (csSpriteAction *)actions [0]; }
  /// Get number of actions in sprite
  int GetNumActions ()
  { return actions.Length (); }
  /// Get action number No
  csSpriteAction* GetAction (int No)
  { return (csSpriteAction *)actions [No]; }

  /// Get the texture
  csTextureHandle* GetTexture () const { return cstxt; }
  /// Get the texture handle.
  iTextureHandle* GetTextureHandle () const { return cstxt->GetTextureHandle (); }
  /// Set the texture used for this sprite
  void SetTexture (csTextureList* textures, const char *texname);

  /**
   * Compute all normals in a frame.
   */
  void ComputeNormals (csFrame* frame, csVector3* object_verts);

  /**
   * Minimize the number of 3D coordinates.
   * Reduces the size of animation frames.
   * Speeds up interpolation and skeletal animation.
   */
  int MergeVertices (csFrame * frame);
  int MergeVertices (char * action, int frame)
    {return MergeVertices (FindAction(action)->GetFrame(frame));}
  int MergeVertices (int frame)
    {return MergeVertices (GetFrame(frame));}

  /**
   * Combine normals of adjacent vertices based on one special frame.
   * Eliminates unwanted gouraud shading "seams".
   */
  int MergeNormals (csFrame * frame);
  int MergeNormals (char * action, int frame)
    {return MergeNormals (FindAction(action)->GetFrame(frame));}
  int MergeNormals (int frame)
    {return MergeNormals (GetFrame(frame));}

  /**
   * Merge identical texel frames
   * Returns number of redundant texel maps deleted
   */
  int MergeTexels ();

  CSOBJTYPE;
};

/// A callback function for csSprite3D::Draw().
typedef void (csSpriteCallback) (csSprite3D* spr, csRenderView* rview);
/// A callback function for csSprite3D::Draw().
typedef void (csSpriteCallback2) (csSprite3D* spr, csRenderView* rview, csObject *callbackData);

/**
 * The base class for all types of sprites.
 */
class csSprite : public csObject
{
  friend class Dumper;

protected:
  /// Points to Actor class which "owns" this sprite.
  csObject* myOwner;

  /// Bounding box for polygon trees.
  csPolyTreeBBox bbox;

  /**
   * Camera space bounding box is cached here.
   * GetCameraBoundingBox() will check the current cookie from the
   * transformation manager to see if it needs to recalculate this.
   */
  csBox3 camera_bbox;

  /// Current cookie for camera_bbox.
  csTranCookie camera_cookie;

  /// Mixmode for the triangles/polygons of the sprite.
  UInt MixMode;

  /**
   * List of light-hits-sprites for this sprite.
   */
  csLightHitsSprite* dynamiclights;

  /// Defered lighting. If > 0 then we have defered lighting.
  int defered_num_lights;

  /// Flags to use for defered lighting.
  int defered_lighting_flags;

  /// The callback which is called just before drawing.
  csSpriteCallback* draw_callback;

  /// This callback is only called if the sprite is actually drawn.
  csSpriteCallback2* draw_callback2;

  /**
   * Flag which is set to true when the sprite is visible.
   * This is used by the c-buffer/bsp routines. The sprite itself
   * will not use this flag in any way at all. It is simply intended
   * for external visibility culling routines.
   */
  bool is_visible;

  /**
   * Update the bounding box for the polygon tree
   * algorithm.
   */
  virtual void UpdatePolyTreeBBox () = 0;

public:
  /// List of sectors where this sprite is.
  csNamedObjVector sectors;

public:
  /// Constructor.
  csSprite ();
  /// Destructor.
  virtual ~csSprite ();

  /// Set owner (actor) for this sprite.
  void SetMyOwner (csObject *newOwner) { myOwner = newOwner; }
  /// Get owner (actor) for this sprite.
  csObject* GetMyOwner () { return myOwner; }

  /// Get the bounding box object for the polygon tree.
  csPolyTreeBBox& GetBBoxObject () { return bbox; }

  /// Mark this sprite as visible.
  void MarkVisible () { is_visible = true; }

  /// Mark this sprite as invisible.
  void MarkInvisible () { is_visible = false; }

  /// Return if this sprite is visible.
  bool IsVisible () { return is_visible; }

  /**
   * Update lighting as soon as the sprite becomes visible.
   * This will call world->GetNearestLights with the supplied
   * parameters.
   */
  void DeferUpdateLighting (int flags, int num_lights);

  /// Sets the mode that is used, when drawing that sprite.
  void SetMixmode (UInt m) { MixMode = m; }

  /**
   * Set a callback which is called just before the sprite is drawn.
   * This is useful to do some expensive computations which only need
   * to be done on a visible sprite.
   */
  void SetDrawCallback (csSpriteCallback* callback) { draw_callback = callback; }

  /**
   * Set a callback which is called only if the sprite is actually drawn.
   */
  void SetDrawCallback2 (csSpriteCallback2* callback) { draw_callback2 = callback; }

  /**
   * Get the draw callback. If there are multiple draw callbacks you can
   * use this function to chain.
   */
  csSpriteCallback* GetDrawCallback () { return draw_callback; }

  /**
   * Get the draw callback. If there are multiple draw callbacks you can
   * use this function to chain.
   */
  csSpriteCallback2* GetDrawCallback2 () { return draw_callback2; }

  /// Move this sprite to one sector (conveniance function).
  void MoveToSector (csSector* s);

  /// Remove this sprite from all sectors it is in (but not from the world).
  void RemoveFromSectors ();

  /**
   * Unlink a light-hits-sprite from the list.
   * Warning! This function does not test if it
   * is really on the list!
   */
  void UnlinkDynamicLight (csLightHitsSprite* lp);

  /**
   * Add a light-hits-sprite to the list.
   */
  void AddDynamicLight (csLightHitsSprite *lp);

  /**
   * Get the list of dynamic lights that hit this sprite.
   */
  csLightHitsSprite* GetDynamicLights () { return dynamiclights; }

  /**
   * Light sprite according to the given array of lights (i.e.
   * fill the vertex color array).
   * No shadow calculation will be done. This is assumed to have
   * been done earlier. This is a primitive lighting process
   * based on the lights which hit one point of the sprite (usually
   * the center). More elaborate lighting systems are possible
   * but this will do for now.
   */
  virtual void UpdateLighting (csLight** lights, int num_lights) = 0;

  /**
   * Draw this sprite given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  virtual void Draw (csRenderView& rview) = 0;

  CSOBJTYPE;
};


/**
 * A 3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames (a frame may be controlled by
 * a skeleton).
 */
class csSprite3D : public csSprite
{
  friend class Dumper;
  friend class csCollider;

private:
  /// Set the size of internally used tables
  static void UpdateWorkTables (int max_size);

  /// Update defered lighting.
  void UpdateDeferedLighting ();

public:
  /**
   * Configuration value for global LOD. 0 is lowest detail, 1 is maximum.
   * If negative then the base mesh is used and no LOD reduction/computation
   * is done.
   */
  static float cfg_lod_detail;

  /**
   * Quality setting for sprite lighting. If true this uses
   * high quality lighting which is more accurate on the vertices.
   * Otherwise an approximation is used. This is a lot faster though.
   */
  static bool do_quality_lighting;

private:
  /// Object to world transformation.
  csVector3 v_obj2world;
  /// Object to world transformation.
  csMatrix3 m_obj2world;
  /// World to object transformation.
  csMatrix3 m_world2obj;

  /**
   * A mesh which contains a number of triangles as generated
   * by the LOD algorithm. This is static since it will likely
   * change every frame anyway. We hold it static also since
   * we don't want to allocate it again every time.
   */
  static csTriangleMesh mesh;

  /**
   * Array of colors for the vertices. If not set then this
   * sprite does not have colored vertices.
   */
  csColor* vertex_colors;

  /// The template.
  csSpriteTemplate* tpl;

  /// The texture handle as returned by iTextureManager.
  csTextureHandle* cstxt;

  /// The current frame number.
  int cur_frame;
  /// The current action.
  csSpriteAction* cur_action;

  /// The last frame time action.
  time_t last_time;

  /// Animation tweening ratio:  next frame / this frame.
  float tween_ratio;

  ///
  bool force_otherskin;

  /// Skeleton state (optional).
  csSkeletonState* skeleton_state;

  /**
   * High quality version of UpdateLighting() which recalculates
   * the distance between the light and every vertex.
   */
  void UpdateLightingHQ (csLight** lights, int num_lights, csVector3* object_vertices);

  /**
   * Low quality version of UpdateLighting() which only
   * calculates the distance once (with the center of the sprite).
   */
  void UpdateLightingLQ (csLight** lights, int num_lights, csVector3* object_vertices);
  
  /**
   * Update the bounding box for the polygon tree
   * algorithm.
   */
  virtual void UpdatePolyTreeBBox ();

public:
  ///
  csSprite3D ();
  ///
  virtual ~csSprite3D ();

  ///
  void SetTemplate (csSpriteTemplate* tmpl);

  ///
  csSpriteTemplate* GetTemplate () { return tpl; }

  /// Get the skeleton state for this sprite.
  csSkeletonState* GetSkeletonState () { return skeleton_state; }

  /// force a new texture skin other than default
  void SetTexture (const char* name, csTextureList* textures);

  /**
   * Set a color for a vertex.
   * As soon as you use this function this sprite will be rendered
   * using gouraud shading. Calling this function for the first time
   * will initialize all colors to black.
   */
  void SetVertexColor (int i, const csColor& col);

  /**
   * Add a color for a vertex.
   * As soon as you use this function this sprite will be rendered
   * using gouraud shading. Calling this function for the first time
   * will initialize all colors to black.
   */
  void AddVertexColor (int i, const csColor& col);

  /**
   * Reset the color list. If you call this function then the
   * sprite will no longer use gouraud shading.
   */
  void ResetVertexColors ();

  /**
   * Clamp all vertice colors to 2.0. This is called inside
   * csSprite3D::UpdateLighting() so that 3D renderer doesn't have
   * to deal with brightness lighter than 2.0
   */
  void FixVertexColors ();

  /**
   * Light sprite according to the given array of lights (i.e.
   * fill the vertex color array).
   */
  virtual void UpdateLighting (csLight** lights, int num_lights);

  ///
  void UnsetTexture ()
  { force_otherskin = false; }

  /**
   * Set the transformation vector to move sprite to some position.
   */
  void SetMove (const csVector3& v) { SetMove (v.x, v.y, v.z); }

  /**
   * Set the transformation vector to move sprite to some position.
   */
  void SetMove (float x, float y, float z);

  /**
   * Set the transformation matrix to rotate the sprite in some
   * orientation
   */
  void SetTransform (const csMatrix3& matrix);

  /**
   * Relative move
   */
  void Move (float dx, float dy, float dz);

  /**
   * Relative move
   */
  void Move (csVector3& v) { Move (v.x, v.y, v.z); }

  /**
   * Absolute move
   */
  bool MoveTo (const csVector3& v);

  /**
   * The same as above
   */
  bool MoveTo (float x, float y, float z) { return MoveTo (csVector3 (x, y, z)); }

  /**
   * Relative transform.
   */
  void Transform (csMatrix3& matrix);

  /**
   * Fill the static mesh with the current sprite
   * for a given LOD level.
   */
  void GenerateSpriteLOD (int num_vts);

  /**
   * Get a 3D bounding box in object space.
   */
  void GetObjectBoundingBox (csBox3& boundingBox);

  /**
   * Get a 3D bounding box in camera space. This function is smart.
   * It will only recompute this information if needed. So if you call
   * this function several times in the same frame it will not recompute
   * the bounding box.
   */
  void GetCameraBoundingBox (const csCamera& camtrans, csBox3& boundingBox);

  /**
   * Get the coordinates of the sprite in screen coordinates.
   * Fills in the boundingBox with the X and Y locations
   * of the sprite.  Returns the max Z location of the sprite,
   * or -1 if the sprite is not on-screen.
   * If the sprite is not on-screen, the X and Y values are
   * not valid.
   */
  float GetScreenBoundingBox (const csCamera& camtrans, csBox& boundingBox);

  /**
   * Draw this sprite given a camera transformation.
   * If needed the skeleton state will first be updated.
   * Optionally update lighting if needed (DeferUpdateLighting()).
   */
  virtual void Draw (csRenderView& rview);

  /**
   * Go to the next frame depending on the current time in milliseconds.
   */
  bool NextFrame (time_t current_time, bool onestep = false, bool stoptoend = false);

  /**
   * Go to a specified frame.
   */
  void SetFrame (int f)
  {
    if (cur_action && f < cur_action->GetNumFrames ()) cur_frame = f;
    UpdatePolyTreeBBox ();
  }

  /**
   * Get the current frame number.
   */
  int GetCurFrame () { return cur_frame; }

  /**
   * Get the current frame number.
   */
  csSpriteAction* GetCurAction () { return cur_action; }

  /**
   * Get the number of frames.
   */
  int GetNumFrames () { return cur_action->GetNumFrames (); }

  /**
   * Select an action.
   */
  void SetAction (const char * name)
  {
    csSpriteAction *act;
    if ((act = tpl->FindAction (name)) != NULL)
    {
      SetFrame (0);
      cur_action = act;
    }
  }

  /**
   * Initialize a sprite. This function is called automatically
   * from within 'load'. However you should call it directly
   * if you created the sprite on the fly (without 'load').
   */
  void InitSprite ();

  /// Get world to local transformation matrix
  inline csMatrix3 GetW2T () const { return m_world2obj; }
  /// Get world to local translation
  inline csVector3 GetW2TTranslation () const { return -v_obj2world; }

  /// Get sprite origin
  inline const csVector3 &GetOrigin () const { return v_obj2world; }
  /// Get sprite transform
  inline const csMatrix3 &GetT2W () const { return m_obj2world; }

  /**
   * Get an array of object vertices which is valid for the given frame.
   * This function correcty acounts for sprites which use skeletons. In
   * that case it will use the current transformation state of the skeleton
   * to compute object space vertices.<br>
   * Warning! The returned array should be used immediatelly or copied. It
   * points to a private static array in the sprite class and can be reused
   * if other calls to the sprite happen.
   */
  csVector3* GetObjectVerts (csFrame* fr);

  CSOBJTYPE;
};

#endif /*CSSPRITE_H*/
