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

#define DECLARE_ACCESSOR_METHODS(type,name)				\
  type Get##name () const;						\
  void Set##name (type);

#define DECLARE_ARRAY_INTERFACE_NONUM(type,sing_name)			\
  type Get##sing_name (int n) const;					\
  void Set##sing_name (int n, type);

#define DECLARE_ARRAY_INTERFACE(type,sing_name)		\
  DECLARE_ARRAY_INTERFACE_NONUM (type, sing_name)			\
  int Get##sing_name##Count () const;					\
  void Add##sing_name (type obj);					\
  void Delete##sing_name (int n);

#define DECLARE_OBJECT_INTERFACE					\
  DECLARE_EMBEDDED_OBJECT (csObject, iObject);				\
  iObject *QueryObject ();

/**
 * @@@ This macro should be cleaned up and moved to SCF!!! It is useful
 * whereever an object should be embedded instead of an interface.
 */
#define DECLARE_EMBEDDED_OBJECT(clname,itf)				\
  struct Embedded_##clname : public clname {				\
    typedef clname __scf_superclass__;					\
    SCF_DECLARE_EMBEDDED_IBASE (iBase);					\
  } scf##itf;

/**
 * @@@ This macro should be cleaned up and moved to SCF!!! It is useful
 * whereever an object should be embedded instead of an interface.
 */
#define IMPLEMENT_EMBEDDED_OBJECT(Class)				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_INCREF (Class);				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_DECREF (Class);				\
  SCF_IMPLEMENT_EMBEDDED_IBASE_GETREFCOUNT (Class);			\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY (Class);				\
    void *o = __scf_superclass__::QueryInterface (iInterfaceID, iVersion); \
    if (o) return o;							\
  SCF_IMPLEMENT_EMBEDDED_IBASE_QUERY_END;

class csModelDataPolygon : public iModelDataPolygon
{
private:
  DECLARE_GROWING_ARRAY (Vertices, int);
  DECLARE_GROWING_ARRAY (Normals, csVector3);
  DECLARE_GROWING_ARRAY (Colors, csColor);
  DECLARE_GROWING_ARRAY (TextureCoords, csVector2);
  iModelDataMaterial *Material;
public:
  SCF_DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelDataPolygon ();
  /// destructor
  virtual ~csModelDataPolygon ();
  
  /// return the number of vertices
  int GetVertexCount () const;
  /// Add a vertex
  void AddVertex (int PositionIndex, const csVector3 &Normal,
    const csColor &Color, const csVector2 &TextureCoords);
  /// Delete a vertex
  void DeleteVertex (int n);

  DECLARE_ARRAY_INTERFACE_NONUM (int, Vertex);
  DECLARE_ARRAY_INTERFACE_NONUM (const csVector3 &, Normal);
  DECLARE_ARRAY_INTERFACE_NONUM (const csVector2 &, TextureCoords);
  DECLARE_ARRAY_INTERFACE_NONUM (const csColor &, Color);
  DECLARE_ACCESSOR_METHODS (iModelDataMaterial*, Material);
};

class csModelDataObject : public iModelDataObject
{
private:
  DECLARE_GROWING_ARRAY (Vertices, csVector3);

public:
  SCF_DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;
  DECLARE_ARRAY_INTERFACE (const csVector3&, Vertex);

  /// Constructor
  csModelDataObject ();
  /// Destructor
  virtual ~csModelDataObject() {}
};

class csModelDataCamera : public iModelDataCamera
{
private:
  csVector3 Position, UpVector, FrontVector, RightVector;

public:
  SCF_DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataCamera ();
  // Destructor
  virtual ~csModelDataCamera () {}

  DECLARE_ACCESSOR_METHODS (const csVector3 &, Position);
  DECLARE_ACCESSOR_METHODS (const csVector3 &, UpVector);
  DECLARE_ACCESSOR_METHODS (const csVector3 &, FrontVector);
  DECLARE_ACCESSOR_METHODS (const csVector3 &, RightVector);

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
  DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataLight ();
  // Destructor
  virtual ~csModelDataLight () {}

  DECLARE_ACCESSOR_METHODS (float, Radius);
  DECLARE_ACCESSOR_METHODS (const csVector3 &, Position);
  DECLARE_ACCESSOR_METHODS (const csColor &, Color);
};

class csModelDataMaterial : public iModelDataMaterial
{
private:

public:
  SCF_DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelDataMaterial ();
  /// Destructor
  virtual ~csModelDataMaterial () {}
};

class csModelData : public iModelData
{
public:
  SCF_DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// Constructor
  csModelData ();
  /// Destructor
  virtual ~csModelData () {}
};

#endif // __MDLDATA_H__
