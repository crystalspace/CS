/*
    Crystal Space Windowing System : grid class
    Copyright (C) 2000 by Norman Kramer <normank@lycosmail.com>

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

#include "cssysdef.h"
#include "csws/csapp.h"
#include "csws/cswindow.h"
#include "csws/csgrid.h"
#include "csws/cssplit.h"
#include "csutil/event.h"

// The minimal width of the new grid view when we split a view horiz. or vert.
#define MIN_GRIDVIEW_SIZE	8
#define GRIDVIEW_BORDER_SIZE    2

/******************************************************************************
 * csRegionTree2D
 ******************************************************************************/

csRegionTree2D::csRegionTree2D ()
{
  region.MakeEmpty ();
  data = 0;
  memset (children, 0, 5 * sizeof (csRegionTree2D *));
}

csRegionTree2D::csRegionTree2D (csRect area, void* data)
{
  region.Set (area);
  csRegionTree2D::data = data;
  memset (children, 0, 5 * sizeof (csRegionTree2D *));
}

csRegionTree2D::~csRegionTree2D ()
{
  int i = 0;
  while (i < 5 && children [i])
    delete children [i++];
}

/**
 * Tiles this rect into the tree and creates new children if needed.
 */
void csRegionTree2D::Insert (csRect &area, void* data)
{
  if (children [0])
  {
    int i = 0;
    while (i < 5 && children [i])
    {
      csRect common (area);
      common.Intersect (children [i]->region);
      if (!common.IsEmpty ())
        children [i]->Insert (common, data);
      i++;
    }
  }
  else
  {
    // leaf
    if (region.Intersects (area))
    {
      // maybe this regions equals the area,
      // then we simply replace the data and are done
      if (region.Equal (area.xmin, area.ymin, area.xmax, area.ymax))
        this->data = data;
      else
      {
        int i = 0;
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // DO NOT CHANGE THE SEQUENCE OF THE FOLLOWING, IT ENSURES FindRegion RETURNS AREAS ORDERED LEFT TO RIGHT
        // FOR SINGLE ROW REGIONS (likewise TOP TO DOWN)
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // does an upper stripe exist ?
        if (region.ymin < area.ymin)
        {
          csRect rc (region); rc.ymax = area.ymin;
          children [i++] = new csRegionTree2D (rc, this->data);
        }
        // does a left stripe exist ?
        if (region.xmin < area.xmin)
        {
          csRect rc (region.xmin, area.ymin, area.xmin, area.ymax);
          children [i++] = new csRegionTree2D (rc, this->data);
        }
        // the region which fully covers area
        children [i++] = new csRegionTree2D (area, data);
        // does a right stripe exist ?
        if (region.xmax > area.xmax)
        {
          csRect rc (area.xmax, area.ymin, region.xmax, area.ymax);
          children [i++] = new csRegionTree2D (rc, this->data);
        }
        // does a lower stripe exist ?
        if (region.ymax > area.ymax)
        {
          csRect rc (region); rc.ymin = area.ymax;
          children [i++] = new csRegionTree2D (rc, this->data);
        }
        // now this leaf became a simple node
      }
    }
  }
}

/**
 * Returns a list of leaves that do all contain parts of area.
 */
void csRegionTree2D::FindRegion (const csRect &area,
	csArray<csRegionTree2D*> &vLeafList)
{
  if (children [0])
  {
    int i = 0;
    while (i < 5 && children [i])
      children [i++]->FindRegion (area, vLeafList);
  }
  else if (region.Intersects (area))
    vLeafList.Push (this);
}

/**
 * Traverse the tree and call user supplied function for every node.
 */
void csRegionTree2D::Traverse (csRegionTreeFunc userFunc, void* databag)
{
  if (userFunc (this, databag))
  {
    int i = 0;
    while (i < 5 && children [i])
      children [i++]->Traverse (userFunc, databag);
  }
}

/******************************************************************************
 * csSparseGrid::csGridRow
 ******************************************************************************/

csSparseGrid::csGridRow::csGridRow (int theCol)
{
  col = theCol;
}

void csSparseGrid::csGridRow::SetAt (int col, void* data)
{
  size_t key = FindSortedKey (KeyCmp(col));
  if (key == (size_t)-1 && data)
    key = InsertSorted (new csGridRowEntry(col, data), Compare);
  else
    if (data)
      Get (key)->data = data;
    else
      DeleteIndex (key);
}

int csSparseGrid::csGridRow::Compare (csGridRowEntry* const& Item1,
				      csGridRowEntry* const& Item2)
{
  return (Item1->col < Item2->col ? -1 : Item1->col > Item2->col ? 1 : 0);
}

int csSparseGrid::csGridRow::CompareKey (csGridRowEntry* const& Item1,
					 int const& Key)
{
  return (Item1->col < Key ? -1 : Item1->col > Key ? 1 : 0);
}

/******************************************************************************
 * csGridCell
 ******************************************************************************/

csGridCell::csGridCell () : csComponent (0), inUse (false)
{
  state |= CSS_SELECTABLE;
  valuePattern = "%s";
  SetPalette (CSPAL_GRIDCELL);
}

void csGridCell::DrawLine (int x1, int y1, int x2, int y2, csCellBorder& border)
{
  bool sel = GetState (CSS_GRIDCELL_SELECTED) ? true : false;

  if (border.style == gcbsLine)
    Box (MIN (x1, x2), y1, MAX (x1, x2), y2,
	 sel ? CSPAL_GRIDCELL_SEL_BORDER_FG : CSPAL_GRIDCELL_BORDER_FG);
  else if (border.style != gcbsNone)
  {
    int maxX, maxY, i = 0, nSegs, xcompo, ycompo;
    static const int linepattern [][13] =
    {
      { 2, 4, 0, 2, 0 },                         // DASH
      { 4, 4, 0, 2, 0, 2, 0, 2, 0 },             // DASHPOINT
      { 6, 4, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0 }, // DASHPOINTPOINT
      { 6, 4, 0, 2, 0, 4, 0, 2, 0, 2, 0, 2, 0 }  // DASHDASHPOINT
    };
    if (x1 <= x2)
      xcompo = 0, ycompo = 1;
    else
      xcompo = 1, ycompo = 0;
    maxX = MAX (x1, x2); maxY = MAX (y1, y2);
    x1 = MIN (x1, x2); x2 = MAX (x1, x2);
    // linesegments in linepattern
    nSegs = linepattern [int (border.style) - 1][0];
    int colFG = sel ? CSPAL_GRIDCELL_SEL_BORDER_FG : CSPAL_GRIDCELL_BORDER_FG;
    int colBG = sel ? CSPAL_GRIDCELL_SEL_BORDER_BG : CSPAL_GRIDCELL_BORDER_BG;
    while (x1 < maxX && y1 < maxY)
    {
      i = i % nSegs;
      x2 = x1 + linepattern [int (border.style) - 1][1 + 2 * i + xcompo];
      y2 = y1 + linepattern [int (border.style) - 1][1 + 2 * i + ycompo];
      Box (x1, y1, MIN (x2, maxX), MIN (y2, maxY),
        (i & 1 ? colBG : colFG));
      //printf("%d,%d -> %d,%d = %d\n", x1, y1, x2, y2,(i&1 ? 0 : 1));
      x1 = x2; y1 = y2;
      i++;
    }
  }
}

void csGridCell::Draw ()
{
  int lx = 0, rx = 0, ty = 0, by = 0; // offsets if borders are drawn;
  bool sel = GetState (CSS_GRIDCELL_SELECTED) ? true : false;

  if (upper.style != gcbsNone)
  {
    ty = upper.thick;
    DrawLine (0, 0, bound.Width () -
      (right.style == gcbsNone ? 0 : right.thick), upper.thick, upper);
  }
  if (right.style != gcbsNone)
  {
    rx = right.thick;
    DrawLine (bound.Width (), 0, bound.Width () - right.thick,
      bound.Height () - (lower.style == gcbsNone ? 0 : lower.thick), right);
  }
  if (lower.style != gcbsNone)
  {
    by = lower.thick;
    DrawLine (0 + (left.style == gcbsNone ? 0 : left.thick),
      bound.Height () - lower.thick, bound.Width (), bound.Height (), lower);
  }
  if (left.style != gcbsNone)
  {
    lx = left.thick;
    DrawLine (left.thick, upper.style == gcbsNone ? 0 : upper.thick, 0,
      bound.Height (), left);
  }
  // fill the canvas with bgcolor
  bound.xmin += lx;
  bound.ymin += ty;
  bound.xmax -= rx;
  bound.ymax -= by;

  Box (0, 0, bound.Width (), bound.Height (),
       sel ? CSPAL_GRIDCELL_SEL_BACKGROUND: CSPAL_GRIDCELL_BACKGROUND);
  if (data)
  {
    const char *t = ((csString*)data)->GetData ();
    if (t)
    {
      int fh, fw = MIN (bound.Width (), GetTextSize (t, &fh));
      fh = MIN (bound.Height (), fh);
      int tx = (bound.Width () - fw) / 2;
      int ty = (bound.Height () - fh) / 2;
      Text (tx, ty, sel ? CSPAL_GRIDCELL_SEL_DATA_FG : CSPAL_GRIDCELL_DATA_FG,
	    sel ? CSPAL_GRIDCELL_SEL_DATA_BG : CSPAL_GRIDCELL_DATA_BG, t);
    }
  }
  bound.xmin -= lx;
  bound.ymin -= ty;
  bound.xmax += rx;
  bound.ymax += by;
}

/******************************************************************************
 * csGridView
 ******************************************************************************/

csGridView::csGridView (csGrid *pParent, const csRect& region, int iStyle)
  : csComponent (pParent)
{
  pGrid = pParent;
  area.Set (region);
  Style = iStyle;

  if (Style & CSGVS_HSCROLL)
    hscroll = new csScrollBar (this, cssfsThinRect);
  else
    hscroll = 0;

  if (Style & CSGVS_VSCROLL)
    vscroll = new csScrollBar (this, cssfsThinRect);
  else
    vscroll = 0;
  SetPalette (CSPAL_GRIDVIEW);
  col = area.xmin;
  row = area.ymin;
}

csGridView::csGridView (const csGridView& view, int iStyle)
  : csComponent (view.pGrid)
{
  pGrid = view.pGrid;
  area.Set (view.area);
  Style = ((iStyle != -1) ? iStyle : view.Style);

  if (Style & CSGVS_HSCROLL)
    hscroll = new csScrollBar (this, cssfsThinRect);
  else
    hscroll = 0;

  if (Style & CSGVS_VSCROLL)
    vscroll = new csScrollBar (this, cssfsThinRect);
  else
    vscroll = 0;
  SetPalette (view.palette, view.palettesize);
  row = view.row;
  col = view.col;
}

bool csGridView::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csComponent::SetRect (xmin, ymin, xmax, ymax))
  {
    if (hscroll)
      hscroll->SetRect (0,
			bound.Height () - CSSB_DEFAULTSIZE,
			bound.Width () - (vscroll ? CSSB_DEFAULTSIZE : 0),
			bound.Height ());
    if (vscroll)
      vscroll->SetRect (bound.Width () - CSSB_DEFAULTSIZE,
			0,
			bound.Width (),
			bound.Height () - (hscroll ? CSSB_DEFAULTSIZE : 0));
    fPlaceItems = true;
    return true;
  }
  return false;
}

void csGridView::PlaceItems ()
{
  fPlaceItems = false;

  // count the number of cells visible in the first row
  // (exact would be the minimum of cells in a row in the visible area)
  csArray<csRegionTree2D*> vRegionList;
  csRect rc;
  size_t i = 0;
  int w1 = 0, w2 = 0;
  int nRowCells = 0, nColCells = 0;
  csRegionTree2D *r;

  if (hscroll)
  {
    rc.Set (col, row, area.xmax, row + 1);
    pGrid->regions->FindRegion (rc, vRegionList);

    while (i < vRegionList.Length() && w1 < bound.Width ())
    {
      r = (csRegionTree2D *)vRegionList.Get (i);
      w2 = (r->region.Width () - MAX (col - r->region.xmin, 0)) *
        ((csGridCell *)r->data)->bound.Width (); // #Cells * CellLength
      if (w1 + w2 < bound.Width ())
      {
	nRowCells += (r->region.Width () - MAX (col - r->region.xmin, 0));
	w1 += w2;
      }
      else
      {
        nRowCells += (bound.Width () - w1) /
          ((csGridCell *)r->data)->bound.Width ();
	w1 = bound.Width ();
      }
      i++;
    }

    csScrollBarStatus hsbstatus;
    hsbstatus.value = col - area.xmin;
    hsbstatus.step = 1;
    hsbstatus.maxsize = area.Width ();
    hsbstatus.maxvalue = hsbstatus.maxsize - nRowCells;
    hsbstatus.size =
    hsbstatus.pagestep = MAX (nRowCells, 1);
    hscroll->SendCommand (cscmdScrollBarSet, (intptr_t)&hsbstatus);

    vRegionList.DeleteAll ();
    i = 0; w1 = 0; w2 = 0;
  }

  if (vscroll)
  {
    // count numbers of cells in first column
    // (exact would be the minimum of cells in a column in the visible area)
    rc.Set (col, row, col + 1, area.ymax);
    pGrid->regions->FindRegion (rc, vRegionList);

    while (i < vRegionList.Length () && w1 < bound.Height ())
    {
      r = (csRegionTree2D*)vRegionList.Get (i);
      // #Cells * CellHeight
      w2 = (r->region.Height ()-MAX (row - r->region.ymin, 0)) *
        ((csGridCell *)r->data)->bound.Height ();
      if (w1 + w2 < bound.Height ())
      {
        nColCells += (r->region.Height () - MAX (row - r->region.ymin, 0));
        w1 += w2;
      }
      else
      {
        nColCells += (bound.Height () - w1) /
          ((csGridCell *)r->data)->bound.Height ();
        w1 = bound.Height ();
      }
      i++;
    }

    csScrollBarStatus vsbstatus;
    vsbstatus.value = row - area.ymin;
    vsbstatus.step = 1;
    vsbstatus.maxsize = area.Height ();
    vsbstatus.maxvalue = vsbstatus.maxsize - nColCells;
    vsbstatus.size =
    vsbstatus.pagestep = MAX (nColCells, 1);
    vscroll->SendCommand (cscmdScrollBarSet, (intptr_t)&vsbstatus);
  }
}

static bool DrawCellComponents (csComponent *child, intptr_t param)
{
  (void)param;
  child->Draw ();
  return false;
}

void csGridView::CooAt (int theX, int theY, int &theRow, int &theCol)
{
  int y = 0, x, c;
  int actRow = row;
  int actCol = col;
  csRect rc;
  csRegionTree2D *r;
  csArray<csRegionTree2D*> vRegions;
  csGridCell *cell = 0;

  theCol = area.xmin -1;
  theRow = area.ymin -1;

  rc.Set (actCol, actRow, actCol+1, area.ymax);
  vRegions.DeleteAll ();
  pGrid->regions->FindRegion (rc, vRegions);
  size_t n = 0;
  c = actRow;
  while (y < bound.Height () && y < theY && n < vRegions.Length ())
  {
    r = (csRegionTree2D*)vRegions.Get (n++);
    for (; c < r->region.ymax && c < area.ymax && y < theY; c++)
    {
      cell = (csGridCell *)r->data;
      y += cell->bound.Height ();
      actRow++;
    }
  }

  if (y >= theY)
  {
    actRow--;
    rc.Set (actCol, actRow, area.xmax, actRow+1);
    vRegions.DeleteAll ();
    pGrid->regions->FindRegion (rc, vRegions);
    x = 0;
    n = 0;
    c = actCol;
    while (x < bound.Width () && x < theX && n < vRegions.Length ())
    {
      r = (csRegionTree2D*)vRegions.Get (n++);
      for (; c < r->region.xmax && x < theX && c < area.xmax; c++)
      {
	cell = (csGridCell *)r->data;
	x += cell->bound.Width ();
	actCol++;
      }
    }

    if (x>=theX)
    {
      actCol--;
      theRow = actRow;
      theCol = actCol;
    }
  }
}

void csGridView::Draw ()
{
  if (fPlaceItems)
    PlaceItems ();

  int y = GRIDVIEW_BORDER_SIZE, x;
  size_t n;
  int c, actRow = row;
  csRect rc;
  csRegionTree2D *r;
  csArray<csRegionTree2D*> vRegions;
  csGridCell *cell = 0;
  int cs = pGrid->GetCursorStyle ();
  int cr, cc;
  bool sel;

  pGrid->GetCursorPos (cr, cc);

  while (y < bound.Height ()-GRIDVIEW_BORDER_SIZE && actRow < area.ymax)
  {
    rc.Set (col, actRow, area.xmax, actRow + 1);
    vRegions.DeleteAll ();
    pGrid->regions->FindRegion (rc, vRegions);
    if (vRegions.Length () == 0)
      break; // no more rows to draw
    x = GRIDVIEW_BORDER_SIZE; n = 0; c = col;
    while (x < bound.Width ()-GRIDVIEW_BORDER_SIZE && n < vRegions.Length () && c < area.xmax)
    {
      r = (csRegionTree2D*)vRegions.Get (n++);
      cell = (csGridCell *)r->data;
      Insert (cell); cell->Show (false); // show but don't focus
      for (; c < r->region.xmax && x < bound.Width () && c < area.xmax; c++)
      {
        cell->SetPos (x, y);
        cell->row = actRow;
        cell->col = c;
        cell->data = pGrid->grid->GetAt (actRow, c);
	// if a grid cursor is to be shown, then
	// we will toggle the selected state of the component
	sel = (cs == CSGCS_CELL && c == cc && actRow == cr) ||
	  (cs == CSGCS_ROW && actRow == cr) ||
	  (cs == CSGCS_COLUMN && c == cc);
	cell->SetState (CSS_GRIDCELL_SELECTED, sel);
        cell->Draw ();
        cell->ForEach (DrawCellComponents, 0, true);
        x += cell->bound.Width ();
      }
      Delete (cell);
    }
    y += cell->bound.Height ();
    actRow++;
  }

  csComponent::Draw ();
  sel = pGrid->GetActiveView () == this;
  Box (0, 0, bound.Width (), GRIDVIEW_BORDER_SIZE,
       sel ? CSPAL_GRIDVIEW_SEL_DARK3D : CSPAL_GRIDVIEW_DARK3D);
  Box (0, bound.Height ()- GRIDVIEW_BORDER_SIZE, bound.Width (), bound.Height (),
       sel ? CSPAL_GRIDVIEW_SEL_DARK3D : CSPAL_GRIDVIEW_DARK3D);
  Box (0, 0, GRIDVIEW_BORDER_SIZE, bound.Height (),
       sel ? CSPAL_GRIDVIEW_SEL_DARK3D : CSPAL_GRIDVIEW_DARK3D);
  Box (bound.Width () - GRIDVIEW_BORDER_SIZE, 0, bound.Width (), bound.Height (),
       sel ? CSPAL_GRIDVIEW_SEL_DARK3D : CSPAL_GRIDVIEW_DARK3D);

  // fill the remainingspace with backcolor
  //  Box (0, y, bound.Width (), bound.Height (), CSPAL_GRIDVIEW_BACKGROUND);
}

bool csGridView::HandleEvent (iEvent& Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdReceiveFocus:
        case cscmdLoseFocus:
	  Invalidate (true);
	  break;
        case cscmdScrollBarValueChanged:
        {
          csScrollBar *bar = (csScrollBar*)Event.Command.Info;
          csScrollBarStatus sbs;
          if (!bar || bar->SendCommand (cscmdScrollBarGetStatus, (intptr_t)&sbs))
            return true;
          if (sbs.maxvalue <= 0)
            return true;
          if (bar == hscroll)
          {
            if (col-area.xmin != sbs.value)
            {
              col = area.xmin + sbs.value;
              PlaceItems ();
              Invalidate (true);
            }
          }
          else if (bar == vscroll)
          {
            if (row-area.ymin != sbs.value)
            {
              row = area.ymin + sbs.value;
              PlaceItems ();
              Invalidate (true);
            }
          }
          return true;
        }
      }
      break;
  case csevMouseClick:
    if (Event.Mouse.Button == 1)
    {
      bool succ = csComponent::HandleEvent (Event);
      if (!succ) // if no scrollbar handle was pressed
      {
	pGrid->SetActiveView (this);
	if (pGrid->GetCursorStyle () != CSGCS_NONE)
	{
	  int atrow, atcol;
	  CooAt (Event.Mouse.x, Event.Mouse.y, atrow, atcol);
	  pGrid->SetCursorPos (atrow, atcol);
	}
	return true;
      }
      return succ;
    }
    break;
  }
  return csComponent::HandleEvent (Event);
}

void csGridView::FixSize (int &newW, int &newH)
{
  if (hscroll && newH < hscroll->bound.Height ())
    newH = hscroll->bound.Height ();
  if (vscroll && newW < vscroll->bound.Width ())
    newW = vscroll->bound.Width ();
}

void csGridView::SuggestSize (int &w, int &h)
{
  w = h = 0;
  if (hscroll) { h += CSSB_DEFAULTSIZE; }
  if (vscroll) { w += CSSB_DEFAULTSIZE; }
}

csGridView *csGridView::CreateCopy (int iStyle)
{
  return new csGridView (*this, iStyle);
}

csGridView *csGridView::SplitX (int x, int iStyle)
{
  csGridView *sp = 0;
  if (x > MIN_GRIDVIEW_SIZE && x < bound.Width () - MIN_GRIDVIEW_SIZE)
  {
    sp = CreateCopy (iStyle);
    if (sp)
    {
      sp->areafactor = (float)x / (float)bound.Width ();
      pGrid->vViews.Push (sp);
      sp->SetRect (bound.xmin, bound.ymin, bound.xmin + x, bound.ymax);
      SetRect (bound.xmin + x, bound.ymin, bound.xmax, bound.ymax);
      pGrid->viewlayout->Insert (sp->bound, sp);
    }
  }
  return sp;
}

csGridView *csGridView::SplitY (int y, int iStyle)
{
  csGridView *sp = 0;
  if (y > MIN_GRIDVIEW_SIZE && y < bound.Height () - MIN_GRIDVIEW_SIZE)
  {
    sp = CreateCopy (iStyle);
    if (sp)
    {
      sp->areafactor = (float)y / (float)bound.Height ();
      pGrid->vViews.Push (sp);
      sp->SetRect (bound.xmin, bound.ymin, bound.xmax, bound.ymin + y);
      SetRect (bound.xmin, bound.ymin+y, bound.xmax, bound.ymax);
      pGrid->viewlayout->Insert (sp->bound, sp);
    }
  }
  return sp;
}

/******************************************************************************
 * csGrid
 ******************************************************************************/

csGrid::csGrid (csComponent *pParent, int nRows, int nCols, int iStyle)
  : csComponent (pParent)
{
  csRect rc (0, 0, nCols, nRows);
  csGridCell *gc = new csGridCell;
  gc->SetRect (0, 0, 50, 30);
  init (pParent, rc, iStyle, gc);
}

csGrid::csGrid (csComponent *pParent, int nRows, int nCols,
  csGridCell *gridpattern, int iStyle) : csComponent (pParent)
{
  csRect rc (0, 0, nCols, nRows);
  init (pParent, rc, iStyle, gridpattern);
}

void csGrid::init (csComponent *pParent, csRect &rc, int iStyle, csGridCell *gc)
{
  grid = new csSparseGrid;
  SetCursorStyle (CSGCS_NONE);
  SetCursorPos (0, 0);

  SetPalette (CSPAL_GRIDVIEW);
  SetState (CSS_SELECTABLE, true);
  vRegionStyles.Push (gc);
  vViews.Push (new csGridView (this, rc, (iStyle & ~(CSGS_HSPLIT|CSGS_VSPLIT))));
  regions = new csRegionTree2D (rc, vRegionStyles[0] );
  // rc below is a dummy and will be recalculated when SetRect is called
  viewlayout = new csRegionTree2D (rc, vViews[0] );
  splitterX = splitterY = 0;
  if (iStyle & CSGS_HSPLIT)
    splitterX = new csSplitter (this);
  if (iStyle & CSGS_VSPLIT)
    splitterY = new csSplitter (this);
  if (pParent)
    pParent->SendCommand (cscmdWindowSetClient, (intptr_t)this);
  SetActiveView (GetRootView ());
}

csGrid::~csGrid ()
{
  size_t i, j;

  for (i = 0; i < grid->rows.Length (); i++)
  {
    csSparseGrid::csGridRow *r = (csSparseGrid::csGridRow*)grid->rows.Get(i)->data;
    for (j = 0; j < r->Length (); j++)
    {
      csString *str = (csString*)r->Get (j)->data;
      delete str;
    }
    delete r;
  }

  delete grid;
  delete regions;

  for (i = 0; i < vRegionStyles.Length (); i++)
    delete vRegionStyles[i];
  //for (i=0; i<vViews.Length (); i++) delete (csGridView*)vViews.Get (i);

  delete viewlayout;
}

void csGrid::Draw ()
{
  Box (0, 0, bound.Width (), bound.Height (), CSPAL_GRIDVIEW_BACKGROUND);
  csComponent::Draw ();
  // views are children, so they are drawn later
}

bool csGrid::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdSplitterPosSet:
        {
          csSplitter *sl = (csSplitter *)Event.Command.Info;
          // find the view containing the mouse pointer
          int x, y;
          sl->GetPos (x, y);
          csRect rc (x, y, x + 1, y + 1);
          csArray<csRegionTree2D*> vSpl;
          viewlayout->FindRegion (rc, vSpl);
          if (vSpl.Length () == 1)
          {
            csGridView *spl = (csGridView*)((csRegionTree2D*)vSpl.Get (0))->data;
            if (sl == splitterX)
              spl->SplitX (x - spl->bound.xmin);
            else
              spl->SplitY (y - spl->bound.ymin);
          }
	  // Place the splitters back
          PlaceGadgets ();
	  return true;
        }
        break;
      }
      break;
  case csevKeyboard:
    if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
    {
      switch (csKeyEventHelper::GetCookedCode (&Event))
      {
      case CSKEY_DOWN:
	if (GetCursorStyle () != CSGCS_NONE)
	{
	  const csRect &rc = GetRootView ()->GetArea ();
	  if (ycur < rc.ymax)
	    SetCursorPos (ycur+1, xcur);
	  return true;
	}
	break;
      case CSKEY_UP:
	if (GetCursorStyle () != CSGCS_NONE)
	{
	  const csRect &rc = GetRootView ()->GetArea ();
	  if (ycur > rc.ymin)
	    SetCursorPos (ycur-1, xcur);
	  return true;
	}
	break;
      case CSKEY_LEFT:
	if (GetCursorStyle () != CSGCS_NONE)
	{
	  const csRect &rc = GetRootView ()->GetArea ();
	  if (xcur > rc.xmin)
	    SetCursorPos (ycur, xcur-1);
	  return true;
	}
	break;
      case CSKEY_RIGHT:
	if (GetCursorStyle () != CSGCS_NONE)
	{
	  const csRect &rc = GetRootView ()->GetArea ();
	  if (xcur < rc.xmax)
	    SetCursorPos (ycur, xcur+1);
	  return true;
	}
	break;
      }
    }
  }
  return csComponent::HandleEvent (Event);
}


/**
 * Resize views proportional to the new size of csGrid.
 * @@@ TODO: TAKE MINIMUM SIZE INTO ACCOUNT
 */
static bool ResizeViews (void* node, void* /*databag*/)
{
  csRegionTree2D *t = (csRegionTree2D*)node;
  if (t->children[0] == 0)
  {
    // leaf - we find the new size in the region variable
    ((csGridView*)t->data)->SetRect (t->region.xmin, t->region.ymin,
      t->region.xmax, t->region.ymax);
    return false;
  }
  else
  {
    csGridView *sp1 = (csGridView*)t->children [0]->data;
    int newWidthSp1 = (int)(t->region.Width () * sp1->areafactor);
    int newHeightSp1 = (int)(t->region.Height () * sp1->areafactor);

    if (t->children [0]->region.xmin != t->children[1]->region.xmin)
    {
      // views were divided along the x axis
      t->children [0]->region.Set (t->region.xmin, t->region.ymin,
        t->region.xmin + newWidthSp1, t->region.ymax);
      t->children [1]->region.Set (t->region.xmin + newWidthSp1,
        t->region.ymin, t->region.xmax, t->region.ymax);
    }
    else
    {
      // views were divided along the y axis
      t->children [0]->region.Set (t->region.xmin, t->region.ymin,
        t->region.xmax, t->region.ymin + newHeightSp1);
      t->children [1]->region.Set (t->region.xmin, t->region.ymin + newHeightSp1,
        t->region.xmax, t->region.ymax);
    }
  }
  return true;
}

/**
 * Calculate the minimal area neeed to display all views.
 */
void csGrid::CalcMinimalSize (csRegionTree2D *node, int &w, int &h)
{
  if (node->children [0] == 0)
  {
    // leaf
    ((csGridView*)node->data)->SuggestSize (w, h);
  }
  else
  {
    int w1, w2, h1, h2;
    csGridView *sp1 = (csGridView*)(node->children [0]->data);
    csGridView *sp2 = (csGridView*)(node->children [1]->data);
    CalcMinimalSize (node->children [0], w1, h1);
    CalcMinimalSize (node->children [1], w2, h2);
    if (sp1->bound.xmin != sp2->bound.xmin)
    {
      w = w1 + w2;
      h = MAX (h1, h2);
    }
    else
    {
      w = MAX (w1, w2);
      h = h1 + h2;
    }
  }

}

bool csGrid::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csComponent::SetRect (xmin, ymin, xmax, ymax))
  {
    viewlayout->region.Set (0, 0,
      bound.Width () - (splitterX ? 3 : 0),
      bound.Height () - (splitterY ? 3 : 0));
    viewlayout->Traverse (ResizeViews);
    PlaceGadgets ();
    return true;
  }
  return false;
}

void csGrid::FixSize (int &newW, int &newH)
{
  int w, h;
  SuggestSize (w, h);
  if (newW < w) newW = w;
  if (newH < h) newH = h;
}

void csGrid::SuggestSize (int &w, int &h)
{
  CalcMinimalSize (viewlayout, w, h);
  w += (splitterX ? splitterX->bound.Width () : 0);
  h += (splitterY ? splitterY->bound.Height () : 0);
}

void csGrid::PlaceGadgets ()
{
  if (splitterX)
    splitterX->SetRect (bound.Width() - 3, 0, bound.Width (), bound.Height ());
  if (splitterY)
    splitterY->SetRect (0, bound.Height () - 3, bound.Width (), bound.Height ());
}

void csGrid::SetStringAt (int row, int col, const char *data)
{
  csString *str = (csString*)grid->GetAt (row, col);
  if (str || data)
  {
    if (!str)
    {
      str = new csString (data);
      grid->SetAt (row, col, str);
    }
    else if (!data)
    {
      delete str;
      grid->SetAt (row, col, 0);
    }
    else
      str->Truncate (0).Append (data);
  }
}

csString *csGrid::GetStringAt (int row, int col)
{
  return (csString*)grid->GetAt (row, col);
}

void csGrid::CreateRegion (csRect& rc, csGridCell *cell)
{
  regions->Insert (rc, cell);
  if (!cell->IsUsed ())
  {
    cell->SetUsed ();
    vRegionStyles.Push (cell);
  }
  Invalidate (true);
}

void csGrid::SetCursorStyle (int iCursorStyle)
{
  cursorStyle = iCursorStyle;
}

int csGrid::GetCursorStyle ()
{
  return cursorStyle;
}

void csGrid::GetCursorPos (int &row, int &col)
{
  row = ycur;
  col = xcur;
}

void csGrid::SetCursorPos (int row, int col)
{
  if (row != ycur || col != xcur)
  {
    ycur = row;
    xcur = col;
    if (parent) parent->SendCommand (cscmdGridCursorChanged, (intptr_t)this);
  }
}

void csGrid::SetActiveView (csGridView *view)
{
  if (GetFocused () != view) SetFocused (view);
  activeView = view;
}
