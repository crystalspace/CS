/*
  Copyright (C) 2003 by Mårten Svanfeldt
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

#ifndef __CS_SOFT_RENDERBUFFER_H__
#define __CS_SOFT_RENDERBUFFER_H__

#include "ivideo/rndbuf.h"

SCF_VERSION(csSoftRenderBuffer, 0,0,2);
/**
* This is a general buffer to be used by the renderer. It can ONLY be
* created by the renderer
*/
class csSoftRenderBuffer : public iRenderBuffer
{
private:
  void *buffer;
  bool locked;

  int size, compcount, compSize;
  csRenderBufferType type;
  csRenderBufferComponentType comptype;
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  csSoftRenderBuffer (void *buffer, int size, csRenderBufferType type,
    csRenderBufferComponentType comptype, int compcount);

  /// Destructor.
  virtual ~csSoftRenderBuffer ();

  /// Get type of buffer (where it's located)
  virtual csRenderBufferType GetBufferType() const { return type; }

  /// Get the size of the buffer (in bytes)
  virtual int GetSize() const { return size; }

  /// Gets the number of components per element
  virtual int GetComponentCount () const { return compcount; }

  /// Gets the component type
  virtual csRenderBufferComponentType GetComponentType () const
  { return comptype; }


  /**
  * Lock the buffer to allow writing and give us a pointer to the data
  * The pointer will be 0 if there was some error
  */
  virtual void* Lock(csRenderBufferLockType lockType)
  {
    locked = true;
    return buffer;
  }

  /// Releases the buffer. After this all writing to the buffer is illegal
  virtual void Release() { locked = false; }
};

#endif //__CS_SOFT_RENDERBUFFER_H__
