/*
    Crystal Space Windowing System : grid class
    Copyright (C) 2000 by Norman Kraemer <normank@lycosmail.com>

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

#ifndef __CS_CSGRID_H__
#define __CS_CSGRID_H__

/**\file
 * Crystal Space Windowing System: grid class
 */

/**
 * \addtogroup csws_comps_grid
 * @{ */
 
#include "csextern.h"
 
#include "csws/csscrbar.h"
#include "csutil/array.h"
#include "csutil/parray.h"
#include "csutil/csstring.h"

/**
 * \page HereIsHowItGoesYa:
 *  We see the whole grid as a set of cells. Since it makes no sense to
 *  actually create an instance for every cell, we simply create an object
 *  that holds all properties for a rectangular region of cells that are all
 *  even.
 */

/**
 * Functions of this type can be used when traversing the tree.
 * return true if you want to dive deeper into tree, false otherwise.
 */
typedef bool (*csRegionTreeFunc) (void* node, void* databag);

class csRegionTree2D;
class csSparseGrid;
class csGridCell;
class csGridView;
class csGrid;
class csSplitter;

class CS_CRYSTALSPACE_EXPORT csRegionTree2D
{
public:
  csRect region;
  csRegionTree2D *children[5]; // max. 5 children possible
  void* data;

public:
  /// Create an empty region
  csRegionTree2D ();
  /// Create a non-empty region with given associated data
  csRegionTree2D (csRect area, void* data);
  /// Finish this region object
  ~csRegionTree2D ();

  /**
   * Tiles this rect into the tree and creates new children if needed.
   */
  void Insert (csRect &area, void* data);

  /**
   * Returns a list of leaves that do all contain parts of area.
   */
  void FindRegion (const csRect &area, csArray<csRegionTree2D*> &vLeafList);

  /**
   * Traverse the tree and call user supplied function for every node.
   */
  void Traverse (csRegionTreeFunc userFunc, void* databag = 0);

};

/**
 * If cells are populated with data or components,
 * we need to store this somewhere and thats what the SparseGrid is for.
 */
class csSparseGrid
{
  friend class csGrid;
  /*
   * A single entry in the "grid row" array.
   */
  struct csGridRowEntry
  {
    int col;
    void* data;
    // Initialize the object with given column and associated data
    csGridRowEntry (int theCol, void* theData) : col (theCol), data (theData) {}
  };

  /*
   * A "grid row" is a horizontal stripe of cells which makes up the
   * entire grid. Every data item in this array is a csGridRowEntry.
   * The grid row object does not contain all the cells as separate objects;
   * this would waste too much memory. Instead, we keep only those cell
   * objects which have associated data items. The cells are kept sorted
   * by column number for faster searching.
   */
  class csGridRow : public csPDelArray<csGridRowEntry>
  {
    int col;
  public:
    // Initialize the object
    csGridRow (int theCol);
    // Set the data at given column
    void SetAt (int col, void* data);
    // Compare two row entries
    static int Compare (csGridRowEntry* const&, csGridRowEntry* const&);
    // Compare a row entry with a key
    static int CompareKey (csGridRowEntry* const&, int const& Key);
    // Functor wrapping CompareKey() for a given number.
    static csArrayCmp<csGridRowEntry*,int> KeyCmp(int n)
    { return csArrayCmp<csGridRowEntry*,int>(n, CompareKey); }
  };
  friend class csSparseGrid::csGridRow;

  /*
   * A "grid row set" is an array of "grid rows",
   * e.g. this is the grid itself.
   */
  class csGridRowSet : public csGridRow
  {
  public:
    // Initialize the grid row set object
    csGridRowSet (int theRow) : csGridRow (theRow) {}
  };

  // The Grid (AKA The Matrix :)
  csGridRowSet rows;

public:
  /// Initialize an empty sparse grid object
  csSparseGrid () : rows (8) {}

  /// Get the data at given row/column
  void* GetAt (int row, int col)
  {
    void* result = 0;
    size_t idx1 = rows.FindSortedKey (rows.KeyCmp(row));
    if (idx1 != (size_t)-1)
    {
      size_t idx2 = ((csGridRow *)rows.Get (idx1)->data)->FindSortedKey (
      	rows.KeyCmp(col));
      if (idx2 != (size_t)-1)
	result = ((csGridRow *)rows.Get (idx1)->data)->Get (idx2)->data;
    }
    return result;
  }

  // Set the data at given row/column
  void SetAt (int row, int col, void* data)
  {
    size_t idx = rows.FindSortedKey (rows.KeyCmp(row));
    if (idx == (size_t)-1)
      idx = rows.InsertSorted (new csGridRowEntry (row, new csGridRow (row)),
      	rows.Compare);
    ((csGridRow *)rows.Get (idx)->data)->SetAt (col, data);
  }
};

/**
 * The possible border styles for grid cells.
 */
enum csGridCellBorderStyle
{
  /// No border (          )
  gcbsNone = 0,
  /// Dashed border (- - - - -)
  gcbsDash,
  /// Interleaved dash-and-point border (- * - * - * -)
  gcbsDashPoint,
  /// Dash-point-point border (- * * - * * -)
  gcbsDashPointPoint,
  /// Dash-dash-point border (- - * - - * - -)
  gcbsDashDashPoint,
  /// Solid line border (----------)
  gcbsLine
};

/// Additional state flag used to mark selected cell
#define CSS_GRIDCELL_SELECTED        0x00010000

/**
 * The following class collects properties for drawing the cell
 * and acts as a container for the csComponent (i.e. the
 * thing that lives inside the cell)
 */
class CS_CRYSTALSPACE_EXPORT csGridCell : public csComponent
{
  /// The property of a specific border of the cell
  class csCellBorder
  {
  public:
    /// Grid cell border style (see above)
    csGridCellBorderStyle style;
    /// Border thickness (in pixels)
    int thick;
    /// Create a default cell border object
    csCellBorder () : style (gcbsLine), thick (1) {}
  };

  /// True if in use by at least one region
  bool inUse;

public:
  /// The styles for upper, lower, left and right borders
  csCellBorder upper, lower, left, right;
  /// The row and column for this cell (set before calling Draw() by grid)
  int row, col;
  /// Data associated with this cell
  void* data;
  /// how content should be formated
  csString valuePattern;

  /// Create a grid cell with default parameters
  csGridCell ();
  /// Draw the cell
  virtual void Draw ();
  /// Query if this cell is used
  bool IsUsed () { return inUse; }
  /// Set the in-use flag for this cell
  void SetUsed (bool iState = true) { inUse = iState; }

protected:
  /// Draw a line with given border style
  void DrawLine (int x1, int y1, int x2, int y2, csCellBorder &border);
};


/**
 * \name GridView styles
 * @{ */
/// View has a horizontal scrollbar
#define CSGVS_HSCROLL  0x00000001
/// View has a vertical scrollbar
#define CSGVS_VSCROLL  0x00000002
/// default value
#define CSGVS_DEFAULTVALUE (CSGVS_HSCROLL | CSGVS_VSCROLL)
/** @} */

/**
 * The GridView displays a continuous rectangular region of the grid.
 * It can be subdivided horizontally or vertically into two smaller views,
 * and can contain a horizontal and a vertical scrollbar for scrolling
 * the contents of the grid.
 */
class CS_CRYSTALSPACE_EXPORT csGridView : public csComponent
{
protected:
  /// The subcells this view is restricted to
  csRect area;
  /// The parent grid object
  csGrid *pGrid;
  /// Row and column of the first visible cell (upper left)
  int row, col;
  /// True if items should be re-positioned (due to a new item inserted etc)
  bool fPlaceItems;
  /// The style of this view
  int Style;
  /// The horizontal and vertical scrollbar objects
  csScrollBar *hscroll, *vscroll;

  /// get the row and column at the pixel (theX, theY)
  void CooAt (int theX, int theY, int &theRow, int &theCol);

public:
  /**
   * if view was split and this is the newly created view,
   * then this value tells us what part of the old area
   * this one covers - needed for resizing view hierachy
   */
  float areafactor;

  /// Create a grid view covering given subregion of the grid
  csGridView (csGrid *pParent, const csRect &region,
    int iStyle = CSGVS_DEFAULTVALUE);
  /// Create a copy of given grid view, and copy its style (or use a new style)
  csGridView (const csGridView &view, int iStyle = -1);

  /// Draw the grid view
  virtual void Draw ();
  /// Handle a event
  virtual bool HandleEvent (iEvent& Event);
  /// Set grid view position and size
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);
  /// return this views area
  const csRect& GetArea (){return area;}
  /// Snap size to nearest grid cell
  virtual void FixSize (int &newW, int &newH);
  /// Suggest the optimal size for this grid view
  virtual void SuggestSize (int &w, int &h);

  /**
   * Create a new grid view by splitting this view along X axis.
   * If values are not positive the split in that direction does not happen.
   */
  csGridView *SplitX (int x, int iStyle = -1);
  /**
   * Create a new grid view by splitting this view along Y axis.
   * If values are not positive the split in that direction does not happen.
   */
  csGridView *SplitY (int y, int iStyle = -1);

  /**
   * Restrict the area that this view can display.
   */
  void SetViewArea (const csRect& rc)
  {
    area.Set (rc.xmin, rc.ymin, rc.xmax, rc.ymax);
    col = area.xmin; row = area.ymin;
  }

protected:
  /**
   * Create a new copy of this view.
   */
  virtual csGridView *CreateCopy (int iStyle);
  /**
   * Place all the items in their places.
   */
  void PlaceItems ();
};

/**
 * \name Grid style flags
 * The csGrid object accepts both CSGS_XXX and CSGVS_XXX styles;
 * the CSGVS_XXX styles are passed to newly-created grid view
 * components.
 * @{ */

/// Show a horizontal splitter handle
#define CSGS_HSPLIT		0x00000004
/// Splitter can be subdivided further (vertical)
#define CSGS_VSPLIT		0x00000008
/// Default grid style
#define CSGS_DEFAULTVALUE	(CSGS_HSPLIT | CSGS_VSPLIT)

/// no cursor
#define CSGCS_NONE   1
/// cell cursor
#define CSGCS_CELL   2
/// row cursor
#define CSGCS_ROW    3
/// column cursor
#define CSGCS_COLUMN 4
/** @} */

/// Grid messages
enum
{
  /**
   * This message is sent to the grids parent to notify whenever the cursor
   * changes (moves)
   */
  cscmdGridCursorChanged = 0x00000F00
};

/**
 * This is the grid object itself.
 * The grid object can contain a number of vertically and
 * horizontally split subviews (called "grid views"), each
 * subview may be limited to certain area within the grid
 * itself.
 */
class CS_CRYSTALSPACE_EXPORT csGrid : public csComponent
{
protected:
  friend class csGridView;
  ///
  csRegionTree2D *regions, *viewlayout;
  /// The grid data
  csSparseGrid *grid;
  /// The array of grid views
  csArray<csGridView*> vViews;
  /// The actiove grid view
  csGridView *activeView;
  /// A vector containing the pattern csGridCell for every region;
  csArray<csGridCell*> vRegionStyles;
  /// The horizontal and vertical dividers
  csSplitter *splitterX, *splitterY;
  /// cursor style
  int cursorStyle;
  /// cursor position
  int xcur, ycur;

  /// Calculate minimal size needed for given region
  void CalcMinimalSize (csRegionTree2D *node, int &w, int &h);
  /// Place the dividers when the grid size changes
  void PlaceGadgets ();

private:
  /// Common method for constructors
  void init (csComponent *pParent, csRect &rc, int iStyle, csGridCell *gc);

public:
  /// Create a grid with given number of rows & columns
  csGrid (csComponent *pParent, int nRows, int nCols,
    int iStyle = CSGS_DEFAULTVALUE | CSGVS_DEFAULTVALUE);
  ///
  csGrid (csComponent *pParent, int nRows, int nCols, csGridCell *gridpattern,
   int iStyle = CSGS_DEFAULTVALUE | CSGVS_DEFAULTVALUE);
  /// Destroy the grid object
  virtual ~csGrid ();

  /// Set a cursor style
  virtual void SetCursorStyle (int iCursorStyle = CSGCS_NONE);
  /// Get cursor style
  virtual int GetCursorStyle ();
  /// Get cursor position
  virtual void GetCursorPos (int &row, int &col);
  /// Set cursor position
  virtual void SetCursorPos (int row, int col);

  /// Draw the grid
  virtual void Draw ();
  /// Set grid size and position
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);
  /// Do not allow to resize us less than needed by scrollbars
  virtual void FixSize (int &newW, int &newH);
  /// Suggest the optimal size for the grid
  virtual void SuggestSize (int &w, int &h);
  /// Handle events
  virtual bool HandleEvent (iEvent &Event);

  /// Create a grid region
  void CreateRegion (csRect& rc, csGridCell *cell);
  /// Get the first grid view object
  csGridView* GetRootView ()
  { return (csGridView*)vViews.Get (0); }
  /// Get the active grid view
  csGridView *GetActiveView () {return activeView;}
  /// Set the active grid view
  void SetActiveView (csGridView *view);

  /**
   * Set string to display in specified cell
   */
  virtual void SetStringAt (int row, int col, const char *data);
  csString *GetStringAt (int row, int col);
};

/** @} */

#endif // __CS_CSGRID_H__
