// awsPanel.h: interface for the awsPanel class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __AWS_PANEL_H__
#define __AWS_PANEL_H__

#include "awscomp.h"
#include "csgeom/csrectrg.h"
#include "aws3dfrm.h"

/** This class serves as a generic component which supports many of the basic
 * graphic options that components use such as background drawing and frame
 * styles. It also clips to children so that drawing is optimized
 */
class awsPanel : public awsComponent  
{
protected:
  // style
  int style;

  // child exclusion region
  csRectRegion todraw;

  // 3D frame drawer
  aws3DFrame frame_drawer;

  // true if the child_exclude region should be recalculated
  bool todraw_dirty;

  // textures for background and overlay
  iTextureHandle *bkg, *ovl;

  // alpha levels for background and overlay
  int bkg_alpha, ovl_alpha;

  /// Subrects of the background and overlay textures to use
  csRect bm_bkgsub, bm_ovlsub;
  
public:
  awsPanel();
  virtual ~awsPanel();

  bool Setup(iAws *_wmgr, iAwsComponentNode *settings);

  void OnDraw(csRect clip);
  
  void AddChild(iAwsComponent *comp);
  void RemoveChild(iAwsComponent *comp);
  
  void Move(int delta_x, int delta_y);
  
  virtual csRect getInsets();

  virtual void OnChildMoved();
  virtual void OnResized();
  virtual void OnChildShow();
  virtual void OnChildHide();

  // frame styles
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
  static const int fsMask;
  static const int fsNormal;
  static const int fsToolbar;
};

#endif

