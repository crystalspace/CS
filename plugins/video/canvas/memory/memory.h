/*
    image rendering support for Crystal Space 3D library
    (c) 2001-2002 Brendan Burns <bburns@cs.umass.edu>
    Written for EKSL <http://eksl.cs.umass.edu> Paul Cohen, Director

    Portions (c) 2002 Peter Amstutz <amstutz@cs.umass.edu>

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

#ifndef __CS_MEMORY_H__
#define __CS_MEMORY_H__

#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"
#include "ivideo/gfxmem.h"

#if THREAD_SUPPORT
extern "C" {
#  include <pthread.h>
}
#endif

/**
 * The memory driver. This is a cross-platform graphics driver which
 * implements drawing via memory replication.
 */
class csGraphicsMemory : public csGraphics2D
{
  typedef csGraphics2D superclass;
protected:
  unsigned char *buff_a, *res;
#if THREAD_SUPPORT
  unsigned char *buff_b
  bool running;
  pthread_mutex_t memoryUpdateLock_;
  pthread_t updateThread_;
  int updateThreadID_;
#endif
  int size;
public:
  SCF_DECLARE_IBASE_EXT(csGraphics2D);
  csGraphicsMemory(iBase* iParent);
  virtual ~csGraphicsMemory();

  virtual bool Initialize(iObjectRegistry*);
  virtual bool Open();
  virtual void Close();

  virtual bool DoubleBuffer(bool) {return false;}

  virtual bool BeginDraw();
  virtual void FinishDraw();

  virtual void Print(csRect const* area = 0);

  unsigned char *GetImage();
  struct eiGraphicsMemory : public iGraphicsMemory
  {
    SCF_DECLARE_EMBEDDED_IBASE(csGraphicsMemory);
    virtual unsigned char *GetImage() { return scfParent->GetImage(); }
  } scfiGraphicsMemory;

#if THREAD_SUPPORT
protected:
  static void *updateThread(void *obj);
#endif
};

#endif // __CS_MEMORY_H__
