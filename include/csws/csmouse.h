/*
    Crystal Space Windowing System: mouse support
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSMOUSE_H__
#define __CS_CSMOUSE_H__

/**\file
 * Crystal Space Windowing System: mouse support
 */

/**
 * \addtogroup csws
 * @{ */
 
#include "csextern.h"
 
#include "ivideo/texture.h"
#include "iengine/texture.h"
#include "csutil/parray.h"
#include "csws/csgfxppl.h"

class csMouse;
struct iEvent;

/**
 * This class encapsulates mouse pointer
 */
class CS_CRYSTALSPACE_EXPORT csMousePointer
{
private:
  friend class csMouse;

  /// Cursor ID
  int id;
  /// Texture X, Y, width, height, hotspot X and Y
  int tX, tY, tW, tH, hsX, hsY;
  /// The parent mouse object
  csMouse *parent;

public:
  /// Initializes mouse cursor
  csMousePointer (csMouse *iParent, int ID,
    int x, int y, int w, int h, int hsx, int hsy);

  /// Draw mouse cursor, keeping first the background
  void Draw (int x, int y, csImageArea *&Under);
};

/**
 * This class handles mouse pointer and generates mouse events.<p>
 * Usually there is only one object of this class in each csApp object.
 */
class CS_CRYSTALSPACE_EXPORT csMouse
{
private:
  friend class csMousePointer;

  /// Mouse X and Y
  int MouseX, MouseY;
  /// Virtual mouse position
  int VirtualX, VirtualY;
  /// Visibility counter
  int Visible;
  /// "Invisible cursor" is selected
  bool invisible;
  /// Application is focused
  bool AppFocused;
  /// Last time we painted a virtual cursor
  bool LastVirtual;
  /// Area under mouse cursor
  csImageArea *Under [MAX_SYNC_PAGES];
  /// The application object
  csApp *app;
  /// The array of pointers
  csPDelArray<csMousePointer> Pointers;
  /// Current active mouse cursor (system cursor if 0)
  csMousePointer *ActiveCursor;
  /// Cursor texture
  iTextureHandle *Texture;

public:
  ///
  csMouse (csApp *iApp);
  ///
  ~csMouse ();

  /// Handle a event and return true if processed
  bool HandleEvent (iEvent &Event);

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

  /// Set "virtual" mouse position, i.e. show cursor at a different location
  void SetVirtualPosition (int x, int y)
  { VirtualX = x; VirtualY = y; }

private:
  friend class csApp;
  friend class csGraphicsPipeline;

  /// Draw mouse pointer
  void Draw (int Page);
  /// Undraw mouse pointer
  void Undraw (int Page);

  /// This function sets up mouse cursor images
  void NewPointer (const char *id, const char *posdef);

  /// Remove all mouse cursors
  void ClearPointers ();
};

/** @} */

#endif // __CS_CSMOUSE_H__
