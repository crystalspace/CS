/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __ABSOLUTELAYOUT_G__
#define __ABSOLUTELAYOUT_G__

#include "cslayout.h"

/**
 * This is just for fun, you probably wont use it since theres no real gain.
 */
class csAbsoluteLayout : public csLayout
{
 public:

  csAbsoluteLayout(csComponent *pParent);

  virtual void SuggestSize (int &sugw, int& sugh);
  virtual void LayoutContainer ();
};

#endif // __ABSOLUTELAYOUT_G__
