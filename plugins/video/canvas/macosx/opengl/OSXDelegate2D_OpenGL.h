//
//  OSXDelegate2D_OpenGL.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//


// C interface to a OSXDelegate2D category for creating an OpenGL context

#if defined(__cplusplus)

#include "OSXDelegate2D.h"

#define DEL2D_FUNC(ret, func) __private_extern__ "C" inline ret OSXDelegate2D_##func


DEL2D_FUNC(CGLContextObj, createOpenGLContext)(OSXDelegate2D delegate, int depth);
DEL2D_FUNC(void, updateOpenGLContext)(OSXDelegate2D delegate);

#undef DEL2D_FUNC

#endif
