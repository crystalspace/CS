/*
    Crystal Space Windowing System: mouse support
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CSMOUSE_H__
#define __CSMOUSE_H__

#include "cscomp.h"
#include "csengine/csspr2d.h"

class csTextureHandle;

/**
 * This class encapsulates mouse pointer
 */
class csMousePointer : public csComponent
{
private:
  friend class csMouse;

  /// Cursor sprite
  csSprite2D *Cursor;
  /// Texture X, Y, width, height, hotspot X and Y
  int tX, tY, tW, tH, hsX, hsY;
  /// Area under mouse cursor
  ImageArea *Under;

public:
  /// Initializes mouse cursor
  csMousePointer (csComponent *iParent, int ID, int x, int y,
    int w, int h, int hsx, int hsy);

  /// Destroy mouse cursor
  virtual ~csMousePointer ();

  /// Draw mouse cursor, keeping first the background
  void Draw (int x, int y);

  /// Restore background under mouse cursor
  void Undraw ();

  /// Free buffer for saved image area covered by mouse pointer
  void Free ();

  /// Set mouse cursor texture
  void SetTexture (csTextureHandle *tex);
};

/**
 * This class handles mouse pointer and generates mouse events.<p>
 * Usually there is only one object of this class in each csApp object.
 */
class csMouse : public csComponent
{
private:
  int MouseX, MouseY;
  int Visible;				// Visibility counter
  bool invisible;			// "Invisible cursor" is selected

public:
  ///
  csMouse (csComponent *iParent);
  ///
  virtual ~csMouse ();

  /// Handle a event and return true if processed
  virtual bool HandleEvent (csEvent &Event);

  /// Set mouse cursor position
  void Move (int x, int y);

  void GetPosition (int &x, int &y)
  { x = MouseX; y = MouseY; }

  /**
   * Increment mouse pointer visibility counter.
   * When counter == 0, mouse is visible.
   */
  void Show ()
  { Visible++; }

  /// Decrement mouse visibility counter
  void Hide ()
  { Visible--; }

  /// Set cursor by ID number; returns false if no cursor with this ID found
  bool SetCursor (csMouseCursorID ID);

  /// Called by csApp when textures has been loaded
  void Setup ();

private:
  friend class csApp;

  /// Draw mouse pointer
  void Draw ();
  /// Undraw mouse pointer
  void Undraw ();

  /// This function sets up mouse cursor images
  void NewPointer (char *id, char *posdef);
};

#endif // __CSMOUSE_H__
