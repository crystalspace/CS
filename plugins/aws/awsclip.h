#ifndef __AWS_CLIPPING_PROCESSOR__
#define __AWS_CLIPPING_PROCESSOR__

#include "iaws/aws.h"

class awsClipper
{
  /// Pointer to g3d device to use
  iGraphics3D *g3d;

  /// Pointer to g2d device to use
  iGraphics2D *g2d;

  /// Clipping rectangle
  csRect clip;

public:
  /// Constructs a clipper
  awsClipper(iGraphics3D *_g3d, iGraphics2D *_g2d);

  /// Destructs a clipper
  ~awsClipper();

  /// Set the clipping rectangle
  void SetClipRect(csRect &r);

  /// Draw a line
  void DrawLine(float x1, float y1, float x2, float y2, int color);

  /// Draw a box
  void DrawBox (int x, int y, int w, int h, int color);

  /// Draw a pixel
  void DrawPixel (int x, int y, int color);

  /// Write a text string into the back buffer
  void Write (iFont *font, int x, int y, int fg, int bg,const char *str);

  /// Draw a pixmap
  void DrawPixmap (iTextureHandle *hTex, int sx, int sy,
    int sw, int sh, int tx, int ty, int tw, int th, uint8 Alpha = 0);
};


#endif
