/*
    SDL 2d canvas for Crystal Space (header)
    Copyright (C) 2000 by George Yohng <yohng@drivex.dosware.8m.com>

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

#ifndef __CS_NULL2D_H__
#define __CS_NULL2D_H__

#include "csutil/scf.h"
#include "csplugincommon/canvas/graph2d.h"

/// Null version.
class csGraphics2DNull : public csGraphics2D
{
public:
  csGraphics2DNull (iBase *iParent);
  virtual ~csGraphics2DNull ();

  virtual bool Initialize (iObjectRegistry *object_reg);
  virtual bool Open ();
  virtual void Close ();

  /// Necessary to access framebuffer
  virtual bool BeginDraw();
  virtual void FinishDraw();

  virtual void Print (csRect const* area = 0);
  virtual void SetRGB (int i, int r, int g, int b);

  virtual bool SetMousePosition (int , int );
  virtual bool SetMouseCursor (csMouseCursorID iShape);

  /// Clear backbuffer
  virtual void Clear (int )
  { }
  /// Clear all video pages
  virtual void ClearAll (int)
  { }

  /// Same but exposed through iGraphics2D interface
  virtual void DrawPixel (int , int , int )
  { }
  virtual void DrawPixels (csPixelCoord const* , int , int )
  { }
  /// Blit a memory block. The format of the image is RGBA in bytes. Row by row.
  virtual void Blit (int , int , int , int ,unsigned char const* )
  { }
  /// Draw a line
  virtual void DrawLine (float , float , float , float , int )
  { }
  /// Draw a box of given width and height
  virtual void DrawBox (int , int , int , int , int )
  { }
 
  virtual void Write (iFont*, int, int, int, int, const char*, unsigned)
  { }
  virtual void WriteBaseline (iFont* , int, int, int, int, const char*)
  { }

  virtual unsigned char* GetPixelAt (int, int)
  { return 0; }
};

#endif // __CS_NULL2D_H__
