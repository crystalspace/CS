/**************************************************************************
    Copyright (C) 2003 by Jorrit Tyberghein

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

#ifndef __CS_AWS_DOCSITE_H__
#define __CS_AWS_DOCSITE_H__

#include "csutil/scf.h"

struct iAwsDockSite;
struct iAwsDockableWindow;

SCF_VERSION (iAwsDockSite, 0, 0, 1);

/**
 * Dock site types, used to determine which bars
 * can dock in which places
 */
const int AWS_DOCK_EAST = 0x1;
const int AWS_DOCK_WEST = 0x2;
const int AWS_DOCK_NORTH = 0x4;
const int AWS_DOCK_SOUTH = 0x8;
const int AWS_DOCK_HORZ = AWS_DOCK_SOUTH | AWS_DOCK_NORTH;
const int AWS_DOCK_VERT = AWS_DOCK_EAST | AWS_DOCK_WEST;
const int AWS_DOCK_ALL = AWS_DOCK_VERT | AWS_DOCK_HORZ;

/// Document me! @@@
struct iAwsDockSite : iBase
{
public:
  /// Document me! @@@
  virtual int GetType () =0;

  /// Insert a window into the dock site
  virtual void AddDockWindow (iAwsDockableWindow* win)=0;

  /// Remove a window from a dock site
  virtual void RemoveDockWindow (iAwsDockableWindow* win)=0;

  /**
   * Gets the docked window frame where the dockable window
   * should draw itself
   */
  virtual csRect GetDockedWindowFrame (iAwsDockableWindow* win)=0;
};
	
SCF_VERSION (iAwsDockableWindow, 0, 0, 1);

/// Document me! @@@
struct iAwsDockableWindow : iBase
{
public:
  /// Returns the current dock site, or 0 if the window is floating
  virtual iAwsDockSite* GetDockSite ()=0;
};

#endif

