/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
  
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

#ifndef __CS_WIN32ITF_H__
#define __CS_WIN32ITF_H__

#include "csutil/scf.h"

#define WINDOWCLASSNAME "Crystal"

SCF_VERSION (iWin32SystemDriver, 0, 0, 1);

struct iWin32SystemDriver : public iBase
{
  /// Returns the HINSTANCE of the program
  virtual HINSTANCE GetInstance() const = 0;
  /// Returns true if the program is 'active', false otherwise.
  virtual bool GetIsActive() const = 0;
  /// Gets the nCmdShow of the WinMain().
  virtual int GetCmdShow() const = 0;
};

#endif // __CS_WIN32ITF_H__
