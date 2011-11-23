/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IVIDEO_NATWIN_H__
#define __CS_IVIDEO_NATWIN_H__

/**\file
 * Native window manager interface
 */

/**
 * \addtogroup gfx2d
 * @{ */
 
#include "csutil/scf.h"

struct iImage;

/**\name Alert types
 * @{ */
/// Display an error
#define CS_ALERT_ERROR 1
/// Display a warning
#define CS_ALERT_WARNING 2
/// Display a note
#define CS_ALERT_NOTE 3
/** @} */

/**
 * This interface represents the native window manager system.
 * At this moment this interface is nearly empty. In the future it
 * will be extended with more functionality to manage windows and so on.
 *
 * Main ways to get pointers to this interface:
 * - scfQueryInterface on iGraphics2D interface.
 */
struct iNativeWindowManager : public virtual iBase
{
  SCF_INTERFACE (iNativeWindowManager, 2, 0, 2);
  
  //@{
  /**
   * Show an alert.
   * Type is one of CS_ALERT_???.
   * \remarks All strings are expected to be UTF-8 encoded.
   * \sa #CS_ALERT_ERROR
   * \sa \ref FormatterNotes
   */
  virtual void Alert (int type, const char* title, const char* okMsg,
  	const char* msg, ...) CS_GNUC_PRINTF (5, 6) = 0;
  virtual void AlertV (int type, const char* title, const char* okMsg,
  	const char* msg, va_list arg) CS_GNUC_PRINTF (5, 0) = 0;
  //@}
  //@{
  /**
   * Show an alert.
   * Type is one of CS_ALERT_???.
   * \sa #CS_ALERT_ERROR
   * \sa \ref FormatterNotes
   */
  virtual void Alert (int type, const wchar_t* title, const wchar_t* okMsg,
  	const wchar_t* msg, ...) CS_GNUC_WPRINTF (5, 6) = 0;
  virtual void AlertV (int type, const wchar_t* title, const wchar_t* okMsg,
  	const wchar_t* msg, va_list arg) CS_GNUC_WPRINTF (5, 0) = 0;
  //@}
};

/**
 * This interface represents a native window.
 *
 * Main ways to get pointers to this interface:
 * - iGraphics2D::GetNativeWindow()
 */
struct iNativeWindow : public virtual iBase
{
  SCF_INTERFACE (iNativeWindow, 2, 0, 4);
  
  /**
   * Set the title for this window.
   * \remarks \p title is expected to be UTF-8 encoded.
   */
  virtual void SetTitle (const char* title) = 0;
  /**
   * Set the title for this window.
   */
  virtual void SetTitle (const wchar_t* title) = 0;
  
  /** Sets the icon of this window with the provided one.
   *
   *  @param image the iImage to set as the icon of this window.
   */  
  virtual void SetIcon (iImage *image) = 0;

  /**\name Window transparency
   * Window transparency allows windows to be transparent, that is, the
   * background behind it shows through (desktop, other windows...)
   * where the framebuffer alpha is not opaque.
   * (Clearing the framebuffer leaves the alpha at zero - fully transparent -
   * so not rendering every pixel would be enough to see something.)
   * 
   * Note that window transparency is not available on all platforms or
   * certain configurations:
   * - Windows: available on Vista and above. Must be enabled by the user.
   *   IsWindowTransparencyAvailable() returns false when compositing is
   *   not available or enabled. Note that the user can enable or disable
   *   compositing on the fly. Currently, the only way for CrystalSpace 
   *   applications to detect this is by polling GetWindowTransparent.
   *   Disabling compositing will obviously disable any window transparency
   *   (GetWindowTransparent returns \a false), however, CrystalSpace
   *   will re-enable transparency if the user re-enables compositing.
   *   (There is currently no way to inhibit this re-enabling.
   *   Polling GetWindowTransparent is, again, the way to detect whether
   *   this will happen.)
   * - X11: compositing window manager is required.
   *   Window transparency must be enabled _before_ the window was created
   *   (i.e. system was opened) and can't be toggled later.
   * @{ */
  /**
   * Return whether window transparency is available.
   * Returns \a true when the current platform and OS version generally support
   *   transparency and the user enabled it.
   */
  virtual bool IsWindowTransparencyAvailable() = 0;
  /**
   * Set whether window transparency is desired.
   * \param transparent "Transparency desired" flag.
   * \returns Whether the flag was successfully changed.
   *   If the system is not open, and compositing is not supported at all,
   *   returns \a false (the flag is not changed).
   *   If the system is not open, and compositing is generally supported,
   *   the flag is stored and the actual window transparency will be set
   *   when the system opens. Returns \a true in that case.     
   *   If the system is open, returns whether the transparency could be
   *   changed.
   */
  virtual bool SetWindowTransparent (bool transparent) = 0;
  /**
   * Query the window transparency state.
   * \returns If the system is not open, returns the value of the
   *   "transparency desired" flag.
   *   If the system is open, returns whether the window is actually
   *   currently transparent (when transparency is desired and available),
   */
  virtual bool GetWindowTransparent () = 0;
  /** @} */

  /// Window decorations that can be shown or hidden.
  enum WindowDecoration
  {
    /**
     * Window caption ("title bar").
     * Caveat: Hiding the caption may prevent the user from moving the window.
     */
    decoCaption,
    /**
     * On Windows, thin frame between "client" area and window borders.
     * This flag actually only has effect on Windows Vista+ with compositing
     * enabled, and it's actual effect depends on whether the window caption
     * is shown or not.
     *
     * If the window has a caption, it controls the thin frame between client
     * area and window border. Removing that frame gives the window the
     * appearance of a continuous "sheet of glass".
     *
     * If the window is transparent, has no caption and is not resizeable,
     * setting this decoration to off actually removes the "glassy" backdrop,
     * making transparent areas of the framebuffer truly translucent.
     */
    decoClientFrame
  };
  /**
   * Show or hide a window decoration.
   * \param decoration Decoration to change
   * \param flag Whether to show (\c true) or hide (\c false) a decoration.
   * \returns Whether changing the visibility of the decoration was successful.
   * \remarks Always fails when in fullscreen mode.
   */
  virtual bool SetWindowDecoration (WindowDecoration decoration, bool flag) = 0;
  /// Return whether a decoration is visible.
  virtual bool GetWindowDecoration (WindowDecoration decoration) = 0;
  
  /**
   * Take a desired window contents size and compute a contents size so the
   * window would (in windowed mode) fit onto the current working area.
   * 
   * This functions computes a size so the window would fit into the current
   * working area, taking window decorations and frame into account. The 
   * “working area” is the area on the screen where windows are typically
   * placed, e.g. on Windows the desktop but without the task bar.
   * On multi-monitor systems, the workspace size depends on the monitor the
   * window is currently on (or would be created on).
   * 
   * Since this function works with “content sizes” the returned sizes
   * are suitable for passing to iGraphics2D::Resize().
   * \param desiredWidth The desired contents width, will be corrected to fit
   *   into the workspace width.
   * \param desiredHeight The desired contents height, will be corrected to
   *   fit into the workspace height.
   * \returns Whether the corrected size could be computed: the function may
   *   not be supported on the current platform. In that case the
   *   \a desiredWidth and \a desiredHeight parameters will be left unchanged.
   */
  virtual bool FitSizeToWorkingArea (int& desiredWidth,
                                     int& desiredHeight) = 0;
};

/** @} */

#endif // __CS_IVIDEO_NATWIN_H__

