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

#ifndef __CS_CSGRIDBAGLAYOUT_H__
#define __CS_CSGRIDBAGLAYOUT_H__

/**\file
 */

/**
 * \addtogroup csws_layout
 * @{ */
 
#include "csextern.h"
 
#include "cslayout.h"
#include "csutil/parray.h"

/**
 * Subclass of csLayoutConstraint for use with csGridBagLayout.
 */
class CS_CRYSTALSPACE_EXPORT csGridBagConstraint : public csLayoutConstraint
{
public:
  csGridBagConstraint (csComponent *comp);
  csGridBagConstraint (const csGridBagConstraint &c);
  csGridBagConstraint (csComponent *comp, int _gridx, int _gridy,
		       int _gridwidth, int _gridheight, float _weightx,
		       float _weighty, int _anchor, int _fill,
		       csRect _insets, int _ipadx, int _ipady);
  csLayoutConstraint *Clone ();
public:
  /**
   * gridx set column for next cell to add:
   * RELATIVE ... right behind last added<br>
   * > 0 ... absolute column
   */
  int gridx;
  /**
   * gridx set row for next cell to aad:
   * RELATIVE ... same as last added or the next row if last
   *              gridwidth == REMAINDER<br>
   * > 0 ... absolute row
   */
  int gridy;
  /**
   * Set gridwidth to define the number of columns the cell spans.
   * Set to REMAINDER to be the last cell on a row.
   */
  int gridwidth;
  /**
   * Set gridheight to define the number of rows the cell spans.
   * Set to REMAINDER to be the last cell on a column.
   */
  int gridheight;
  /**
   * weightx defines how much space is added or substracted to a cell if
   * the layout size changes. Additional space is given columnwise. For this
   * the largest weightx in a column is determined and used for all cells in
   * the column.
   */
  float weightx;
  /**
   * weighty defines how much space is added or substracted to a cell if
   * the layout size changes. Additional space is given rowwise. For this
   * the largest weighty in a row is determined and used for all cells in
   * the row.
   */
  float weighty;
  /**
   * If you choose to not size the component to fit in a cell you can set the
   * position of the component in a cell. Use the following values:
   * CENTER ... center component in the middle of cell<br>
   * WEST   ... align component left horizontally and center vertically<br>
   * NORTH  ... center component horizontally and align on top of cell<br>
   * EAST   ... align component right horizontally and center vertically<br>
   * SOUTH  ... center component horizontally and align on bottom of cell
   * to put it in the corner of the cell use the following<br>
   * NORTHWEST ... upper left corner<br>
   * NORTHEAST ... upper right corner<br>
   * SOUTHEAST ... lower right corner<br>
   * SOUTHWEST ... lower left corner<br>
   */
  int anchor;
  /**
   * To size the component to fit in cell set fill to BOTH. This will scale
   * the component vertical and horizontal. Set to HORIZONTAL or VERTICAL to
   * scale component horizontal and vertical resp.
   * To disallow scaling at all set to NONE.
   */
  int fill;
  /**
   * This adds additional space to the component width and height.
   */
  csRect insets;
  /**
   * ipadx preserves ipadx pixels between component and cell left and right
   * edges.
   */
  int ipadx;
  /**
   * ipady preserves ipady pixels between component and cell upper and lower
   * edges.
   */
  int ipady;

#undef RELATIVE
#undef _LEFT
#undef _CENTER

  enum GRID_BAG_CONSTANTS
  {
    RELATIVE   = -1,
    REMAINDER  = 0,

    NONE       = 0,
    BOTH       = 1,
    HORIZONTAL = 2,
    VERTICAL   = 3,

    CENTER    = 10,
    NORTH     = 11,
    NORTHEAST = 12,
    EAST      = 13,
    SOUTHEAST = 14,
    SOUTH     = 15,
    SOUTHWEST = 16,
    WEST      = 17,
    NORTHWEST = 18
  };

  enum GRID_BAG_CONSTANTS_INTERNAL
  {
    _LEFT     = 20,
    _CENTER   = 21,
    _RIGHT    = 22
  };

  bool bSized;
  csPoint mPrefCompSize;
};

/**
 * csGridBagLayout is the most flexible layout class.
 * Here is how it works:
 *
 * A grid is laid over the layout components canvas. This yields
 * rows and columns (oh, really ?) which cross in cells. Each cell is
 * able to hold a single component (which again can be layout components).
 * You are able to define whether a cell should span one or more columns and
 * same with rows. Furthermore you can define how additional space is
 * distributed among columns/rows if the layout is resized and becomes larger
 * or smaller.
 * A component in a cell is not set to fit a priori. You must tell if it
 * should be resized to cover the cell canvas. If you choose to not resize the
 * component in a cell you can set a relative position within the cell like:
 * CENTER ... center the component in the middle of the cell
 * NORTH  ... center the component horizontally and put it at top of cell
 * For more variations see below.
 * To make sure there is space at left/right and/or top/bottom between cell
 * borders and the components edges use ipadx and ipady.
 *
 * The grids cells are filled from left to right and top to bottom in the
 * sequence components with the csGridBagLayout as parent are created.
 * The filling starts at cell (0,0). To mark the last cell in a row simply
 * set gridwidth to REMAINDER. The next component added is set in the next
 * rows first cell. You can temporary break the filling sequence by setting
 * gridx and/or gridy to a particular value. To continue the normal filling
 * process set gridx/gridy to RELATIVE.
 *
 * <p>
 * <pre>
 *  csComponent *window = new csWindow (this, "Gridbag Layout test",
 *    CSWS_DEFAULTVALUE & ~CSWS_MENUBAR);
 *  window->SetSize (400, 300);
 *  window->Center ();
 *  window->Select ();
 *
 *  csGridBagLayout *gb = new csGridBagLayout (window);
 *
 *  window->SendCommand (cscmdWindowSetClient, (void*)gb);
 *
 *  gb->c.fill = csGridBagConstraint::NONE; // dont resize components
 *  gb->c.weightx = 1.0; // scale cell horizontally only
 *  gb->c.weighty = 0.0;
 *  // align component within cell
 *  gb->c.anchor = csGridBagConstraint::SOUTHWEST;
 *  gb->c.ipady = 5; // distance to listbox below
 *  csStatic *label = new csStatic (gb, 0, "Message history");
 *  label->SetSize (300, 20);
 *
 *  gb->c.ipady = 0;
 *  csSpinBox *spb = new csSpinBox (gb);
 *  spb->SetSize (80,20);
 *  spb->SetLimits (1, 1000);
 *  spb->SetValue (200);
 *
 *  gb->c.ipady = 5;
 *  gb->c.weightx = 1.0;
 *  gb->c.weighty = 0.0;
 *  gb->c.anchor = csGridBagConstraint::SOUTHWEST;
 *  gb->c.gridwidth = csGridBagConstraint::REMAINDER; // last component in row
 *  gb->label = new csStatic (gb, 0, "Players");
 *  gb->label->SetSize (300, 20);
 *
 *  // add a listbox that shows the last few messages sent
 *  gb->c.gridwidth = 2;
 *  gb->c.fill = csGridBagConstraint::BOTH;
 *  gb->c.weightx = 1.0;
 *  gb->c.weighty = 0.0;
 *  gb->c.insets.xmin = 0;
 *  gb->c.ipady = 0;
 *  csListBox *hist = new csListBox (gb, CSLBS_VSCROLL);
 *  hist->SetSize (100, 100);
 *  hist->SetState (CSS_SELECTABLE, false);
 *
 *  gb->c.fill = csGridBagConstraint::BOTH;
 *  gb->c.weightx = 0.0;
 *  gb->c.weighty = 1.0;
 *  gb->c.gridwidth = csGridBagConstraint::REMAINDER;
 *  // add a listbox for the players in the arena
 *  csListBox *playerlist = new csListBox (gb);
 *  playerlist->SetSize (100, 100);
 *
 *   // add a editline for sending HOST - messages
 *  gb->c.fill = csGridBagConstraint::NONE;
 *  gb->c.gridx = csGridBagConstraint::RELATIVE;
 *  gb->c.gridwidth = 2;
 *  gb->c.gridheight = csGridBagConstraint::REMAINDER;;
 *  gb->c.weighty = 0.1;
 *  gb->c.weightx = 1.0;
 *  gb->c.anchor = csGridBagConstraint::NORTH;
 *  gb->c.insets.xmin = 20;
 *  csInputLine *msgline = new csInputLine (gb);
 *  msgline->SetSize (300, 20);
 *  msgline->SetText ("Test Test");
 *  msgline->SetSelection (0,999);
 *
 *  gb->c.fill = csGridBagConstraint::BOTH;
 *  gb->c.gridwidth = csGridBagConstraint::REMAINDER;
 *  gb->c.weighty = 1.0;
 *  csFlowLayout *flow = new csFlowLayout (gb);
 *  flow->SetSize (100, 100);
 *  CreateButton (flow, 7000, "Kick", 0, 0);
 *  CreateButton (flow, 7001, "Observe", 10, 0);
 *  CreateButton (flow, 7002, "Fly w/", 20, 0);
 *  CreateButton (flow, 7003, "Fool", 30, 0);
 *  CreateButton (flow, 7004, "Strip Weapons", 40, 0);
 *
 * ...
 * void CreateButton (csComponent *parent, int id, const char *text,
 *                     int xpos, int ypos)
 * {
 *   csButton *b= new csButton (parent, id);
 *   b->SetPos (xpos, ypos);
 *   b->SetSuggestedSize (0, 0);
 *   b->SetText (text);
 * }
 *
 * </pre>
 * </p>
 */
 
class CS_CRYSTALSPACE_EXPORT csGridBagLayout : public csLayout2
{
  struct CellInfo
  {
    csComponent* comp;

    int prefSize;	 // actually, it can be calculated on-the-fly
    int prefCompSize;

    int cellSpan;
    int leftInset;
    int rightInset;
    int pad;
    int fill;
    int anchor;

    float extraSpace;
    float weight;

    int finalSize;
    int finalPos;
    int finalCompSize;
    int finalCompPos;
  };

  struct CellHolder
  {
    csGridBagConstraint* constr;
    float weightx;
    float weighty;

    int gridwidth;
    int gridheight;
    int actualWidth;
    int actualHeight;

    bool isFirstCellForComp;
    int x, y;
  };

  typedef csPDelArray<CellHolder> CellHolderArrayT;

 public:
  csGridBagLayout (csComponent *pParent);
  ~csGridBagLayout ();

  virtual void RemoveLayoutComponent (csComponent* comp);
  virtual void SuggestSize (int &sugw, int &sugh);
  virtual void LayoutContainer ();

  virtual void MaximumLayoutSize (int &w, int &h);
  virtual float GetLayoutAlignmentX ();
  virtual float GetLayoutAlignmentY ();

  csGridBagConstraint c;

protected:
  int CalcPrefSize (CellInfo* cells, int xCnt, int yCnt, int _arrayWidth);
  void LayoutCells (CellInfo* cells, int xCnt, int yCnt,
		    int outterSize, int outterPos, int _arrayWidth );

  void InitializeCellArray (CellInfo* cells, int size);
  void InitCellFromHolder (CellHolder& holder);
  void CreateMatrix ();
  long GetCellCode (int x, int y);
  void CleanupConstraints ();
  void ClearCachedData ();
  bool HasCashedInfo ();
  void SetComponentLocations ();

protected:
  CellInfo* mpHorizCellInfos;
  CellInfo* mpVertCellInfos;
  int mColCount;
  int mRowCount;
};

/** @} */

#endif // __CS_CSGRIDBAGLAYOUT_H__
