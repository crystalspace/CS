#ifndef __AWS_WINDOW_H__
# define __AWS_WINDOW_H__

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
# include "iaws/aws.h"
# include "iutil/eventh.h"
# include "iutil/comp.h"
# include "csgeom/csrect.h"
# include "csgeom/csrectrg.h"
# include "awscomp.h"
# include "awsPanel.h"
# include "awscmdbt.h"
# include "awsMenu.h"

/***************************************************************************************************************************
*   This file details the interface of awsWindow awsComponents.  Windows, while they are just normal awsComponents, have some    *
* special properties that most other awsComponents will not have.  For example, windows are moveable, hideable, maximizeable, *
* windows can be minimized, closed, and they can sit on top of or below other windows.  Windows, while being peers, also   *
* have a depth ordering that is implicit in their hierarchy.                                                               *
*                                                                                                                          *
***************************************************************************************************************************/
class awsWindow : 
  public awsPanel
{
private:

  /// Individual frame options
  int frame_options;

  /// The size of the title bar as of last draw
  int title_bar_height;

  /// The title text offset
  int title_offset;

  /// The title
  iString *title;

  /// the title bar text color
  int title_text_color;

  /// Active and inactive title bar colors
  /// They are stored as rgb triples in order active1, active2, inactive1, inactive2
  /// because there doesn't seem to be a way to convert from pallete
  /// back to rgb we have to store the rgb values
  unsigned char title_color[12];

  /// The position where the mouse went down
  int down_x, down_y;

  /// multi-purpose: stores the original coordinates of Frame().xmin, Frame().ymin while moving
  /// or stores the original width and height while resizing
  int orig_x, orig_y;

  /// True if we are currently resizing
  bool resizing_mode;

  /// True if we are currently moving
  bool moving_mode;

  /// control buttons
  awsCmdButton min_button, zoom_button, close_button;

  /// slots and a sink for getting events from the buttons
  awsSlot slot_min, slot_zoom, slot_close;
  awsSink sink;

  /// True if the window is in one of these various states
  bool is_minimized;

  /// a popup menu we can show when right clicked
  awsPopupMenu* popup;

  /// a menu currently showing
  awsMenuBar* menu;

protected:

  /// Returns true if this is the topmost window
  bool IsActiveWindow();


  void DrawGradient(csRect frame, unsigned char r1, unsigned char g1, unsigned char b1,
	                              unsigned char r2, unsigned char g2, unsigned char b2);

  /// Sets the option flags
  virtual void SetOptions(int frame_options);

  /// Resizes the window and all associated items
  virtual void Resize(int width, int height);

public:
  static const unsigned long sWindowRaised;
  static const unsigned long sWindowLowered;
  static const unsigned long sWindowHidden;
  static const unsigned long sWindowShown;
  static const unsigned long sWindowClosed;
  static const unsigned long sWindowZoomed;
  static const unsigned long sWindowMinimized;

  /******* Frame Options **********************/

  /// Should draw control box
  static const int foControl;

  /// Should draw zoom (maximize) button
  static const int foZoom;

  /// Should draw minimize button
  static const int foMin;

  /// Should draw close button
  static const int foClose;

  /// Should draw title
  static const int foTitle;

  /// Should draw grip
  static const int foGrip;

  /// Should draw round border (default)
  static const int foRoundBorder;

  /// Should draw beveled border
  static const int foBeveledBorder;

  /// Should draw no border
  static const int foNoBorder;

  SCF_DECLARE_IBASE_EXT(awsComponent);

  /// This is a awsComponent of type window
  virtual const char *Type ()
  { return "Window"; }

public:
  /// Constructs window class, clear some variables to defaults
  awsWindow ();

  /// empty deconstructor
  virtual ~awsWindow ();

public:

  /// Does some additional setup for windows, including linking into the window hierarchy.
  virtual bool Setup (iAws *_wmgr, iAwsComponentNode *settings);

  /// Gets properties for this window
  bool GetProperty (const char *name, void **parm);

  /// Sets properties for this window
  bool SetProperty (const char *name, void *parm);

  /// Executes scriptable actions for this window
  bool Execute (const char *action, iAwsParmList* parmlist);

  /// Gets the preferred size of the awsComponent
  virtual csRect getPreferredSize ();

  /// Gets the minimum size that the awsComponent can be
  virtual csRect getMinimumSize ();

  /// Gets the inset amounts that are need to fit awsComponents properly.
  virtual csRect getInsets ();

  /// Returns true if the window is in the process of moving
  virtual bool IsMoving();

  virtual void SetMenu(awsMenuBar* menu);
  virtual awsMenuBar* GetMenu();

public:
  static void OnCloseClick(void* p, iAwsSource* source);
  static void OnZoomClick(void* p, iAwsSource* source);
  static void OnMinClick(void* p, iAwsSource* source);

public:
  /// Event triggered when a window is about to be raised
  virtual void OnRaise ();

  /// Event triggered when a window is about to be lowered
  virtual void OnLower ();

  /// Triggered when the awsComponent needs to draw
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when the user moves the mouse
  virtual bool OnMouseMove (int button, int x, int y);
};


class awsWindowFactory : public awsComponentFactory
{
public:
	awsWindowFactory(iAws *wmgr);
	~awsWindowFactory();

	iAwsComponent* Create();
};

#endif

