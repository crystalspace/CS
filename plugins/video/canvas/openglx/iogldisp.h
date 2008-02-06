/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef __CS_IOGLDISP_H__
#define __CS_IOGLDISP_H__

#include "csutil/scf.h"
#include "iutil/comp.h"

SCF_VERSION (iOpenGLDisp, 0, 0, 1);

/**
 * iOpenGLDisp interface -- for special support of OpenGL displaydrivers.
 * Place in your implementation everything a special GL displaydriver needs
 * to do. This merely breaks down to initialization and shutdown stuff (
 * like grShutdown for Glide)
 */
struct iOpenGLDisp : public iComponent
{
  virtual bool open() = 0;
  virtual bool close() = 0;
};

#endif // __CS_IOGLDISP_H__
