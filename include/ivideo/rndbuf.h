/*
    Copyright (C) 2002 by Mårten Svanfeldt
                          Anders Stenberg

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

#ifndef __IVIDEO_RNDBUF_H__
#define __IVIDEO_RNDBUF_H__


/// Where the buffer is placed
enum 
{
  BUF_SYSTEM,   /// In system main memory (RAM)
  BUF_AGP,      /// In special AGP allocated memory
  BUF_VIDEO     /// In videoram
} RENDERBUFFER_LOCATION;


struct iRenderBufferManager
{
  /// Allocate a buffer of the specified type and return it
  virtual iRenderBuffer* GetBuffer(int buffersize, RENDERBUFFER_LOCATION location) = 0;
  
  /// Lock a specified buffer. Return true if successful
  virtual bool LockBuffer(iRenderBuffer* buffer) = 0;

  /// Unlock buffer
  virtual void UnlockBuffer(iRenderBuffer* buffer) = 0;
};

struct iStreamSource
{
  /// Get a named buffer
  iRenderBuffer* GetBuffer(csStringID name) = 0;
};

struct csRenderMesh
{
  /// Type of mesh
  enum
  {
    MESHTYPE_TRIANGLES,
    MESHTYPE_QUADS,
    MESHTYPE_TRIANGLESTRIP,
    MESHTYPE_TRIANGLEFAN,
    MESHTYPE_POINT,
    MESHTYPE_LINES,
    MESHTYPE_LINESTRIP
  } meshtype;

  ///Special attributes. Please don't change, it's used as flags
  enum
  {
    SPECIAL_NONE = 0
    SPECIAL_BILLBOARD = 1,
    SPECIAL_ZFILL = 2
  } specialattributes;

  /// Get buffer source
  virtual iStreamSource* GetStreamSource() = 0;

  /// Number of buffers
  virtual int GetBufferCount() = 0;
  
  /// References to defaultbuffers
  virtual csStringID GetVertexBuffer() = 0;
  virtual csStringID GetNormalBuffer() = 0;
  virtual csStringID GetColorBuffer() = 0;
  virtual csStringID GetIndexBuffer() = 0;

  /// Objecttypes
  virtual meshtype GetType() = 0;
  virtual void SetType(meshtype type) = 0;

  /// Lighting information
  virtual iLightingInfo* GetLightingInfo() = 0;

  /// Special case for lightmap. To get lightmaps into renderers
  /// not able to do multitexture, here is a handle to the lightmap
  /// texture
  virtual iTextureHandle* GetLightmapHandle() = 0;
};

/**
 * This is a general buffer to be used by the renderer. It can ONLY be
 * created by the VB manager
 */
struct iRenderBuffer : public iBase
{
  
  /// Get a raw pointer to the bufferdata as different datatypes
  virtual float* GetFloatBuffer() = 0;
  virtual unsigned char* GetUCharBuffer() = 0;
  virtual unsigned int* GetUIntBuffer() = 0;
  virtual csVector3* GetVector3Buffer() = 0;
  virtual csVector2* GetVector2Buffer() = 0;
  virtual csColor* GetColorBuffer() = 0;

  /// Get type of buffer (where it's located)
  virtual RENDERBUFFER_LOCATION GetBufferType() = 0;

  /// Get number of indices in the data (as different datatypes)
  virtual int GetFloatLength() = 0;
  virtual int GetUCharLength() = 0;
  virtual int GetUintLength() = 0;
  virtual int GetVec3Length() = 0;
  virtual int GetVec2Length() = 0;
  virtual int GetColorLength() = 0;
};

#endif //  __IVIDEO_RNDBUF_H__