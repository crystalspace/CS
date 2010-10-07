/*
    Copyright (C) 2009 by Frank Richter

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

#ifndef __CS_PLUGINCOMMON_WIN32_ICONTOOLS_H__
#define __CS_PLUGINCOMMON_WIN32_ICONTOOLS_H__

/**\file
 * Helpers for dealing with Windows icons
 */

#include "csextern_win.h"
#include "csutil/ref.h"

struct iImage;
struct _ICONINFO;

namespace CS
{
  namespace Platform
  {
    namespace Win32
    {
      /**
       * Helpers for dealing with Windows icons
       */
      class IconTools
      {
      protected:
	static CS_CSPLUGINCOMMON_WIN_EXPORT HICON LoadStockIconSize (uintptr_t id, int desiredSize);
      public:
	//@{
	/**
	 * Load a stock icon (IDI_something) closest to a given size.
	 * The main benefit over <tt>LoadIcon (0, IDI_something)</tt> is the
	 * possibility to specify a desired size. (All standard Win32
	 * mechanisms to obtain an icon of a certain size either don't
	 * provide different sizes or deliver bad quality icons.)
	 *
	 * Stock icons are likely available in the sizes 16x16, 32x32 and 48x48.
	 * Asking for a size other than that will actually return an icon with the
	 * size closest (well, what Windows deems to be close) to the requested
	 * size.
	 *
	 * \param id Icon ID to load. Must be one of the default Win32
	 *   IDI_something constants
	 *   (see http://msdn.microsoft.com/en-us/library/ms648072%28VS.85%29.aspx).
	 * \param desiredSize Desired size of the icon.
	 * \return An icon with a size that Windows deems closest to
	 *   \a desiredSize, or 0 in case of error.
	 */
	static inline HICON LoadStockIconSize (LPCSTR id, int desiredSize)
	{ return LoadStockIconSize (uintptr_t (id), desiredSize); }
	static inline HICON LoadStockIconSize (LPCWSTR id, int desiredSize)
	{ return LoadStockIconSize (uintptr_t (id), desiredSize); }
	//@}
	/**
	 * Create a CS image from a Windows icon.
	 * Useful when drawing an icon (stock or from somewhere else) inside CS is
	 * desired.
	 * \param icon Icon to create image from.
	 */
	static csPtr<iImage> CS_CSPLUGINCOMMON_WIN_EXPORT IconToImage (HICON icon);
	/**
	 * Create a Windows icon from a CS image.
	 * The color depth depends on the color depth of the screen.
	 * The handling of the alpha channel of the image - if it has one -
	 * depends on the Windows version and screen color depth: in particular,
	 * icons with alpha are only supported on Windows XP and a screen color
	 * depth of at least 24bpp. In other cases the alpha will be reduced to
	 * binary alpha.
	 * \param image Image to create icon from.
	 * \param iconTemplate (Optional) Template for information used to create
	 *   icon.
	 */
	static HICON CS_CSPLUGINCOMMON_WIN_EXPORT IconFromImage (iImage* image,
	  const _ICONINFO* iconTemplate = 0);
      };
    } // namespace CS
  } // namespace Platform
} // namespace Win32

#endif // __CS_PLUGINCOMMON_WIN32_ICONTOOLS_H__
