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

#ifndef __MDLDATA_H__
#define __MDLDATA_H__

#include "imesh/mdldata.h"
#include "csutil/garray.h"
#include "csutil/csobject.h"
#include "csutil/typedvec.h"

#define CS_DECLARE_ACCESSOR_METHODS(type,name)				\
  type Get##name () const;						\
  void Set##name (type);

#define CS_DECLARE_ARRAY_INTERFACE_NONUM(type,sing_name)		\
  type Get##sing_name (int n) const;					\
  void Set##sing_name (int n, type);

#define CS_DECLARE_ARRAY_INTERFACE(type,sing_name)			\
  CS_DECLARE_ARRAY_INTERFACE_NONUM (type, sing_name)			\
  int Get##sing_name##Count () const;					\
  int Add##sing_name (type obj);					\
  void Delete##sing_name (int n);

#define CS_DECLARE_OBJECT_INTERFACE					\
  CS_DECLARE_EMBEDDED_OBJECT (csObject, iObject);			\
  iObject *QueryObject ();

/**
 * @@@ This macro should be cleaned up and moved to SCF!!! It is useful
 * whereever an object should be embedded instead of an interface.
 */
#define CS_DECLARE_EMBEDDED_OBJECT(clname,itf)				\
  struct Embedded_##clname : public clname {				\
    typedef clname __scf_superclass__;					\
    SCF_DECLARE_EMBEDDED_IBASE (iBase);					\
  } scf##itf;

/**
 * @@@ This macro should be cleaned up and moved to SCF!!! It is useful
 * whereever an object should be embedded instead of an interface.
 */
#define CS_IMPLEMENT_EMBEDDED_OBJECT(Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_INCREF (Class);				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_DECREF (Class);				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_GETREFCOUNT (Class);			\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY (Class);				\
    void *o = __scf_superclass__::QueryInterface (iInterfaceID, iVersion); \
    if (o) return o;							\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY_END;

CS_DECLARE_TYPED_IBASE_VECTOR (csObjectVector, iObject);

class csIntArray;

//----------------------------------------------------------------------------

class csModelDataTexture : public iModelDataTexture
{
private:
  char *FileName;
  iImage *Image;
  iTextureWrapper *TextureWrapper;
public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataTexture ();
  /// Destructor
  virtual ~csModelDataTexture ();

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
};

class csModelDataMaterial : public iModelDataMaterial
{
private:
  iMaterial *BaseMaterial;
  iMaterialWrapper *MaterialWrapper;
public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataMaterial ();
  /// Destructor
  virtual ~csModelDataMaterial ();

  CS_DECLARE_ACCESSOR_METHODS (iMaterial*, BaseMaterial);
  CS_DECLARE_ACCESSOR_METHODS (iMaterialWrapper*, MaterialWrapper);

  /// Create a material wrapper from the given material list
  void Register (iMaterialList *ml);
};

class csModelDataVertices : public iModelDataVertices
{
private:
  CS_DECLARE_GROWING_ARRAY (Vertices, csVector3);
  CS_DECLARE_GROWING_ARRAY (Normals, csVector3);
  CS_DECLARE_GROWING_ARRAY (Colors, csColor);
  CS_DECLARE_GROWING_ARRAY (Texels, csVector2);

public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelDataVertices ();
  /// Destructor
  virtual ~csModelDataVertices() {}

  CS_DECLARE_ARRAY_INTERFACE (const csVector3 &, Vertex);
  CS_DECLARE_ARRAY_INTERFACE (const csVector3 &, Normal);
  CS_DECLARE_ARRAY_INTERFACE (const csColor &, Color);
  CS_DECLARE_ARRAY_INTERFACE (const csVector2 &, Texel);
};

class csModelDataAction : public iModelDataAction
{
private:
  CS_DECLARE_GROWING_ARRAY (Times, float);
  csObjectVector States;

public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataAction ();
  /// Destructor
  virtual ~csModelDataAction () { }
  
  /// Return the number of key frames
  virtual int GetFrameCount () const;
  /// Get the time value for a frame
  virtual float GetTime (int Frame) const;
  /// Get the state information for a frame
  virtual iObject *GetState (int Frame) const;
  /// Set the time value for a frame
  virtual void SetTime (int Frame, float NewTime);
  /// Set the state information for a frame
  virtual void SetState (int Frame, iObject *State);
  /// Add a frame
  virtual void AddFrame (float Time, iObject *State);
  /// Delete a frame
  virtual void DeleteFrame (int Frame);
  /// Return the total time of one animation cycle
  virtual float GetTotalTime () const;
};

class csModelDataPolygon : public iModelDataPolygon
{
private:
  CS_DECLARE_GROWING_ARRAY (Vertices, int);
  CS_DECLARE_GROWING_ARRAY (Normals, int);
  CS_DECLARE_GROWING_ARRAY (Colors, int);
  CS_DECLARE_GROWING_ARRAY (Texels, int);
  iModelDataMaterial *Material;

public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelDataPolygon ();
  /// destructor
  virtual ~csModelDataPolygon ();
  
  /// Add a vertex
  int AddVertex (int ver, int nrm, int col, int tex);
  /// Return the number of vertices
  int GetVertexCount () const;
  /// Delete a vertex
  void DeleteVertex (int n);

  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Vertex);
  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Normal);
  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Color);
  CS_DECLARE_ARRAY_INTERFACE_NONUM (int, Texel);
  CS_DECLARE_ACCESSOR_METHODS (iModelDataMaterial*, Material);
};

class csModelDataObject : public iModelDataObject
{
private:
  iModelDataVertices *DefaultVertices;

public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataObject ();
  /// Destructor
  virtual ~csModelDataObject();

  CS_DECLARE_ACCESSOR_METHODS (iModelDataVertices*, DefaultVertices);
};

class csModelDataCamera : public iModelDataCamera
{
private:
  csVector3 Position, UpVector, FrontVector, RightVector;

public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataCamera ();
  // Destructor
  virtual ~csModelDataCamera () {}

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
};

class csModelDataLight : public iModelDataLight
{
private:
  float Radius;
  csColor Color;
  csVector3 Position;

public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataLight ();
  // Destructor
  virtual ~csModelDataLight () {}

  CS_DECLARE_ACCESSOR_METHODS (float, Radius);
  CS_DECLARE_ACCESSOR_METHODS (const csVector3 &, Position);
  CS_DECLARE_ACCESSOR_METHODS (const csColor &, Color);
};

class csModelData : public iModelData
{
public:
  SCF_DECLARE_IBASE;
  CS_DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelData ();
  /// Destructor
  virtual ~csModelData () {}

  /// Load all texture images from the CWD of the given file system
  void LoadImages (iVFS *VFS, iImageIO *il, int Format);
  /// Register all textures using the given texture list
  void RegisterTextures (iTextureList *tl);
  /// Register all materials using the given material list
  void RegisterMaterials (iMaterialList *ml);
};

#endif // __MDLDATA_H__
