/*
    Crystal Space Windowing System : grid class
    Copyright (C) 2000 by Norman Krämer <normank@lycosmail.com>
  
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

#ifndef __CSWSGRID_H__
#define __CSWSGRID_H__

#include "csws/csslider.h"
#include "csws/csscrbar.h"
#include "csutil/csvector.h"
#include "csutil/csstring.h"

/**
 * HereIsHowItGoesYa:
 *  We see the whole grid as a set of cells. Since it makes no sense to actually create an instance for every cell,
 *  we simply create an object that holds all properties for a rectangular region of cells that are all even.
 *  
 */

/**
 * Functions of this type can be used when traversing the tree. return true if you want to dive deeper into tree, 
 * false otherwise.
 */
typedef bool (*RegionTreeFunc)(csSome node, csSome databag);

class csRegionTree2D;
class csSparseGrid;
class csGridCell;
class csGridView;
class csGrid;

class csRegionTree2D
{
 public:
  csRect region;
  csRegionTree2D* children[5]; // max. 5 children possible
  csSome data;

 public:
  csRegionTree2D () { region.MakeEmpty (); data = NULL; memset( children, 0, 5*sizeof(csRegionTree2D*)); }
  csRegionTree2D (csRect area, csSome data) { 
    region.Set (area); this->data = data; memset( children, 0, 5*sizeof(csRegionTree2D*)); 
  }
  ~csRegionTree2D () { int i=0; while (i<5 && children[i]) delete children[i++]; }

  /**
   * Tiles this rect into the tree and creates new children if needed.
   */
  void Insert (csRect &area, csSome data);

  /**
   * Returns a list of leaves that do all contain parts of area.
   */
  void FindRegion (const csRect &area, csVector &vLeafList);

  /**
   * Traverse the tree and call user supplied function for every node.
   */
  void Traverse (RegionTreeFunc userFunc, csSome databag = NULL);

};

/**
 * If cells are populated with data or components, we need to store this somewhere and thats what the SparseGrid is for.
 */
class csSparseGrid
{
  friend class csGrid;
  struct GridRowEntry{
    GridRowEntry (int theCol, csSome theData){ col=theCol; data=theData; }
    int col;
    csSome data;
  };

  class csGridRow : public csVector{
    int col;
  public:
    csGridRow (int theCol);
    virtual ~csGridRow ();
    void SetAt (int col, csSome data);
    GridRowEntry* Get (int index);
    virtual int Compare (csSome Item1, csSome Item2, int Mode=0) const;
    virtual int CompareKey (csSome Item1, csConstSome Key, int Mode=0) const;
    virtual bool FreeItem (csSome Item);
  };
  friend class csSparseGrid::csGridRow;

  class csGridRowSet : public csGridRow{
  public:
    csGridRowSet (int theRow):csGridRow (theRow){}
    virtual bool FreeItem (csSome Item){ 
      delete (csGridRow*)((GridRowEntry*)Item)->data; delete (GridRowEntry*)Item; return true; 
    }
  };

  csGridRowSet rows;

 public:
  csSparseGrid ():rows(8){}

  csSome GetAt (int row, int col){
    csSome result=NULL;
    int idx1 = rows.FindSortedKey ((csConstSome)row);
    if (idx1 != -1){
      int idx2 = ((csGridRow*)rows.Get (idx1)->data)->FindSortedKey ((csConstSome)col);
      if (idx2 != -1)
	result = ((csGridRow*)rows.Get (idx1)->data)->Get (idx2)->data;
    }
    return result;
  }

  void SetAt (int row, int col, csSome data){
    int idx = rows.FindSortedKey ((csConstSome)row);
    if (idx == -1)
      idx = rows.InsertSorted ( new GridRowEntry (row, new csGridRow (row)));
    ((csGridRow*)rows.Get (idx)->data)->SetAt (col, data);
  }

};

// the following class collects properties for drawing the cell and acts as a container for the csComponent (i.e. the 
// thing that lives inside the cell )

enum csGridCellBorderStyle
{ 
  GCBS_NONE=-1,
  GCBS_DASH, 
  GCBS_DASHPOINT, 
  GCBS_DASHPOINTPOINT, 
  GCBE_DASHDASHPOINT,
  GCBS_LINE
};

class csGridCell : public csComponent
{
  class csCellBorder{
  public:
    csCellBorder (){ style = GCBS_LINE; thick = 1; }
    csGridCellBorderStyle style;
    int thick; // in pixel
  };
  
  bool inUse; // in use by at least one region
  void DrawLine (int x1, int y1, int x2, int y2, csCellBorder& border);
 public:
  csGridCell ():csComponent (NULL){ valuePattern = "%s";  SetPalette (CSPAL_GRIDCELL); inUse = false; }

  virtual void Draw ();

  bool IsUsed () { return inUse; }
  void SetUsed () { inUse = true; }

  csCellBorder upper, lower, left, right;
  int row, col; // row and column of cell - this is set in the drawloop of GridView
  csSome data; 
  csString valuePattern; // how content should be formated
};


/**
 * The GridView displays a continuous rectangular region of the grid.
 * It can be subdivided horizontally or vertically into two smaller views
 */

/**
 * GridView styles
 */

#define CSGVS_HSCROLL  0x00000004
/// View has a vertical scrollbar
#define CSGVS_VSCROLL  0x00000008
/// default value
#define CSGVS_DEFAULTVALUE (CSGVS_HSCROLL | CSGVS_VSCROLL)

class csGridView : public csComponent
{
  
 protected:
  csRect area; // the subcells this view  is restricted to
  csGrid *pGrid;
  int row, col; // row and column of the first visible cell (upper left)
  bool fPlaceItems;

  /**
   * Create a new copy of this view.
   */
  virtual csGridView* CreateCopy (int iStyle);
  void PlaceItems ();


 public:
  csGridView (csGrid *pParent, const csRect& region, int iStyle);
  csGridView (const csGridView& view, int iStyle=0);

  virtual void Draw ();
  virtual bool HandleEvent (iEvent& Event);
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);
  virtual void FixSize (int &newW, int &newH);
  virtual void SuggestSize (int &w, int &h);

  /**
   * Split this view along x and y. If values are not positive the split in that direction does not happen.
   */
  csGridView* SplitX (int x, int iStyle = 0);
  csGridView* SplitY (int y, int iStyle = 0);

  /**
   * Restrict the area that this view can display.
   */
  void SetViewArea (const csRect& rc){ area.Set (rc.xmin, rc.ymin, rc.xmax, rc.ymax); col=area.xmin; row=area.ymin; }

  int ViewStyle;
  // if view was split and this is the newly created view, then this value tells us what part of the old area
  // this one covers - needed for resizing viewhierachy
  float areafactor;
  csScrollBar *hscroll, *vscroll;
  csScrollBarStatus hsbstatus, vsbstatus;

};

/// Show a horizontal splitter handle
#define CSGS_HSPLIT   0x00000001
/// Splitter can be subdivided further (vertical)
#define CSGS_VSPLIT   0x00000002
/// Splitter has a horizontal scrollbar

class csGrid : public csComponent
{
  friend class csGridView;
 protected:
  csRegionTree2D *regions, *viewlayout;
  csSparseGrid *grid;
  csVector vViews;
  csVector vRegionStyles; // a vector containing the pattern csGridCell for every region;
  csSlider *sliderX, *sliderY;

  void CalcMinimalSize (csRegionTree2D *node, int &w, int &h);
  void PlaceItems ();

  void init (csComponent *pParent, csRect &rc, int iStyle, csGridCell *gc);

 public:
   // @@@ we can make cols and rows "unsigned int" but then we have to translate since csRect works with "int"
  csGrid (csComponent *pParent, int nRows, int nCols, int iStyle);
  csGrid (csComponent *pParent, int nRows, int nCols, int iStyle, csGridCell *gridpattern);
  ~csGrid ();

  virtual void Draw ();
  virtual bool SetRect (int xmin, int ymin, int xmax, int ymax);
  virtual void FixSize (int &newW, int &newH);
  virtual void SuggestSize (int &w, int &h);
  virtual bool HandleEvent (iEvent &Event);

  void CreateRegion (csRect& rc, csGridCell *cell);
  csGridView* GetRootView () { return (csGridView*)vViews.Get (0); }

  /**
   * Set string to display in specified cell
   */
  virtual void SetStringAt (int row, int col, const char *data);


};

#endif
