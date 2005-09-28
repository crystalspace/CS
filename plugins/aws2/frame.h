/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __AWS_FRAME_H__
#define __AWS_FRAME_H__

#include "cstool/pen.h"
#include "csgeom/csrect.h"



namespace aws
{

  /**
   * A frame presents different transformation methods for various vector
   * operations.  It may also attempt to transform certain raster/blitting
   * operations.
   */
  class frame
  {     
    /// The box that we can draw in. This defines the "frame."
    csRect bounds;
 
    /// The parent bounding box.
    frame *parent;

  public:
    frame();
    virtual ~frame();

    /// Accessor for the frame's bounds.
    const csRect& Bounds() { return bounds; }

    /// Sets the size of the frame.
    void SetSize(int width, int height)
    {
      bounds.SetSize(width, height);
    }

    /**
     * Gets the screen absolute x and y coordinates of this frame. The 'x'
     * and 'y' variables should be initialized to zero before the call.
     */
    void GetScreenPos(float &x, float &y);

    /**
     * This function prepares the coordinate system for drawing, then calls
     * the OnDraw method.
     */
    void Draw(iPen *pen);

    /**
     * Makes transforming a shape very easy.  It first rotates the object
     * around it's axis by the number of radians specified in 'angle'.  Next,
     * it translates the object to the position specified
     * by 'x' and 'y'.  Note that this position is relative to the frame that
     * this object lives in. Note that Transform CLEARS the current transform,
     * and sets it to the transform you specify here.
     */
    void Transform(iPen *pen, float angle, float x, float y);

    /**
     * Override this function in the widget in order to draw it.  The drawing
     * space is 0,0 to bounds.width, bounds.height.
     */
    virtual void OnDraw(iPen *pen) {}
  };

  /** This manages frames.  It can find frames within a geometric area. */
  class frame_manager
  {

  };


}

#endif
