/*
    Copyright (C) 2003 by Mat Sutcliffe <oktal@gmx.co.uk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_IVIDEO_GFXMEM_H__
#define __CS_IVIDEO_GFXMEM_H__

#include "csutil/scf.h"

/**\file
 * Memory canvas interfaces
 */

/**
 * This is the interface used to access the csGraphicsMemory plugin.
 */
struct iGraphicsMemory : public virtual iBase
{
  SCF_INTERFACE (iGraphicsMemory, 0, 0, 1);

  /**
   * Get a pointer to the memory containing the canvas image.
   */
  virtual unsigned char* GetImage () = 0;
};

#endif // __CS_IVIDEO_GFXMEM_H__
