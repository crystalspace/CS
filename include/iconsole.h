/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Written by Michael Dale Long
  
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

#ifndef __ICONSOLE_H__
#define __ICONSOLE_H__

#include "iplugin.h"
#include "csutil/scf.h"

class csRect;
struct iTextureManager;

SCF_VERSION(iConsole, 0, 0, 1);
struct iConsole : public iPlugIn
{
  /// Show the console
  virtual void Show() = 0;
  /// Hide the console
  virtual void Hide() = 0;

  /// Print a string to the console
  virtual void PutText(const char *text) = 0;

  // Update the console on the window.
  virtual void Draw(csRect *rect = NULL) = 0;

  /// Return true if console is active
  virtual bool IsActive() const = 0;

  /// Clear console
  virtual void Clear() = 0;

  /// Set the buffer size in lines
  virtual void SetBufferSize(int maxlines) = 0;

  /// Retrieve the console colors from the current palette
  virtual void CacheColors(iTextureManager *txtmgr) = 0;

  /// Get the foreground color
  virtual void GetForeground(int &red, int &green, int &blue) const = 0;
  /// Set the foreground color.  CacheColor() must be called before it goes into effect!
  virtual void SetForeground(int red, int green, int blue) = 0;

  /// Get the background color.  Includes transparency.
  virtual void GetBackground(int &red, int &green, int &blue, int &alpha) const = 0;
  /// Set the background color.  CacheColor() must be called before it goes into effect!
  virtual void SetBackground(int red, int green, int blue, int alpha = 0) = 0;

  /* Needs some more features, like:
   * Font control
   * Background texture(s)
   * Scroll control
   * ...
   */

};

#endif // ! __ICONSOLE_H__
