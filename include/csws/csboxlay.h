/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __BOXLAYOUT_G__
#define __BOXLAYOUT_G__

#include "cslayout.h"

/**
 * Components are scaled to fir in one row or one column of the canvas.
 */

class csBoxLayout : public csLayout
{
 public:
  csBoxLayout (csComponent* pParent, int axis);

  virtual void SuggestSize (int &sugw, int& sugh);
  virtual void LayoutContainer ();

  enum AXIS_ORIENTATIONS
  {
    X_AXIS = 0,
    Y_AXIS = 1
  };

 protected:
  int mAxis;
};

#endif // __BOXLAYOUT_G__
