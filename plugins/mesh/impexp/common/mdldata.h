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

#define DECLARE_ARRAY_INTERFACE(type,sing_name,mult_name)		\
  DECLARE_ARRAY_INTERFACE_NONUM (type, sing_name)			\
  int GetNum##mult_name () const;					\
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
    DECLARE_EMBEDDED_IBASE (iBase);					\
  } scf##itf;

/**
 * @@@ This macro should be cleaned up and moved to SCF!!! It is useful
 * whereever an object should be embedded instead of an interface.
 */
#define IMPLEMENT_EMBEDDED_OBJECT(Class)				\
  IMPLEMENT_EMBEDDED_IBASE_INCREF (Class);				\
  IMPLEMENT_EMBEDDED_IBASE_DECREF (Class);				\
  IMPLEMENT_EMBEDDED_IBASE_QUERY (Class);				\
    void *o = __scf_superclass__::QueryInterface (iInterfaceID, iVersion); \
    if (o) return o;							\
  IMPLEMENT_EMBEDDED_IBASE_QUERY_END;

class csModelDataPolygon : public iModelDataPolygon
{
private:
  DECLARE_GROWING_ARRAY (Vertices, int);
  DECLARE_GROWING_ARRAY (Normals, csVector3);
  DECLARE_GROWING_ARRAY (Colors, csColor);
  DECLARE_GROWING_ARRAY (TextureCoords, csVector2);
  iModelDataMaterial *Material;
public:
  DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelDataPolygon ();
  /// destructor
  virtual ~csModelDataPolygon ();
  
  /// return the number of vertices
  int GetNumVertices () const;
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
  DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;
  DECLARE_ARRAY_INTERFACE (const csVector3&, Vertex, Vertices);

  /// constructor
  csModelDataObject ();
};

class csModelDataCamera : public iModelDataCamera
{
private:
  csVector3 Position, UpVector, FrontVector, RightVector;

public:
  DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelDataCamera ();

  /// return the position of the camera
  const csVector3 &GetPosition () const;
  /// set the position of the camera
  void SetPosition (const csVector3 &v);

  /// return the 'up' vector of the camera
  const csVector3 &GetUpVector () const;
  /// set the 'up' vector of the camera
  void SetUpVector (const csVector3 &v);
  /// compute the 'up' vector as the normal to the 'front' and 'right' vectors
  void ComputeUpVector ();

  /// return the 'front' vector of the camera
  const csVector3 &GetFrontVector () const;
  /// set the 'front' vector of the camera
  void SetFrontVector (const csVector3 &v);
  /// compute the 'front' vector as the normal to the 'up' and 'right' vectors
  void ComputeFrontVector ();

  /// return the 'right' vector of the camera
  const csVector3 &GetRightVector () const;
  /// set the 'right' vector of the camera
  void SetRightVector (const csVector3 &v);
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

public:
  DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelDataLight ();

  /// Return the radius (brightness) of this light
  float GetRadius () const;
  /// Set the radius (brightness) of this light
  void SetRadius (float r);

  /// Return the color of the light
  const csColor &GetColor () const;
  /// Set the color of the light
  void SetColor (const csColor &);
};

class csModelDataMaterial : public iModelDataMaterial
{
private:

public:
  DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelDataMaterial ();
};

class csModelData : public iModelData
{
public:
  DECLARE_IBASE;
  DECLARE_OBJECT_INTERFACE;

  /// constructor
  csModelData ();
};

#endif // __MDLDATA_H__
