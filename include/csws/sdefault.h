/*
    Crystal Space Windowing System: default skin
    Copyright (C) 2000 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __SDEFAULT_H__
#define __SDEFAULT_H__

#include "csskin.h"
#include "csbackgr.h"

#define SKIN_DECLARE_DEFAULT(var)	\
  SKIN_DECLARE (my##var##Type, csSkin);	\
    SKIN_SLICE (DefaultScrollBar);		\
    SKIN_SLICE (DefaultButton);		\
    SKIN_SLICE (DefaultWindow);		\
    SKIN_SLICE (DefaultDialog);		\
    SKIN_SLICE (DefaultTitlebar);		\
    SKIN_SLICE (DefaultListBox);		\
    SKIN_SLICE (DefaultListBoxItem);		\
  SKIN_DECLARE_END var

class csButton;
class csListBox;
struct iTextureHandle;

/**
 * This is the default skin for buttons.
 */
class csDefaultButtonSkin : public csButtonSkin
{
public:
  /// Draw the component we are responsible for
  virtual void Draw (csComponent &iComp);

  /// Suggest the optimal size of the button, given an already filled object
  virtual void SuggestSize (csButton &This, int &w, int &h);
};

/**
 * This is the default skin for windows.
 */
class csDefaultWindowSkin : public csWindowSkin
{
  // The texture for titlebar buttons
  iTextureHandle *ButtonTex;
  // Window background
  csBackground Back;
  // The parent skin object
  csSkin *Skin;

public:
  /// Initialize the window skin slice
  csDefaultWindowSkin () : ButtonTex (NULL), Skin (NULL) {}

  /// Query the required resources from application
  virtual void Initialize (csApp *iApp, csSkin *Parent);

  /// Free the resources allocated in Initialize()
  virtual void Deinitialize ();

  /// Draw the component we are responsible for
  virtual void Draw (csComponent &iComp);

  /// Place all gadgets (titlebar, buttons, menu and toolbar)
  virtual void PlaceGadgets (csWindow &This);

  /// Create a button for window's titlebar
  virtual csButton *CreateButton (csWindow &This, int ButtonID);

  /// Called to reflect some specific window state change on gagdets
  virtual void SetState (csWindow &This, int Which, bool State);

  /// Set window border width and height depending on frame style
  virtual void SetBorderSize (csWindow &This);

protected:
  void SetButtBitmap (csButton *button, const char *id);
};

/**
 * This is the default skin for dialogs.
 */
class csDefaultDialogSkin : public csDialogSkin
{
  // The background
  csBackground Back;

public:
  /// Query the required resources from application
  virtual void Initialize (csApp *iApp, csSkin *Parent);

  /// Free the resources allocated in Initialize()
  virtual void Deinitialize ();

  /// Draw the component we are responsible for
  virtual void Draw (csComponent &iComp);

  /// Set dialog border width and height depending on frame style
  virtual void SetBorderSize (csDialog &This);
};

/**
 * This is the default skin for window titlebars.
 */
class csDefaultTitlebarSkin : public csTitlebarSkin
{
  // The active window titlebar background
  csBackground ABack;
  // The inactive window titlebar background
  csBackground IBack;
  /// Whenever to enable titlebar hashing
  bool Hash;

public:
  /// Query the required resources from application
  void Initialize (csApp *iApp, csSkin *Parent);

  /// Free the resources allocated in Initialize()
  virtual void Deinitialize ();

  /// Draw the component we are responsible for
  virtual void Draw (csComponent &iComp);
};

/**
 * This is the default skin for listboxes
 */
class csDefaultListBoxSkin : public csListBoxSkin
{
public:
  /// Draw the component we are responsible for
  virtual void Draw (csComponent &iComp);
  
 /// Suggest the optimal size of the ListBox
 virtual void SuggestSize (csListBox &This, int &w, int &h);
};

/**
 * This is the default skin for listbox items
 */
class csDefaultListBoxItemSkin : public csListBoxItemSkin
{
public:
  /// Draw the component we are responsible for
  virtual void Draw (csComponent &iComp);
};


/**
 * This is the default skin for scroll bars
 */
 class csDefaultScrollBarSkin : public csScrollBarSkin 
 {
 public:
   /// Draw the component we are responsible for
   virtual void Draw(csComponent &iComp);
 };
 

#endif // __SDEFAULT_H__
