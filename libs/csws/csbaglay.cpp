/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Kraemer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#include "cssysdef.h"
#include "csws/csbaglay.h"
#include "csutil/hashmap.h"

csGridBagConstraint::csGridBagConstraint (csComponent *comp)
  : csLayoutConstraint (comp), gridx (RELATIVE), gridy (RELATIVE),
    gridwidth (1), gridheight (1), weightx (0), weighty (0),
    anchor (CENTER), fill (NONE), insets (0,0,0,0),
    ipadx (0), ipady (0), bSized (false)
{ }

csGridBagConstraint::csGridBagConstraint (csComponent *comp, int _gridx,
  int _gridy, int _gridwidth, int _gridheight, float _weightx,
  float _weighty, int _anchor, int _fill, csRect _insets,
  int _ipadx, int _ipady) : csLayoutConstraint (comp),
  gridx (_gridx), gridy (_gridy), gridwidth (_gridwidth), gridheight(_gridheight),
  weightx(_weightx), weighty(_weighty), anchor (_anchor), fill (_fill),
  insets (_insets), ipadx (_ipadx), ipady (_ipady), bSized (false)
{ }

csGridBagConstraint::csGridBagConstraint (const csGridBagConstraint &c)
  : csLayoutConstraint (c.comp)
{
  gridx = c.gridx;
  gridy = c.gridy;
  gridwidth = c.gridwidth;
  gridheight = c.gridheight;
  weightx = c.weightx;
  weighty = c.weighty;
  anchor = c.anchor;
  fill = c.fill;
  insets.Set (c.insets);
  ipadx = c.ipadx;
  ipady = c.ipady;
  bSized = false;
}

csLayoutConstraint *csGridBagConstraint::Clone ()
{
  return new csGridBagConstraint (*this);
}

/***** Implementation for class GridBagLayout ******/

csGridBagLayout::csGridBagLayout (csComponent *pParent) : csLayout2 (pParent),
  c (0), mpHorizCellInfos (0), mpVertCellInfos (0)
{
  lc = &c;
}

csGridBagLayout::~csGridBagLayout ()
{
  ClearCachedData();
  CleanupConstraints();
}

// impl. of LayoutManager

void csGridBagLayout::RemoveLayoutComponent (csComponent* comp)
{
  // force matrix recalc
  ClearCachedData();

  csLayout2::RemoveLayoutComponent (comp);
}

void csGridBagLayout::SuggestSize (int &sugw, int &sugh)
{
  if (!HasCashedInfo ())
    CreateMatrix ();

  sugw  = CalcPrefSize (mpHorizCellInfos, mColCount, mRowCount, mColCount);
  sugh = CalcPrefSize (mpVertCellInfos,  mRowCount, mColCount, mRowCount);

  sugw += insets.xmin + insets.xmax;
  sugh += insets.ymin + insets.ymax;
}

void csGridBagLayout::LayoutContainer ()
{
  int w = bound.Width ();
  int h = bound.Height ();
  int x, y;

  x = y = 0;

  x += insets.xmin;
  y += insets.ymin;

  w -= insets.xmin + insets.xmax;
  h -= insets.ymin + insets.ymax;

  if (!HasCashedInfo ())
    CreateMatrix ();

  // calculate horizontal extents and sizes of all cells
  LayoutCells (mpHorizCellInfos, mColCount, mRowCount, w, x, mColCount);
  // now the same alg. is applied independently for vertical positions/sizes
  LayoutCells (mpVertCellInfos, mRowCount, mColCount, h, y, mRowCount);

  SetComponentLocations ();
}

// impl. of LayoutManager2

void csGridBagLayout::MaximumLayoutSize (int &w, int &h)
{
  // FOR NOW::
  if (parent)
  {
    w = bound.Width ();
    h = bound.Height ();
  }
  else
    w = h = 1 << 31;
}

float csGridBagLayout::GetLayoutAlignmentX ()
{
  return 0;
}

float csGridBagLayout::GetLayoutAlignmentY ()
{
  return 0;
}

/*** protected mehtods ***/

#define CELL_AT(x,y) cells[ (y) * _arrayWidth + (x) ]

int csGridBagLayout::CalcPrefSize (CellInfo* cells, int xCnt, int yCnt, int _arrayWidth)
{
  int prefered = 0;

  int x, y;
  for (x = 0; x != xCnt; ++x)
  {
    int maxColSize = 0;

    for (y = 0; y != yCnt; ++y)
      if (CELL_AT(x,y).prefSize > maxColSize)
	maxColSize = CELL_AT(x,y).prefSize;

    prefered += maxColSize;
  }

  return prefered;
}

void csGridBagLayout::LayoutCells (CellInfo* cells, int xCnt, int yCnt,
  int outterSize, int outterPos, int _arrayWidth)
{
  int actualSpaceUsed = 0;
  int x, y, i;

  // calculate prefered size of each column
  int *prefColSizes = new int [xCnt];

  for (x = 0; x != xCnt; ++x)
  {
    int prefColSize = 0;
    for (y = 0; y != yCnt; ++y)
      if (CELL_AT(x,y).prefSize > prefColSize)
	prefColSize = CELL_AT(x,y).prefSize;

    prefColSizes [x] = prefColSize;
  }

  int totalPrefSize = 0;

  for (x = 0; x != xCnt; ++x)
    totalPrefSize += prefColSizes[x];

  int extraSpace = outterSize - totalPrefSize;

  if (extraSpace <= 0)
  {
    // redistribute spaces to fit (i.e. shrink to) given outter-size
    int spaceUsed = 0;
    for (x = 0; x != xCnt; ++x)
    {
      int colSize = (outterSize * prefColSizes [x]) / totalPrefSize;
      // elimniate round-off errors at the expence of last column
      if (x == xCnt - 1)
        colSize = outterSize - spaceUsed;
      for (y = 0; y != yCnt; ++y)
        CELL_AT(x,y).finalSize = colSize;

      spaceUsed += colSize;
    }
    actualSpaceUsed = outterSize; // all space is cosumed when
    // extra-space is absent
  }
  else
  {
    // defferent alg. if there is extra space available
    int hasStrechedCells = 0;
    for (y = 0; y != yCnt; ++y)
    {
      // calc weight-sum
      float totalWeight = 0;
      int x = 0;
      for (; x != xCnt; ++x)
	totalWeight += CELL_AT(x,y).weight;
      // distribute extraSpace in a row according to cell weights
      if (totalWeight > 0)
      {
	hasStrechedCells = 1;
	for (x = 0; x != xCnt; ++x)
	  CELL_AT(x,y).extraSpace = extraSpace * (CELL_AT(x,y).weight / totalWeight);
      }
      else
	for (x = 0; x != xCnt; ++x)
	  CELL_AT(x,y).extraSpace = 0;
    }
    // calc total extra space consumed by each column
    float *colSpaces = new float [xCnt];
    float sum = 0;
    for (x = 0; x != xCnt; ++x)
    {
      float total = 0;
      for (y = 0; y != yCnt; ++y)
	total += CELL_AT(x,y).extraSpace;

      colSpaces[x] = total;
      sum += total;
    }

    // equation should hold true : "extraSpace * yCnt == sum"
    // now redistribute extra space among all cesll in each columns using,
    // giving each column a fraction of space which corresponds to column's
    // "totalSpace" value's relation with the "sum"

    int spaceUsed = 0;
    actualSpaceUsed = 0;
    for (x = 0; x != xCnt; ++x)
    {
      int extraSpaceForCol = (int)(sum != 0 ? (extraSpace * (colSpaces[x] / sum )) : 0.0);
      // elimniate round-off errors at the expence of last column
      if (x == xCnt - 1 && hasStrechedCells)
        extraSpaceForCol = extraSpace - spaceUsed;
      int spaceForCol = (int)(prefColSizes[x] + extraSpaceForCol);
      actualSpaceUsed += spaceForCol;
      for (y = 0; y != yCnt; ++y)
        CELL_AT(x,y).finalSize = spaceForCol;
      spaceUsed += extraSpaceForCol;
    }
    delete [] colSpaces;
  } // end of else {....}

  // now do positioning of cells and components inside of them
  // (at this point, "actualSpaceUsed <= outterSpace" should be true)

  // center grid w/respect to bounds of outter component
  int curPos = (outterSize - actualSpaceUsed) / 2;

  for (x = 0; x != xCnt; ++x)
  {
    for (y = 0; y != yCnt; ++y)
    {
      CellInfo& cell = CELL_AT(x,y);
      if (cell.comp != 0)
      {
	cell.finalPos = curPos;
	int compSize = 0;
	int compPos  = curPos;

	// account for spaning multiple cells
	for(i = 0; i != cell.cellSpan && x + i < xCnt; ++i )
	  compSize += CELL_AT(x+i,y).finalSize;
	// align compoenent within cell's display area

	compPos  += cell.leftInset;
	compSize -= cell.leftInset + cell.rightInset;

	cell.finalCompPos  = compPos;
	cell.finalCompSize = compSize;

	if (!cell.fill && cell.prefCompSize < compSize)
	{
	  cell.finalCompSize = cell.prefCompSize;
	  if (cell.anchor == csGridBagConstraint::_LEFT)
	    cell.finalCompPos  = compPos;
	  else
	    if (cell.anchor == csGridBagConstraint::_CENTER)
	      cell.finalCompPos = compPos + ( compSize - cell.prefCompSize ) / 2;
	    else
	      if (cell.anchor == csGridBagConstraint::_RIGHT)
		cell.finalCompPos = compPos + ( compSize - cell.prefCompSize );
	}
	cell.finalCompPos += outterPos;
      } // end of if(...)
    } // end of for(...)

    curPos += CELL_AT(x,0).finalSize;

  } // end of for(..)

  delete [] prefColSizes;
}

void csGridBagLayout::CleanupConstraints ()
{
  vConstraints.DeleteAll ();
  vConstraints.SetLength (0);
}

void csGridBagLayout::InitializeCellArray (CellInfo* cells, int size)
{
  int i;
  for (i = 0; i != size; ++i)
  {
    memset (cells + i, 0, sizeof (CellInfo));
    cells [i].weight = 0; // floats cannot be zero'ed with memset(..)
  }
}

void csGridBagLayout::InitCellFromHolder (CellHolder& holder)
{
  CellInfo& hCell = mpHorizCellInfos [holder.y * mColCount + holder.x]; // note the trick...
  CellInfo& vCell = mpVertCellInfos  [holder.x * mRowCount + holder.y]; // -/-

  csGridBagConstraint& c = *holder.constr;

  hCell.comp = c.comp;
  vCell.comp = c.comp;

  if (c.fill == csGridBagConstraint::BOTH
   || c.fill == csGridBagConstraint::HORIZONTAL)
    hCell.fill = 1;

  if (c.fill == csGridBagConstraint::BOTH
   || c.fill == csGridBagConstraint::VERTICAL)
    vCell.fill = 1;

  hCell.leftInset  = c.insets.xmin;
  hCell.rightInset = c.insets.xmax;
  vCell.leftInset  = c.insets.ymin;
  vCell.rightInset = c.insets.ymax;

  hCell.pad = c.ipadx;
  vCell.pad = c.ipady;

  hCell.prefCompSize = c.mPrefCompSize.x + 2 * hCell.pad;
  vCell.prefCompSize = c.mPrefCompSize.y + 2 * vCell.pad;

  hCell.prefSize = hCell.prefCompSize + hCell.leftInset + hCell.rightInset;
  vCell.prefSize = vCell.prefCompSize + hCell.leftInset + hCell.rightInset;

  if (holder.isFirstCellForComp)
  {
    hCell.cellSpan = holder.actualWidth;
    vCell.cellSpan = holder.actualHeight;

    // non-zero weights are applied to the last
    // cell of the area covered by component
    // (this is how AWT's csGridBagLayout behaves!)

    int x = holder.x + holder.actualWidth  - 1;
    int y = holder.y + holder.actualHeight - 1;

    CellInfo& hCell1 = mpHorizCellInfos [y * mColCount + x]; // note the trick...
    CellInfo& vCell1 = mpVertCellInfos  [x * mRowCount + y]; // -/-

    hCell1.weight = holder.weightx;
    vCell1.weight = holder.weighty;
  }
  // otherwise cellSpan are 0 for intermediate cells

  // adjust alginment for the horizontal-info carrying cell's

  if (c.anchor == csGridBagConstraint::NORTHWEST
   || c.anchor == csGridBagConstraint::WEST
   || c.anchor == csGridBagConstraint::SOUTHWEST)
    hCell.anchor = csGridBagConstraint::_LEFT;

  if (c.anchor == csGridBagConstraint::NORTH
   || c.anchor == csGridBagConstraint::CENTER
   || c.anchor == csGridBagConstraint::SOUTH)
    hCell.anchor = csGridBagConstraint::_CENTER;

  if (c.anchor == csGridBagConstraint::NORTHEAST
   || c.anchor == csGridBagConstraint::EAST
   || c.anchor == csGridBagConstraint::SOUTHEAST)
    hCell.anchor = csGridBagConstraint::_RIGHT;

  // adjust alginment for the vertical-info carrying cell's

  if (c.anchor == csGridBagConstraint::NORTHWEST
   || c.anchor == csGridBagConstraint::NORTH
   || c.anchor == csGridBagConstraint::NORTHEAST)
    vCell.anchor = csGridBagConstraint::_LEFT;

  if (c.anchor == csGridBagConstraint::WEST
   || c.anchor == csGridBagConstraint::CENTER
   || c.anchor == csGridBagConstraint::EAST)
    vCell.anchor = csGridBagConstraint::_CENTER;

  if (c.anchor == csGridBagConstraint::SOUTHWEST
   || c.anchor == csGridBagConstraint::SOUTH
   || c.anchor == csGridBagConstraint::SOUTHEAST)
    vCell.anchor = csGridBagConstraint::_RIGHT;
}

long csGridBagLayout::GetCellCode (int x, int y)
{
  return (x << 8) | y;
}

void csGridBagLayout::CreateMatrix ()
{
#define MAX_GRID_WIDTH  100
#define MAX_GRID_HEIGHT 100

  ClearCachedData ();

  CellHolderArrayT holders;
  csHashMap usedCellsHash (8087);

  mColCount = 0;
  mRowCount = 0;

  int nextX = 0;
  int nextY = 0;

  // creating cells for all added components according
  // to the info in their constraints

  size_t i = 0;
  for (; i != vConstraints.Length (); ++i)
  {
    int w = 0, h = 0;
    csGridBagConstraint& c = *(csGridBagConstraint *)vConstraints.Get (i);
    if (!c.bSized)
    {
      c.comp->SuggestSize (w, h);
      c.mPrefCompSize.Set (w, h);
      c.bSized = true;
    }

    if (c.gridx != csGridBagConstraint::RELATIVE) nextX = c.gridx;
    if (c.gridy != csGridBagConstraint::RELATIVE) nextY = c.gridy;

    // TBD:: handle situations when grix - give, but gridy is RELATIVE,
    //       and v.v. (should use vert/horiz scanning for not-used cells)

    while (usedCellsHash.Get (GetCellCode (nextX,nextY)))
      ++nextX;

    int width  = c.gridwidth;
    int height = c.gridheight;

    // add this stage, we treat relavtive/REMAINDER
    // sizes as equal to 1, because we cannot yet
    // predict the actual expansion of the coponent

    if (width == csGridBagConstraint::RELATIVE
     || width == csGridBagConstraint::REMAINDER)
      width = 1;
    if (height == csGridBagConstraint::RELATIVE
     || height == csGridBagConstraint::REMAINDER)
      height = 1;

    /*
      // OLD STUF::
      // distribute weigths uniformly among all
      // cells which are covered by current compoenent

      //float weightx = c.weightx / (float)width;
      //float weighty = c.weighty / (float)height;
    */

    // create cells for the area covered by the component

    CellHolder* pHolder = 0;

	int xofs, yofs, x, y;
    for (xofs = 0; xofs != width; ++xofs)
      for (yofs = 0; yofs != height; ++yofs)
      {
        pHolder = new CellHolder ();
        pHolder->constr = &c;
        pHolder->isFirstCellForComp = (xofs == 0 && yofs == 0);

        pHolder->x = nextX + xofs;
        pHolder->y = nextY + yofs;

        pHolder->weightx = c.weightx;
        pHolder->weighty = c.weighty;

        pHolder->gridwidth  = c.gridwidth;
        pHolder->gridheight = c.gridheight;

        holders.Push (pHolder);

        usedCellsHash.Put (GetCellCode (nextX + xofs, nextY + yofs),
          (csHashObject)pHolder);
      }

    if (c.gridwidth == csGridBagConstraint::REMAINDER)
      // mark all possible reminding cells in the row as used
      for (x = nextX + width; x < MAX_GRID_WIDTH; ++x)
        usedCellsHash.Put (GetCellCode (x, nextY), (csHashObject)pHolder);

    if (c.gridheight == csGridBagConstraint::REMAINDER)
      // mark all possible eminding cells in the column as used
      for (y = nextY+height; y < MAX_GRID_HEIGHT; ++y)
        usedCellsHash.Put (GetCellCode (nextX, y), (csHashObject)pHolder);
    // adjust estimated dimensions of the matrix (grid)

    if (nextX + width  > mColCount)
      mColCount = nextX + width;

    if (nextY + height > mRowCount)
      mRowCount = nextY + height;

    // move on to next cell

    if (c.gridwidth == csGridBagConstraint::REMAINDER)
    {
      nextX = 0; ++nextY;
    }
    else
    {
      if (c.gridwidth == csGridBagConstraint::RELATIVE)
        nextX += 1;
      else
        nextX += width;
    }

  }  // end of for(...)

  // now actually create matrix

  int sz = mColCount * mRowCount;

  mpHorizCellInfos = new CellInfo [sz];
  mpVertCellInfos  = new CellInfo [sz];
  InitializeCellArray (mpHorizCellInfos, sz);
  InitializeCellArray (mpVertCellInfos,  sz);

  // and fill in cells with info

  for (i = 0; i != holders.Length (); ++i)
  {
    CellHolder& h = *holders [i];
    if (h.isFirstCellForComp)
    {
      // now set-up actual gridwidth, and gridheigh parameters
      int actualWidth  = h.constr->gridwidth;
      int actualHeight = h.constr->gridheight;
      int x = h.x, y = h.y;

      // extend widths/heights if given as relative/REMAINDER by
      // traversing grid until end is reached or cell occupied by
      // anther component is encountered

      if (h.constr->gridwidth == csGridBagConstraint::RELATIVE
       || h.constr->gridwidth == csGridBagConstraint::REMAINDER)
      {
        // TBD:: comments.. this is how AWT's GridBagLayout behaves...
        ++x;
        while (x < mColCount)
        {
          CellHolder* pHolder = (CellHolder*)usedCellsHash.Get (GetCellCode(x,y));
          if (!pHolder || (pHolder && pHolder->constr != h.constr) )
            break;
          ++x;
        }
        actualWidth = x - h.x;
      }

      if (h.constr->gridheight == csGridBagConstraint::RELATIVE
       || h.constr->gridheight == csGridBagConstraint::REMAINDER)
      {
        ++y;
        while (y < mRowCount)
        {
          CellHolder* pHolder = (CellHolder*)usedCellsHash.Get (GetCellCode (x,y));
          if (!pHolder || (pHolder && pHolder->constr != h.constr))
            break;
          ++y;
        }
        actualHeight = y - h.y;
      }

      h.actualWidth  = actualWidth;
      h.actualHeight = actualHeight;

      // split info contained in holder and constraints among two cell objects:
      // one - carrying vertical info, another -carrying horizontal info

      InitCellFromHolder (*holders [i]);
    }
  }
}

void csGridBagLayout::ClearCachedData ()
{
  delete [] mpHorizCellInfos;
  delete [] mpVertCellInfos;

  mpHorizCellInfos = 0;
  mpVertCellInfos  = 0;
}

bool csGridBagLayout::HasCashedInfo ()
{
  return mpHorizCellInfos != 0;
}

void csGridBagLayout::SetComponentLocations ()
{
  int x, y;
  for (y = 0; y != mRowCount; ++y)
    for (x = 0; x != mColCount; ++x)
    {
      CellInfo& hCell = mpHorizCellInfos [y * mColCount + x]; // note the trick...
      CellInfo& vCell = mpVertCellInfos  [x * mRowCount + y]; // -/-
      if (hCell.comp != 0)
	hCell.comp->SetRect (hCell.finalCompPos, vCell.finalCompPos,
          hCell.finalCompPos + hCell.finalCompSize,
          vCell.finalCompPos + vCell.finalCompSize);
    }
}
