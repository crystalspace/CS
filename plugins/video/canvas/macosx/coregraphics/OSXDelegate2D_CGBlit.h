//
//  OSXDelegate2D_CGBlit.h
//
//
//  Created by mreda on Wed Oct 31 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//

#ifndef __CS_OSXDELEGATE2D_CGBLIT_H__
#define __CS_OSXDELEGATE2D_CGBLIT_H__

// C interface to a OSXDelegate2D category for using CG to blit to a window

#if defined(__cplusplus)

#include "csplugincommon/macosx/OSXDelegate2D.h"

#define DEL2D_FUNC(ret, func) __private_extern__ "C" ret OSXDelegate2D_##func

DEL2D_FUNC(bool, blitToWindow)(OSXDelegate2D delegate, unsigned char *buffer, int width, int height, int depth);

#undef DEL2D_FUNC

#endif // __cplusplus

#endif // __CS_OSXDELEGATE2D_CGBLIT_H__
