/*
    Copyright (C) 2003 by Philipp Aumayr

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

#ifndef __CS_CSTOOL_USERRNDBUF_H__
#define __CS_CSTOOL_USERRNDBUF_H__

/**\file
 */

#include "csextern.h"

#include "csutil/ref.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "csutil/objreg.h"
#include "ivideo/rndbuf.h"
#include "iutil/strset.h"

SCF_VERSION(iUserRenderBufferIterator, 0, 0, 1);

/**
 * Interface to iterate over all buffers stored in a csUserRenderBufferManager.
 */
struct iUserRenderBufferIterator : public iBase
{
  /// Whether a next buffer is available.
  virtual bool HasNext() = 0;
  /// Get name of next buffer and optionally the buffer.
  virtual csStringID Next (csRef<iRenderBuffer>* buf = 0) = 0;
  /// Rewind to beginning.
  virtual void Reset() = 0;
};

/**
 * Helper class to manage multiple render buffers, usually provided by the
 * user.
 */
class CS_CRYSTALSPACE_EXPORT csUserRenderBufferManager
{
  struct userbuffer
  {
    csRef<iRenderBuffer> buf;
    csStringID name;
  };
  class UserBufArrayCmp : public csArrayCmp<userbuffer, csStringID>
  {
    static int BufKeyCompare (userbuffer const& b, csStringID const& k)
    { 
      return b.name - k;
    }
  public:
    UserBufArrayCmp (csStringID key) : 
      csArrayCmp<userbuffer, csStringID> (key, &BufKeyCompare)
    {
    }
  };
  static int BufCompare (userbuffer const& r, userbuffer const& k);

  csArray<userbuffer> userBuffers;
public:
  /// Retrieve a buffer.
  iRenderBuffer* GetRenderBuffer (csStringID name) const;
  /**
   * Add a buffer. Returns false if a buffer of the same name was already 
   * added.
   */
  bool AddRenderBuffer (csStringID name, iRenderBuffer* buffer);
  /**
   * Remove a buffer. Returns false if no buffer of the specified name was 
   * added.
   */
  bool RemoveRenderBuffer (csStringID name);

  csRef<iUserRenderBufferIterator> GetBuffers() const;
};

#endif // __CS_CSTOOL_USERRNDBUF_H__
