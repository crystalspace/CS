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

/** \file 
 * Render buffer interface
 */
 
/**
 * \addtogroup gfx3d
 * @{ */

#include "csutil/strset.h"

#include "ivideo/render3d.h"
#include "iengine/material.h"

class csVector3;
class csVector2;
class csColor;

struct iLightingInfo;
struct iTextureHandle;


/**
 * Where the buffer is placed
 * CS_BUF_INDEX is special and only to be used by indexbuffers
 */
typedef enum _CS_RENDERBUFFER_TYPE
{
  CS_BUF_DYNAMIC,
  CS_BUF_STATIC,
  CS_BUF_INDEX
} CS_RENDERBUFFER_TYPE;


SCF_VERSION (iRenderBuffer, 0, 0, 2);

/**
 * This is a general buffer to be used by the renderer. It can ONLY be
 * created by the VB manager
 */
struct iRenderBuffer : public iBase
{
  /**
   * Type of lock
   * CS_BUF_LOCK_NORMAL: Just get a point to the buffer, nothing special
   * CS_BUF_LOCK_DISCARD: Get a buffer to use just this frame/rendering
   * CS_BUF_LOCK_RENDER: Special lock only to be used by renderer
   */
  typedef enum _CS_BUFFER_LOCK_TYPE
  {
    CS_BUF_LOCK_NOLOCK,
    CS_BUF_LOCK_NORMAL,
    CS_BUF_LOCK_DISCARD,
    CS_BUF_LOCK_RENDER
  } CS_BUFFER_LOCK_TYPE;

  /**
   * Lock the buffer to allow writing and give us a pointer to the data
   * The pointer will be NULL if there was some error
   */
  virtual void* Lock(CS_BUFFER_LOCK_TYPE lockType) = 0;

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() = 0;


  /// Get type of buffer (where it's located)
  virtual CS_RENDERBUFFER_TYPE GetBufferType() = 0;

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() = 0;

};

SCF_VERSION (iRenderBufferManager, 0, 0, 2);

struct iRenderBufferManager : public iBase
{
  /// Allocate a buffer of the specified type and return it

  virtual csPtr<iRenderBuffer> GetBuffer(int buffersize, CS_RENDERBUFFER_TYPE type) = 0;

};


SCF_VERSION (iStreamSource, 0, 0, 1);

struct iStreamSource : public iBase
{
  /// Get a named buffer
  virtual iRenderBuffer* GetBuffer (csStringID name) = 0;
};

class csRenderMesh
{
public:
  /// Type of mesh
  typedef enum
  {
    MESHTYPE_TRIANGLES,
    MESHTYPE_QUADS,
    MESHTYPE_TRIANGLESTRIP,
    MESHTYPE_TRIANGLEFAN,
    MESHTYPE_POINT,
    MESHTYPE_LINES,
    MESHTYPE_LINESTRIP
  } meshtype;

private: 
  meshtype type;
  unsigned int indexstart, indexend;
  csRef<iStreamSource> streamsource;
  csRef<iMaterialWrapper> matwrap;
  const char *defaultvertices;
  const char *defaulttexcoords;
  const char *defaultnormals;
  const char *defaultindices;

public:

  csRenderMesh () 
  {
    defaultvertices = "vertices";
    defaulttexcoords = "texture coordinates";
    defaultnormals = "normals";
    defaultindices = "indices";
    mixmode = CS_FX_COPY;
  }

  /// Special attributes. Please don't change, it's used as flags
  typedef enum
  {
    SPECIAL_NONE = 0,
    SPECIAL_BILLBOARD = 1,
    SPECIAL_ZFILL = 2
  } specialattributes;

  /// Z mode to use
  csZBufMode z_buf_mode;

  /// mixmode to use
  uint mixmode;

  /// Clipping parameter
  int clip_portal;
  
  /// Clipping parameter
  int clip_plane;
  
  /// Clipping parameter
  int clip_z_plane;

  /// Mirror mode
  bool do_mirror;


  /// Set buffer source
  virtual void SetStreamSource (iStreamSource* streamsource)
    { csRenderMesh::streamsource = streamsource; }
  /// Get buffer source
  virtual iStreamSource* GetStreamSource ()
    { return streamsource; }

  /// Set default vertex buffer name
  virtual void SetDefaultVertexBuffer (const char *name)
    { defaultvertices = name; }
  /// Get default vertex buffer name
  virtual const char *GetDefaultVertexBuffer ()
    { return defaultvertices; }
  /// Set default vertex buffer name
  virtual void SetDefaultTexCoordBuffer (const char *name)
    { defaulttexcoords = name; }
  /// Get default vertex buffer name
  virtual const char *GetDefaultTexCoordBuffer ()
    { return defaulttexcoords; }
  /// Set default vertex buffer name
  virtual void SetDefaultNormalBuffer (const char *name)
    { defaultnormals = name; }
  /// Get default vertex buffer name
  virtual const char *GetDefaultNormalBuffer ()
    { return defaultnormals; }
  /// Set default vertex buffer name
  virtual void SetDefaultIndexBuffer (const char *name)
    { defaultindices = name; }
  /// Get default vertex buffer name
  virtual const char *GetDefaultIndexBuffer ()
    { return defaultindices; }

  /// Set range of indices to use
  virtual void SetIndexRange (unsigned int start, unsigned int end)
    { indexstart = start; indexend = end; }
  /// Get start of index range
  virtual unsigned int GetIndexStart ()
    { return indexstart; }
  /// Get end of index range
  virtual unsigned int GetIndexEnd ()
    { return indexend; }

  /// Set mesh type
  virtual void SetType (meshtype type) 
    { csRenderMesh::type = type; }
  /// Get mesh type
  virtual meshtype GetType () 
    { return type; }

  /// Set material wrapper
  virtual void SetMaterialWrapper (iMaterialWrapper* matwrap)
    { csRenderMesh::matwrap = matwrap; }
  /// Get material wrapper
  virtual iMaterialWrapper* GetMaterialWrapper ()
    { return matwrap; }

  /// Get lighting information
  virtual iLightingInfo* GetLightingInfo () { return NULL; }

  /**
   * Special case for lightmap. To get lightmaps into renderers
   * not able to do multitexture, here is a handle to the lightmap
   * texture
   */
  virtual iTextureHandle* GetLightmapHandle () { return NULL; }
};

/** @} */

#endif //  __IVIDEO_RNDBUF_H__
