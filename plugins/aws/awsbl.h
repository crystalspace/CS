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

#ifndef __CS_AWS_BL_H__
#define __CS_AWS_BL_H__

#include "awslayot.h"

struct iAwsComponent;

/**
 * Lays out a class according to the Java AWT/Swing BorderLayout defintion.
 */
class awsBorderLayout : public awsLayoutManager
{
  /// Contains all components, there may be one or more missing.
  iAwsComponent *components[5];

  /// The horizontal gap for components.
  int hGap;

  /// The vertical gap for components.
  int vGap;
public:
  enum
  {
    /**
     * Put the component in the center of its display area.
     */
    GBS_CENTER = 0,

    /**
     * Put the component at the top of its display area,
     * centered horizontally. 
     */
    GBS_NORTH = 1,

    /**
     * Put the component on the right side of its display area, 
     * centered vertically.
     */
    GBS_EAST = 2,

    /**
     * Put the component at the bottom of its display area, centered 
     * horizontally. 
     */
    GBS_SOUTH = 3,

    /**
     * Put the component on the left side of its display area, 
     * centered vertically.
     */
    GBS_WEST = 4
  };
public:
  /// Constructor, clears all components to 0.
  awsBorderLayout (
    iAwsComponent *owner,
    iAwsComponentNode* settings,
    iAwsPrefManager *pm);

  /// Empty destructor.
  virtual ~awsBorderLayout () {}

  /**
   * Adds a component to the layout, returning it's actual rect. 
   */
  virtual csRect AddComponent (iAwsComponent *cmp, iAwsComponentNode* settings);

  /// Lays out components properly.
  virtual void LayoutComponents ();
};

#endif // __CS_AWS_BL_H__
