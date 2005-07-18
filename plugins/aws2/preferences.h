/*
    Copyright (C) 2005 by Christopher Nelson

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

#ifndef __AWS_PREFERENCES_MGR_2_H__
#define __AWS_PREFERENCES_MGR_2_H__

#include "registry.h"
#include "iutil/objreg.h"

/**\file
 * Defines the preferences object, which is used as a key tree where settings for window creation are stored.  Other information may also be
 * stored in a preferences object.
 */


namespace aws
{
  enum AWS_COLORS
  {
    AC_HIGHLIGHT,
    AC_HIGHLIGHT2,
    AC_SHADOW,
    AC_SHADOW2,
    AC_FILL,
    AC_DARKFILL,
    AC_BACKFILL,
    AC_TEXTFORE,
    AC_TEXTBACK,
    AC_SELECTTEXTFORE,
    AC_SELECTTEXTBACK,
    AC_TEXTDISABLED,
    AC_BUTTONTEXT,
    AC_TRANSPARENT,
    AC_BLACK,
    AC_WHITE,
    AC_RED,
    AC_GREEN,
    AC_BLUE,
    AC_COLOR_COUNT
  };

  /** This maintains a set of preferences.  Generally only one of these exists at a time, but there's no reason why there couldn't be more. */
  class preferences
  {
	  /** The root registry.  All important registries (i.e. for windows or skins) hang off this registry. */
	  registry root;

          int sys_colors[AC_COLOR_COUNT];

  public:
    preferences():root("root") {}
    virtual ~preferences() {}

    /** Loads an xml-based definitions file into this preferences object.  Multiple files may be loaded, one after the other.  The contents are essentially merged. */
    bool load(iObjectRegistry* objreg, const scfString& filename);		

    /** Clears all definitions for this preferences object. */
    void clear() { root.clear(); }

    /** Finds a registry in the given category. If the reference is invalid, then the given registry doesn't exist. */
    csRef< registry > findReg(const csString &category, const csString &name)
    {
      return root.findChild(category, name);			
    }

  /////////////////////////////////////////////
  /////// Global Color Palette ////////////////
  /////////////////////////////////////////////

  /// Sets the value of a color in the global AWS palette.
  virtual void setColor (int index, int color)
  {
    if (index<AC_COLOR_COUNT)
      sys_colors[index]=color;
  }

  /// Gets the value of a color from the global AWS palette.
  virtual int getColor (int index)
  { 
  if (index<AC_COLOR_COUNT)
    return sys_colors[index];

  else
    return 0;
  }
 };

} // end namespace

#endif
