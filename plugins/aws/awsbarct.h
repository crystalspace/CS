#ifndef __AWS_BAR_CHART_H__
#define __AWS_BAR_CHART_H__

/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson

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
*****************************************************************************/
#include "awscomp.h"
#include "awstimer.h"

class awsBarChart :
  public awsComponent
{
  /// Flags for frame style.
  int frame_style;

  /// Flag for inner frame style.
  int inner_frame_style;

  /// Flags for chart options.
  int chart_options;

  /// Alpha level for this component
  int alpha_level;

  /// Handle for texture background
  iTextureHandle *bkg;

  /// Caption text for this component
  iString *caption;

  /// Text for "Y" legend
  iString *yText;

  /// Text for "X" legend
  iString *xText;

  /// Each item has a value and label.
  struct BarItem
  {
    float value;
    iString *label;
  };

  /// Keeps track of all chart items
  csBasicVector items;

  /// Maximum number of items to include in chart
  int max_items;

  /// Color for bars
  int bar_color;

  /// The timer that drives the update for the chart
  awsTimer *update_timer;


public:
  awsBarChart ();
  virtual ~awsBarChart();

  /******* Frame Styles **********************/

  /// A frame that's a bump
  static const int fsBump;

  /// A simple frame
  static const int fsSimple;

  /// A frame that looks like a sunk button
  static const int fsSunken;

  /// A frame that looks like a raised button
  static const int fsRaised;

  /// A frame that looks flat
  static const int fsFlat;

  /// no frame at all
  static const int fsNone;

  /******* Chart Options **********************/
  
  /** The chart rolls, so that when it has reached MaxItems width, the oldest item
   * in the chart falls off the edge and the new one is added to the other side.
   */
  static const int coRolling;

  /// Chart rolls left
  static const int coRollLeft;

  /// Chart rolls right
  static const int coRollRight;

  /// Chart has vertical gridlines
  static const int coVertGridLines;

  /// Chart has horizontal gridlines
  static const int coHorzGridLines;

  /// Chart is vertical rather than horizontal
  static const int coVerticalChart;


  /******* Signals **********************/

  /// An up and down motion for the button
  static const int signalClicked;

  /// A signal that tells the app it should update the rolling chart.
  static const int signalTimer;

public:
  /// Get's the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, awsComponentNode *settings);

  /// Gets properties
  bool GetProperty (char *name, void **parm);

  /// Sets properties
  bool SetProperty (char *name, void *parm);

  /// Performs "scripted" execution.
  bool Execute (char *action, iAwsParmList &parmlist);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual char *Type ();

  /// Gets the insets of the frame depending on what style it's in.
  virtual csRect getInsets();

public:
  SCF_DECLARE_IBASE;

  /// Triggered when the component needs to draw
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when the user moves the mouse
  virtual bool OnMouseMove (int button, int x, int y);

  /// Triggered when the user clicks the mouse
  virtual bool OnMouseClick (int button, int x, int y);

  /// Triggered when the user double clicks the mouse
  virtual bool OnMouseDoubleClick (int button, int x, int y);

  /// Triggered when this component loses mouse focus
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus
  virtual bool OnMouseEnter ();

  /// Triggered when the user presses a key
  virtual bool OnKeypress (int key, int modifiers);

  /// Triggered when the keyboard focus is lost
  virtual bool OnLostFocus ();

  /// Triggered when the keyboard focus is gained
  virtual bool OnGainFocus ();
};

class awsBarChartFactory :
  public awsComponentFactory
{
public:
  SCF_DECLARE_IBASE;

  /// Calls register to register the component that it builds with the window manager
  awsBarChartFactory (iAws *wmgr);

  /// Does nothing
  virtual ~awsBarChartFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};
#endif
