/*
    Copyright (C) 2001 by Norman Kraemer

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

#ifndef __CS_CSPLUGINCOMMON_IOPENGL_OPENGLINTERFACE_H__
#define __CS_CSPLUGINCOMMON_IOPENGL_OPENGLINTERFACE_H__

#include "csutil/scf.h"

/**\file
 * OpenGL-specific interfaces
 */

/**
 * A common interface to be implemented by the platform specific opengl 
 * canvases.
 * Currently covers only aspects that are different for opengl on different 
 * platforms like the "GetProcAddress" function.
 */
struct iOpenGLInterface : public virtual iBase
{
  SCF_INTERFACE (iOpenGLInterface, 2, 0, 0);
  
  /**
   * Retrieve the address of the function \a funcname.
   * Return 0 if not available.
   */
  virtual void *GetProcAddress (const char *funcname) = 0;
};

#endif // __CS_CSPLUGINCOMMON_IOPENGL_OPENGLINTERFACE_H__
