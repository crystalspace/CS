//
//  OSXDelegate2D_OpenGL.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#ifndef __CS_OSXDELEGATE2D_OPENGL_H__
#define __CS_OSXDELEGATE2D_OPENGL_H__

// C interface to a OSXDelegate2D category for creating an OpenGL context

#if defined(__cplusplus)

#include "csplugincommon/macosx/OSXDelegate2D.h"

#define DEL2D_FUNC(ret, func) __private_extern__ "C" ret OSXDelegate2D_##func


DEL2D_FUNC(CGLContextObj, createOpenGLContext)(OSXDelegate2D delegate, int depth,
                                                CGDirectDisplayID display);
DEL2D_FUNC(void, updateOpenGLContext)(OSXDelegate2D delegate);

#undef DEL2D_FUNC

#endif // __cplusplus

#endif // __CS_OSXDELEGATE2D_OPENGL_H__
