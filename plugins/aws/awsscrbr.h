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

#ifndef __CS_AWS_SCRBR_H__
#define __CS_AWS_SCRBR_H__

#include "awsPanel.h"
#include "awscmdbt.h"
#include "awstimer.h"

class awsSliderButton;

class awsScrollBar : public awsPanel
{
private:
  /// True when button is down, false if up.
  bool is_down;

  /// True if the component has the mouse over it.
  bool mouse_is_over;

  /// True if button was down, and button is in switch mode (toggle = yes).
  bool was_down;

  /// Holds the texture handle for the dec arrow.
  iTextureHandle *decimg;

  /// Holds the texture handle for the inc arrow.
  iTextureHandle *incimg;

  /// Flags for frame style.
  int orientation;

  /// Button for up or left.
  awsSliderButton *decVal;

  /// Button for down or right.
  awsSliderButton *incVal;

  /// The knob.
  awsSliderButton *knob;

  /// timer.
  awsTimer *timer;

  /// The sink for button messages.
  iAwsSink *sink;

  /// The slot for decrement button messages.
  iAwsSlot *dec_slot;

  /// The slot for increment button messages.
  iAwsSlot *inc_slot;

  /// The slot for knob button messages.
  iAwsSlot *knob_slot;

  iAwsSlot *tick_slot;

  int last_x, last_y;

  /// Value of scroll bar.
  float value;

  /// Maximum value scroll can attain.
  float max;

  /// Minimum value scroll can attain.
  float min;

  /**
   * Amount of total visible for proportional scroll bars, set to 0
   * turns off proportional drawing.
   */
  float amntvis;

  /// Amount to move scroll bar when inc/dec buttons clicked.
  float value_delta;

  /// Amount to move scroll bar when scroll area clicked.
  float value_page_delta;
protected:
  bool HandleClicking (int btn, int x, int y);
  bool captured;
public:
  awsScrollBar ();
  virtual ~awsScrollBar ();

  /// Signal constants.
  enum
  {
    /// The value of the scroll bar changed.
    signalChanged = 0x1,
    /// The component becomes focused.
    signalFocused = 0x2
  };

  /// Scrollbar orientation.
  enum
  {
    sboVertical = 0x0,
    sboHorizontal = 0x1
  };

  /// Trigger called when inc button is clicked.
  static void IncClicked (intptr_t sk, iAwsSource *source);

  /// Trigger called when dec button is clicked.
  static void DecClicked (intptr_t sk, iAwsSource *source);

  static void TickTock (intptr_t sk, iAwsSource *source);
  static void KnobTick (intptr_t sk, iAwsSource *source);

  /// Trigger called when area above/left of knob button is clicked.
  static void DecPageClicked (intptr_t sk, iAwsSource *source);

  /// Trigger called when area below/right of knob button is clicked.
  static void IncPageClicked (intptr_t sk, iAwsSource *source);

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Gets properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Sets properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

  /// Triggered when the user presses a mouse button down.
  virtual bool OnMouseDown (int button, int x, int y);

  /// Triggered when the user unpresses a mouse button.
  virtual bool OnMouseUp (int button, int x, int y);

  /// Triggered when the user moves the mouse.
  virtual bool OnMouseMove (int button, int x, int y);

  /// Triggered when the user clicks the mouse.
  virtual bool OnMouseClick (int button, int x, int y);

  /// Triggered when the user double clicks the mouse.
  virtual bool OnMouseDoubleClick (int button, int x, int y);

  /// Triggered when this component loses mouse focus.
  virtual bool OnMouseExit ();

  /// Triggered when this component gains mouse focus.
  virtual bool OnMouseEnter ();

  /// Triggered when the user presses a key.
  virtual bool OnKeyboard (const csKeyEventData& eventData);

  /// Triggered when component becomes focused.
  virtual void OnSetFocus ();

  /// Adds in the inc and dec buttons appropriately.
  virtual void OnAdded ();

  /// Fixes the inc and dec buttons for layouts.
  virtual void OnResized ();
};

class awsScrollBarFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with
   * the window manager.
   */
  awsScrollBarFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsScrollBarFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

class awsSliderButton : public awsCmdButton
{
protected:
  /// The timer we use for repeated firing of "clicked" signal.
  awsTimer *timer;

  /// Signal if we have captured the mouse.
  bool captured;

  /// Fire the signal every nTick milliseconds.
  csTicks nTicks;

  iAwsSink *sink;
  iAwsSlot *tick_slot;

  static void TickTock (intptr_t sk, iAwsSource *);
public:

  awsSliderButton ();
  virtual ~awsSliderButton ();

  /// Get the texture handle and the title, plus style if there is one.
  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Get properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Set properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Returns the named TYPE of the component, like "Radio Button", etc.
  virtual const char *Type ();

  virtual bool OnMouseDown (int btn, int x, int y);
  virtual bool OnMouseUp (int btn, int x, int y);
  virtual bool OnMouseMove (int button, int x, int y);
  virtual bool OnMouseClick (int, int, int);
  virtual bool OnMouseDoubleClick (int, int, int);

  /// Mouse position while capturing in progress.
  int last_x, last_y;
};

class awsSliderButtonFactory : public awsCmdButtonFactory
{
public:
  /**
   * Calls register to register the component that it builds with
   * the window manager.
   */
  awsSliderButtonFactory (iAws *wmgr);

  /// Does nothing.
  virtual ~awsSliderButtonFactory ();

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_SCRBR_H__
