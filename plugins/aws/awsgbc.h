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

#ifndef __CS_AWS_GBC_H__
#define __CS_AWS_GBC_H__

struct awsGridBagConstraints
{
  enum
  {
    GBS_RELATIVE = -1,

    /**
     * Specify that this component is the 
     * last component in its column or row. 
     */
    GBS_REMAINDER = 0,

    /**
     * Do not resize the component. 
     */
    GBS_NONE = 0,

    /**
     * Resize the component both horizontally and vertically. 
     */
    GBS_BOTH = 1,

    /**
     * Resize the component horizontally but not vertically. 
     */
    GBS_HORIZONTAL = 2,

    /**
     * Resize the component vertically but not horizontally. 
     */
    GBS_VERTICAL = 3,

    /**
     * Put the component in the center of its display area.
     */
    GBS_CENTER = 10,

    /**
     * Put the component at the top of its display area,
     * centered horizontally. 
     */
    GBS_NORTH = 11,

    /**
     * Put the component at the top-right corner of its display area. 
     */
    GBS_NORTHEAST = 12,

    /**
     * Put the component on the right side of its display area, 
     * centered vertically.
     */
    GBS_EAST = 13,

    /**
     * Put the component at the bottom-right corner of its display area. 
     */
    GBS_SOUTHEAST = 14,

    /**
     * Put the component at the bottom of its display area, centered 
     * horizontally. 
     */
    GBS_SOUTH = 15,

    /**
     * Put the component at the bottom-left corner of its display area. 
     */
    GBS_SOUTHWEST = 16,

    /**
     * Put the component on the left side of its display area, 
     * centered vertically.
     */
    GBS_WEST = 17,

    /**
     * Put the component at the top-left corner of its display area. 
     */
    GBS_NORTHWEST = 18
  };

  /**
   * Specifies the cell at the left of the component's display area, 
   * where the leftmost cell has <code>gridx=0</code>. The value 
   * <code>RELATIVE</code> specifies that the component be placed just 
   * to the right of the component that was added to the container just 
   * before this component was added. 
   * <p>
   * The default value is <code>RELATIVE</code>. 
   * gridx should be a non-negative value.
   */
  int gridx;

  /**
   * Specifies the cell at the top of the component's display area, 
   * where the topmost cell has <code>gridy=0</code>. The value 
   * <code>RELATIVE</code> specifies that the component be placed just 
   * below the component that was added to the container just before 
   * this component was added. 
   * <p>
   * The default value is <code>RELATIVE</code>.
   * gridy should be a non-negative value.
   */
  int gridy;

  /**
   * Specifies the number of cells in a row for the component's 
   * display area. 
   * <p>
   * Use <code>REMAINDER</code> to specify that the component be the 
   * last one in its row. Use <code>RELATIVE</code> to specify that the 
   * component be the next-to-last one in its row. 
   * <p>
   * gridwidth should be non-negative and the default value is 1.
   */
  int gridwidth;

  /**
   * Specifies the number of cells in a column for the component's 
   * display area. 
   * <p>
   * Use <code>REMAINDER</code> to specify that the component be the 
   * last one in its column. Use <code>RELATIVE</code> to specify that 
   * the component be the next-to-last one in its column. 
   * <p>
   * gridheight should be a non-negative value and the default value is 1.
   */
  int gridheight;

  /**
   * Specifies how to distribute extra horizontal space. 
   * <p>
   * The grid bag layout manager calculates the weight of a column to 
   * be the maximum <code>weightx</code> of all the components in a 
   * column. If the resulting layout is smaller horizontally than the area 
   * it needs to fill, the extra space is distributed to each column in 
   * proportion to its weight. A column that has a weight of zero receives 
   * no extra space. 
   * <p>
   * If all the weights are zero, all the extra space appears between 
   * the grids of the cell and the left and right edges. 
   * <p>
   * The default value of this field is <code>0</code>.
   * weightx should be a non-negative value.
   */
  double weightx;

  /**
   * Specifies how to distribute extra vertical space. 
   * <p>
   * The grid bag layout manager calculates the weight of a row to be 
   * the maximum <code>weighty</code> of all the components in a row. 
   * If the resulting layout is smaller vertically than the area it 
   * needs to fill, the extra space is distributed to each row in 
   * proportion to its weight. A row that has a weight of zero receives no 
   * extra space. 
   * <p>
   * If all the weights are zero, all the extra space appears between 
   * the grids of the cell and the top and bottom edges. 
   * <p>
   * The default value of this field is <code>0</code>. 
   * weighty should be a non-negative value.
   */
  double weighty;

  /**
   * This field is used when the component is smaller than its display 
   * area. It determines where, within the display area, to place the 
   * component. Possible values are <code>CENTER</code>, 
   * <code>NORTH</code>, <code>NORTHEAST</code>, <code>EAST</code>, 
   * <code>SOUTHEAST</code>, <code>SOUTH</code>, <code>SOUTHWEST</code>, 
   * <code>WEST</code>, and <code>NORTHWEST</code>.
   * The default value is <code>CENTER</code>. 
   */
  int anchor;

  /**
   * This field is used when the component's display area is larger 
   * than the component's requested size. It determines whether to 
   * resize the component, and if so, how. 
   * <p>
   * The following values are valid for <code>fill</code>: 
   * <p>
   * <ul>
   * <li>
   * <code>NONE</code>: Do not resize the component. 
   * <li>
   * <code>HORIZONTAL</code>: Make the component wide enough to fill 
   * its display area horizontally, but do not change its height. 
   * <li>
   * <code>VERTICAL</code>: Make the component tall enough to fill its 
   * display area vertically, but do not change its width. 
   * <li>
   * <code>BOTH</code>: Make the component fill its display area entirely. 
   * </ul>
   * <p>
   * The default value is <code>NONE</code>. 
   */
  int fill;

  /**
   * This field specifies the external padding of the component, the 
   * minimum amount of space between the component and the edges of its 
   * display area. 
   * <p>
   * The default value is <code>new Insets(0, 0, 0, 0)</code>. 
   */
  csRect insets;

  /**
   * This field specifies the internal padding of the component, how much 
   * space to add to the minimum width of the component. The width of 
   * the component is at least its minimum width plus 
   * <code>(ipadx&nbsp;*&nbsp;2)</code> pixels. 
   * <p>
   * The default value is <code>0</code>. 
   */
  int ipadx;

  /**
   * This field specifies the internal padding, that is, how much 
   * space to add to the minimum height of the component. The height of 
   * the component is at least its minimum height plus 
   * <code>(ipady&nbsp;*&nbsp;2)</code> pixels. 
   * <p>
   * The default value is 0. 
   */
  int ipady;

  /**
   * Temporary place holder for the x coordinate.
   */
  int tempX;

  /**
   * Temporary place holder for the y coordinate.
   * @serial
   */
  int tempY;

  /**
   * Temporary place holder for the Width of the component.
   */
  int tempWidth;

  /**
   * Temporary place holder for the Height of the component.
   */
  int tempHeight;

  /**
   * The minimum width of the component.  It is used to calculate
   * <code>ipady</code>, where the default will be 0.
   */
  int minWidth;

  /**
   * The minimum height of the component. It is used to calculate
   * <code>ipadx</code>, where the default will be 0.
   */
  int minHeight;

  /**
   * Creates a <code>GridBagConstraint</code> object with 
   * all of its fields set to their default value. 
   */
  awsGridBagConstraints ();

  /**
   * Creates a <code>GridBagConstraints</code> object with all of its
   * fields set to the passed-in arguments.
   */
  awsGridBagConstraints (
    int gridx,
    int gridy,
    int gridwidth,
    int gridheight,
    double weightx,
    double weighty,
    int anchor,
    int fill,
    csRect insets,
    int ipadx,
    int ipady);

  awsGridBagConstraints *Clone ();
};

#endif // __CS_AWS_GBC_H__
