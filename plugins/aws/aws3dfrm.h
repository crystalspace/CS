#ifndef __AWS_3D_FRAME_DRAWER__
# define __AWS_3D_FRAME_DRAWER__


# include "csgeom/csrectrg.h"
# include "ivideo/texture.h"
struct iGraphics2D;
struct iGraphics3D;

struct iAws;
struct iAwsComponent;


/// This class draws several different 3d frame types to avoid code duplication.
class aws3DFrame
{
protected:

  // a ref to the current 2d drawing context
  iGraphics2D* g2d;

  // a ref to the current 3d drawing context
  iGraphics3D* g3d;

  // the basic colors
  int hi, hi2, lo, lo2, fill, dfill, black,
	  bfill;


  // background/overlay textures
  iTextureHandle *bkg, *ovl;

  // background/overlay alpha levels
  int bkg_alpha, ovl_alpha;


  void DrawTexturedBackground(csRectRegion* todraw,
                              iTextureHandle* txt,
                              int alpha_level,
							                csRect txt_align);

  void DrawFlatBackground(csRectRegion* todraw,
                          int color);

  void DrawRaisedFrame(csRect frame);

  void DrawSmallRaisedFrame(csRect frame);

  void DrawSunkenFrame(csRect frame);

  void DrawSmallSunkenFrame(csRect frame);

  void DrawBumpFrame(csRect frame);

  void DrawBevelFrame(csRect frame);

  void DrawThickFrame(csRect frame);

public:
  static const int fsBump;
  static const int fsSimple;
  static const int fsRaised;
  static const int fsSunken;
  static const int fsFlat;
  static const int fsNone;
  static const int fsBevel;
  static const int fsThick;
  static const int fsBitmap;
  static const int fsSmallRaised;
  static const int fsSmallSunken;
  static const int fsMask;

public:
  /// Creates a frame drawer
  aws3DFrame ();

  /// Destroys a frame drawer
  ~aws3DFrame ();

  // Retrieves the default pallete along with
  // textures and alpha levels
  void Setup(iAws *wmgr, 
             iTextureHandle* bkg = NULL,
             int bkg_alpha = 0,
             iTextureHandle* ovl = NULL,
             int ovl_alpha = 0);

  /// Draws a frame of the given type
  /// Three different versions are presented
  /// depending on whether you need offsets 
  /// on the background/overlay textures

  void Draw (
        csRect frame,
        int frame_style, 
		    csRectRegion* rgn = NULL);

  void Draw(
        csRect frame,
        int frame_style,
        csRect bkg_align,
        csRectRegion* rgn = NULL);
  
  void Draw(
        csRect frame,
        int frame_style,
        csRect bkg_align,
        csRect ovl_align,
        csRectRegion* rgn = NULL);


  csRect SubRectToAlign(csRect comp_frame, csRect txt_sub_rect);

  void SetBackgroundTexture(iTextureHandle* bkg);
  void SetOverlayTexture(iTextureHandle* ovl);
  void SetBackgroundAlpha(int bkg_alpha);
  void SetOverlayAlpha(int ovl_alpha);
  void SetBackgroundColor(int color);

  csRect GetInsets(int style);

};

// These are the frame styles actually support by the 3d frame class, and thus globally

const int _3dfsBump = 0;
const int _3dfsSimple = 1;
const int _3dfsRaised = 2;
const int _3dfsSunken = 3;
const int _3dfsFlat = 4;
const int _3dfsNone = 5;
const int _3dfsBevel = 6;
const int _3dfsThick = 7;
const int _3dfsBitmap = 8;
const int _3dfsSmallRaised = 9;
const int _3dfsSmallSunken = 10;
const int _3dfsMask  = 0xf;

#endif
