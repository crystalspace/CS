/*
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
*/

#ifndef __CS_AWS_BARCT_H__
#define __CS_AWS_BARCT_H__

#include "awsPanel.h"
#include "awstimer.h"

class awsBarChart : public awsPanel
{
private:
  /// Flag for inner frame style.
  int inner_frame_style;

  /// Flags for chart options.
  int chart_options;

  /// Caption text for this component
  csRef<iString> caption;

  /// Text for "Y" legend.
  csRef<iString> yText;

  /// Text for "X" legend.
  csRef<iString> xText;

  /// Each item has a value and label.
  struct BarItem
  {
    float value;
    iString *label;
  };

  /// Keeps track of all chart items (fifo buffer).
  BarItem *items;

  /// Number of items in chart.
  int count_items;

  /// Size of items buffer.
  int items_buffer_size;

  /// Maximum number of items to include in chart.
  int max_items;

  /// Color for bars.
  int bar_color;

  /// The timer that drives the update for the chart.
  awsTimer *update_timer;
public:
  awsBarChart ();
  virtual ~awsBarChart ();
  
  /**
   * The chart rolls, so that when it has reached MaxItems width, the
   * oldest item in the chart falls off the edge and the new one is added
   * to the other side.
   */
  static const int coRolling;

  /// Chart rolls left.
  static const int coRollLeft;

  /// Chart rolls right.
  static const int coRollRight;

  /// Chart has vertical gridlines.
  static const int coVertGridLines;

  /// Chart has horizontal gridlines.
  static const int coHorzGridLines;

  /// Chart is vertical rather than horizontal.
  static const int coVerticalChart;

  /// An up and down motion for the button.
  static const int signalClicked;

  /// A signal that tells the app it should update the rolling chart.
  static const int signalTimer;

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Gets properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Sets properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Performs "scripted" execution.
  bool Execute (const char *action, iAwsParmList* parmlist);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

protected:
  /// Add a new item to the top.
  void Push (BarItem &i, bool normal = true);

  /// Kill an item from the bottom .
  void Pop (bool normal = true);
};

class awsBarChartFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with the
   * window manager.
   */
  awsBarChartFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsBarChartFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_BARCT_H__
