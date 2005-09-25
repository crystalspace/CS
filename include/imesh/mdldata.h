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

#ifndef __CS_IMESH_MDLDATA_H__
#define __CS_IMESH_MDLDATA_H__

/**\file
 * Mesh model data
 */ 

#include "csutil/scf.h"

/**\addtogroup meshplugins
 * @{ */

struct iImage;
struct iImageIO;
struct iMaterial;
struct iMaterialList;
struct iMaterialWrapper;
struct iObject;
struct iTextureList;
struct iTextureWrapper;
struct iVFS;

class csColor;
class csVector2;
class csVector3;

SCF_VERSION (iModelDataTexture, 0, 0, 1);

/**
 * This structure contains the information about a texture for an
 * imported model. 
 *
 * There are three possible types of representation for a texture: As a file
 * name, as an iImage or as a texture wrapper. The model data
 * structures will not convert between them automatically, but some
 * convenience functions are included to make conversion easier. 
 *
 * As an example, exporting to a model file which does not contain the
 * textures themselves but only their file name will require that the
 * file name is set. Exporting to a model file which contains the textures
 * directly will contain an iImage to work. Exporting to a mesh object will
 * require a texture wrapper.
 */
struct iModelDataTexture : public iBase
{
  /// Query the iObject for this texture
  virtual iObject* QueryObject () = 0;

  /// Set the file name of the texture
  virtual void SetFileName (const char *fn) = 0;
  /// Return the file name of the texture
  virtual const char *GetFileName () const = 0;

  /// Set the image object
  virtual void SetImage (iImage *image) = 0;
  /// Return the image object
  virtual iImage *GetImage () const = 0;

  /// Set the texture wrapper
  virtual void SetTextureWrapper (iTextureWrapper *th) = 0;
  /// Return the texture wrapper
  virtual iTextureWrapper *GetTextureWrapper () const = 0;

  /**
   * Load the image from a file with the current filename (i.e. this
   * texture must have a file name) from the CWD of the given file
   * system. Note: This leaves the texture wrapper untouched.
   */
  virtual void LoadImage (iVFS *VFS, iImageIO *ImageIO, int Format) = 0;

  /**
   * Create a texture wrapper from the given texture list.
   * Requires that an image object exists.
   */
  virtual void Register (iTextureList *tl) = 0;

  /**
   * Clone this texture object. Note: The underlying texture or image will
   * not be cloned!
   */
  virtual iModelDataTexture *Clone () const = 0;
};


SCF_VERSION (iModelDataMaterial, 0, 0, 1);

/**
 * This structure contains the information about a material for an imported
 * model. This information can be stored in two different way: Either as a
 * base iMaterial, or as a material wrapper. Conversion between the two is
 * usually possible, but does not happen automatically. 
 *
 * Whatever you use the model data for determines which representation is
 * used. As an example, exporting to a model file uses the base material.
 * Exporting to a mesh object uses the material wrapper.
 */
struct iModelDataMaterial : public iBase
{
  /// Query the iObject for this material
  virtual iObject* QueryObject () = 0;

  /// Return the base material
  virtual iMaterial *GetBaseMaterial () const = 0;
  /// Set the base material
  virtual void SetBaseMaterial (iMaterial *mat) = 0;

  /// Return the material wrapper
  virtual iMaterialWrapper *GetMaterialWrapper () const = 0;
  /// Set the base material
  virtual void SetMaterialWrapper (iMaterialWrapper *mat) = 0;

  /**
   * Create a material wrapper from the given material list.
   * Requires that the base material exists.
   */
  virtual void Register (iMaterialList *ml) = 0;

  /**
   * Clone this texture object. Note: The underlying material will
   * not be cloned!
   */
  virtual iModelDataMaterial *Clone () const = 0;
};


SCF_VERSION (iModelDataVertices, 0, 0, 1);

/**
 * A set of vertices, including position, normal, color and texture
 * coordinates. They are stored in independent lists, i.e. it is for example
 * possible to have different numbers of vertices and normals.
 */
struct iModelDataVertices : public iBase
{
  /// Query the iObject for this vertex set
  virtual iObject* QueryObject () = 0;

  /// Return the number of contained vertices
  virtual size_t GetVertexCount () const = 0;
  /// Return the coordinates of a vertex
  virtual const csVector3 &GetVertex (size_t n) const = 0;
  /// Set the coordinates of a vertex
  virtual void SetVertex (size_t n, const csVector3 &v) = 0;
  /// Add a vertex
  virtual size_t AddVertex (const csVector3 &v) = 0;
  /// Delete a vertex
  virtual void DeleteVertex (size_t n) = 0;
  /// Find a vertex
  virtual size_t FindVertex (const csVector3 &v) const = 0;

  /// Return the number of contained normals
  virtual size_t GetNormalCount () const = 0;
  /// Return a normal
  virtual const csVector3 &GetNormal (size_t n) const = 0;
  /// Set a normal
  virtual void SetNormal (size_t n, const csVector3 &v) = 0;
  /// Add a normal
  virtual size_t AddNormal (const csVector3 &v) = 0;
  /// Delete a normal
  virtual void DeleteNormal (size_t n) = 0;
  /// Find a normal
  virtual size_t FindNormal (const csVector3 &v) const = 0;

  /// Return the number of contained colors
  virtual size_t GetColorCount () const = 0;
  /// Return a color
  virtual const csColor &GetColor (size_t n) const = 0;
  /// Set a color
  virtual void SetColor (size_t n, const csColor &v) = 0;
  /// Add a color
  virtual size_t AddColor (const csColor &v) = 0;
  /// Delete a color
  virtual void DeleteColor (size_t n) = 0;
  /// Find a color
  virtual size_t FindColor (const csColor &v) const = 0;

  /// Return the number of contained texels
  virtual size_t GetTexelCount () const = 0;
  /// Return a texel
  virtual const csVector2 &GetTexel (size_t n) const = 0;
  /// Set a texel
  virtual void SetTexel (size_t n, const csVector2 &v) = 0;
  /// Add a texel
  virtual size_t AddTexel (const csVector2 &v) = 0;
  /// Delete a texel
  virtual void DeleteTexel (size_t n) = 0;
  /// Find a texel
  virtual size_t FindTexel (const csVector2 &v) const = 0;

  /// Add all data from another vertex frame to this one
  virtual void CopyFrom (const iModelDataVertices *Other) = 0;

  /// Clone this vertex frame
  virtual iModelDataVertices *Clone () const = 0;
};


SCF_VERSION (iModelDataAction, 0, 0, 1);

/**
 * An action. This is mainly a list of key frames. Note that the key frames
 * are not added as sub-objects, but instead they are added directly through
 * the iModelDataAction interface. The reason is that together with every
 * frame a time value has to be stored. 
 *
 * The idea of time values is the following: Every frame comes with the point
 * in time when the frame *ends*, measured in seconds. For example, if your
 * frames last 100msec, 200msec, 50msec and 250msec, the time values are: 0.1,
 * 0.3, 0.35, 0.6. As this is the ending time for each frame, the last time
 * value has three meanings:
 * - It is the end of the whole action
 * - It is the length of the action
 * - It wraps around to the time value 0.0
 * 
 * Another effect of this is that when you start playing an animation cycle,
 * the animation begins at the *last* frame and moves on to the first frame
 * within the time that is stored with the first frame. 
 *
 * Note that the frames are automatically sorted by time. 
 *
 * There are different types of frames. One could imagine vertex states,
 * skeleton states and transformation states (or other types?). Currently
 * only vertex states (iModelDataVertices) are used.
 */
struct iModelDataAction : public iBase
{
  /// Query the iObject for this action
  virtual iObject* QueryObject () = 0;

  /// Return the number of key frames
  virtual size_t GetFrameCount () const = 0;
  /// Get the time value for a frame
  virtual float GetTime (size_t Frame) const = 0;
  /// Get the state information for a frame
  virtual iObject *GetState (size_t Frame) const = 0;
  /// Set the time value for a frame
  virtual void SetTime (size_t Frame, float NewTime) = 0;
  /// Set the state information for a frame
  virtual void SetState (size_t Frame, iObject *State) = 0;
  /// Add a frame
  virtual void AddFrame (float Time, iObject *State) = 0;
  /// Delete a frame
  virtual void DeleteFrame (size_t Frame) = 0;
  /// Return the length (in time) of a complete animation cycle
  virtual float GetTotalTime () const = 0;
};


SCF_VERSION (iModelDataPolygon, 0, 0, 1);

/**
 * One polygon in a model. The vertices, normals, colors and texels are only
 * indices for lists in the parent iModelDataObject. In addition, a polygon
 * contains a material.
 */
struct iModelDataPolygon : public iBase
{
  /// Query the iObject for this material
  virtual iObject* QueryObject () = 0;

  /// return the number of vertices
  virtual size_t GetVertexCount () const = 0;
  /// Add a vertex
  virtual size_t AddVertex (int Vertex, int Normal, int Color, int Texel) = 0;
  /// Delete a vertex
  virtual void DeleteVertex (size_t n) = 0;

  /// return the index of a vertex
  virtual int GetVertex (size_t n) const = 0;
  /// set the index of a vertex
  virtual void SetVertex (size_t n, int Index) = 0;
  /// return the index of a normal
  virtual int GetNormal (size_t n) const = 0;
  /// set the index of a normal
  virtual void SetNormal (size_t n, int Index) = 0;
  /// return the index of a color
  virtual int GetColor (size_t n) const = 0;
  /// set the index of a color
  virtual void SetColor (size_t n, int Index) = 0;
  /// return the index of a texel
  virtual int GetTexel (size_t n) const = 0;
  /// set the index of a texel
  virtual void SetTexel (size_t n, int Index) = 0;

  /// return the current material
  virtual iModelDataMaterial *GetMaterial () const = 0;
  /// set the material
  virtual void SetMaterial (iModelDataMaterial *m) = 0;

  /// Clone this polygon
  virtual iModelDataPolygon *Clone () const = 0;
};


SCF_VERSION (iModelDataObject, 0, 1, 0);

/**
 * One object in the scene. This structure is intended for solid objects, i.e.
 * not for lights or cameras. Children should be polygons, curves etc. 
 *
 * Every object contains a list of vertices. These vertices are shared between
 * polygons (and curves if possible).
 */
struct iModelDataObject : public iBase
{
  /// Query the iObject for the model data
  virtual iObject* QueryObject () = 0;

  /// Return the default vertex set
  virtual iModelDataVertices* GetDefaultVertices () const = 0;
  /// Set the default vertex set
  virtual void SetDefaultVertices (iModelDataVertices*) = 0;
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

  /// Clone this camera
  virtual iModelDataCamera *Clone () const = 0;
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

  /// Set the position of the light
  virtual const csVector3 &GetPosition () const = 0;
  /// Return the position of the light
  virtual void SetPosition (const csVector3 &) = 0;

  /// Clone this light source
  virtual iModelDataLight *Clone () const = 0;
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

  /// Load all textures from the CWD of the given file system
  virtual void LoadImages (iVFS *VFS, iImageIO *ImageIO, int Format) = 0;
  /// Register all textures using the given texture list
  virtual void RegisterTextures (iTextureList *tl) = 0;
  /// Register all materials using the given material list
  virtual void RegisterMaterials (iMaterialList *ml) = 0;
};

/** @} */

#endif // __CS_IMESH_MDLDATA_H__
