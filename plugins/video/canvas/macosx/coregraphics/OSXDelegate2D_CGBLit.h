//
//  OSXDelegate2D_OpenGL.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//


// C interface to a OSXDelegate2D category for using CG to blit to a window

#if defined(__cplusplus)

#include "OSXDelegate2D.h"

#define DEL2D_FUNC(ret, func) __private_extern__ "C" inline ret OSXDelegate2D_##func

DEL2D_FUNC(bool, blitToWindow)(OSXDelegate2D delegate, unsigned char *buffer, int width, int height, int depth);

#undef DEL2D_FUNC

#endif
