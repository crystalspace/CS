/*
    Copyright (C) Aleksandras Gluchovas
    CS port by Norman Kraemer <norman@users.sourceforge.net>

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

#ifndef __CS_CSFLOWLAYOUT_H__
#define __CS_CSFLOWLAYOUT_H__

/**
 * \addtogroup csws_layout
 * @{ */

#include "csextern.h"
 
#include "cslayout.h"

/**
 * In a flow layout components are displayed in a row
 * and wraped at parents boundaries.
 *
 * Sample:
 * <p>
 * <code>
 * csWindow *wnd = new csWindow (theApp, "FlowLayout",
 *                               CSWS_DEFAULTVALUE & ~CSWS_MENUBAR);
 * wnd->SetSize (400, 300);
 * wnd->Center ();
 * wnd->Select ();
 *
 * csFlowLayout *flow = new csFlowLayout (wnd);
 * wnd->SendCommand (cscmdWindowSetClient, (void*)flow);
 * for (int k=0; k<10; k++)
 * {
 *   char tt[20];
 *   sprintf (tt, "t %d", k);
 *   csButton *b= new csButton (flow, 7000+k);
 *   b->SetPos (k*20, 20);
 *   b->SetSuggestedSize (0, 0);
 *   b->SetText (tt);
 * }
 * </code>
 * </p>
 */

class CS_CRYSTALSPACE_EXPORT csFlowLayout : public csLayout
{
protected:
  int mAlign;
  int mHgap;
  int mVgap;

  csPoint mPrefDimOfPhase1;
public:
  /**
   * Create a flow layout. The default aligning is CENTER.
   * The default horizontal and vertical gap between components is 0.
   */
  csFlowLayout (csComponent *pParent);
  csFlowLayout (csComponent *pParent, int align);
  csFlowLayout (csComponent *pParent, int align, int hgap, int vgap);

  /// Get the aligning of components that dont fill the canvas width.
  int GetAlignment ();
  /// Get the horizontal gap between components.
  int GetHgap ();
  /// Get the vertical gap between components.
  int GetVgap ();

  /// Set the aligning of components that dont fill the canvas width.
  void SetAlignment (int align);
  /// Set the horizontal gap between components.
  void SetHgap (int hgap);
  /// Set the vertical gap between components.
  void SetVgap (int vgap);

  // impl. of LayoutManager interface

  virtual void SuggestSize (int &sugw, int& sugh);

  virtual void LayoutContainer ();

public:

  enum ALIGNMNET_ENUM
  {
    /**
     * If the last row of components do not fill the entire width of the
     * layout then they are aligned either left or right or they are centered.
     */
    CENTER   = 0,
    LEFT     = 1,
    RIGHT    = 2,

    LEADING  = LEFT,  // for Western, Europian text-orientation
    TRAILING = RIGHT  // -/-
  };
};

/** @} */

#endif // __CS_CSFLOWLAYOUT_H__
