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

#define DECLARE_ACCESSOR_METHODS(type,name)				\
  type Get##name () const;						\
  void Set##name (type);

#define DECLARE_ARRAY_INTERFACE_NONUM(type,name)			\
  type Get##name (int n) const;						\
  void Set##name (int n, type);

#define DECLARE_ARRAY_INTERFACE(type,name)				\
  DECLARE_ARRAY_INTERFACE_NONUM (type, name)				\
  int GetNum##name##s () const;

class csModelDataPolygon : public iModelDataPolygon
{
private:
  DECLARE_IBASE;
  DECLARE_GROWING_ARRAY (VertexList, csVector3);
  DECLARE_GROWING_ARRAY (NormalList, csVector3);
  DECLARE_GROWING_ARRAY (ColorList, csColor);
  DECLARE_GROWING_ARRAY (TextureCoordsList, csVector2);
  iModelDataMaterial *Material;
public:

  /// constructor
  csModelDataPolygon ();
  /// destructor
  virtual ~csModelDataPolygon ();
  
  /// return the number of vertices
  int GetNumVertices () const;
  /// Add a vertex
  void AddVertex (const csVector3 &Position, const csVector3 &Normal,
    const csColor &Color, const csVector2 &TextureCoords);
  /// Delete a vertex
  void DeleteVertex (int n);

  DECLARE_ARRAY_INTERFACE_NONUM (const csVector3 &, Vertex);
  DECLARE_ARRAY_INTERFACE_NONUM (const csVector3 &, Normal);
  DECLARE_ARRAY_INTERFACE_NONUM (const csVector2 &, TextureCoords);
  DECLARE_ARRAY_INTERFACE_NONUM (const csColor &, Color);
  DECLARE_ACCESSOR_METHODS (iModelDataMaterial*, Material);

/*
  /// return the coordinates of a vertex
  const csVector3 &GetVertex (int n) const;
  /// set the coordinates of a vertex
  void SetVertex (int n, const csVector3 &v);

  /// return the normal of a vertex
  const csVector3 &GetNormal (int n) const;
  /// set the normal of a vertex
  void SetNormal (int n, const csVector3 &v);

  /// return the color for a vertex
  const csColor &GetColor (int n) const;
  /// set the color for a vertex
  void SetColor (int n, const csColor &c);

  /// return the texture coordinates for a vertex
  const csVector2 &GetTextureCoords (int n) const;
  /// set the texture coordinates for a vertex
  void SetTextureCoords (int n, const csVector2 &v);

  /// return the current material
  iModelDataMaterial *GetMaterial () const;
  /// set the material
  void SetMaterial (iModelDataMaterial *m);
  */
};

DECLARE_TYPED_SCF_VECTOR (csModelDataPolygonVector, csModelDataPolygon);

class csModelDataObject : public iModelDataObject
{
private:
  DECLARE_IBASE;
  csModelDataPolygonVector Polygons;

public:
  /// return the number of polygons in this object
  int GetNumPolygons () const;
  /// return a single polygon
  iModelDataPolygon* GetPolygon (int n);
  /// add a polygon
  iModelDataPolygon* CreatePolygon ();
  /// delete a polygon
  void DeletePolygon (int n);
};

class csModelDataCamera : public iModelDataCamera
{
private:
  DECLARE_IBASE;
  csVector3 Position, UpVector, FrontVector, RightVector;

public:
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
  DECLARE_IBASE;
  float Radius;
  csColor Color;

public:
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
  DECLARE_IBASE;

public:
};

DECLARE_TYPED_SCF_VECTOR (csModelDataObjectVector, csModelDataObject);
DECLARE_TYPED_SCF_VECTOR (csModelDataCameraVector, csModelDataCamera);
DECLARE_TYPED_SCF_VECTOR (csModelDataLightVector, csModelDataLight);

class csModelData : public iModelData
{
private:
  DECLARE_IBASE;

  /// List of all objects
  csModelDataObjectVector Objects;

  /// List of all cameras
  csModelDataCameraVector Cameras;

  /// List of all lights
  csModelDataLightVector Lights;

public:
  /// Get the number of objects in the scene
  int GetNumObjects () const;
  /// Return a single object
  iModelDataObject* GetObject (int n);
  /// Add an object
  iModelDataObject* CreateObject ();
  /// Delete an object
  void DeleteObject (int n);

  /// Get the number of cameras in the scene
  int GetNumCameras () const;
  /// Return a single camera
  iModelDataCamera* GetCamera (int n);
  /// Add an camera
  iModelDataCamera* CreateCamera ();
  /// Delete an camera
  void DeleteCamera (int n);

  /// Get the number of lights in the scene
  int GetNumLights () const;
  /// Return a single light
  iModelDataLight* GetLight (int n);
  /// Add an light
  iModelDataLight* CreateLight ();
  /// Delete an light
  void DeleteLight (int n);
};

#endif // __MDLDATA_H__
