#ifndef __AWS_SCROLL_BAR_H__
#define __AWS_SCROLL_BAR_H__
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
#include "awscmdbt.h"
#include "awstimer.h"

class awsSliderButton;

class awsScrollBar : public awsComponent
{
  /// True when button is down, false if up
  bool is_down;

  /// True if the component has the mouse over it
  bool mouse_is_over;

  /// True if button was down, and button is in switch mode (toggle=yes)
  bool was_down;

  /// Holds the texture handle for the background
  iTextureHandle *tex;

  /// Holds the texture handle for the dec arrow
  iTextureHandle *decimg;

  /// Holds the texture handle for the inc arrow
  iTextureHandle *incimg;

  /// Flags for frame style.
  int frame_style;

  /// Alpha level for this component
  int alpha_level;

  /// Button for up or left
  awsSliderButton *decVal;

  /// Button for down or right
  awsSliderButton *incVal;

  /// the knob
  awsSliderButton *knob;

  /// timer
  awsTimer *timer;

  /// The sink for button messages
  iAwsSink *sink;

  /// The slot for decrement button messages
  iAwsSlot *dec_slot;

  /// The slot for increment button messages
  iAwsSlot *inc_slot;

  /// The slot for knob button messages
  iAwsSlot *knob_slot;

  iAwsSlot *tick_slot;

  int last_x, last_y;

  /// Value of scroll bar
  float value;

  /// Maximum value scroll can attain
  float max;

  /// Minimum value scroll can attain
  float min;

  /// Amount of total visible for proportional scroll bars, set to 0 turns off proportional drawing
  float amntvis;

  /// Amount to move scroll bar when inc/dec buttons clicked
  float value_delta;

  /// Amount to move scroll bar when scroll area clicked
  float value_page_delta;

protected:

  bool awsScrollBar::HandleClicking (int btn,int x,int y);
  bool captured;
  public:
  awsScrollBar();
  virtual ~awsScrollBar();

  /******** Frame styles ***********/

  static const int fsVertical;
  static const int fsHorizontal;

  /******* Signals **********************/

  /// The value of the scroll bar changed
  static const int signalChanged;

  /// Trigger called when inc button is clicked
  static void IncClicked(void *sk, iAwsSource *source);

  /// Trigger called when dec button is clicked
  static void DecClicked(void *sk, iAwsSource *source);

  static void TickTock(void *sk, iAwsSource *source);
  static void KnobTick(void *sk, iAwsSource *source);

  /// Trigger called when area above/left of knob button is clicked
  static void DecPageClicked(void *sk, iAwsSource *source);

  /// Trigger called when area below/right of knob button is clicked
  static void IncPageClicked(void *sk, iAwsSource *source);

public:
  /// Get's the texture handle and the title, plus style if there is one.
  virtual bool Setup(iAws *wmgr, awsComponentNode *settings);

  /// Gets properties
  bool GetProperty(char *name, void **parm);

  /// Sets properties
  bool SetProperty(char *name, void *parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual char *Type();

public:
  SCF_DECLARE_IBASE;

  /// Triggered when the component needs to draw
  virtual void OnDraw(csRect clip);

  /// Triggered when the user presses a mouse button down
  virtual bool OnMouseDown(int button, int x, int y);

  /// Triggered when the user unpresses a mouse button
  virtual bool OnMouseUp(int button, int x, int y);

  /// Triggered when the user moves the mouse
  virtual bool OnMouseMove(int button, int x, int y);

  /// Triggered when the user clicks the mouse
  virtual bool OnMouseClick(int button, int x, int y);

  /// Triggered when the user double clicks the mouse
  virtual bool OnMouseDoubleClick(int button, int x, int y);

  /// Triggered when this component loses mouse focus
  virtual bool OnMouseExit();

  /// Triggered when this component gains mouse focus
  virtual bool OnMouseEnter();

  /// Triggered when the user presses a key
  virtual bool OnKeypress(int key, int modifiers);

  /// Triggered when the keyboard focus is lost
  virtual bool OnLostFocus();

  /// Triggered when the keyboard focus is gained
  virtual bool OnGainFocus();

  /// Adds in the inc and dec buttons appropriately
  virtual void OnAdded();
};

class awsScrollBarFactory : public awsComponentFactory
{
public:
  SCF_DECLARE_IBASE;

  /// Calls register to register the component that it builds with the window manager
  awsScrollBarFactory(iAws *wmgr);

  /// Does nothing
  virtual ~awsScrollBarFactory();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create();
};

class awsSliderButton : public awsCmdButton
{
 protected:

  // the timer we use for repeated firing of "clicked" signal
  awsTimer *timer;
  // signals if we have captured the mouse
  bool captured;
  // fire the signal every nTick milliseconds
  csTicks nTicks;

  iAwsSink *sink;
  iAwsSlot *tick_slot;

  /**************** signal handlers ************************/
  static void TickTock(void *sk, iAwsSource *);
  
 public:
  SCF_DECLARE_IBASE_EXT (awsCmdButton);

  awsSliderButton();
  virtual ~awsSliderButton();

  // Get's the texture handle and the title, plus style if there is one.
  virtual bool Setup(iAws *wmgr, awsComponentNode *settings);

  // Gets properties
  bool GetProperty(char *name, void **parm);

  // Sets properties
  bool SetProperty(char *name, void *parm);

  // Returns the named TYPE of the component, like "Radio Button", etc.
  virtual char *Type();

  virtual bool OnMouseDown(int btn, int x, int y);
  virtual bool OnMouseUp(int btn, int x, int y);
  virtual bool OnMouseMove(int button, int x, int y);
  virtual bool OnMouseClick(int ,int ,int );
  virtual bool OnMouseDoubleClick(int ,int ,int );

 public:
  // mouse position while capturing in progress
  int last_x, last_y;
};

class awsSliderButtonFactory : public awsCmdButtonFactory
{
 public:
  SCF_DECLARE_IBASE_EXT (awsCmdButtonFactory);

  // Calls register to register the component that it builds with the window manager
  awsSliderButtonFactory(iAws *wmgr);

  // Does nothing
  virtual ~awsSliderButtonFactory();

  // Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create();
};

#endif

