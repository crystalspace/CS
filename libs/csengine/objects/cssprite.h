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

#ifndef CSSPRITE_H
#define CSSPRITE_H

#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csengine/basic/csobjvec.h"
#include "csengine/rview.h"
#include "csengine/cscolor.h"
#include "csengine/texture.h"
#include "csengine/colldet/collp.h"

class csTextureList;
class Dumper;
class csTextureHandle;
interface ITextureHandle;

/**
 * A frame for a 3D sprite animation.
 */
class csFrame : public csBase
{
private:
  csVector3* vertices;
  csVector2* texels;
  char *name;
  int max_vertex;

public:
  ///
  csFrame (int max_vertices);
  ///
  virtual ~csFrame ();

  ///
  void SetVertex (int i, float x, float y, float z)
  {
    vertices[i].x = x;
    vertices[i].y = y;
    vertices[i].z = z;
  }
  ///
  void SetTexel (int i, float u, float v)
  {
    texels[i].x = u;
    texels[i].y = v;
  }
  ///
  void SetName (char * n);

  ///
  csVector3& GetVertex (int i)
  { return vertices[i]; }
  ///
  csVector2& GetTexel (int i)
  { return texels[i]; }
  ///
  char* GetName () { return name; }

  ///
  void AddVertex (int num_vertices);
  ///
  int GetMaxVertices ()
  { return max_vertex; }
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

// A triangle for 3D sprites.
struct csTriangle
{
  int a, b, c;
};

class csSprite3D;
class csSpriteTemplate;
class csSpriteEdges;

/**
 * A LOD level for a sprite.
 * This is basicly a collection of triangles. At any one time
 * one LOD is active for the sprite.
 */
class csSpriteLOD
{
private:
  /// The triangles.
  csTriangle* triangles;
  int num_triangles;
  int max_triangles;

public:
  ///
  csSpriteLOD ();
  ///
  ~csSpriteLOD ();

  /// Add a triangle to the LOD level.
  void AddTriangle (int a, int b, int c);
  /// Query the array of LOD triangles.
  csTriangle* GetTriangles () { return triangles; }
  /// Query the number of LOD triangles.
  int GetNumTriangles () { return num_triangles; }

  /**
   * Generate a lower-level LOD for this LOD by using
   * the given edge table (unsorted). The 'pct' parameter is the
   * percentage of triangles to retain. A 'pct' of 20%
   * means that the new set of triangles will only have
   * 20% of the triangles of the base set.
   */
  csSpriteLOD* GenerateLOD (int pct, csSpriteEdges* edges);
};

class csSpriteEdge
{
  public:
    int triangles[10];	// Maximum 10 triangles for every edge@@@!!!
    int num_triangles;	// Number of triangles that this edge connects.
    float sqlength;	// Squared length of this edge.
    int v1, v2;		// Vertex index a and b for this edge.
};

/**
 * A class which holds edge information for a sprite
 * template. It is temporary and created specifically for
 * making LOD levels.
 */
class csSpriteEdges
{
private:
  csSpriteEdge* edges;
  int num_edges;

  void AddEdge (int v1, int v2, int triangle, csVector3& vec1, csVector3& vec2);

public:
  /// Build edge table for this sprite.
  csSpriteEdges (csSpriteTemplate* tpl);
  ///
  ~csSpriteEdges ();

  /// Sort the edges based on some criterium.
  void SortEdges ();

  ///
  int GetNumEdges () { return num_edges; }
  ///
  csSpriteEdge& GetEdge (int idx) { return edges[idx]; }
};

/**
 * A 3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 * This class represents a template from which a csSprite3D
 * class can be made.
 */
class csSpriteTemplate : public csObject
{
  friend Dumper;

private:
  friend class csSprite3D;
  friend class csCollider;

  /// Texture handle as returned by ITextureManager.
  csTextureHandle* cstxt;

  /// The vertices.
  int num_vertices;

  /// The triangles.
  csSpriteLOD* lod;
  csSpriteLOD* lod2;

  /// The frames
  csObjVector frames;
  /// The actions (a vector of csSpriteAction objects)
  csObjVector actions;

public:
  /// Create the sprite template
  csSpriteTemplate ();
  /// Destroy the template
  virtual ~csSpriteTemplate ();

  /// Query the number of vertices
  int GetNumVertices ()
  { return num_vertices; }

  /// Get the highest detail LOD of this sprite (the base LOD).
  csSpriteLOD* GetBaseLOD () { return lod; }

  /// Get some LOD level.
  csSpriteLOD* GetLOD (int level) { return level ? lod2 : lod; }

  /// Generate some LOD level.
  void GenerateLOD (int level, int pct);

  /// Set the number of vertices
  void SetNumVertices (int v) { num_vertices = v; }

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
  csSpriteAction* FindAction (char * name);
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
  ITextureHandle* GetTextureHandle () const { return cstxt->GetTextureHandle (); }
  /// Set the texture used for this sprite
  void SetTexture (csTextureList* textures, char *texname);
  
  CSOBJTYPE;
};

/**
 * A 3D sprite based on a triangle mesh with a single texture.
 * Animation is done with frames.
 */
class csSprite3D : public csObject
{
  friend Dumper;
  friend csCollider;

public:
  /// List of sectors where this sprite is.
  csObjVector sectors;

private:
  /// Object to world transformation.
  csVector3 v_obj2world;
  /// Object to world transformation.
  csMatrix3 m_obj2world;
  /// World to object transformation.
  csMatrix3 m_world2obj;

  /**
   * Array of colors for the vertices. If not set then this
   * sprite does not have colored vertices.
   */
  csColor* vertex_colors;

  /// The template.
  csSpriteTemplate* tpl;

  /// The texture handle as returned by ITextureManager.
  csTextureHandle* cstxt;

  /// The current frame number.
  int cur_frame;
  /// The current action.
  csSpriteAction* cur_action;

  /// The transformed frame (from object->camera space).
  csFrame* tr_frame;

  /// The perspective corrected vertices (screen space) for tr_frame.
  csVector2* persp;

  /// Array which indicates which vertices are visible and which are not.
  bool* visible;

  /// The last frame time action
  int last_time;

  ///
  bool force_otherskin;

public:
  ///
  csSprite3D ();
  ///
  virtual ~csSprite3D ();

  ///
  void SetTemplate (csSpriteTemplate* tmpl);

  ///
  csSpriteTemplate* GetTemplate () { return tpl; }

  /// force a new texture skin other than default
  void SetTexture (char * name, csTextureList* textures);

  /**
   * Set a color for a vertex.
   * As soon as you use this function this sprite will be rendered
   * using gouroud shading. Calling this function for the first time
   * will initialize all colors to black.
   */
  void SetVertexColor (int i, const csColor& col);

  /**
   * Reset the color list. If you call this function then the
   * sprite will no longer use gouroud shading.
   */
  void ResetVertexColors ();

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
   * orientation.
   */
  void SetTransform (csMatrix3& matrix);

  /**
   * Relative move.
   */
  void Move (float dx, float dy, float dz);

  /**
   * Relative move.
   */
  void Move (csVector3& v) { Move (v.x, v.y, v.z); }

  /**
   * Relative transform.
   */
  void Transform (csMatrix3& matrix);

  /**
   * Draw this sprite given a camera transformation.
   */
  void Draw (csRenderView& rview);

  /**
   * Go to the next frame depending on the current time in milliseconds.
   */
  bool NextFrame (long current_time, bool onestep = false, bool stoptoend = false);

  /**
   * Go to a specified frame.
   */
  void SetFrame (int f)
  {
    if (cur_action && f < cur_action->GetNumFrames ()) cur_frame = f;
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
  void SetAction (char * name)
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

  /// Move this sprite to one sector (conveniance function).
  void MoveToSector (csSector* s);

  /// Remove this sprite from all sectors it is in (but not from the world).
  void RemoveFromSectors ();

  /**
   * Collision Detection data.
   */
  csColliderP collider;

  CSOBJTYPE;
};

#endif /*CSSPRITE_H*/
