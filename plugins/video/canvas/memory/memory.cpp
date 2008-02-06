/*
    Copyright (C) 2001-2002 by Brendan Burns <bburns@cs.umass.edu>
    Written for EKSL <http://eksl.cs.umass.edu> Paul Cohen Director.

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

#include "cssysdef.h"
#include "csgeom/csrect.h"

#define THREAD_SUPPORT 0

#include "memory.h"
#include "iutil/objreg.h"

#if THREAD_SUPPORT
extern "C" {
#  include <pthread.h>
}
#endif

CS_IMPLEMENT_PLUGIN
SCF_IMPLEMENT_FACTORY (csGraphicsMemory)

#if THREAD_SUPPORT
void *csGraphicsMemory::updateThread(void *obj)
{
  csGraphicsMemory *theObj = (csGraphicsMemory *)obj;
  theObj->running = true;
  while (theObj->running)
  {
    pthread_mutex_lock(&(theObj->memoryUpdateLock_));
    
    pthread_mutex_unlock(&(theObj->memoryUpdateLock_));
  }
  pthread_exit(0);
  return 0;
}
#endif

csGraphicsMemory::csGraphicsMemory (iBase* p) :
  scfImplementationType (this, p), buff_a(0), res(0)
{
#if THREAD_SUPPORT
	buff_b = 0;
#endif
}

csGraphicsMemory::~csGraphicsMemory()
{
  delete (buff_a);
#if THREAD_SUPPORT
  delete (buff_b);
#endif
  delete (res);
  Memory = 0;
}

bool csGraphicsMemory::BeginDraw()
{
#if THREAD_SUPPORT
  pthread_mutex_lock(&memoryUpdateLock_);
#endif
  csGraphics2D::BeginDraw();
  return true;
}

void csGraphicsMemory::FinishDraw()
{
#if THREAD_SUPPORT
  if (Memory == buff_a)
  {
    Memory = buff_b;
  }
  else
  {
    Memory = buff_a;
  }
  pthread_mutex_unlock(&memoryUpdateLock_);
#endif
  csGraphics2D::FinishDraw();
}

bool csGraphicsMemory::Initialize(iObjectRegistry* obj_reg)
{
  bool ok = csGraphics2D::Initialize(obj_reg);
  obj_reg->Register(this, "crystalspace.canvas.memory");
  if (ok)
  {
    Depth = 16;
    pfmt.RedMask   = 0x1f << 11;
    pfmt.GreenMask = 0x3f << 5;
    pfmt.BlueMask  = 0x1f << 0;
    pfmt.AlphaMask = 0x00;
    pfmt.PalEntries = 0;
    pfmt.PixelBytes = 2;
    pfmt.complete();
		
    size=Height*Width;
    buff_a = new unsigned char[size * pfmt.PixelBytes];
#if THREAD_SUPPORT
    buff_b = new unsigned char[size * pfmt.PixelBytes];
#endif
    res = new unsigned char[size * pfmt.PixelBytes];
#if THREAD_SUPPORT
    if (!buff_a || !buff_b)
      return false;
#else
    if (!buff_a)
      return false;
#endif
    
    Memory = buff_a;
  }
  return ok;
}

void csGraphicsMemory::Print (csRect const*)
{
}

bool csGraphicsMemory::Open ()
{
  bool ok = csGraphics2D::Open();
  return ok;
}

void csGraphicsMemory::Close ()
{
  csGraphics2D::Close();
}

unsigned char *csGraphicsMemory::GetImage()
{
#if THREAD_SUPPORT
  pthread_mutex_lock(&memoryUpdateLock_);
  if (Memory == buff_a)
  {
    memcpy(res, buff_b, size * pfmt.PixelBytes);
  }
  else
  {
    memcpy(res, buff_a, size * pfmt.PixelBytes);
  }
  pthread_mutex_unlock(&memoryUpdateLock_);
#else
  memcpy(res, buff_a, size * pfmt.PixelBytes);
#endif
  return res;
}

