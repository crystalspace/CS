/*
    Copyright (C) Aleksandras Gluchovas
    CS port by Norman Krämer <norman@users.sourceforge.net>
  
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

#ifndef __BORDERLAYOUT_G__
#define __BORDERLAYOUT_G__

#include "cslayout.h"

/**
 * Displays upto 5 components. They are located in the north, south,
 * west, east and center of the canvas.
 *
 * Sample:
 * <p>
 * <code>
 * csWindow *wnd = new csWindow (theApp, "BorderLayout", 
 *                               CSWS_DEFAULTVALUE & ~CSWS_MENUBAR);
 * wnd->SetSize (400, 300);
 * wnd->Center ();
 * wnd->Select ();
 * 
 * csBorderLayout *border = new csFlowLayout (wnd);
 * wnd->SendCommand (cscmdWindowSetClient, (void*)border);
 *
 * csBorderConstraint *blc[5] = {csBorderLayout::CENTER, 
 *				 csBorderLayout::EAST, 
 *				 csBorderLayout::NORTH,
 *				 csBorderLayout::WEST,
 *				 csBorderLayout::SOUTH};
 * for (int k=0; k<5; k++)
 * {
 *   border->c = *blc[k];
 *   csButton *b= new csButton (border, 7000+k);
 *   b->SetPos ((9+k)*10, 20);
 *   b->SetSuggestedSize (0, 0);
 *   char text[20];
 *   sprintf (text, "Test %d", k);
 *   b->SetText (text);
 * }
 * </code>
 * </p>
 */

/**
 * This subclass of csLayoutConstraint additionally stores the location
 * of the attached control.
 */

class csBorderConstraint : public csLayoutConstraint
{
 public:
  int mAlign;
 public:
  /**
   * use the following values for aligning
   * 0 ... center
   * 1 ... left
   * 2 ... top
   * 3 ... bottom
   * 4 ... right
   */
  csBorderConstraint (int align): mAlign( align ) {}
  /// copy constructor
  csBorderConstraint (const csBorderConstraint &c) : 
    csLayoutConstraint (c.comp)
    { mAlign = c.mAlign; }
  virtual csLayoutConstraint *Clone ();
};

class csBorderLayout : public csLayout2
{
 public:
  csBorderLayout (csComponent *pParent);
  /**
   * Make sure there is at least a horizontal gap of hgap pixels
   * and a vertical gap of vgap pixels.
   */
  csBorderLayout (csComponent *pParent, int hgap, int vgap);

  ~csBorderLayout();

  /**
   * A components is added to a layout by creating it and passing
   * the layout component as its parent component.
   * If you insist of doing some voodoo you should use AddLayoutComponent
   * to add it to the layout.
   * <pre>
   * IN:  the component to add
   * OUT: the constraint that is used to layout the component 
   *      (a copy of variable c  see above)
   * </pre>
   */
  virtual csLayoutConstraint *AddLayoutComponent (csComponent* comp);
  /// remove a component from the layout
  virtual void RemoveLayoutComponent (csComponent* comp);

  virtual void SuggestSize (int &w, int &h);

  /// recalc positions and sizes of components
  virtual void LayoutContainer ();

  virtual void MaximumLayoutSize (int &w, int &h);
  virtual float GetLayoutAlignmentX ();
  virtual float GetLayoutAlignmentY ();

 public:

  csBorderConstraint c;

  /// for your convenience here are prebuild constraints
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
    _BEFORE_LINE_BEGINS = _WEST
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
