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

struct iObject;

SCF_VERSION (iModelDataMaterial, 0, 0, 1);

/**
 * This structure contains the information about a material for an
 * imported model.
 */
struct iModelDataMaterial : public iBase
{
  /// Query the iObject for this material
  virtual iObject* QueryObject () = 0;
};


SCF_VERSION (iModelDataPolygon, 0, 0, 1);

/**
 * One polygon in a model. The normals, texture coordinates and color are kept
 * per vertex. The vertex positions are indices for the vertex list of the
 * parent iModelDataObject.
 */
struct iModelDataPolygon : public iBase
{
  /// Query the iObject for this material
  virtual iObject* QueryObject () = 0;

  /// return the number of vertices
  virtual int GetNumVertices () const = 0;
  /// Add a vertex
  virtual void AddVertex (int PositionIndex, const csVector3 &Normal,
    const csColor &Color, const csVector2 &TextureCoords) = 0;
  /// Delete a vertex
  virtual void DeleteVertex (int n) = 0;

  /// return the coordinate index of a vertex
  virtual int GetVertex (int n) const = 0;
  /// set the coordinate index of a vertex
  virtual void SetVertex (int n, int Index) = 0;

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

/**
 * One object in the scene. This structure is intended for solid objects, i.e.
 * not for lights or cameras. Children should be polygons, curves etc. <p>
 *
 * Every object contains a list of vertices. These vertices are shared between
 * polygons (and curves if possible).
 */
struct iModelDataObject : public iBase
{
  /// Query the iObject for the model data
  virtual iObject* QueryObject () = 0;

  /// Return the number of vertices in the object
  virtual int GetNumVertices () const = 0;
  /// Return the coordinates of a vertex
  virtual const csVector3 &GetVertex (int n) const = 0;
  /// Set the coordinates of a vertex
  virtual void SetVertex (int n, const csVector3 &v) = 0;
  /// Add a vertex
  virtual void AddVertex (const csVector3 &v) = 0;
};


SCF_VERSION (iModelDataCamera, 0, 0, 1);

/// A camera in the scene.
struct iModelDataCamera : public iBase
{
  /// Query the iObject for this camera
  virtual iObject* QueryObject () = 0;

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

/// A light source in the scene.
struct iModelDataLight : public iBase
{
  /// Query the iObject for this light
  virtual iObject* QueryObject () = 0;

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

/**
 * This structure represents a complete scene with objects, light sources,
 * cameras etc. All these objects are added as children in the iObject
 * hierarchy.
 */
struct iModelData : public iBase
{
  /// Query the iObject for the model data
  virtual iObject* QueryObject () = 0;
};

#endif // __IVARIA_MDLDATA_H__
