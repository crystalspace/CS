/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_CSFXSCR_H__
#define __CS_CSFXSCR_H__

#include "csextern.h"

#include "ivideo/graph3d.h"

class csColor;
struct iGraphics2D;
struct iTextureManager;

/*
 * Some full screen special effects.
 * Most need to be called, either during 2d drawing phase - between
 * the g2d->BeginDraw and FinishDraw calls, or they need to be called
 * in the 3d drawing phase, between the g3d->BeginDraw and FinishDraw calls.
 */

/**
 * Shows lines of interference on the screen.
 * anim is an animation value. (0..1) change it a bit for animation.
 * amount is the number of stripes, 0=almost none, 1=screen is mostly filled.
 * amount must be >= 0;
 * length is the maximum width in pixels for the stripes.
 * On 3d hardware, drawing 2d lines is expensive (compared to drawing
 * polygons). For good speed, use amount ~0.3, length >= 30 or so.
 * Use this routine in 2D drawing mode.
 */
CS_CRYSTALSPACE_EXPORT void csfxInterference (iGraphics2D *g2d, 
  float amount, float anim, float length);

/**
 * Fade the screen to black. The fadevalue determines how much fading is
 * done. fadevalue 0: no fading. fadevalue 1: all black.
 * Hardware acceleration is used, if available.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxFadeOut (iGraphics3D *g3d, float fadevalue);

/**
 * Fade to given pixmap (which must be prepared for 3D usage),
 * fadevalue 0: no fading, fadevalue 1: only given texture is visible.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxFadeTo (iGraphics3D *g3d, iTextureHandle *tex, 
  float fadevalue);

/**
 * Fade the screen to a specific color, passed as parameter.
 * fadevalue 0: no fading, fadevalue 1: screen filled with the given color.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxFadeToColor (iGraphics3D *g3d, float fadevalue, 
  const csColor& color);

/**
 * Make the screen look like an old fashioned green-only monitor.
 * fadevalue 0: no fading, fadevalue 1: only green is visible.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxGreenScreen (iGraphics3D *g3d, float fadevalue);

/**
 * Similar to csfxGreenScreen, but makes the screen show only red.
 * fadevalue 0: no fading, fadevalue 1: only green is visible.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxRedScreen (iGraphics3D *g3d, float fadevalue);

/**
 * Similar to csfxGreenScreen, but makes the screen show only blue
 * fadevalue 0: no fading, fadevalue 1: only green is visible.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxBlueScreen (iGraphics3D *g3d, float fadevalue);

/**
 * Increase the brightness of the image.
 * fadevalue 0: no fading, fadevalue 1: full white.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxWhiteOut (iGraphics3D *g3d, float fadevalue);

/**
 * Create a vertical shading from topcolor to bottomcolor.
 * default this is copied to the screen, overwriting the old contents,
 * also other mixmodes can be used.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxShadeVert (iGraphics3D *g3d, const csColor& topcolor,
  const csColor& bottomcolor, uint mixmode = CS_FX_COPY);


/**
 * Do a fullscreen drawpolygonFX draw, used by some other routines.
 * This routine must only be used between g3d->BeginDraw and FinishDraw calls.
 */
CS_CRYSTALSPACE_EXPORT void csfxScreenDPFX (iGraphics3D *g3d, iTextureHandle *tex, 
  uint mixmode, uint8 r, uint8 g, uint8 b);

/**
 * Do a drapolygonFX draw, but only part of the screen is covered.
 * Rest the same as the fullscreen version.
 */
CS_CRYSTALSPACE_EXPORT void csfxScreenDPFXPartial(iGraphics3D *g3d, int x, int y, 
  int w, int h, iTextureHandle *tex, uint mixmode, uint8 r, uint8 g, uint8 b);

#endif // __CS_CSFXSCR_H__

