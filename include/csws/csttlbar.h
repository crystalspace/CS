/*
    Crystal Space Windowing System: title bar class
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

#ifndef __CS_CSTTLBAR_H__
#define __CS_CSTTLBAR_H__

/**
 * \addtogroup csws_comps_title
 * @{ */
 
#include "csextern.h"
 
#include "cscomp.h"

/**
 * The TitleBar class represents a bar with a text string written across
 * which is usually drawn at the top of complex windows :-).
 */
class CS_CRYSTALSPACE_EXPORT csTitleBar : public csComponent
{
public:
  /// Create title bar object
  csTitleBar (csComponent *iParent, const char *iTitle);

  /// Handle input events
  virtual bool HandleEvent (iEvent &Event);

  /// Get the name of the skin slice for this component
  virtual char *GetSkinName ()
  { return "Titlebar"; }
};

/** @} */

#endif // __CS_CSTTLBAR_H__
