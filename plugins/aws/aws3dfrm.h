#ifndef __AWS_3D_FRAME_DRAWER__
#define __AWS_3D_FRAME_DRAWER__

#include "aws.h"


/// This class draws several different 3d frame types to avoid code duplication.
class aws3DFrame
{
public:
  static const int fsBump;
  static const int fsSimple;
  static const int fsRaised;
  static const int fsSunken;
  static const int fsFlat;

public:
  /// Creates a frame drawer
  aws3DFrame();

  /// Destroys a frame drawer
  ~aws3DFrame();

  /// Draws a frame of the given type
  void Draw(iAws *wmgr, iAwsWindow *window, csRect &frame, int frame_style, iTextureHandle *bkg=NULL, int alpha_level=0);
};

#endif
