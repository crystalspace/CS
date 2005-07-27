/*
    Copyright (C) 2001 by Christopher Nelson

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

#ifndef __CS_AWS_GBL_H__
#define __CS_AWS_GBL_H__

#include "iaws/aws.h"
#include "awslayot.h"
#include "awsgbc.h"
#include "csutil/hash.h"
#include "awscomp.h"

/**
 * GridBag layout ala Java AWT/Swing.  It tries to stay as close as
 * possible to the original within the limits of AWS's grammar.
 */
 
class awsGridBagLayout : public awsLayoutManager
{
public:
  /// Class constants.
  enum
  {
    /**
     * The maximum number of grid positions (both horizontally and
     * vertically) that can be laid out by the grid bag layout.
     */
    MAXGRIDSIZE = 512,

    /// The smallest grid that can be laid out by the grid bag layout.
    MINSIZE = 1,

    /// Preferred size of grid.
    PREFERREDSIZE = 2
  };

private:
  struct GridBagLayoutInfo
  {
    /// Number of cells horizontally, vertically.
    int width, height;

    /// Starting point for layout.
    int startx, starty;

    /// Largest minWidth in each column.
    int *minWidth;

    /// Largest minHeight in each row.
    int *minHeight;

    /// Largest weight in each column.
    double *weightX;

    /// Largest weight in each row.
    double *weightY;

    /// Builds a new default layout.
    GridBagLayoutInfo ();

    /// Destroy this layout properly.
    ~GridBagLayoutInfo ();

    /// Shallow copy.
    void Set (GridBagLayoutInfo &l2);

    /// Deep copy.
    GridBagLayoutInfo *Clone ();
  };

  /**
   * This hash table maintains the association between a component and
   * its gridbag constraints. The Keys in comptable are the components
   * and thevalues are the instances of GridBagConstraints.
   */
  csHash<awsGridBagConstraints, csPtrKey<iAwsComponent> > comptable;

  /**
   * This field holds a gridbag constraints instance containing the
   * default values, so if a component does not have gridbag constraints
   * associated with it, then the component will be assigned a copy of
   * the <code>defaultConstraints</code>.
   */
  awsGridBagConstraints defaultConstraints;

  /**
   * This field holds tha layout information for the gridbag. The
   * information in this field is based on the most recent validation
   * of the gridbag.
   *<p>
   * If <code>layoutInfo</code> is <code>null</code> this indicates that
   * there are no components in the gridbag or if there are components,
   * they have not yet been validated.
   */
  GridBagLayoutInfo *layoutInfo;

  /**
   * This field holds the overrides to the column minimum width. If this
   * field is non-null the values are applied to the gridbag after all
   * of the minimum columns widths have been calculated. If columnWidths
   * has more elements than the number of columns, columns are added to
   * the gridbag to match the number of elements in columnWidth.
   */
  int *columnWidths;

  /// length of column widths.
  int columnWidthsLength;

  /**
   * This field holds the overrides to the row minimum heights. If this
   * field is non-null the values are applied to the gridbag after all
   * of the minimum row heights have been calculated. If rowHeights has
   * more elements than the number of rows, rowa are added to the gridbag
   * to match the number of elements in rowHeights.
   */
  int* rowHeights;

  /// Length of column widths.
  int rowHeightsLength;

  /**
   * This field holds the overrides to the column weights. If this field
   * is non-null the values are applied to the gridbag after all of the
   * columns weights have been calculated. If columnWeights[i] &gt; weight
   * for column i, then column i is assigned the weight in columnWeights[i].
   * If columnWeights has more elements than the number of columns, the
   * excess elements are ignored - they do not cause more columns to be
   * created.
	 */
  double *columnWeights;

  /// Length of columnWeigths.
  int columnWeightsLength;

  /**
   * This field holds the overrides to the row weights. If this field is
   * non-null the values are applied to the gridbag after all of the rows
   * weights have been calculated. If rowWeights[i] &gt; weight for
   * row i, then row i is assigned the weight in rowWeights[i]. If
   * rowWeights has more elements than the number of rows, the excess
   * elements are ignored - they do not cause more rows to be created.
   */
  double* rowWeights;

  /// Length of rowWeights.
  int rowWeightsLength;

  /// The components that are in this layout
  awsComponentVector comps;
public:
  /**
   * Constructs a gridbag layout.  Note that columns and rows have a
   * fixed percentage size that does not change.
	 */
  awsGridBagLayout (
    iAwsComponent *o,
    iAwsComponentNode* settings,
    iAwsPrefManager *pm);

  /// Adds a component into this GridBagLayout.
  virtual csRect AddComponent (iAwsComponent *cmp, iAwsComponentNode* settings);

  /// Lays out components properly.
  virtual void LayoutComponents ();

  /**
   * Sets the constraints for the specified component in this layout.
   * @param cmp - the component to be modified.
   * @param constraints - the constraints to be applied.
   */
  void setConstraints (
    iAwsComponent *cmp,
    awsGridBagConstraints &constraints);

  /**
   * Gets the constraints for the specified component. A copy of the
   * actual <code>GridBagConstraints</code> object is returned.
   * @param cmp - the component to be queried.
   * @return the constraint for the specified component in this
   * grid bag layout; a copy of the actual constraint object is returned.
   */
  awsGridBagConstraints getConstraints (iAwsComponent *cmp);

  /**
   * Retrieves the constraints for the specified component. The return
   * value is not a copy, but is the actual <code>GridBagConstraints</code>
   * object used by the layout mechanism.
   * @param cmp - the component to be queried
   * @return the contraints for the specified component.
   */
  awsGridBagConstraints *lookupConstraints (iAwsComponent *cmp);

  /**
   * Removes the constraints for the specified component in this layout
   * @param cmp - the component to be modified.
   */
  void removeConstraints (iAwsComponent *cmp);

  /**
   * Determines the origin of the layout grid. Most applications do not
   * call this method directly.
   * @return the origin of the cell in the top-left corner of the layout grid.
   */
  csVector2 getLayoutOrigin ();

  /**
   * Determines column widths and row heights for the layout grid.
   * <p>
   * Most applications do not call this method directly.
   * @return an array of two arrays, containing the widths of the
   * layout columns and the heights of the layout rows.
   */
  void getLayoutDimensions (int **row, int **col);

  /**
   * Determines the weights of the layout grid's columns and rows. Weights
   * are used to calculate how much a given column or row stretches beyond
   * its preferred size, if the layout has extra room to fill.
   * <p>
   * Most applications do not call this method directly.
   * @return an array of two arrays, representing the horizontal weights
   * of the layout columns and the vertical weights of the layout rows.
   */
  void getLayoutWeights (double **row, double **col);

  /**
   * Determines which cell in the layout grid contains the point
   * specified by <code>(x,&nbsp;y)</code>. Each cell is identified
   * by its column index (ranging from 0 to the number of columns minus 1)
   * and its row index (ranging from 0 to the number of rows minus 1).
   * <p>
   * If the <code>(x,&nbsp;y)</code> point lies outside the grid, the
   * following rules are used. The column index is returned as zero if
   * <code>x</code> lies to the left of the layout, and as the number of
   * columns if <code>x</code> lies to the right of the layout. The row
   * index is returned as zero if <code>y</code> lies above the layout,
   * and as the number of rows if <code>y</code> lies below the layout.
   * @param x - the <i>x</i> coordinate of a point.
   * @param y - the <i>y</i> coordinate of a point.
   * @return an ordered pair of indexes that indicate which cell in the
   * layout grid contains the point (<i>x</i>,&nbsp;<i>y</i>).
   */
  csVector2 location (int x, int y);

  /**
   * Adds the specified component to the layout, using the specified
   * constraint object.
   * @param cmp - the component to be added.
   * @param constraints - an object that determines how the component
   * is added to the layout.
   */
  void addLayoutComponent (
    iAwsComponent *cmp,
    awsGridBagConstraints &constraints);

  /**
   * Removes the specified component from this layout.
   * <p>
   * Most applications do not call this method directly.
   * @param cmp - the component to be removed.
   */
  void removeLayoutComponent (iAwsComponent *cmp);
protected:
  /**
   * Fill in an instance of the above structure for the current set
   * of managed children.  This requires three passes through the
   * set of children:
   *<p>
   * 1) Figure out the dimensions of the layout grid
   * 2) Determine which cells the components occupy
   * 3) Distribute the weights and min sizes amoung the rows/columns.
   *<p>
   * This also caches the minsizes for all the children when they are
   * first encountered (so subsequent loops don't need to ask again).
   */
  GridBagLayoutInfo *GetLayoutInfo (iAwsComponent *parent, int sizeflag);

  /**
   * Adjusts the x, y, width, and height fields to the correct
   * values depending on the constraint geometry and pads.
   */
  void AdjustForGravity (awsGridBagConstraints *constraints, csRect r);

  /**
   * Figure out the minimum size of the master based on the
   * information from GetLayoutInfo ().
   */
  csRect GetMinSize (iAwsComponent *parent, GridBagLayoutInfo *info);

  /**
   * Layout the grid.
   */
  void ArrangeGrid (iAwsComponent *parent);
};

#endif // __CS_AWS_GBL_H__
