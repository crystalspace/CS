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

#ifndef __CS_ANONRNDBUF_H__
#define __CS_ANONRNDBUF_H__

#include "csextern.h"

#include "csutil/ref.h"
#include "csutil/garray.h"
#include "csutil/refarr.h"
#include "csutil/objreg.h"
#include "ivideo/rndbuf.h"
#include "iutil/strset.h"

/// Document me! @@@
class CS_CSTOOL_EXPORT csAnonRenderBufferManager
{
  class anonbuffer
  {
  public:
    anonbuffer() {};
    ~anonbuffer() {};

    csRef<iRenderBuffer> buf;
    csStringID name;
    int size;

  };

  csDirtyAccessArray<anonbuffer *> anon_buffers;
  iObjectRegistry *object_reg;

public:
  csAnonRenderBufferManager(iObjectRegistry *object_reg);
  virtual ~csAnonRenderBufferManager();

  iRenderBuffer * GetRenderBuffer(csStringID name);
  bool AddRenderBuffer (const char *name,
	csRenderBufferComponentType component_type, int component_size,
	int num_verts);
  bool SetRenderBufferComponent (const char *name, int index,
  	int component, float value);
  bool SetRenderBufferComponent (const char *name, int index,
  	int component, int value);
  bool SetRenderBuffer (const char *name, float *value, int num_verts);
  bool SetRenderBuffer (const char *name, int *value, int num_verts);
};

#endif // __CS_ANONRNDBUF_H_
