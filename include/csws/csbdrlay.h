/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __BORDERLAYOUT_G__
#define __BORDERLAYOUT_G__

#include "cslayout.h"

/**
 * Displays upto 5 components. They are located in the north, south west, east and center of the
 * canvas.
 */

class csBorderConstraint : public csLayoutConstraint
{
 public:
  int mAlign;
 public:
  csBorderConstraint (int align): mAlign( align ) {}
  csBorderConstraint (const csBorderConstraint &c) : csLayoutConstraint (c.comp)
    { mAlign = c.mAlign; }
  virtual csLayoutConstraint *Clone ();
};

class csBorderLayout : public csLayout2
{
 public:
  csBorderLayout (csComponent *pParent);
  csBorderLayout (csComponent *pParent, int hgap, int vgap);

  ~csBorderLayout();

  virtual void RemoveLayoutComponent (csComponent* comp);
  virtual void SuggestSize (int &w, int &h);

  virtual void LayoutContainer ();

  virtual csLayoutConstraint *AddLayoutComponent (csComponent* comp);
  virtual void MaximumLayoutSize (int &w, int &h);

  virtual float GetLayoutAlignmentX ();
  virtual float GetLayoutAlignmentY ();

 public:

  csBorderConstraint c;

  static csBorderConstraint* CENTER;
  static csBorderConstraint* EAST;
  static csBorderConstraint* NORTH;
  static csBorderConstraint* SOUTH;
  static csBorderConstraint* WEST;
  static csBorderConstraint* AFTER_LAST_LINE;
  static csBorderConstraint* AFTER_LINE_ENDS;
  static csBorderConstraint* BEFORE_FIRST_LINE;
  static csBorderConstraint* BEFORE_LINE_BEGINS;

 protected:

  static csBorderConstraint mCENTER;
  static csBorderConstraint mEAST;
  static csBorderConstraint mNORTH;
  static csBorderConstraint mSOUTH;
  static csBorderConstraint mWEST;
  static csBorderConstraint mAFTER_LAST_LINE;
  static csBorderConstraint mAFTER_LINE_ENDS;
  static csBorderConstraint mBEFORE_FIRST_LINE;
  static csBorderConstraint mBEFORE_LINE_BEGINS;

 protected:

  enum ALIGNMENT_ENUM_INTERNAL
  {
    _CENTER = 0,
    _EAST   = 1,
    _NORTH  = 2,
    _SOUTH  = 3,
    _WEST   = 4,

    // for Western, top-to-bottom, left-to-right orientations

    _AFTER_LAST_LINE    = _SOUTH,
    _AFTER_LINE_ENDS    = _EAST,
    _BEFORE_FIRST_LINE  = _NORTH,
    _BEFORE_LINE_BEGINS = _WEST,
  };

 protected:
  void DistributeSizes( int& left, int& center, 
			int& right, int totalWidth, int gap,
			bool hasLeft, bool hasCenter, bool hasRight );

  int GetWidthSum( int left, int center, int right, int gap, 
		   bool hasLeft, bool hasCenter, bool hasRight );

  int Hgap (csComponent* pComp);
  int Vgap (csComponent* pComp);

 protected:
  int mHgap;
  int mVgap;

};

#endif // __BORDERLAYOUT_G__
