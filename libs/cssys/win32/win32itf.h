/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#include "cscom/com.h"

#define WINDOWCLASSNAME "CrystalSpace"

extern const IID IID_IWin32SystemDriver;

interface IWin32SystemDriver : public IUnknown
{
  /// Returns the HINSTANCE of the program
  STDMETHOD (GetInstance) (HINSTANCE* retval) = 0;
  /// Returns S_OK if the program is 'active', S_FALSE otherwise.
  STDMETHOD (GetIsActive) () = 0;
  /// Gets the nCmdShow of the WinMain ().
  STDMETHOD (GetCmdShow) (int* retval) = 0;
  /// Query whenever driver should use hardware mouse cursor
  STDMETHOD (GetHardwareCursor) (bool *enabled) = 0;
  /// Set the mouse cursor
  STDMETHOD (SetMouseCursor) (LPCTSTR cursorid) = 0;
};
