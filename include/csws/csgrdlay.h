/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __GRIDLAYOUT_G__
#define __GRIDLAYOUT_G__

/**
 * \addtogroup csws_layout
 * @{ */
 
#include "cslayout.h"

/**
 * Components are displayed in a grid fashion.
 */
class csGridLayout : public csLayout
{
protected:
  int mRows;
  int mCols;
  int mHgap;
  int mVgap;
public:
  csGridLayout (csComponent *pParent);

  csGridLayout (csComponent *pParent, int rows, int cols);

  csGridLayout (csComponent *pParent, int rows, int cols, int hgap, int vgap);

  virtual int GetColumns () { return mCols; }
  virtual int GetHgap () { return mHgap; }
  virtual int GetRows () { return mRows; }
  virtual int GetVgap () { return mVgap; }

  virtual void SetColumns (int columns ) { mCols = columns; }
  virtual void SetHgap (int hgap) { mHgap = hgap; }
  virtual void SetRows (int rows) { mRows = rows; }
  virtual void SetVgap (int vgap) { mVgap = vgap; }

  virtual void SuggestSize (int &sugw, int &sugh);

  virtual void LayoutContainer ();
};

/** @} */

#endif // __GRIDLAYOUT_G__
