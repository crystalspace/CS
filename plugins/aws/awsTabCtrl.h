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

#ifndef __CS_AWS_TABCTRL_H__
#define __CS_AWS_TABCTRL_H__

#include "awscomp.h"
#include "awsscrbr.h"
#include "csutil/parray.h"

/**
 * This class implements a basic tab button on a tab control.
 */
 
class awsTab : public awsComponent
{
protected:
  /// Texture background, overlay and icon.
  iTextureHandle *tex[3];

  /// Does this button depict the active tab?
  bool is_active;

  /// Is this the first button that is drawn? Important for color selection.
  bool is_first;

  /// Does this bottom reside on the top of the notebook or below.
  bool is_top;

  /// Caption.
  csRef<iString> caption;

  /// Is the mouse currently held doen .
  bool captured;

  /// If theres an icon, how is it align.
  int icon_align;

  /// Alpha level for overlay texture if any.
  int alpha_level;

  /**
   * A parameter that the user can store with this tab. Often its helpful
   * as an identifier for which tab was pressed or what to do about it.
   */
  intptr_t user_param;

  // trigger event if needed
  bool HandleClick (int x, int y);
public:
  awsTab ();
  virtual ~awsTab ();

  virtual bool Setup (iAws *_wmgr, iAwsComponentNode *settings);
  virtual bool GetProperty (const char *name, intptr_t *parm);
  virtual bool SetProperty (const char *name, intptr_t parm);

  virtual void OnDraw (csRect clip);
  bool OnMouseDown (int, int, int);
  bool OnMouseUp (int, int x, int y);
  bool OnMouseClick (int, int x, int y);
  bool OnMouseDoubleClick (int, int x, int y);
  virtual csRect getMinimumSize ();

  virtual const char *Type () { return "Tab Button"; }

  void SetActive (bool what);
  void SetFirst (bool what) { is_first = what; }
  void SetTop (bool what) { is_top = what; }

  enum  {
    signalActivateTab = 1,
    signalDeactivateTab = 2,
    iconLeft = 0,
    iconRight = 1,
    iconTop = 2,
    iconBottom = 3
  };
};

/**
 * This class implements a simple tab control.
 *<p>
 * Basic usage is to insert tabs using the Add action, supplying a caption
 * and optional parameter. Add will return an iAwsSource* which you would
 * then connect however you like. When a tab is selcted/deselected a
 * sActivateTab/sDeactivateTab signal will be sent through the source you
 * received. Also from the source's component you can query the "User Param"
 * property to retrieve the parameter you passed in Add. Either the index,
 * the source, or the parameter can be used to identify tabs in other
 * method calls. However indexes can change by adding/removing tabs, and it
 * is your responsibilty to ensure the user_param is unique if you desire
 * to use it.
 */

class awsTabCtrl : public awsComponent  
{
protected:
  class TabVector : public csPDelArray<awsTab>
  {
  public:
    static int Compare (void const* Item1, void const* Item2)
    {
      awsTab *te1 = (awsTab *)Item1;
      awsTab *te2 = (awsTab *)Item2;
      return (te1 < te2 ? -1 : te1 > te2 ? 1 : 0);
    }
  };

  TabVector vTabs;

  /// First visible button.
  int first;
  
  /// The active tab.
  int active;
  
  /// Button bar on top of notebook or below?
  bool is_top;

  /// True if the client area should be shrunk because of the scroll buttons.
  bool clip_to_scroll;

  /// Our kitchen sink.
  awsSink *sink;

  /// Max height of buttons in bar.
  int maxheight;

  /// The two "next/prev" buttons.
  awsSliderButton next, prev;

  /// Slots for the next/prev buttons.
  awsSlot slot_activate, slot_next, slot_prev;

  /// Images for next/prev buttons.
  iTextureHandle *nextimg, *previmg;

  /// Layout the buttons, hide or show them, align the next/prev handles.
  void DoLayout ();

  /// Finds the index of the first tab which uses this param.
  int FindTab (intptr_t user_param);
public:
  awsTabCtrl ();
  virtual ~awsTabCtrl ();

  virtual void OnDraw (csRect clip);
  virtual bool Setup (iAws *_wmgr, iAwsComponentNode *settings);
  virtual const char *Type () { return "Simple Tab Control"; }

  virtual csRect getInsets ();
  virtual csRect getPreferredSize ();

  /**
   * This will create a button based on the caption. This source will fire
   * sActivateTab and sDeactivateTab. The parameter can also be retrieved
   * from the source component by querying it's "User Param" property. If
   * this component is the first that has been added becomes the active one.
   */
  iAwsSource* AddTab (iString* caption, intptr_t user_param = 0);

  /**
   * This will remove the tab at index. The next tab will become active
   * (or the prev if no next exist) if this was the active one.
   */
  void RemoveTabIndex (int index);

  /**
   * Remove the tab that uses this source. The next tab will become active
   * (or the prev if no next exist) if this was the active one.
   */
  void RemoveTab (iAwsSource* src);

  /**
   * Remove the tab that has this user param. The next tab will become
   * active (or the prev if no next exist) if this was the active one.
   */
  void RemoveTabParam (intptr_t user_param);

  /// Activate the <idx>-th tab.
  void ActivateTabIndex (int idx);
  /// Activate the tab that uses this src.
  void ActivateTab (iAwsSource* src);
  /// Activate the tab that uses this user_param.
  void ActivateTabParam (intptr_t param);

  /// Returns the source for the currently active tab.
  iAwsSource* GetActiveTab ();
  /// Returns the index of the currently active tab. 
  int GetActiveTabIndex ();
  /// Returns the user_param of the currently active tab.
  intptr_t GetActiveTabParam (); 

  /// Scroll list of button left.
  void ScrollLeft ();
  /// Scroll list of button right.
  void ScrollRight ();

  /// Scroll buttons until the <idx>-th becomes visible.
  void MakeVisible (int idx);

  virtual void OnResized ();

  static void ActivateTabCallback (intptr_t sk, iAwsSource *source);

  static void PrevClicked (intptr_t sk, iAwsSource *source);
  static void NextClicked (intptr_t sk, iAwsSource *source);

  static const int HandleSize;

  /// Show buttonbar at top or bottom.
  void SetTopBottom (bool to_top);
};

#endif // __CS_AWS_TABCTRL_H__
