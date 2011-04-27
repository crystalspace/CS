/*
  Copyright (C) 2003-2005 by Marten Svanfeldt
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

#ifndef __CS_CSGFX_RENDERBUFFER_H__
#define __CS_CSGFX_RENDERBUFFER_H__

/**\file
 * Render buffer.
 */

#include "csextern.h"
#include "csutil/csstring.h"
#include "csutil/leakguard.h"
#include "csutil/scf_implementation.h"
#include "csutil/weakref.h"
#include "imap/renderbufferpersistence.h"
#include "ivideo/rndbuf.h"

/**\addtogroup gfx
 * @{ 
 */

/**
 * Structure describing the properties of the individual buffers to be 
 * interleaved.
 */
struct csInterleavedSubBufferOptions
{
  /// Components Types; usually CS_BUFCOMP_UNSIGNED_INT
  csRenderBufferComponentType componentType;
  /// Number of components per element (e.g. 4 for RGBA)
  uint componentCount;
};

#ifdef CS_DEBUG
class csCallStack;
#endif

/**
 * Render buffer - basic container for mesh geometry data.
 */
class CS_CRYSTALSPACE_EXPORT csRenderBuffer :
  public scfImplementation1<csRenderBuffer, iRenderBuffer>
{
protected:
  /**
   * Constructor.
   */
  csRenderBuffer (size_t size, csRenderBufferType type, 
    csRenderBufferComponentType componentType, uint componentCount, 
    size_t rangeStart, size_t rangeEnd, bool copy = true);
public:
  CS_LEAKGUARD_DECLARE (csRenderBuffer);

  /**
   * Destructor
   */
  virtual ~csRenderBuffer ();

  /**\name iRenderBuffer implementation
   * @{ */
  virtual void* Lock (csRenderBufferLockType lockType);

  virtual void Release ();

  virtual void CopyInto (const void* data, size_t elementCount,
    size_t elemOffset = 0);

  virtual int GetComponentCount () const
  {
    return props.compCount;
  }

  virtual csRenderBufferComponentType GetComponentType () const 
  {
    return props.comptype;
  }

  virtual csRenderBufferType GetBufferType () const
  {
    return props.bufferType;
  }

  virtual size_t GetSize () const
  {
    return bufferSize;
  }

  virtual size_t GetStride () const 
  {
    return props.stride;
  }

  virtual size_t GetElementDistance () const
  {
    return props.stride ? props.stride :
      props.compCount * csRenderBufferComponentSizes[props.comptype & ~CS_BUFCOMP_NORMALIZED];
  }

  virtual size_t GetOffset () const
  { return props.offset; }

  virtual uint GetVersion ()
  {
    return version;
  }

  bool IsMasterBuffer ()
  {
    return !masterBuffer.IsValid ();
  }

  virtual iRenderBuffer* GetMasterBuffer () const
  {
    return masterBuffer;
  }

  virtual bool IsIndexBuffer () const
  { return props.isIndex; }

  virtual size_t GetRangeStart () const
  { return rangeStart; }
  virtual size_t GetRangeEnd () const
  { return rangeEnd; }

  virtual size_t GetElementCount () const;

  virtual void SetCallback (iRenderBufferCallback *cb)
  {
    callback = cb;
  }
  virtual void SetData (const void *data);
  /** @} */

  /**\name Render buffer creation
   * @{ */
  /**
   * Create a render buffer.
   * \param elementCount Number of elements in the buffer.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param componentType Components Types; CS_BUFCOMP_FLOAT, CS_BUFCOMP_INT,
   *        etc
   * \param componentCount Number of components per element (e.g. 4 for RGBA)
   * \param copy if true (default) then this buffer will make a copy of the
   *        data, else just save the buffer pointers provided by the caller.
   *        This has some implications: CopyInto() does not copy, merely update
   *	    the internal buffer pointer. Lock() just returns that pointer.
   *        The pointer passed to CopyInto() must be valid over the lifetime of
   *        the render buffer.
   */
  static csRef<csRenderBuffer> CreateRenderBuffer (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType, 
    uint componentCount);
    
  CS_DEPRECATED_METHOD_MSG("Using the 'copy' flag is deprecated; "
    "use SetData() instead if data copying is not desired") 
  static csRef<csRenderBuffer> CreateRenderBuffer (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType, 
    uint componentCount, bool copy);
    
  /**
   * Create an index buffer.
   * \param elementCount Number of elements in the buffer.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param componentType Components Types; usually CS_BUFCOMP_UNSIGNED_INT
   * \param rangeStart Minimum index value that is expected to be written to 
   *  the created buffer.
   * \param rangeEnd Maximum index value that is expected to be written to 
   *  the created buffer.
   * \param copy if true (default) then this buffer will make a copy of the
   *        data, else just save the buffer pointers provided by the caller.
   *        This has some implications: CopyInto() does not copy, merely update
   *	    the internal buffer pointer. Lock() just returns that pointer.
   *        The pointer passed to CopyInto() must be valid over the lifetime of
   *        the render buffer.
   */
  static csRef<csRenderBuffer> CreateIndexRenderBuffer (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd);

  CS_DEPRECATED_METHOD_MSG("Using the 'copy' flag is deprecated; "
    "use SetData() instead of data copying is not desired") 
  static csRef<csRenderBuffer> CreateIndexRenderBuffer (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd, bool copy);
    
  /**
   * Create an interleaved renderbuffer (You would use this then set stride to
   * determine offset and stride of the interleaved buffer
   * \param elementCount Number of elements in the buffer.
   * \param type Type of buffer; CS_BUF_DYNAMIC, CS_BUF_STATIC or 
   *  CS_BUF_STREAM.
   * \param count number of render buffers you want
   * \param elements Array of csInterleavedSubBufferOptions describing the 
   *  properties of the individual buffers to be interleaved.
   * \param buffers an array of render buffer references that can hold
   *  at least 'count' render buffers.
   *
   * Example on creating an interleaved buffer consisting of one three and two
   * component float buffer:
   * \code
   *  static const csInterleavedSubBufferOptions interleavedElements[2] =
   *    {{CS_BUFCOMP_FLOAT, 3}, {CS_BUFCOMP_FLOAT, 2}};
   *  csRef<iRenderBuffer> buffers[2];
   *  csRenderBuffer::CreateInterleavedRenderBuffers (num_verts, CS_BUF_STATIC,
   *    2, interleavedElements, buffers);
   *  csRef<iRenderBuffer> vertex_buffer = buffers[0];
   *  csRef<iRenderBuffer> texel_buffer = buffers[1];
   * \endcode
   */
  static csRef<csRenderBuffer> CreateInterleavedRenderBuffers (
    size_t elementCount, 
    csRenderBufferType type, uint count, 
    const csInterleavedSubBufferOptions* elements, 
    csRef<iRenderBuffer>* buffers);
  /** @} */

  /**\name "Friendly" name utilities
   * @{ */
  /**
   * Utility to retrieve the "friendly" string name of a buffer description,
   * e.g. "position" for CS_BUFFER_POSITION.
   */
  static const char* GetDescrFromBufferName (csRenderBufferName bufferName);
  /**
   * Retrieve the buffer name for a "friendly" buffer description.
   * Can be used to parse e.g. shader files.
   */
  static csRenderBufferName GetBufferNameFromDescr (const char* name);
  /** @} */
  
  //@{
  /**
   * Change properties of a render buffer after creation (DANGEROUS).
   * \warning If the buffer was already passed around and is thus probably 
   *   used somewhere, changing properties with this methiod is a sure-fire 
   *   way to wreak havoc. This function is useful in very very specific 
   *   scenarios; only use it if you really know what you're doing.
   */
  void SetRenderBufferProperties (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType, 
    uint componentCount, bool copy = true);
  void SetIndexBufferProperties (size_t elementCount, 
    csRenderBufferType type, csRenderBufferComponentType componentType,
    size_t rangeStart, size_t rangeEnd, bool copy = true);
  //@}
protected:
  /// Total size of the buffer
  size_t bufferSize;

  /**
   * To scrape off a few bytes use bitfields; assumes values are in sane 
   * limits.
   */
  struct Props
  {
    /// hint about main usage
    csRenderBufferType bufferType : 2;
    /// datatype for each component
    csRenderBufferComponentType comptype : 5;
    
    // padding
    uint unused0 : 1;
  
    /// number of components per element
    uint compCount : 8;
    /// buffer stride
    size_t stride : 8;
    /// offset from buffer start to data
    size_t offset : 8;

    /// should we copy data, or just use supplied buffer
    bool doCopy : 1; 
    /// if buffer should be deleted on deallocation
    bool doDelete : 1;
    /// currently locked? (to prevent recursive locking)
    bool isLocked : 1;
    /// if this is index-buffer  
    bool isIndex : 1;

    // padding
    uint unused1 : 2;
    
    /// last type of lock used
    uint lastLock : 2;

    Props (csRenderBufferType type, csRenderBufferComponentType componentType,
      uint componentCount, bool copy) : bufferType (type), 
      comptype (componentType), compCount (componentCount), stride(0), 
      offset (0), doCopy (copy), doDelete (false), isLocked (false), 
      isIndex (false), lastLock (0)
    {
      CS_ASSERT (componentCount <= 255); // Just to be sure...
    }
  } props;

  /// range start for index-buffer
  size_t rangeStart; 
  /// range start for index-buffer
  size_t rangeEnd; 
  
  /// modification number
  unsigned int version; 

  /// buffer holding the data
  unsigned char* buffer; 
  
  csRef<iRenderBuffer> masterBuffer;

  csWeakRef<iRenderBufferCallback> callback;

#ifdef CS_DEBUG
  csCallStack* lockStack;
#endif
};

/** @} */

namespace CS
{
  /**\addtogroup gfx
   * @{ 
   */

  /// Render buffer wrapper with additional persistence information.
  class RenderBufferPersistent : 
    public scfImplementation2<RenderBufferPersistent,
      iRenderBuffer, iRenderBufferPersistence>
  {
    csRef<iRenderBuffer> wrappedBuffer;
    csString filename;
  public:
    RenderBufferPersistent (iRenderBuffer* wrappedBuffer) : 
      scfImplementationType (this), wrappedBuffer (wrappedBuffer) {}

    void SetFileName (const char* filename) { this->filename = filename; }
    const char* GetFileName () { return filename; }

    /**\name iRenderBuffer implementation
     * @{ */
    void* Lock (csRenderBufferLockType lockType)
    { return wrappedBuffer->Lock (lockType); }
    void Release () { wrappedBuffer->Release (); }
    void CopyInto (const void* data, size_t elementCount,
      size_t elemOffset = 0) 
    { wrappedBuffer->CopyInto (data, elementCount, elemOffset); }
    int GetComponentCount () const
    { return wrappedBuffer->GetComponentCount (); }
    csRenderBufferComponentType GetComponentType () const 
    { return wrappedBuffer->GetComponentType (); }
    csRenderBufferType GetBufferType () const
    { return wrappedBuffer->GetBufferType (); }
    size_t GetSize () const
    { return wrappedBuffer->GetSize (); }
    size_t GetStride () const 
    { return wrappedBuffer->GetStride (); }
    size_t GetElementDistance () const
    { return wrappedBuffer->GetElementDistance (); }
    size_t GetOffset () const
    { return wrappedBuffer->GetOffset (); }
    uint GetVersion ()
    { return wrappedBuffer->GetVersion (); }
    iRenderBuffer* GetMasterBuffer () const
    { return wrappedBuffer->GetMasterBuffer (); }
    bool IsIndexBuffer () const
    { return wrappedBuffer->IsIndexBuffer (); }
    size_t GetRangeStart () const
    { return wrappedBuffer->GetRangeStart (); }
    size_t GetRangeEnd() const
    { return wrappedBuffer->GetRangeEnd (); }
    size_t GetElementCount () const
    { return wrappedBuffer->GetElementCount (); }
    void SetCallback (iRenderBufferCallback *cb)
    { wrappedBuffer->SetCallback (cb); }
    void SetData (const void *data)
    { wrappedBuffer->SetData (data); }
    /** @} */
  };

  /** @} */
} // namespace CS

#endif // __CS_CSGFX_RENDERBUFFER_H__
