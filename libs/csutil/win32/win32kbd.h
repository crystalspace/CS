/*
    Copyright (C) 2003 by Jorrit Tyberghein
	      (C) 2003 by Frank Richter

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

#ifndef __CS_SYS_WIN32_WIN32KBD_H__
#define __CS_SYS_WIN32_WIN32KBD_H__

#include "csextern.h"
#include "iutil/csinput.h"
#include "csutil/csinput.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/scf_implementation.h"

// Some #defines from newer PSDKs
#ifndef WM_UNICHAR
#define WM_UNICHAR	0x0109
#endif

#ifndef UNICODE_NOCHAR
#define UNICODE_NOCHAR	0xffff
#endif

struct iEventOutlet;

class CS_CRYSTALSPACE_EXPORT csWin32KeyComposer : 
  public scfImplementation1<csWin32KeyComposer, iKeyComposer>
{
protected:
  utf32_char lastDead;
  SHORT lastDeadVk;
  BYTE lastDeadKbdState[256];

public:

  csWin32KeyComposer ();
  virtual ~csWin32KeyComposer ();

  virtual csKeyComposeResult HandleKey (const csKeyEventData& keyEventData,
    utf32_char* buf, size_t bufChars, int* resultChars = 0);
  virtual void ResetState ();
};

class csWin32KeyboardDriver : public csKeyboardDriver
{
protected:
  csDirtyAccessArray<WCHAR> IMEComposeBuf;
  bool Win32KeyToCSKey (LONG vKey, LONG keyFlags,
    utf32_char& rawCode, utf32_char& cookedCode, csKeyCharType& charType);
  const char* GetVKName (LONG vKey);
public:
  csWin32KeyboardDriver (iObjectRegistry* r);
  virtual ~csWin32KeyboardDriver ();

  virtual void Reset ();
  virtual void RestoreKeys ();

  virtual csPtr<iKeyComposer> CreateKeyComposer ();

  bool HandleKeyMessage (HWND hWnd, UINT message,
    WPARAM wParam, LPARAM lParam);
};

#endif // __CS_SYS_WIN32_WIN32KBD_H__
