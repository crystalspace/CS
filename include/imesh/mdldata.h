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

#ifndef __IVARIA_MDLDATA_H__
#define __IVARIA_MDLDATA_H__

#include "csgeom/vector2.h"
#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/csvector.h"
#include "csutil/scf.h"


SCF_VERSION (iModelDataMaterial, 0, 0, 1);

struct iModelDataMaterial : public iBase
{
};


SCF_VERSION (iModelDataPolygon, 0, 0, 1);

struct iModelDataPolygon : public iBase
{
  /// return the number of vertices
  virtual int GetNumVertices () const = 0;
  /// Add a vertex
  virtual void AddVertex (const csVector3 &Position, const csVector3 &Normal,
    const csColor &Color, const csVector2 &TextureCoords) = 0;
  /// Delete a vertex
  virtual void DeleteVertex (int n) = 0;

  /// return the coordinates of a vertex
  virtual const csVector3 &GetVertex (int n) const = 0;
  /// set the coordinates of a vertex
  virtual void SetVertex (int n, const csVector3 &v) = 0;

  /// return the normal of a vertex
  virtual const csVector3 &GetNormal (int n) const = 0;
  /// set the normal of a vertex
  virtual void SetNormal (int n, const csVector3 &v) = 0;

  /// return the color for a vertex
  virtual const csColor &GetColor (int n) const = 0;
  /// set the color for a vertex
  virtual void SetColor (int n, const csColor &c) = 0;

  /// return the texture coordinates for a vertex
  virtual const csVector2 &GetTextureCoords (int n) const = 0;
  /// set the texture coordinates for a vertex
  virtual void SetTextureCoords (int n, const csVector2 &v) = 0;

  /// return the current material
  virtual iModelDataMaterial *GetMaterial () const = 0;
  /// set the material
  virtual void SetMaterial (iModelDataMaterial *m) = 0;
};


SCF_VERSION (iModelDataObject, 0, 0, 1);

struct iModelDataObject : public iBase
{
  /// return the number of polygons in this object
  virtual int GetNumPolygons () const = 0;
  /// return a single polygon
  virtual iModelDataPolygon* GetPolygon (int n) = 0;
  /// add a polygon
  virtual iModelDataPolygon* CreatePolygon () = 0;
  /// delete a polygon
  virtual void DeletePolygon (int n) = 0;
};


SCF_VERSION (iModelDataCamera, 0, 0, 1);

struct iModelDataCamera : public iBase
{
  /// return the position of the camera
  virtual const csVector3 &GetPosition () const = 0;
  /// set the position of the camera
  virtual void SetPosition (const csVector3 &v) = 0;

  /// return the 'up' vector of the camera
  virtual const csVector3 &GetUpVector () const = 0;
  /// set the 'up' vector of the camera
  virtual void SetUpVector (const csVector3 &v) = 0;
  /// compute the 'up' vector as the normal to the 'front' and 'right' vectors
  virtual void ComputeUpVector () = 0;

  /// return the 'front' vector of the camera
  virtual const csVector3 &GetFrontVector () const = 0;
  /// set the 'front' vector of the camera
  virtual void SetFrontVector (const csVector3 &v) = 0;
  /// compute the 'front' vector as the normal to the 'up' and 'right' vectors
  virtual void ComputeFrontVector () = 0;

  /// return the 'right' vector of the camera
  virtual const csVector3 &GetRightVector () const = 0;
  /// set the 'right' vector of the camera
  virtual void SetRightVector (const csVector3 &v) = 0;
  /// compute the 'right' vector as the normal to the 'up' and 'front' vectors
  virtual void ComputeRightVector () = 0;

  /// normalize all direction vectors
  virtual void Normalize () = 0;
  /// test if all direction vectors are orthogonal
  virtual bool CheckOrthogonality () const = 0;
};


SCF_VERSION (iModelDataLight, 0, 0, 1);

struct iModelDataLight : public iBase
{
  /// Return the radius (brightness) of this light
  virtual float GetRadius () const = 0;
  /// Set the radius (brightness) of this light
  virtual void SetRadius (float r) = 0;

  /// Return the color of the light
  virtual const csColor &GetColor () const = 0;
  /// Set the color of the light
  virtual void SetColor (const csColor &) = 0;
};


SCF_VERSION (iModelData, 0, 0, 1);

struct iModelData : public iBase
{
  /// Get the number of objects in the scene
  virtual int GetNumObjects () const = 0;
  /// Return a single object
  virtual iModelDataObject* GetObject (int n) = 0;
  /// Add an object
  virtual iModelDataObject* CreateObject () = 0;
  /// Delete an object
  virtual void DeleteObject (int n) = 0;

  /// Get the number of cameras in the scene
  virtual int GetNumCameras () const = 0;
  /// Return a single camera
  virtual iModelDataCamera* GetCamera (int n) = 0;
  /// Add an camera
  virtual iModelDataCamera* CreateCamera () = 0;
  /// Delete an camera
  virtual void DeleteCamera (int n) = 0;

  /// Get the number of lights in the scene
  virtual int GetNumLights () const = 0;
  /// Return a single light
  virtual iModelDataLight* GetLight (int n) = 0;
  /// Add an light
  virtual iModelDataLight* CreateLight () = 0;
  /// Delete an light
  virtual void DeleteLight (int n) = 0;
};

#endif // __IVARIA_MDLDATA_H__
