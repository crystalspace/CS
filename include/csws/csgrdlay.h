/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Kraemer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __CS_CSGRIDLAYOUT_H__
#define __CS_CSGRIDLAYOUT_H__

/**
 * \addtogroup csws_layout
 * @{ */
 
#include "csextern.h"
 
#include "cslayout.h"

/**
 * Components are displayed in a grid fashion.
 */
class CS_CRYSTALSPACE_EXPORT csGridLayout : public csLayout
{
protected:
  size_t mRows;
  size_t mCols;
  int mHgap;
  int mVgap;
public:
  csGridLayout (csComponent *pParent);

  csGridLayout (csComponent *pParent, int rows, int cols);

  csGridLayout (csComponent *pParent, int rows, int cols, int hgap, int vgap);

  virtual size_t GetColumns () { return mCols; }
  virtual int GetHgap () { return mHgap; }
  virtual size_t GetRows () { return mRows; }
  virtual int GetVgap () { return mVgap; }

  virtual void SetColumns (size_t columns ) { mCols = columns; }
  virtual void SetHgap (int hgap) { mHgap = hgap; }
  virtual void SetRows (size_t rows) { mRows = rows; }
  virtual void SetVgap (int vgap) { mVgap = vgap; }

  virtual void SuggestSize (int &sugw, int &sugh);

  virtual void LayoutContainer ();
};

/** @} */

#endif // __CS_CSGRIDLAYOUT_H__
