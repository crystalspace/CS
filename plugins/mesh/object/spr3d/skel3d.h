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

#ifndef SKEL3D_H
#define SKEL3D_H

#include "csgeom/math3d.h"
#include "csgeom/math2d.h"
#include "csgeom/box.h"
#include "csgeom/transfrm.h"
#include "plugins/mesh/object/spr3d/spr3d.h"
#include "iskel.h"
#include "iskelbon.h"

class csSkeletonLimbState;
class csPoly3D;

/**
 * A limb in a skeletal system. The Crystal Space skeletal system
 * works by controlling the vertices in a frame. So we combine
 * frame based sprites and skeletal sprites.
 */
class csSkeletonLimb : public iSkeletonLimb
{
private:
  /// Next in the list.
  csSkeletonLimb* next;

  /// The vertices from the parent sprite that it controls.
  int* vertices;

  /// The number of vertices to control.
  int num_vertices;

  /// Children of this limb.
  csSkeletonLimb* children;

  /// Bounding box in object space for this limb.
  csBox3 box;

  /// The name of this Limb.
  char* name;

protected:
  /// Update state information.
  void UpdateState (csSkeletonLimbState* limb);

public:
  /// Create an empty limb.
  csSkeletonLimb ();

  /// Destructor.
  virtual ~csSkeletonLimb ();

  /// Add a vertex index.
  virtual void AddVertex (int v);

  /// Get the vertex index array.
  virtual int* GetVertices () { return vertices; }

  /// Get the number of vertices.
  virtual int GetNumVertices () { return num_vertices; }

  /// Get the bounding box.
  void GetBoundingBox (csBox3& b) { b = box; }

  /// Add a child limb.
  void AddChild (csSkeletonLimb* child);

  /// Linked list.
  void SetNext (csSkeletonLimb* n) { next = n; }
  /// Linked list.
  csSkeletonLimb* GetNext () { return next; }
  /// Linked list.
  virtual iSkeletonLimb* GetNextSibling () { return (iSkeletonLimb*)next; }

  /// Create a csSkeletonState from this csSkeleton.
  virtual csSkeletonLimbState* CreateState ();

  /// For LOD. Remap vertices.
  void RemapVertices (int* mapping);

  /**
   * Compute the object space bounding box for this limb.
   */
  void ComputeBoundingBox (csPoly3D* source);

  /// Set the name for this limb.
  virtual void SetName (const char* name);
  /// Get the name for this limb.
  virtual const char* GetName () const { return name; }
  /// Create a connection.
  virtual iSkeletonConnection* CreateConnection ();

  DECLARE_IBASE;
};

/**
 * A connection. This is basically a transformation with a limb that
 * is controlled by the transformation.
 */
class csSkeletonConnection : public csSkeletonLimb
{
private:
  /// A transformation with respect to the parent limb.
  csTransform trans;

public:
  /// Create an empty limb with an identity transformation.
  csSkeletonConnection ();

  /// Create an empty limb with the given transformation.
  csSkeletonConnection (csTransform& tr) : trans (tr) { }

  /// Destructor.
  virtual ~csSkeletonConnection () { }

  /// Set the transformation.
  virtual void SetTransformation (const csTransform& tr) { trans = tr; }

  /// Get the transformation.
  virtual csTransform& GetTransformation () { return trans; }

  /// Create a csSkeletonState from this csSkeleton.
  virtual csSkeletonLimbState* CreateState ();

  DECLARE_IBASE_EXT (csSkeletonLimb);

  struct SkeletonConnection : public iSkeletonConnection
  {
    DECLARE_EMBEDDED_IBASE (csSkeletonConnection);
    virtual void SetTransformation (const csTransform& t)
      { scfParent->SetTransformation(t); }
    virtual csTransform& GetTransformation ()
      { return scfParent->GetTransformation(); }
  } scfiSkeletonConnection;
};

/**
 * The base skeleton class (or the 'body').
 */
class csSkeleton : public csSkeletonLimb
{
public:
  /// Create an empty skeleton.
  csSkeleton ();

  /// Destructor.
  virtual ~csSkeleton () { }

  /// Create a csSkeletonState from this csSkeleton.
  virtual csSkeletonLimbState* CreateState ();

  DECLARE_IBASE_EXT (csSkeletonLimb);

  struct Skeleton : public iSkeleton
  {
    DECLARE_EMBEDDED_IBASE (csSkeleton);
  } scfiSkeleton;
};

/**
 * A limb in a skeletal system (state version, see csSkeletonState for
 * more information).
 */
class csSkeletonLimbState : public iSkeletonLimbState
{
  friend class csSkeletonLimb;

private:
  /// Next in the list.
  csSkeletonLimbState* next;

  /// Pointer to original skeleton node.
  csSkeletonLimb* tmpl;

  /// The vertices from the parent sprite that it controls.
  int* vertices;

  /// The number of vertices to control.
  int num_vertices;

  /// Children of this limb.
  csSkeletonLimbState* children;

  /// The name of this Limb.
  char* name;

  /// Userdata.
  void* data;

  /// Add a child limb.
  void AddChild (csSkeletonLimbState* child);

  /// Linked list.
  void SetNext (csSkeletonLimbState* n) { next = n; }

protected:
  /// Create an empty limb (protected constructor).
  csSkeletonLimbState ();

public:
  /// Destructor.
  virtual ~csSkeletonLimbState ();

  /**
   * Transform the vertices in the given frame to the destination frame.
   * This is a recursive function which traverses all limbs and creates
   * a combined transformation along the way.
   */
  virtual void Transform (const csTransform& tr, csVector3* source, csVector3* dest);

  /**
   * Calculate the real bounding box for the given state.
   */
  virtual void ComputeBoundingBox (const csTransform& tr, csBox3& box);

  /// Get first child.
  csSkeletonLimbState* GetFirstChild () { return children; }
  /// Get first child.
  iSkeletonLimbState* GetChildren () { return (iSkeletonLimbState*)children; }

  /// Get next sibling in list.
  csSkeletonLimbState* GetNext () { return next; }
  /// Get next sibling in list.
  iSkeletonLimbState* GetNextSibling () { return (iSkeletonLimbState*)next; }

  /// Set the name for this limb.
  virtual void SetName (const char* name);
  /// Get the name for this limb.
  virtual const char* GetName () const { return name; }
  /// Set userdata object.
  virtual void SetUserData (void* data) { csSkeletonLimbState::data = data; }
  /// Get userdata object.
  virtual void* GetUserData () { return data; }


  DECLARE_IBASE;
};

/**
 * A connection (state version, see csSkeletonState for more information).
 * This is basicly a transformation with a limb that
 * is controlled by the transformation.
 */
class csSkeletonConnectionState : public csSkeletonLimbState
{
  friend class csSkeletonConnection;

private:
  /// A transformation with respect to the parent limb.
  csTransform trans;

protected:
  /// Create an empty limb with an identity transformation (protected constructor).
  csSkeletonConnectionState ();

public:
  /// Destructor.
  virtual ~csSkeletonConnectionState () { }

  /// Transform the vertices in the given frame to the destination frame.
  virtual void Transform (const csTransform& tr, csVector3* source, csVector3* dest);

  /// Calculate the real bounding box for the given state.
  virtual void ComputeBoundingBox (const csTransform& tr, csBox3& box);

  /// Set the transformation.
  void SetTransformation (const csTransform& tr) { trans = tr; }

  /// Get the transformation.
  csTransform& GetTransformation () { return trans; }

  DECLARE_IBASE_EXT (csSkeletonLimbState);

  struct SkeletonBone : public iSkeletonBone
  {
    DECLARE_EMBEDDED_IBASE (csSkeletonConnectionState);

    virtual iSkeletonBone* GetNext ()
    {
      csSkeletonLimbState* ls=scfParent->GetNext();
      if (!ls) return NULL;
      return QUERY_INTERFACE(ls, iSkeletonBone);
    }
    virtual iSkeletonBone* GetChildren ()
    {
      csSkeletonLimbState* ls=scfParent->GetFirstChild ();
      if (!ls) return NULL;
      return QUERY_INTERFACE(ls, iSkeletonBone);
    }
    virtual const char* GetName ()
    {
      return scfParent->GetName();
    }
    virtual void SetTransformation (const csTransform& tr)
    {
      scfParent->SetTransformation (tr);
    }
    virtual csTransform& GetTransformation ()
    {
      return scfParent->GetTransformation ();
    }
  } scfiSkeletonBone;
  friend struct SkeletonBone;

  struct SkeletonConnectionState : iSkeletonConnectionState
  {
    DECLARE_EMBEDDED_IBASE (csSkeletonConnectionState);
    virtual void SetTransformation (const csTransform& tr)
    {
      scfParent->SetTransformation (tr);
    }
    virtual csTransform& GetTransformation ()
    {
      return scfParent->GetTransformation ();
    }
  } scfiSkeletonConnectionState;
};

/**
 * Skeleton state. This class is the runtime version of a skeleton.
 * The csSkeleton classes are used in a template. They contain a
 * static description of the skeleton code. There is at most one csSkeleton
 * for every csSpriteTemplate. This structure is the run-time
 * version. There is one csSkeletonState for every csSprite3D which
 * uses a skeleton.
 */
class csSkeletonState : public csSkeletonLimbState
{
  friend class csSkeleton;

protected:
  /**
   * Create an empty skeleton.
   * The constructor is protected because it is csSkeleton which
   * creates instances of this class.
   */
  csSkeletonState ();

public:
  /// Destructor.
  virtual ~csSkeletonState () { }

  /// Calculate the real bounding box for the given state.
  virtual void ComputeBoundingBox (const csTransform& tr, csBox3& box);

  DECLARE_IBASE_EXT (csSkeletonLimbState);

  struct SkeletonBone : public iSkeletonBone
  {
    DECLARE_EMBEDDED_IBASE (csSkeletonState);

    virtual iSkeletonBone* GetNext ()
    {
      csSkeletonLimbState* ls=scfParent->GetNext();
      if(!ls) return NULL;
      return QUERY_INTERFACE(ls, iSkeletonBone);
    }
    virtual iSkeletonBone* GetChildren ()
    {
      csSkeletonLimbState* ls=scfParent->GetFirstChild();
      if(!ls) return NULL;
      return QUERY_INTERFACE(ls, iSkeletonBone);
    }
    virtual const char* GetName ()
    {
      return scfParent->GetName();
    }
    // @@@This is ugly. Skeleton doesn't support transformations!
    virtual void SetTransformation (const csTransform&) {}
    csTransform dummy;
    virtual csTransform& GetTransformation () { return dummy; }

  } scfiSkeletonBone;
  friend struct SkeletonBone;

  struct SkeletonState : public iSkeletonState
  {
    DECLARE_EMBEDDED_IBASE (csSkeletonState);
  } scfiSkeletonState;
};

#endif /*SKEL3D_H*/
