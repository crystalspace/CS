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
/* Note: csgfx/renderbuffer.h stores this in _very_ few bits, take care when
   changing this! */
enum csRenderBufferComponentType
{
  /// Signed byte
  CS_BUFCOMP_BYTE = 0,
  /// Unsigned byte
  CS_BUFCOMP_UNSIGNED_BYTE,
  /// Signed short
  CS_BUFCOMP_SHORT,
  /// Unsigned short
  CS_BUFCOMP_UNSIGNED_SHORT,
  /// Signed integer
  CS_BUFCOMP_INT,
  /// Unsigned integer
  CS_BUFCOMP_UNSIGNED_INT,
  /// Float (aka "single precision floating point number")
  CS_BUFCOMP_FLOAT,
  /// Double (aka "double precision floating point number")
  CS_BUFCOMP_DOUBLE,
  /// Half float (aka "half precision floating point number")
  CS_BUFCOMP_HALF,
  
  CS_BUFCOMP_BASE_TYPECOUNT,
  
  /// Normalization flag
  CS_BUFCOMP_NORMALIZED = 0x10,
  
  /// Signed byte, normalized to [-1;1]
  CS_BUFCOMP_BYTE_NORM = CS_BUFCOMP_BYTE | CS_BUFCOMP_NORMALIZED,
  /// Unsigned byte, normalized to [0;1]
  CS_BUFCOMP_UNSIGNED_BYTE_NORM =
      CS_BUFCOMP_UNSIGNED_BYTE | CS_BUFCOMP_NORMALIZED,
  /// Signed short, normalized to [-1;1]
  CS_BUFCOMP_SHORT_NORM =
      CS_BUFCOMP_SHORT | CS_BUFCOMP_NORMALIZED,
  /// Unsigned short, normalized to [0;1]
  CS_BUFCOMP_UNSIGNED_SHORT_NORM =
      CS_BUFCOMP_UNSIGNED_SHORT | CS_BUFCOMP_NORMALIZED,
  /// Signed integer, normalized to [-1;1]
  CS_BUFCOMP_INT_NORM = CS_BUFCOMP_INT | CS_BUFCOMP_NORMALIZED,
  /// Unsigned integer, normalized to [0;1]
  CS_BUFCOMP_UNSIGNED_INT_NORM = 
      CS_BUFCOMP_UNSIGNED_INT | CS_BUFCOMP_NORMALIZED
};

/**
 * Type of lock of a render buffer.
 */
enum csRenderBufferLockType
{
  /// Lock used for reading only from the buffer
  CS_BUF_LOCK_READ = 1,
  /// Get a (writeable) pointer to the buffer
  CS_BUF_LOCK_NORMAL
};

#include "csutil/deprecated_warn_off.h"

namespace CS
{
  namespace Deprecated
  {
    struct CS_DEPRECATED_TYPE_MSG("You shouldn't use CS_BUF_LOCK_NOLOCK in "
      "the first place")
      CS_BUF_LOCK_NOLOCK
    {
      static const uint value = 0;
    };
  }
}

#include "csutil/deprecated_warn_on.h"

#define CS_BUF_LOCK_NOLOCK	CS::Deprecated::CS_BUF_LOCK_NOLOCK::value

/**
 * Sizes of individual buffer components in bytes.
 */
static const size_t csRenderBufferComponentSizes[CS_BUFCOMP_BASE_TYPECOUNT] = 
{
  sizeof (char), sizeof (unsigned char), 
  sizeof (short), sizeof (unsigned short),
  sizeof (int), sizeof (unsigned int),
  sizeof (float),
  sizeof (double),
  sizeof (uint16)
};

struct iRenderBuffer;

/**
 * Callback function used upon destruction of render buffer. Used by renderer
 * to properly uncache render buffer.
 */
struct iRenderBufferCallback : public virtual iBase
{
  SCF_INTERFACE (iRenderBufferCallback, 1,0,0);
  /**
   * Called when render buffer is about to be destroyed and removed
   */
  virtual void RenderBufferDestroyed (iRenderBuffer* buffer) = 0;
};

/**
 * This is a general buffer.
 *
 * Main creators of instances implementing this interface:
 * - csRenderBuffer::CreateRenderBuffer()
 * - csRenderBuffer::CreateIndexRenderBuffer()
 * - csRenderBuffer::CreateInterleavedRenderBuffers()
 *
 * \sa csRenderBuffer
 */
struct iRenderBuffer : public virtual iBase
{
  SCF_INTERFACE (iRenderBuffer, 2, 1, 1);

  /**
   * Lock the buffer to allow reading or writing to it directly.
   * \param lockType The type of lock desired.
   * \return A pointer to the first element or (void*)-1 if there was some 
   *   error.
   * \remarks After the data was used as desired Release() \b must be called.
   */
  virtual void* Lock(csRenderBufferLockType lockType) = 0;

  /**
   * Releases the buffer. After this all access to the buffer pointer is 
   * illegal.
   */
  virtual void Release() = 0;

  /**
   * Copy data to the render buffer.
   * \remarks 
   *   Do not call on a locked buffer.
   *   Does not work with interleaved buffer, copy to master buffer instead.
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

  /// Set callback object to use
  virtual void SetCallback (iRenderBufferCallback* cb) = 0;

  /**
   * Set the buffer data. This changes the internal pointer to the buffer data 
   * to \a buffer instead. It will also be returned by Lock(). It is the
   * responsibility of the caller to ensure that the memory pointed to by
   * \a data is valid for as long as the render buffer is used.
   * \remarks 
   *   Do not call on a locked buffer.
   *   Does not work with interleaved buffer, set data on master buffer
   *   instead.
   */
  virtual void SetData (const void *data) = 0;
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
  /// Primary color 
  CS_BUFFER_COLOR_MASK = 1 << CS_BUFFER_COLOR,
  /// Primary color multiplied with lighting
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
  csRenderBufferHolder () 
    : accessorMask (0)
  {    
  }

  csRenderBufferHolder (const csRenderBufferHolder& other)
    : csRefCount (), accessorMask (other.accessorMask),
    accessor (other.accessor)
  {
    for (size_t i = 0; i < CS_BUFFER_COUNT; ++i)
    {
      buffers[i] = other.buffers[i];
    }
  }

  csRenderBufferHolder& operator= (const csRenderBufferHolder& other)
  {
    accessorMask = other.accessorMask;
    accessor = other.accessor;

    for (size_t i = 0; i < CS_BUFFER_COUNT; ++i)
    {
      buffers[i] = other.buffers[i];
    }

    return *this;
  }

  /**
   * Get buffer by name.
   * If an accessor is set, it will first be called, after that the buffer
   * will be fetched.
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
   * Get buffer by name.
   * This version won't call accessor
   */
  iRenderBuffer* GetRenderBufferNoAccessor (csRenderBufferName bufferName)
  {
    if (bufferName < CS_BUFFER_INDEX) return 0;
    
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
