/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_MDLDATA_H__
#define __CS_MDLDATA_H__

#include "csextern.h"

#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/csobject.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"

#include "imesh/mdldata.h"

#define CS_DECLARE_ACCESSOR_METHODS(type,name)				\
  type Get##name () const;						\
  void Set##name (type)

#define CS_DECLARE_ARRAY_INTERFACE_NONUM(type,sing_name)		\
  type Get##sing_name (size_t n) const;					\
  void Set##sing_name (size_t n, type)

#define CS_DECLARE_ARRAY_INTERFACE(type,sing_name)			\
  CS_DECLARE_ARRAY_INTERFACE_NONUM (type, sing_name);			\
  size_t Get##sing_name##Count () const;				\
  size_t Add##sing_name (type obj);					\
  void Delete##sing_name (size_t n)

#define CS_DECLARE_OBJECT_INTERFACE					\
  CS_DECLARE_EMBEDDED_OBJECT (csObject, iObject);			\
  iObject *QueryObject ()

/**
 * @@@ This macro should be cleaned up and moved to SCF!!! It is useful
 * whereever an object should be embedded instead of an interface.
 */
#define CS_DECLARE_EMBEDDED_OBJECT(clname,itf)				\
  struct Embedded_##clname : public clname {				\
    typedef clname __scf_superclass__;					\
    SCF_DECLARE_EMBEDDED_IBASE (iBase);					\
  } scf##itf

/**
 * @@@ This macro should be cleaned up and moved to SCF!!! It is useful
 * whereever an object should be embedded instead of an interface.
 */
#define CS_IMPLEMENT_EMBEDDED_OBJECT(Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_INCREF (Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_DECREF (Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_GETREFCOUNT (Class)			\
  SCF_IMPLEMENT_EMBEDDED_IBASE_REFOWNER (Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY (Class)				\
    void *o = __scf_superclass__::QueryInterface (iInterfaceID, iVersion); \
    if (o) return o;							\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY_END

typedef csRefArray<iObject> csObjectVector;

class csIntArray;

//----------------------------------------------------------------------------

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataTexture : 
  public scfImplementationExt1<csModelDataTexture, csObject, iModelDataTexture>
{
private:
  char *FileName;
  csRef<iImage> Image;
  csRef<iTextureWrapper> TextureWrapper;

public:
  /// Constructor
  csModelDataTexture ();
  /// Destructor
  virtual ~csModelDataTexture ();
  
  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  /// Set the file name of the texture
  void SetFileName (const char *fn);
  /// Return the file name of the texture
  const char *GetFileName () const;

  CS_DECLARE_ACCESSOR_METHODS (iImage*, Image);
  CS_DECLARE_ACCESSOR_METHODS (iTextureWrapper*, TextureWrapper);

  /**
   * Load the image from a file with the current filename (i.e. this
   * texture must have a file name) from the CWD of the given file
   * system. Note: This leaves the texture wrapper untouched.
   */
  void LoadImage (iVFS *VFS, iImageIO *ImageIO, int Format);

  /// Create a texture wrapper from the given texture list
  void Register (iTextureList *tl);

  iModelDataTexture *Clone () const;
};

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataMaterial : 
  public scfImplementationExt1<csModelDataMaterial, csObject, iModelDataMaterial>
{
private:
  csRef<iMaterial> BaseMaterial;
  csRef<iMaterialWrapper> MaterialWrapper;

public:

  /// Constructor
  csModelDataMaterial ();
  /// Destructor
  virtual ~csModelDataMaterial ();

  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  CS_DECLARE_ACCESSOR_METHODS (iMaterial*, BaseMaterial);
  CS_DECLARE_ACCESSOR_METHODS (iMaterialWrapper*, MaterialWrapper);

  /// Create a material wrapper from the given material list
  void Register (iMaterialList *ml);

  iModelDataMaterial *Clone () const;
};

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataVertices : 
  public scfImplementationExt1<csModelDataVertices, csObject, iModelDataVertices>
{
private:
  csDirtyAccessArray<csVector3> Vertices;
  csDirtyAccessArray<csVector3> Normals;
  csDirtyAccessArray<csColor> Colors;
  csDirtyAccessArray<csVector2> Texels;

public:
  /// constructor
  csModelDataVertices ();
  /// concat constructor
  csModelDataVertices (const iModelDataVertices *orig1,
    const iModelDataVertices *orig2);
  /// Destructor
  virtual ~csModelDataVertices();

  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  /// Add all data from another vertex frame to this one
  void CopyFrom (const iModelDataVertices *Other);

  CS_DECLARE_ARRAY_INTERFACE (const csVector3 &, Vertex);
  CS_DECLARE_ARRAY_INTERFACE (const csVector3 &, Normal);
  CS_DECLARE_ARRAY_INTERFACE (const csColor &, Color);
  CS_DECLARE_ARRAY_INTERFACE (const csVector2 &, Texel);
  virtual size_t FindVertex (const csVector3 &v) const;
  virtual size_t FindNormal (const csVector3 &v) const;
  virtual size_t FindColor (const csColor &v) const;
  virtual size_t FindTexel (const csVector2 &v) const;

  iModelDataVertices *Clone () const;
};

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataAction : 
  public scfImplementationExt1<csModelDataAction, csObject, iModelDataAction>
{
private:
  csDirtyAccessArray<float> Times;
  csObjectVector States;

public:
  /// Constructor
  csModelDataAction ();
  /// Destructor
  virtual ~csModelDataAction ();

  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  /// Return the number of key frames
  virtual size_t GetFrameCount () const;
  /// Get the time value for a frame
  virtual float GetTime (size_t Frame) const;
  /// Get the state information for a frame
  virtual iObject *GetState (size_t Frame) const;
  /// Set the time value for a frame
  virtual void SetTime (size_t Frame, float NewTime);
  /// Set the state information for a frame
  virtual void SetState (size_t Frame, iObject *State);
  /// Add a frame
  virtual void AddFrame (float Time, iObject *State);
  /// Delete a frame
  virtual void DeleteFrame (size_t Frame);
  /// Return the total time of one animation cycle
  virtual float GetTotalTime () const;
};

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataPolygon : 
  public scfImplementationExt1<csModelDataPolygon, csObject, iModelDataPolygon>
{
private:
  csDirtyAccessArray<int> Vertices;
  csDirtyAccessArray<int> Normals;
  csDirtyAccessArray<int> Colors;
  csDirtyAccessArray<int> Texels;
  iModelDataMaterial *Material;

public:
  /// constructor
  csModelDataPolygon ();
  /// destructor
  virtual ~csModelDataPolygon ();
  
  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }
  /// Add a vertex
  size_t AddVertex (int ver, int nrm, int col, int tex);
  /// Return the number of vertices
  size_t GetVertexCount () const;
  /// Delete a vertex
  void DeleteVertex (size_t n);

  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Vertex);
  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Normal);
  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Color);
  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Texel);
  CS_DECLARE_ACCESSOR_METHODS (iModelDataMaterial*, Material);

  iModelDataPolygon *Clone () const;
};

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataObject : 
  public scfImplementationExt1<csModelDataObject, csObject, iModelDataObject>
{
private:
  csRef<iModelDataVertices> DefaultVertices;

public:
  /// Constructor
  csModelDataObject ();
  /// Destructor
  virtual ~csModelDataObject();

  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  CS_DECLARE_ACCESSOR_METHODS (iModelDataVertices*, DefaultVertices);
};

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataCamera : 
  public scfImplementationExt1<csModelDataCamera, csObject, iModelDataCamera>
{
private:
  csVector3 Position, UpVector, FrontVector, RightVector;

public:
  /// Constructor
  csModelDataCamera ();
  // Destructor
  virtual ~csModelDataCamera ();

  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  CS_DECLARE_ACCESSOR_METHODS (const csVector3 &, Position);
  CS_DECLARE_ACCESSOR_METHODS (const csVector3 &, UpVector);
  CS_DECLARE_ACCESSOR_METHODS (const csVector3 &, FrontVector);
  CS_DECLARE_ACCESSOR_METHODS (const csVector3 &, RightVector);

  /// compute the 'up' vector as the normal to the 'front' and 'right' vectors
  void ComputeUpVector ();
  /// compute the 'front' vector as the normal to the 'up' and 'right' vectors
  void ComputeFrontVector ();
  /// compute the 'right' vector as the normal to the 'up' and 'front' vectors
  void ComputeRightVector ();

  /// normalize all direction vectors
  void Normalize ();
  /// test if all direction vectors are orthogonal
  bool CheckOrthogonality () const;

  iModelDataCamera *Clone () const;
};

/// Document me!@@@
class CS_CRYSTALSPACE_EXPORT csModelDataLight : 
  public scfImplementationExt1<csModelDataLight, csObject, iModelDataLight>
{
private:
  float Radius;
  csColor Color;
  csVector3 Position;

public:
  /// Constructor
  csModelDataLight ();
  // Destructor
  virtual ~csModelDataLight ();

  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  CS_DECLARE_ACCESSOR_METHODS (float, Radius);
  CS_DECLARE_ACCESSOR_METHODS (const csVector3 &, Position);
  CS_DECLARE_ACCESSOR_METHODS (const csColor &, Color);
  iModelDataLight *Clone () const;
};

class CS_CRYSTALSPACE_EXPORT csModelData : 
  public scfImplementationExt1<csModelData, csObject, iModelData>
{
public:
  /// Constructor
  csModelData ();
  /// Destructor
  virtual ~csModelData ();

  /// Query the iObject
  virtual iObject* QueryObject ()
  {
    return this;
  }

  /// Load all texture images from the CWD of the given file system
  void LoadImages (iVFS *VFS, iImageIO *il, int Format);
  /// Register all textures using the given texture list
  void RegisterTextures (iTextureList *tl);
  /// Register all materials using the given material list
  void RegisterMaterials (iMaterialList *ml);
};

#endif // __CS_MDLDATA_H__
