#include "cssysdef.h"
#include "awslayot.h"
#include "awsgbl.h"
#include "awsgbc.h"
#include <stdlib.h>
#include <math.h>

const int awsGridBagLayout::MAXGRIDSIZE = 512;
const int awsGridBagLayout::MINSIZE = 1;
const int awsGridBagLayout::PREFERREDSIZE = 2;

const int awsGridBagConstraints::RELATIVE = -1;
const int awsGridBagConstraints::REMAINDER = 0;
const int awsGridBagConstraints::NONE = 0;
const int awsGridBagConstraints::BOTH = 1;
const int awsGridBagConstraints::HORIZONTAL = 2;
const int awsGridBagConstraints::VERTICAL = 3;
const int awsGridBagConstraints::CENTER = 10;
const int awsGridBagConstraints::NORTH = 11;
const int awsGridBagConstraints::NORTHEAST = 12;
const int awsGridBagConstraints::EAST = 13;
const int awsGridBagConstraints::SOUTHEAST = 14;
const int awsGridBagConstraints::SOUTH = 15;
const int awsGridBagConstraints::SOUTHWEST = 16;
const int awsGridBagConstraints::WEST = 17;
const int awsGridBagConstraints::NORTHWEST = 18;

const int MAXINT = 0xeffffff;

inline int
max(int i, int j)
{
  return (i>j ? i : j);
}

/////////////////////////////////////////////////////////////////////////////////

awsGridBagConstraints::awsGridBagConstraints():
	gridx(RELATIVE),
    gridy(RELATIVE),
    gridwidth(1),
    gridheight(1),
    weightx(0),
    weighty(0),
    anchor(CENTER),
    fill(NONE),
	insets(0, 0, 0, 0),
    ipadx(0),
    ipady(0)
{

}

awsGridBagConstraints::awsGridBagConstraints(int gridx, int gridy,
                            int gridwidth, int gridheight,
                            double weightx, double weighty,
                            int anchor, int fill,
                            csRect insets, int ipadx, int ipady):
 
    gridx(gridx),
    gridy(gridy),
    gridwidth(gridwidth),
    gridheight(gridheight),
    fill(fill),
    ipadx(ipadx),
    ipady(ipady),
    insets(insets),
    anchor (anchor),
    weightx(weightx),
    weighty(weighty)
{ 
}

awsGridBagConstraints *
awsGridBagConstraints::Clone()
{
	return new awsGridBagConstraints(gridx, gridy, gridwidth, gridheight, 
					 weightx, weighty, anchor, fill, 
					 insets, ipadx, ipady);
}



/////////////////////////////////////////////////////////////////////////////////////////

awsGridBagLayout::GridBagLayoutInfo::GridBagLayoutInfo()
{
  minWidth= new int[awsGridBagLayout::MAXGRIDSIZE];		
  minHeight = new int[awsGridBagLayout::MAXGRIDSIZE];		
  weightX = new double[awsGridBagLayout::MAXGRIDSIZE];
  weightY = new double[awsGridBagLayout::MAXGRIDSIZE];
}

awsGridBagLayout::GridBagLayoutInfo::~GridBagLayoutInfo()
{
  if (minWidth) delete [] minWidth;
  if (minHeight) delete [] minHeight;
  if (weightX) delete [] weightX;
  if (weightY) delete [] weightY;
}

void
awsGridBagLayout::GridBagLayoutInfo::Set(awsGridBagLayout::GridBagLayoutInfo &l2)
{
  if (minWidth) delete [] minWidth;
  if (minHeight) delete [] minHeight; 
  if (weightX) delete [] weightX;
  if (weightY) delete [] weightY;


  width=l2.width;
  height=l2.height;
  startx=l2.startx;
  starty=l2.starty;
  minWidth=l2.minWidth;
  minHeight=l2.minHeight;
  weightX=l2.weightX;
  weightY=l2.weightY;
}


awsGridBagLayout::awsGridBagLayout(iAwsComponent *o):
        awsLayoutManager(o),
	layoutInfo(NULL),
	columnWidths(NULL), columnWidthsLength(0), 
	rowHeights(NULL), rowHeightsLength(0), 
	columnWeights(NULL), columnWeightsLength(0), 
	rowWeights(NULL), rowWeightsLength(0)
	
{		
}

void 
awsGridBagLayout::LayoutComponents()
{
  ArrangeGrid(owner);
}	

csRect 
awsGridBagLayout::AddComponent(iAwsPrefManager *pm, awsComponentNode *settings, iAwsComponent *cmp)
{
	awsGridBagConstraints c;

	int wx=0, wy=0;

	pm->GetInt(settings,"GridX",  c.gridx);
	pm->GetInt(settings,"GridY",  c.gridy);
	pm->GetInt(settings,"GridWidth",  c.gridwidth);
	pm->GetInt(settings,"GridHeight", c.gridheight);
	pm->GetInt(settings,"Fill",   c.fill);
	pm->GetInt(settings,"Anchor", c.anchor);
	pm->GetInt(settings,"iPadX",  c.ipadx);
	pm->GetInt(settings,"iPadY",  c.ipady);
	pm->GetInt(settings,"WeightX",  wx);
	pm->GetInt(settings,"WeightY",  wy);
	pm->GetRect(settings,"Insets",  c.insets);

	c.weightx = wx/100.0;
	c.weighty = wy/100.0;

	setConstraints(cmp, c);

	return csRect(0,0,0,0);
}

void 
awsGridBagLayout::setConstraints(iAwsComponent *cmp, awsGridBagConstraints &constraints) {
	// check if the component exists.
    awsGridBagConstraints *p = STATIC_CAST(awsGridBagConstraints *, comptable.Get((csHashKey)cmp));

	// kill it if so.
	if (p) comptable.DeleteAll((csHashKey)cmp);

	// add in the new constraints.
    comptable.Put((csHashKey)cmp, constraints.Clone());
}

awsGridBagConstraints 
awsGridBagLayout::getConstraints(iAwsComponent *cmp) 
{
    awsGridBagConstraints *constraints = STATIC_CAST(awsGridBagConstraints *, comptable.Get((csHashKey)cmp));    
	
	if (constraints == NULL) 
	{
      setConstraints(cmp, defaultConstraints);
      constraints = (awsGridBagConstraints *)comptable.Get((csHashKey)cmp);
    }

	return awsGridBagConstraints(constraints->gridx, 
								 constraints->gridy, 
								 constraints->gridwidth, 
								 constraints->gridheight, 
								 constraints->weightx, 
								 constraints->weighty,
								 constraints->anchor,
								 constraints->fill, 
								 constraints->insets, 
								 constraints->ipadx, 
								 constraints->ipady);
   
}

awsGridBagConstraints *
awsGridBagLayout::lookupConstraints(iAwsComponent *cmp) 
{
  awsGridBagConstraints *constraints = STATIC_CAST(awsGridBagConstraints *, comptable.Get((csHashKey)cmp));    

  if (constraints == NULL) 
  {
    setConstraints(cmp, defaultConstraints);
    constraints = (awsGridBagConstraints *)comptable.Get((csHashKey)cmp);
  }
	    
  return constraints;
}

void 
awsGridBagLayout::removeConstraints(iAwsComponent *cmp)
{
  comptable.DeleteAll((csHashKey)cmp);
}


csPoint 
awsGridBagLayout::getLayoutOrigin () 
{
    csPoint origin(0,0);
    if (layoutInfo != NULL) {
      origin.x = layoutInfo->startx;
      origin.y = layoutInfo->starty;
    }
    return origin;
}
  
void
awsGridBagLayout::getLayoutDimensions (int **row, int **col) 
{
	if (!row || !col) return;

    if (layoutInfo == NULL)
	{
      *row=NULL;
	  *col=NULL;
      return;
	}

    *col = new int[layoutInfo->width];
    *row = new int[layoutInfo->height];

    memcpy(*col, layoutInfo->minWidth,  layoutInfo->width * sizeof(int));
    memcpy(*row, layoutInfo->minHeight, layoutInfo->height * sizeof(int));
}


void 
awsGridBagLayout::getLayoutWeights (double **row, double **col) 
{
	if (!row || !col) return;

    if (layoutInfo == NULL)
	{
      *row=NULL;
	  *col=NULL;
      return; 
	}
        
    *col = new double[layoutInfo->width];
    *row = new double[layoutInfo->height];

    memcpy(*col, layoutInfo->weightX, layoutInfo->width * sizeof(double));
    memcpy(*row, layoutInfo->weightY, layoutInfo->height * sizeof(double));
  }

csPoint 
awsGridBagLayout::location(int x, int y) 
{
    csPoint loc(0,0);
    int i, d;

    if (layoutInfo == NULL)
      return loc;

    d = layoutInfo->startx;
    for (i=0; i<layoutInfo->width; ++i) 
	{
      d += layoutInfo->minWidth[i];
      if (d > x)
		break;
    }

    loc.x = i;

    d = layoutInfo->starty;
    for (i=0; i<layoutInfo->height; ++i) 
	{
      d += layoutInfo->minHeight[i];
      if (d > y)
		break;
    }

    loc.y = i;

    return loc;
}


void 
awsGridBagLayout::addLayoutComponent(iAwsComponent *cmp, awsGridBagConstraints &constraints) 
{
  setConstraints(cmp, constraints);
}

void 
awsGridBagLayout::removeLayoutComponent(iAwsComponent *cmp) 
{
    removeConstraints(cmp);
}


awsGridBagLayout::GridBagLayoutInfo 
awsGridBagLayout::GetLayoutInfo(iAwsComponent *parent, int sizeflag) 
{	

    GridBagLayoutInfo r;
    iAwsComponent *cmp;
    awsGridBagConstraints *constraints;
    
    int compindex, i, /*j,*/ k, px, py, pixels_diff, nextSize;
    int curX, curY, curWidth, curHeight, curRow, curCol;
    double weight_diff, weight /*, start, size*/;
    int xMax[MAXGRIDSIZE], yMax[MAXGRIDSIZE];
	csRect d;

    /*
     * Pass #1
     *
     * Figure out the dimensions of the layout grid (use a value of 1 for
     * zero or negative widths and heights).
     */

    r.width = r.height = 0;
    curRow = curCol = -1;
	memset(xMax, 0, sizeof(int)*MAXGRIDSIZE);
    memset(yMax, 0, sizeof(int)*MAXGRIDSIZE);

    
    for (compindex = 0 ; compindex < parent->GetChildCount(); ++compindex) 
	{
      cmp = parent->GetChildAt(compindex);

      if (cmp->isHidden()) continue;

	  
      constraints = lookupConstraints(cmp);

      curX = constraints->gridx;
      curY = constraints->gridy;

      curWidth = constraints->gridwidth;
      if (curWidth <= 0)
		curWidth = 1;

      curHeight = constraints->gridheight;
      if (curHeight <= 0)
		curHeight = 1;

      /* If x or y is negative, then use relative positioning: */
      if (curX < 0 && curY < 0) 
      {
	if (curRow >= 0)
    	  curY = curRow;
	else if (curCol >= 0)
	  curX = curCol;
	else
	  curY = 0;
      }

      if (curX < 0) 
      {
	px = 0;
		
	for (i = curY; i < (curY + curHeight); i++)
	  px = max(px, xMax[i]);

	curX = px - curX - 1;
	if(curX < 0)
	  curX = 0;
      }
      else if (curY < 0) 
      {
	py = 0;
	for (i = curX; i < (curX + curWidth); i++)
	  py = max(py, yMax[i]);

	curY = py - curY - 1;
	if(curY < 0)
	  curY = 0;
      }

      /* Adjust the grid width and height */
      for (px = curX + curWidth; r.width < px; r.width++);
      for (py = curY + curHeight; r.height < py; r.height++);

      /* Adjust the xMax and yMax arrays */
      for (i = curX; i < (curX + curWidth); i++) { yMax[i] = py; }
      for (i = curY; i < (curY + curHeight); i++) { xMax[i] = px; }

      /* Cache the current slave's size. */
      if (sizeflag == PREFERREDSIZE)
		d = cmp->getPreferredSize();
      else
		d = cmp->getMinimumSize();

      constraints->minWidth = d.Width();
      constraints->minHeight = d.Height();

      /* Zero width and height must mean that this is the last item (or
       * else something is wrong). */
      if (constraints->gridheight == 0 && constraints->gridwidth == 0)
		curRow = curCol = -1;

      /* Zero width starts a new row */
      if (constraints->gridheight == 0 && curRow < 0)
		curCol = curX + curWidth;

      /* Zero height starts a new column */
      else if (constraints->gridwidth == 0 && curCol < 0)
		curRow = curY + curHeight;
    } // end first pass 

    /*
     * Apply minimum row/column dimensions
     */
    if (columnWidths != NULL && r.width < columnWidthsLength)
      r.width = columnWidthsLength;

    if (rowHeights != NULL && r.height < rowHeightsLength)
      r.height = rowHeightsLength;

    /*
     * Pass #2
     *
     * Negative values for gridX are filled in with the current x value.
     * Negative values for gridY are filled in with the current y value.
     * Negative or zero values for gridWidth and gridHeight end the current
     *  row or column, respectively.
     */

    curRow = curCol = -1;
    memset(xMax, 0, sizeof(int)*MAXGRIDSIZE);
    memset(yMax, 0, sizeof(int)*MAXGRIDSIZE);

    for (compindex = 0 ; compindex < parent->GetChildCount(); compindex++) 
    {
      cmp = parent->GetChildAt(compindex);

      if (cmp->isHidden()) continue;

      constraints = lookupConstraints(cmp);

      curX = constraints->gridx;
      curY = constraints->gridy;
      curWidth = constraints->gridwidth;
      curHeight = constraints->gridheight;

      /* If x or y is negative, then use relative positioning: */
      if (curX < 0 && curY < 0)
      {
	if(curRow >= 0)
  	  curY = curRow;
	else if(curCol >= 0)
	  curX = curCol;
	else
	  curY = 0;
      }

      if (curX < 0) 
      {
	if (curHeight <= 0) 
	{
  	  curHeight += r.height - curY;
	  if (curHeight < 1)
	    curHeight = 1;
	}
      }

      px = 0;
      for (i = curY; i < (curY + curHeight); i++)
        px = max(px, xMax[i]);

      curX = px - curX - 1;
      if(curX < 0)
        curX = 0;
      else if (curY < 0) 
      {
	if (curWidth <= 0) 
	{
	  curWidth += r.width - curX;
	  if (curWidth < 1)
	    curWidth = 1;
	}

	py = 0;
	for (i = curX; i < (curX + curWidth); i++)
	  py = max(py, yMax[i]);

	curY = py - curY - 1;
	if(curY < 0)
	  curY = 0;
      }

      if (curWidth <= 0) 
      {
	curWidth += r.width - curX;
	if (curWidth < 1)
	  curWidth = 1;
      }

      if (curHeight <= 0) 
      {
	curHeight += r.height - curY;
	if (curHeight < 1)
	  curHeight = 1;
      }

      px = curX + curWidth;
      py = curY + curHeight;

      for (i = curX; i < (curX + curWidth); i++) { yMax[i] = py; }
      for (i = curY; i < (curY + curHeight); i++) { xMax[i] = px; }

      /* Make negative sizes start a new row/column */
      if (constraints->gridheight == 0 && constraints->gridwidth == 0)
	curRow = curCol = -1;
      if (constraints->gridheight == 0 && curRow < 0)
	curCol = curX + curWidth;
      else if (constraints->gridwidth == 0 && curCol < 0)
        curRow = curY + curHeight;

      /* Assign the new values to the gridbag child */
      constraints->tempX = curX;
      constraints->tempY = curY;
      constraints->tempWidth = curWidth;
      constraints->tempHeight = curHeight;
    }  // end pass 2

    /*
     * Apply minimum row/column dimensions and weights
     */
    if (columnWidths != NULL)
      memcpy(r.minWidth, columnWidths, columnWidthsLength * sizeof(int));
    if (rowHeights != NULL)
      memcpy(r.minHeight, rowHeights, rowHeightsLength * sizeof(int));
    if (columnWeights != NULL)
      memcpy(r.weightX, columnWeights, columnWeightsLength * sizeof(double));
    if (rowWeights != NULL)
      memcpy(r.weightY, rowWeights, rowWeightsLength * sizeof(double));

    /*
     * Pass #3
     *
     * Distribute the minimun widths and weights:
     */

    nextSize = MAXINT;

    for (i = 1;	 i != MAXINT; i = nextSize, nextSize = MAXINT) 
    {
      for (compindex = 0 ; compindex < parent->GetChildCount(); ++compindex) 
      {
	cmp = parent->GetChildAt(compindex);

	if (cmp->isHidden())
	  continue;

	constraints = lookupConstraints(cmp);

	if (constraints->tempWidth == i) {
	  px = constraints->tempX + constraints->tempWidth; /* right column */

	  /*
	   * Figure out if we should use this child\'s weight.  If the weight
	   * is less than the total weight spanned by the width of the cell,
	   * then discard the weight.  Otherwise split the difference
	   * according to the existing weights.
	   */

	  weight_diff = constraints->weightx;
	  for (k = constraints->tempX; k < px; k++)
	    weight_diff -= r.weightX[k];
	  if (weight_diff > 0.0) {
	    weight = 0.0;
	    for (k = constraints->tempX; k < px; k++)
	      weight += r.weightX[k];
	    for (k = constraints->tempX; weight > 0.0 && k < px; k++) {
	      double wt = r.weightX[k];
	      double dx = (wt * weight_diff) / weight;
	      r.weightX[k] += dx;
	      weight_diff -= dx;
	      weight -= wt;
	    }
	    /* Assign the remainder to the rightmost cell */
	    r.weightX[px-1] += weight_diff;
	  }

	  /*
	   * Calculate the minWidth array values.
	   * First, figure out how wide the current child needs to be.
	   * Then, see if it will fit within the current minWidth values.
	   * If it will not fit, add the difference according to the
	   * weightX array.
	   */

	  pixels_diff =
	    constraints->minWidth + constraints->ipadx +
	    constraints->insets.xmin + constraints->insets.xmax;

	  for (k = constraints->tempX; k < px; k++)
	    pixels_diff -= r.minWidth[k];
	  if (pixels_diff > 0) {
	    weight = 0.0;
	    for (k = constraints->tempX; k < px; k++)
	      weight += r.weightX[k];
	    for (k = constraints->tempX; weight > 0.0 && k < px; k++) {
	      double wt = r.weightX[k];
	      int dx = (int)((wt * ((double)pixels_diff)) / weight);
	      r.minWidth[k] += dx;
	      pixels_diff -= dx;
	      weight -= wt;
	    }
	    /* Any leftovers go into the rightmost cell */
	    r.minWidth[px-1] += pixels_diff;
	  }
	}
	else if (constraints->tempWidth > i && constraints->tempWidth < nextSize)
	  nextSize = constraints->tempWidth;


	if (constraints->tempHeight == i) {
	  py = constraints->tempY + constraints->tempHeight; /* bottom row */

	  /*
	   * Figure out if we should use this child\'s weight.  If the weight
	   * is less than the total weight spanned by the height of the cell,
	   * then discard the weight.  Otherwise split it the difference
	   * according to the existing weights.
	   */

	  weight_diff = constraints->weighty;
	  for (k = constraints->tempY; k < py; k++)
	    weight_diff -= r.weightY[k];
	  if (weight_diff > 0.0) {
	    weight = 0.0;
	    for (k = constraints->tempY; k < py; k++)
	      weight += r.weightY[k];
	    for (k = constraints->tempY; weight > 0.0 && k < py; k++) {
	      double wt = r.weightY[k];
	      double dy = (wt * weight_diff) / weight;
	      r.weightY[k] += dy;
	      weight_diff -= dy;
	      weight -= wt;
	    }
	    /* Assign the remainder to the bottom cell */
	    r.weightY[py-1] += weight_diff;
	  }

	  /*
	   * Calculate the minHeight array values.
	   * First, figure out how tall the current child needs to be.
	   * Then, see if it will fit within the current minHeight values.
	   * If it will not fit, add the difference according to the
	   * weightY array.
	   */

	  pixels_diff =
	    constraints->minHeight + constraints->ipady +
	    constraints->insets.ymin + constraints->insets.ymax;
	  for (k = constraints->tempY; k < py; k++)
	    pixels_diff -= r.minHeight[k];
	  if (pixels_diff > 0) {
	    weight = 0.0;
	    for (k = constraints->tempY; k < py; k++)
	      weight += r.weightY[k];
	    for (k = constraints->tempY; weight > 0.0 && k < py; k++) {
	      double wt = r.weightY[k];
	      int dy = (int)((wt * ((double)pixels_diff)) / weight);
	      r.minHeight[k] += dy;
	      pixels_diff -= dy;
	      weight -= wt;
	    }
	    /* Any leftovers go into the bottom cell */
	    r.minHeight[py-1] += pixels_diff;
	  }
	}
	else if (constraints->tempHeight > i &&
		 constraints->tempHeight < nextSize)
	  nextSize = constraints->tempHeight;
      }
    }

    return r;
  
}


void 
awsGridBagLayout::AdjustForGravity(awsGridBagConstraints *constraints, csRect r) 
{
    int diffx, diffy;

    r.xmin += constraints->insets.xmin;
    r.xmax -= constraints->insets.xmax;
    r.ymin += constraints->insets.ymin;
    r.ymax -= constraints->insets.ymax;

    diffx = 0;
    if ((constraints->fill != awsGridBagConstraints::HORIZONTAL &&
	 constraints->fill != awsGridBagConstraints::BOTH)
	&& (r.Width() > (constraints->minWidth + constraints->ipadx))) 
    {
      diffx = r.Width() - (constraints->minWidth + constraints->ipadx);
      r.xmax = r.xmin + constraints->minWidth + constraints->ipadx;
    }

    diffy = 0;
    if ((constraints->fill != awsGridBagConstraints::VERTICAL &&
	 constraints->fill != awsGridBagConstraints::BOTH)
	&& (r.Height() > (constraints->minHeight + constraints->ipady))) 
    {
      diffy = r.Height() - (constraints->minHeight + constraints->ipady);
      r.ymax = r.ymin + constraints->minHeight + constraints->ipady;
    }

    switch (constraints->anchor) {
    case awsGridBagConstraints::CENTER:
      r.Move(diffx/2, diffx/2);
      break;
    case awsGridBagConstraints::NORTH:
      r.Move(diffx/2, 0);
      break;
    case awsGridBagConstraints::NORTHEAST:
      r.Move(diffx,0);
      break;
    case awsGridBagConstraints::EAST:
      r.Move(diffx, diffy/2);
      break;
    case awsGridBagConstraints::SOUTHEAST:
      r.Move(diffx, diffy);
      break;
    case awsGridBagConstraints::SOUTH:
      r.Move(diffx/2, diffy);
      break;
    case awsGridBagConstraints::SOUTHWEST:
      r.Move(0, diffy);
      break;
    case awsGridBagConstraints::WEST:
      r.Move(0,diffy/2);
      break;
    case awsGridBagConstraints::NORTHWEST:
      break;
    default:
      break;
    }
}

  
csRect 
awsGridBagLayout::GetMinSize(iAwsComponent *parent, GridBagLayoutInfo info) 
{
    csRect d;
    int i, t;
    //csRect insets = parent.getInsets();

    t = 0;
    for(i = 0; i < info.width; i++)
      t += info.minWidth[i];

    d.xmax = t; // + insets.left + insets.right;

    t = 0;
    for(i = 0; i < info.height; i++)
      t += info.minHeight[i];

    d.ymax = t; //+ insets.top + insets.bottom;

    return d;
}

void 
awsGridBagLayout::ArrangeGrid(iAwsComponent *parent) 
{
    iAwsComponent *cmp;
    int compindex;
    awsGridBagConstraints *constraints;
    csRect insets;
    csRect d;
    csRect r;
    int i, diffw, diffh;
    double weight;
    GridBagLayoutInfo info;

    /*
     * If the parent has no slaves anymore, then don't do anything
     * at all:  just leave the parent's size as-is.
     */
    if (parent->GetChildCount() == 0 &&
	(columnWidths == NULL || columnWidthsLength == 0) &&
	(rowHeights == NULL || rowHeightsLength == 0)) 
    {
      return;
    }

    /*
     * Pass #1: scan all the children to figure out the total amount
     * of space needed.
     */

    info = GetLayoutInfo(parent, PREFERREDSIZE);
    d = GetMinSize(parent, info);

    if (parent->Frame().Width() < d.Width() || 
        parent->Frame().Height() < d.Height()) 
    {
      info = GetLayoutInfo(parent, MINSIZE);
      d = GetMinSize(parent, info);
    }

    if (layoutInfo) delete layoutInfo;
    
    layoutInfo = &info;
    
    r.xmax= d.xmax;
    r.ymax = d.xmax;
    
    /*
     * If the current dimensions of the window don't match the desired
     * dimensions, then adjust the minWidth and minHeight arrays
     * according to the weights.
     */

    diffw = parent->Frame().Width() - r.Width();
    if (diffw != 0) {
      weight = 0.0;
      for (i = 0; i < info.width; i++)
	weight += info.weightX[i];
      if (weight > 0.0) {
	for (i = 0; i < info.width; i++) {
	  int dx = (int)(( ((double)diffw) * info.weightX[i]) / weight);
	  info.minWidth[i] += dx;
	  r.xmax += dx;
	  if (info.minWidth[i] < 0) {
	    r.xmax -= info.minWidth[i];
	    info.minWidth[i] = 0;
	  }
	}
      }
      diffw = parent->Frame().Width() - r.Width();
    }
    else {
      diffw = 0;
    }

    diffh = parent->Frame().Height() - r.Height();
    if (diffh != 0) {
      weight = 0.0;
      for (i = 0; i < info.height; i++)
	weight += info.weightY[i];
      if (weight > 0.0) {
	for (i = 0; i < info.height; i++) {
	  int dy = (int)(( ((double)diffh) * info.weightY[i]) / weight);
	  info.minHeight[i] += dy;
	  r.ymax += dy;
	  if (info.minHeight[i] < 0) {
	    r.ymax -= info.minHeight[i];
	    info.minHeight[i] = 0;
	  }
	}
      }
      diffh = parent->Frame().Height() - r.Height();
    }
    else {
      diffh = 0;
    }
    
    /*
     * Now do the actual layout of the slaves using the layout information
     * that has been collected.
     */

    info.startx = diffw/2 + insets.xmin;
    info.starty = diffh/2 + insets.ymin;

    for (compindex = 0 ; compindex < parent->GetChildCount(); ++compindex) {
      cmp = parent->GetChildAt(compindex);
      if (cmp->isHidden()) continue;

      constraints = lookupConstraints(cmp);

      r.xmin = info.startx;
      for(i = 0; i < constraints->tempX; i++)
	r.Move(info.minWidth[i],0);

      r.ymin = info.starty;
      for(i = 0; i < constraints->tempY; i++)
	r.Move(0,info.minHeight[i]);

      r.SetSize(0,0);
      for(i = constraints->tempX;
	  i < (constraints->tempX + constraints->tempWidth);
	  i++)
	    {
	r.xmax += info.minWidth[i];
      }
      
      for(i = constraints->tempY;
	  i < (constraints->tempY + constraints->tempHeight);
	  i++)
	  {
	r.ymax += info.minHeight[i];
      }

      AdjustForGravity(constraints, r);

      /*
       * When is a window ever too small to be interesting in AWS?
       * Not sure, but we don't have an unmap paradigm, so we just make
       * it really, really small.  I need to add a pseudo flag which means
       * that the component is too small to be seen but not actually hidden.
       */

      if ((r.Width() <= 0) || (r.Height() <= 0)) 
      {
	cmp->Frame().MakeEmpty();
      }
      else 
      {
	if (cmp->Frame().xmin != r.xmin || 
	    cmp->Frame().ymin != r.ymin ||
	    cmp->Frame().Width() != r.Width() || 
	    cmp->Frame().Height() != r.Height()) 
	{
	    cmp->Frame().Set(r);
	}
      }
    }
  }
