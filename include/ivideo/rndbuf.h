/*
    Copyright (C) 2002 by Marten Svanfeldt
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

#ifndef __CS_IVIDEO_RNDBUF_H__
#define __CS_IVIDEO_RNDBUF_H__

/** \file 
 * Render buffer interface
 */
 
#include "csutil/scf.h"
#include "csutil/strset.h"
#include "csutil/refcount.h"

/**
 * \addtogroup gfx3d
 * @{ */

class csVector3;
class csVector2;
class csColor;
class csReversibleTransform;
struct iTextureHandle;
struct iMaterialWrapper;
class csRenderBufferHolder;

/**
 * Buffer usage type.
 * Drivers may do some optimizations based on this value. Use a type that most
 * closely matches the intended use.
 */
enum csRenderBufferType
{
  /// Data that changes every couple of frames and is drawn repeatedly.
  CS_BUF_DYNAMIC,
  /// Data that is seldom modified and often drawn.
  CS_BUF_STATIC,
  /// Data that changes every time it is drawn.
  CS_BUF_STREAM
};

/// Type of components
enum csRenderBufferComponentType
{
  CS_BUFCOMP_BYTE = 0,
  CS_BUFCOMP_UNSIGNED_BYTE,
  CS_BUFCOMP_SHORT,
  CS_BUFCOMP_UNSIGNED_SHORT,
  CS_BUFCOMP_INT,
  CS_BUFCOMP_UNSIGNED_INT,
  CS_BUFCOMP_FLOAT,
  CS_BUFCOMP_DOUBLE,

  CS_BUFCOMP_TYPECOUNT
};

/**
 * Type of lock of a render buffer.
 */
enum csRenderBufferLockType
{
  CS_BUF_LOCK_NOLOCK,
  /// Lock used for reading only from the buffer
  CS_BUF_LOCK_READ,
  /// Just get a point to the buffer, nothing special
  CS_BUF_LOCK_NORMAL
};

/**
 * Sizes of individual buffer components in bytes.
 */
static const size_t csRenderBufferComponentSizes[CS_BUFCOMP_TYPECOUNT] = 
{
  sizeof (char), sizeof (unsigned char), 
  sizeof (short), sizeof (unsigned short),
  sizeof (int), sizeof (unsigned int),
  sizeof (float),
  sizeof (double)
};

/**
 * This is a general buffer.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>csRenderBuffer::CreateRenderBuffer()
 *   <li>csRenderBuffer::CreateIndexRenderBuffer()
 *   <li>csRenderBuffer::CreateInterleavedRenderBuffers()
 *   </ul>
 * \sa csRenderBuffer
 */
struct iRenderBuffer : public virtual iBase
{
  SCF_INTERFACE (iRenderBuffer, 2, 0, 0);

  /**
   * Lock the buffer to allow writing and return a pointer to the first 
   * element.
   * The pointer will be (void*)-1 if there was some error.
   * \param lockType The type of lock desired.
   */
  virtual void* Lock(csRenderBufferLockType lockType) = 0;

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() = 0;

  /**
   * Copy data to the render buffer.
   * \remarks Does not work with interleaved buffer, copy to master buffer
   *  instead.
   */
  virtual void CopyInto (const void *data, size_t elementCount,
    size_t elemOffset = 0) = 0;

  /// Gets the number of components per element
  virtual int GetComponentCount () const = 0;

  /// Gets the component type (float, int, etc)
  virtual csRenderBufferComponentType GetComponentType () const = 0;

  /// Get type of buffer (static/dynamic)
  virtual csRenderBufferType GetBufferType() const = 0;

  /// Get the size of the buffer (in bytes)
  virtual size_t GetSize() const = 0;

  /// Get the stride of the buffer (in bytes)
  virtual size_t GetStride() const = 0;

  /// Get the distance between two elements (in bytes, includes stride)
  virtual size_t GetElementDistance() const = 0;

  /// Get the offset of the buffer (in bytes)
  virtual size_t GetOffset() const = 0;

  /// Get version
  virtual uint GetVersion () = 0;

  /**
   * Get the master buffer in case this is an interleaved buffer.
   * The master buffer is the buffer that actually holds the data;
   * while it can be used to retrieve or set data, it must not be
   * used for actual rendering. Use the interleaved buffers instead.
   */
  virtual iRenderBuffer* GetMasterBuffer () const = 0;

  /// Whether the buffer is an index buffer.
  virtual bool IsIndexBuffer() const = 0;
  /// The lowest index contained in this buffer, only valid for index buffers.
  virtual size_t GetRangeStart() const = 0;
  /// The highest index contained in this buffer, only valid for index buffers.
  virtual size_t GetRangeEnd() const = 0;

  /// Number of elements in a buffer.
  virtual size_t GetElementCount() const = 0;
};

/**
 * Defines the names of the renderbuffers as provided by the
 * meshes.
 * \remark These constants are used to generate a mask (see
 *  csRenderBufferNameMask), hence their number should be kept
 *  below 32.
 */
enum csRenderBufferName
{
  CS_BUFFER_NONE = -1,
  /// Index buffer
  CS_BUFFER_INDEX,
  /// Vertex positions
  CS_BUFFER_POSITION,
  /// Normals
  CS_BUFFER_NORMAL,
  /// Primary color multiplied with lighting
  CS_BUFFER_COLOR,
  /// Primary color 
  CS_BUFFER_COLOR_UNLIT,
  /// Texture coordinate 0
  CS_BUFFER_TEXCOORD0,
  /// Texture coordinate 1
  CS_BUFFER_TEXCOORD1,
  /// Texture coordinate 2
  CS_BUFFER_TEXCOORD2,
  /// Texture coordinate 3
  CS_BUFFER_TEXCOORD3,
  /// Texture coordinates for lightmap
  CS_BUFFER_TEXCOORD_LIGHTMAP,
  /// Generic buffer 0, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC0,
  /// Generic buffer 1, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC1,
  /// Generic buffer 2, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC2,
  /// Generic buffer 3, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC3,
  /// Tangents
  CS_BUFFER_TANGENT,
  /// Binormals
  CS_BUFFER_BINORMAL,

  /**
   * Not really a renderbuffer. Use to determine the number of available
   * default buffer names.
   */
  CS_BUFFER_COUNT
};

#define CS_BUFFER_MAKE_MASKABLE(x) (1<<(x))

/// Masks for renderbuffer accessors
enum csRenderBufferNameMask
{
  /// Index buffer
  CS_BUFFER_INDEX_MASK = 1 << CS_BUFFER_INDEX,
  /// Vertex positions
  CS_BUFFER_POSITION_MASK = 1 << CS_BUFFER_POSITION,
  /// Normals
  CS_BUFFER_NORMAL_MASK = 1 << CS_BUFFER_NORMAL,
  /// Primary color multiplied with lighting
  CS_BUFFER_COLOR_MASK = 1 << CS_BUFFER_COLOR,
  /// Primary color 
  CS_BUFFER_COLOR_LIGHTING_MASK = 1 << CS_BUFFER_COLOR_UNLIT,
  /// Texture coordinate 0
  CS_BUFFER_TEXCOORD0_MASK = 1 << CS_BUFFER_TEXCOORD0,
  /// Texture coordinate 1
  CS_BUFFER_TEXCOORD1_MASK = 1 << CS_BUFFER_TEXCOORD1,
  /// Texture coordinate 2
  CS_BUFFER_TEXCOORD2_MASK = 1 << CS_BUFFER_TEXCOORD2,
  /// Texture coordinate 3
  CS_BUFFER_TEXCOORD3_MASK = 1 << CS_BUFFER_TEXCOORD3,
  /// Texture coordinates for lightmap
  CS_BUFFER_TEXCOORD_LIGHTMAP_MASK = 1 << CS_BUFFER_TEXCOORD_LIGHTMAP,
  /// Generic buffer 0, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC0_MASK = 1 << CS_BUFFER_GENERIC0,
  /// Generic buffer 1, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC1_MASK = 1 << CS_BUFFER_GENERIC1,
  /// Generic buffer 2, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC2_MASK = 1 << CS_BUFFER_GENERIC2,
  /// Generic buffer 3, can be used for example for matrix-palette skinning etc
  CS_BUFFER_GENERIC3_MASK = 1 << CS_BUFFER_GENERIC3,
  /// Tangents
  CS_BUFFER_TANGENT_MASK = 1 << CS_BUFFER_TANGENT,
  /// Binormals
  CS_BUFFER_BINORMAL_MASK = 1 << CS_BUFFER_BINORMAL,
  /// All buffers
  CS_BUFFER_ALL_MASK = ~0
};


/**
 * Interface for renderbuffer accessor.
 * The renderbuffer accesor is similar to the shadervariable accessor system,
 * and is used to delay calculation of the content of renderbuffers.
 */
struct iRenderBufferAccessor : public virtual iBase
{
  SCF_INTERFACE (iRenderBufferAccessor,0,0,1);
  /// Called before associated renderbuffer is fetched from the holder.
  virtual void PreGetBuffer (csRenderBufferHolder* holder, 
    csRenderBufferName buffer) = 0;
};

/**
 * Holder of standard renderbuffers.
 * @@DOC MORE
 */
class csRenderBufferHolder : public csRefCount
{
public:
  /** initialize */
  csRenderBufferHolder() {accessorMask=0;}
  /**
   * Get buffer by name.
   * If an accessor is set, it will first be called, after that the buffer will be 
   * fetched.
   */
  iRenderBuffer* GetRenderBuffer (csRenderBufferName bufferName)
  {
    if (bufferName < CS_BUFFER_INDEX) return 0;
    if (accessor && 
        accessorMask & CS_BUFFER_MAKE_MASKABLE(bufferName))
      accessor->PreGetBuffer (this, bufferName);

    return buffers[bufferName];
  }

  /**
   * Set renderbuffer.
   */
  void SetRenderBuffer (csRenderBufferName bufferName, iRenderBuffer* buffer)
  {
    CS_ASSERT(bufferName >= 0 && bufferName < CS_BUFFER_COUNT);
    buffers[bufferName] = buffer;
  }

  /**
   * Set accessor.
   */
  void SetAccessor (iRenderBufferAccessor* a, uint32 mask)
  {
    accessorMask = mask;
    accessor = a;
  }
  /// Query the accessor mask
  uint GetAccessorMask() const { return accessorMask; }
  /// Query the accessor
  iRenderBufferAccessor* GetAccessor() const { return accessor; }
protected:
  uint32 accessorMask;
  csRef<iRenderBufferAccessor> accessor;
  csRef<iRenderBuffer> buffers[CS_BUFFER_COUNT];
};

/** @} */

#endif // __CS_IVIDEO_RNDBUF_H__
