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

#ifndef __CS_AWS_CONTROLBAR_H__
#define __CS_AWS_CONTROLBAR_H__

#include "awscomp.h"
#include "awsPanel.h"

/**
 * This is the base class for a number of different component types. A
 * control bar is just a simple component that has any number of any type
 * of component in it, layed out in a row.
 */
 
class awsControlBar : public awsPanel
{
private:
  /// The components in the bar.
  awsComponentVector comps;

  /// Spacing desired.
  int hGap, vGap;

  /// Vertical alignment.
  int vert_align;

  /**
   * Returns true if the control bar should automatically resize its
   * width to fit all components.
   */
  bool size_to_fit_horz;

  /**
   * Returns true if the control bar should automatically resize its
   * height to fit all components.
   */
  bool size_to_fit_vert;

  /**
   * Returns true if the components are to be layed out in a column
   * rather than row.
   */
  bool vertical;

  /**
   * Returns true if items should be stretched perpendicular to the
   * row direction.
   */
  bool stretch_items;

  /// Layout the child components and resize this component if necessary.
  void DoLayout ();
public:
  awsControlBar ();
  virtual ~awsControlBar ();

  virtual bool Setup (iAws *_wmgr, iAwsComponentNode *settings);
  virtual const char *Type () { return "Control Bar"; }

  bool Execute (const char* action, iAwsParmList *parmlist);

  /// Adds a component to the bar.
  void AddChild (iAwsComponent *comp);

  /// This will remove the component from the bar.
  void RemoveChild (iAwsComponent *comp);

  /**
   * Returns true if the component resizes width to fit the controls
   * when they change.
   */
  bool GetSizeToFitHorz ();

  /**
   * Returns true if the component resizes height to fit the controls
   * when they change.
   */
  bool GetSizeToFitVert ();

  /**
   * Sets whether the component resizes width to fit the controls
   * when they change.
   */
  void SetSizeToFitHorz (bool b);

  /**
   * Sets whether the component resizes height to fit the controls
   * when they change.
   */
  void SetSizeToFitVert (bool b);

  /// Resizes the component to fit the controls snuggly.
  void SizeToFit ();

  /// Resizes the component to fit the width of the controls snuggly.
  void SizeToFitHorz ();

  /// Resizes the component to fit the height of the controls snuggly.
  void SizeToFitVert ();

  bool GetVertical ();
  void SetVertical (bool vertical);

  int GetAlignment ();
  void SetAlignment (int alignment);

  int GetHorzGap ();
  void SetHorzGap (int gap);

  int GetVertGap ();
  void SetVertGap (int gap);

  bool GetStretchComponents ();
  void SetStretchComponents (bool stretch);

  static const int alignTop;
  static const int alignBottom;
  static const int alignCenter;
};

class awsControlBarFactory : public awsComponentFactory
{
public:
  awsControlBarFactory (iAws* _wmgr);
  
  iAwsComponent* Create ();
};

#endif // __CS_AWS_CONTROLBAR_H__
