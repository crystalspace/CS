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

#include "cssysdef.h"
#include "csutil/csuctransform.h"
#include "iutil/event.h"
#include <windows.h>

#include "win32kbd.h"
#include "csutil/win32/wintools.h"

#ifdef CS_KEY_DEBUG_ENABLE
#include "csutil/sysfunc.h"
#include "win32kbd_identstrs.h"
#endif

static void ModifiersToKeyState (const csKeyModifiers& modifiersState,
				 BYTE* keystate)
{
  if (modifiersState.modifiers[csKeyModifierTypeShift] & 
    (1 << csKeyModifierNumLeft))
    keystate[VK_LSHIFT] |= 0x80;
  if (modifiersState.modifiers[csKeyModifierTypeShift] & 
    (1 << csKeyModifierNumRight))
    keystate[VK_RSHIFT] |= 0x80;
  if (modifiersState.modifiers[csKeyModifierTypeShift] != 0)
    keystate[VK_SHIFT] |= 0x80;

  if (modifiersState.modifiers[csKeyModifierTypeCtrl] & 
    (1 << csKeyModifierNumLeft))
    keystate[VK_LCONTROL] |= 0x80;
  if (modifiersState.modifiers[csKeyModifierTypeCtrl] & 
    (1 << csKeyModifierNumRight))
    keystate[VK_RCONTROL] |= 0x80;
  if (modifiersState.modifiers[csKeyModifierTypeCtrl] != 0)
    keystate[VK_CONTROL] |= 0x80;

  if (modifiersState.modifiers[csKeyModifierTypeAlt] & 
    (1 << csKeyModifierNumLeft))
    keystate[VK_LMENU] |= 0x80;
  if (modifiersState.modifiers[csKeyModifierTypeAlt] & 
    (1 << csKeyModifierNumRight))
    keystate[VK_RMENU] |= 0x80;
  if (modifiersState.modifiers[csKeyModifierTypeAlt] != 0)
    keystate[VK_MENU] |= 0x80;

  if (modifiersState.modifiers[csKeyModifierTypeCapsLock] != 0)
    keystate[VK_CAPITAL] |= 0x01;
  if (modifiersState.modifiers[csKeyModifierTypeNumLock] != 0)
    keystate[VK_NUMLOCK] |= 0x01;
  if (modifiersState.modifiers[csKeyModifierTypeScrollLock] != 0)
    keystate[VK_SCROLL] |= 0x01;
}

static bool CookedCodeToVKey (utf32_char codeCooked, SHORT& vKey,
			      BYTE* keystate)
{
  if (codeCooked >= 0x10000)
    return false;

  if (cswinIsWinNT ())
  {
    vKey = VkKeyScanW (codeCooked);
  }
  else
  {
    utf16_char wCh[2];
    CHAR ch;
    int wSize = csUnicodeTransform::EncodeUTF16 (codeCooked, 
      wCh, sizeof (wCh) / sizeof (utf16_char));
    if (WideCharToMultiByte (CP_ACP, 0, (WCHAR*)wCh, wSize, &ch, 1, 0, 0) == 0)
      return false;

    vKey = VkKeyScanA (ch);
  }

  if (vKey == -1) 
    return false;

  // Set up the appropriate key state
  memset (keystate, 0, 256 * sizeof (BYTE));
  if (vKey & 0x0100) keystate[VK_SHIFT] |= 0x80;
  if (vKey & 0x0200) keystate[VK_CONTROL] |= 0x80;
  if (vKey & 0x0400) keystate[VK_MENU] |= 0x80;
  vKey &= 0xff;

  return true;
}

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csWin32KeyComposer)
  SCF_IMPLEMENTS_INTERFACE(iKeyComposer)
SCF_IMPLEMENT_IBASE_END

csWin32KeyComposer::csWin32KeyComposer ()
{
  SCF_CONSTRUCT_IBASE(0);
  lastDead = 0;
  lastDeadVk = 0;
}

csWin32KeyComposer::~csWin32KeyComposer ()
{
  SCF_DESTRUCT_IBASE();
}

csKeyComposeResult csWin32KeyComposer::HandleKey (
  const csKeyEventData& keyEventData, utf32_char* buf, size_t bufChars, 
  int* resultChars)
{
#define RETURN(ret, bs)			\
  {					\
    if (resultChars) *resultChars = bs;	\
    return ret;				\
  }

#define RETURN0(ret)	      RETURN(ret, 0)

#define RETURN1(ret, a)	      \
  {			      \
    if (bufChars >= 1)	      \
    {			      \
      buf[0] = a;	      \
      RETURN(ret, 1);	      \
    }			      \
    else		      \
      RETURN0(ret);	      \
  }

#define RETURN2(ret, a, b)    \
  {			      \
    if (bufChars >= 2)	      \
    {			      \
      buf[0] = a;	      \
      buf[1] = b;	      \
      RETURN(ret, 2);	      \
    }			      \
    else		      \
      RETURN1(ret, b);	      \
  }

  if (CSKEY_IS_SPECIAL (keyEventData.codeRaw))
    RETURN0(csComposeNoChar)

  if (lastDeadVk != 0)
  {
    /*
      Urg. Windows saves the dead key state in a variable somewhere
      when ToAscii() or ToUnicode() is called with one.
      So to get a properly composed char, we need to first call
      ToAscii() with the dead key, so that internal variable is 
      properly set.
      */
    char outCh[2];
    int ret = ToAscii (lastDeadVk, MapVirtualKey (lastDeadVk, 0), 
      lastDeadKbdState, (PWORD)&outCh, 0);

    lastDeadVk = 0;

    WCHAR wCh[2];

    SHORT vKey;
    BYTE keystate[256];

    if (!CookedCodeToVKey (keyEventData.codeCooked, vKey, keystate))
      RETURN0(csComposeNoChar)

    // Now, do the actual composing.
    if (cswinIsWinNT ())
    {
      ret = ToUnicode (vKey, 0, keystate, wCh, 2, 0);
    }
    else
    {
      char outCh[2];
      ret = ToAscii (vKey, 0, keystate, (PWORD)&outCh, 0);
      if (ret != 0)
      {
	MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, outCh, 
	  (ret > 0) ? ret : 1, wCh,  sizeof (wCh) / sizeof (WCHAR));
      }
    }
    if (ret == 2)
    {
      RETURN2(csComposeUncomposeable, wCh[0], wCh[1])
    } 
    else if (ret == 1)
    {
      RETURN1(csComposeComposedChar, wCh[0])
    }
    else
    {
      RETURN0(csComposeNoChar)
    }
  }
  else
  {
    /*
      Not much to do else... the keyboard driver already does some
      translation...
     */
    if (keyEventData.charType == csKeyCharTypeDead)
    {
      if (!CookedCodeToVKey (keyEventData.codeCooked, lastDeadVk, 
	lastDeadKbdState))
	lastDeadVk = 0;

      RETURN0(csComposeNoChar)
    }
    else
    {
      RETURN1(csComposeNormalChar, keyEventData.codeCooked);
    }
  }
}

void csWin32KeyComposer::ResetState ()
{
  lastDead = 0;
  lastDeadVk = 0;
}

//-----------------------------------------------------------------------------

#define CS_API_NAME		Imm32
#define CS_API_FUNCTIONS	"libs/csutil/win32/Imm32API.fun"
#define CS_API_DLL		"imm32.dll"

#include "csutil/win32/APIdeclare.inc"
#include "APIdefine.inc"

csWin32KeyboardDriver::csWin32KeyboardDriver (iObjectRegistry* r) :
  csKeyboardDriver (r)
{
  Imm32::IncRef();
}

csWin32KeyboardDriver::~csWin32KeyboardDriver ()
{
  Imm32::DecRef();
}

void csWin32KeyboardDriver::Reset ()
{
  csKeyboardDriver::Reset ();
}

void csWin32KeyboardDriver::RestoreKeys ()
{
  uint8 keystate[256];
  GetKeyboardState (keystate);

  /*
    Check the pressed keys and emit proper events for them.
    @@@ Issue: Windows doesn't allow means to distinguish
    extended keys here. If you press 'up' on the keypad, you'll
    get a normal (non-kp) key code.
   */

  for (int vk = 0; vk < 255; vk++)
  {
    if ((vk <= 0x07) ||			  // Skip mouse buttons + unassigned
      ((vk >= 0x0a) && (vk <= 0x0b)) ||	  // Skip reserved range
      (vk == 0x5e) ||			  // Reserved
      ((vk >= 0x88) && (vk <= 0x8f)) ||	  // Unassigned
      ((vk >= 0x97) && (vk <= 0x9f)) ||	  // Unassigned
      ((vk >= 0xb8) && (vk <= 0xb9)) ||	  // Reserved
      ((vk >= 0xc1) && (vk <= 0xda)) ||	  // Reserved, Unassigned
      (vk == 0xe0) ||			  // Reserved
      (vk == 0xe8)			  // Unassigned
      ) continue;

    utf32_char rawCode;
    utf32_char cookedCode;
    csKeyCharType charType;

    if ((vk == VK_MENU) || (vk == VK_CONTROL) || (vk == VK_SHIFT))
      continue;

    if ((keystate[vk] & 0x81) && 
      Win32KeyToCSKey (vk, 
      ((vk >= VK_PRIOR) && (vk <= VK_DOWN)) || // Check 'Arrow keys weirdness'
      ((vk >= VK_INSERT) || (vk <= VK_DELETE)) ? 0x01000000 : 0, // below
      rawCode, cookedCode, charType))
    {
      //SetKeyState (rawCode, true, false);
      if (CSKEY_IS_MODIFIER (rawCode))
      {
	int modType = CSKEY_MODIFIER_TYPE(rawCode);
	int modNum = CSKEY_MODIFIER_NUM(rawCode);

	if (modifiersState.modifiers[modType] ^ (1 << modNum))
	  continue;
	DoKey (rawCode, cookedCode, keystate[vk] & 0x01, false, charType);
      }
      else
      {
	if (keystate[vk] & 0x80) 
	  DoKey (rawCode, cookedCode, true, false, charType);
      }
    }
  }
}

bool csWin32KeyboardDriver::HandleKeyMessage (HWND hWnd, UINT message, 
					      WPARAM wParam, LPARAM lParam)
{
  csRef<iEvent> ev;
  switch (message)
  {
    case WM_IME_COMPOSITION:
      if (Imm32::ImmAvailable())
      {
	HIMC hIMC;
	if (lParam & GCS_RESULTSTR)
	{
	  hIMC = Imm32::ImmGetContext (hWnd);
	  
	  if (cswinIsWinNT ())
	  {
	    // Get the size of the result string.
	    DWORD dwSize = Imm32::ImmGetCompositionStringW (hIMC, 
	      GCS_RESULTSTR, 0, 0);

	    IMEComposeBuf.SetLength (dwSize / sizeof (WCHAR));

	    // Get the result strings that is generated by the IME.
	    Imm32::ImmGetCompositionStringW (hIMC, GCS_RESULTSTR, 
	      IMEComposeBuf.GetArray(), dwSize);
	  }
	  else
	  {
	    // Get the size of the result string.
	    DWORD dwSize = Imm32::ImmGetCompositionStringA (hIMC, 
	      GCS_RESULTSTR, 0, 0);

	    CS_ALLOC_STACK_ARRAY(char, composeResult, dwSize);

	    // Get the result strings that is generated by the IME.
	    Imm32::ImmGetCompositionStringA (hIMC, GCS_RESULTSTR, 
	      composeResult, dwSize);

	    int bufsize = MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, 
	      composeResult, dwSize, 0, 0);
	    IMEComposeBuf.SetLength (bufsize);
	    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, 
	      composeResult, -1, IMEComposeBuf.GetArray(), bufsize);
	  }

	  Imm32::ImmReleaseContext (hWnd, hIMC);

	  size_t remaining = IMEComposeBuf.Length();
	  utf16_char* str = (utf16_char*)IMEComposeBuf.GetArray();
          while (remaining > 0)
	  {
	    utf32_char code;
	    int skip = csUnicodeTransform::UTF16Decode (str, remaining, 
	      code);
	    if (skip == 0) break;
	    str += skip;
	    remaining -= skip;

	    DoKey (0, code, true);
	    DoKey (0, code, false);
	  }
	}
      }
      return false;
    case WM_KEYUP:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      {
	utf32_char raw, cooked;
	bool down = (message == WM_KEYDOWN) || 
	    (message == WM_SYSKEYDOWN);
	bool autoRep = ((lParam & 0x40000000) != 0);
	csKeyCharType type;
      #ifdef CS_KEY_DEBUG_ENABLE
	if (IsKeyboardDebugging())
	{
	  csPrintf ("msg: %s wp: %s lp: %.8x\n", 
	    Win32KeyMsgNames.StringForIdent (message), GetVKName (wParam), 
	    (uint)lParam);
	}
      #endif
	if (Win32KeyToCSKey (wParam, lParam, raw, cooked, type))
	{
	  bool doDoKey = true;
	  if (raw == CSKEY_CTRL_LEFT)
	  {
	    /*
	      Quirk: When pressing AltGr, messages for both LCtrl and RAlt are 
	      emitted. To work this around, peek into the queue and check 
	      whether an RAlt stroke is ahead. If yes, discard the LCtrl. The 
	      message time is compared to make sure no "legal" 
	      LCtrl-before-RAlt strokes are discarded.
	    */
	    DWORD strokeTime = (DWORD)GetMessageTime ();
	    MSG msg;
	    if (PeekMessage (&msg, hWnd, WM_KEYFIRST, WM_KEYLAST, PM_NOREMOVE))
	    {
	      if ((msg.message & ~4) == (message & ~4)) 
		// mask out 4 so WM_KEY* and WM_SYSKEY* messages can be compared
	      {
		utf32_char nextRaw, nextCooked;
		csKeyCharType nextType;
		if (Win32KeyToCSKey (msg.wParam, msg.lParam, nextRaw, 
		  nextCooked, nextType))
		{
		  doDoKey = !((nextRaw == CSKEY_ALT_RIGHT) 
		    && (strokeTime == msg.time));
		}
	      }
	    }
	  }
	  else if (raw == CSKEY_SHIFT_LEFT)
	  {
	    /*
	      Quirk: when RShift is pressed, an LShift is emitted nevertheless.
	      So check which shifts are _really_ down and call DoKey()
	      appropriately.
	    */
	    bool lshiftState = keyStates.Get (CSKEY_SHIFT_LEFT, false);
	    bool rshiftState = keyStates.Get (CSKEY_SHIFT_RIGHT, false);
	    bool lshiftDown, rshiftDown;
            if (cswinIsWinNT())
            {
              lshiftDown = ::GetKeyState (VK_LSHIFT) & 0x8000;
              rshiftDown = ::GetKeyState (VK_RSHIFT) & 0x8000;
            }
            else
            {
              /* @@@ Bah. Win9x can't really distinguish between left and right shift.
                Can't help it. Only emit LShift in all cases. */
              lshiftDown = ::GetKeyState (VK_SHIFT) & 0x8000;
              rshiftDown = false;
            }

	    if ((lshiftState != lshiftDown) || lshiftDown)
	    {
	    #ifdef CS_KEY_DEBUG_ENABLE
	      if (IsKeyboardDebugging()) csPrintf ("Shift quirk: ");
	    #endif
	      DoKey (CSKEY_SHIFT_LEFT, CSKEY_SHIFT, lshiftDown, autoRep, type);
	    }
	    if ((rshiftState != rshiftDown) || rshiftDown)
	    {
	    #ifdef CS_KEY_DEBUG_ENABLE
	      if (IsKeyboardDebugging()) csPrintf ("Shift quirk: ");
	    #endif
	      DoKey (CSKEY_SHIFT_RIGHT, CSKEY_SHIFT, rshiftDown, autoRep, type);
	    }

	    doDoKey = false;
	  }
  	  if (doDoKey) DoKey (raw, cooked, down, autoRep, type);
	}
      }
      return !(wParam == VK_F4 && 
	(modifiersState.modifiers[csKeyModifierTypeAlt] != 0));   // so Alt+F4 still works
    case WM_DEADCHAR:
    case WM_SYSDEADCHAR:
    case WM_CHAR:
    case WM_UNICHAR:
    case WM_SYSCHAR:
      if (wParam != UNICODE_NOCHAR)
      {
	return true; // Pretend we handle it. May get beeps otherwise.
      }
      return false;
    default:
      return false;
  }
}

csPtr<iKeyComposer> csWin32KeyboardDriver::CreateKeyComposer ()
{
  return csPtr<iKeyComposer> (new csWin32KeyComposer ());
}

bool csWin32KeyboardDriver::Win32KeyToCSKey (LONG vKey, LONG keyFlags, 
					     utf32_char& rawCode, 
					     utf32_char& cookedCode,
					     csKeyCharType& charType)
{
  charType = csKeyCharTypeNormal;

#define DISTINGUISH_EXTENDED(nonExt, ext, cooked)	\
      rawCode = (keyFlags & 0x01000000) ? ext : nonExt;	\
      cookedCode = cooked;

  switch (vKey)
  {
    case VK_MENU:
      DISTINGUISH_EXTENDED (CSKEY_ALT_LEFT, CSKEY_ALT_RIGHT, CSKEY_ALT);
      return true;
    case VK_LMENU:
      rawCode = CSKEY_ALT_LEFT;
      cookedCode = CSKEY_ALT;
      return true;
    case VK_RMENU:
      rawCode = CSKEY_ALT_RIGHT;
      cookedCode = CSKEY_ALT;
      return true;
    case VK_CONTROL:  
      DISTINGUISH_EXTENDED (CSKEY_CTRL_LEFT, CSKEY_CTRL_RIGHT, CSKEY_CTRL);
      return true;
    case VK_LCONTROL:
      rawCode = CSKEY_CTRL_LEFT;
      cookedCode = CSKEY_CTRL;
      return true;
    case VK_RCONTROL:
      rawCode = CSKEY_CTRL_RIGHT;
      cookedCode = CSKEY_CTRL;
      return true;
    case VK_SHIFT:
      DISTINGUISH_EXTENDED (CSKEY_SHIFT_LEFT, CSKEY_SHIFT_RIGHT, CSKEY_SHIFT);
      return true;
    case VK_LSHIFT:
      rawCode = CSKEY_SHIFT_LEFT;
      cookedCode = CSKEY_SHIFT;
      return true;
    case VK_RSHIFT:
      rawCode = CSKEY_SHIFT_RIGHT;
      cookedCode = CSKEY_SHIFT;
      return true;
    case VK_UP:       
      /*
        Arrow keys weirdness: if the 'extended' flag is set, they are the block
	left to the keypad. If it's not, then they are KP keys w/ numlock off.
       */
      DISTINGUISH_EXTENDED (CSKEY_PAD8, CSKEY_UP, CSKEY_UP);
      return true;
    case VK_DOWN:     
      DISTINGUISH_EXTENDED (CSKEY_PAD2, CSKEY_DOWN, CSKEY_DOWN);
      return true;
    case VK_LEFT:     
      DISTINGUISH_EXTENDED (CSKEY_PAD4, CSKEY_LEFT, CSKEY_LEFT);
      return true;
    case VK_RIGHT:    
      DISTINGUISH_EXTENDED (CSKEY_PAD6, CSKEY_RIGHT, CSKEY_RIGHT);
      return true;
    case VK_CLEAR:    
      rawCode = CSKEY_PAD5; 
      cookedCode = CSKEY_CENTER; 
      return true;
    case VK_INSERT:   
      DISTINGUISH_EXTENDED (CSKEY_PAD0, CSKEY_INS, CSKEY_INS);
      return true;
    case VK_DELETE:   
      DISTINGUISH_EXTENDED (CSKEY_PADDECIMAL, CSKEY_DEL, CSKEY_DEL);
      return true;
    case VK_PRIOR:    
      DISTINGUISH_EXTENDED (CSKEY_PAD9, CSKEY_PGUP, CSKEY_PGUP);
      return true;
    case VK_NEXT:     
      DISTINGUISH_EXTENDED (CSKEY_PAD3, CSKEY_PGDN, CSKEY_PGDN);
      return true;
    case VK_HOME:     
      DISTINGUISH_EXTENDED (CSKEY_PAD7, CSKEY_HOME, CSKEY_HOME);
      return true;
    case VK_END:
      DISTINGUISH_EXTENDED (CSKEY_PAD1, CSKEY_END, CSKEY_END);
      return true;
    case VK_RETURN:   
      DISTINGUISH_EXTENDED (CSKEY_ENTER, CSKEY_PADENTER, CSKEY_ENTER);
      return true;
    case VK_NUMPAD0:
      rawCode = CSKEY_PAD0;
      cookedCode = '0';
      return true;
    case VK_NUMPAD1:
      rawCode = CSKEY_PAD1;
      cookedCode = '1';
      return true;
    case VK_NUMPAD2:
      rawCode = CSKEY_PAD2;
      cookedCode = '2';
      return true;
    case VK_NUMPAD3:
      rawCode = CSKEY_PAD3;
      cookedCode = '3';
      return true;
    case VK_NUMPAD4:
      rawCode = CSKEY_PAD4;
      cookedCode = '4';
      return true;
    case VK_NUMPAD5:
      rawCode = CSKEY_PAD5;
      cookedCode = '5';
      return true;
    case VK_NUMPAD6:
      rawCode = CSKEY_PAD6;
      cookedCode = '6';
      return true;
    case VK_NUMPAD7:
      rawCode = CSKEY_PAD7;
      cookedCode = '7';
      return true;
    case VK_NUMPAD8:
      rawCode = CSKEY_PAD8;
      cookedCode = '8';
      return true;
    case VK_NUMPAD9:
      rawCode = CSKEY_PAD9;
      cookedCode = '9';
      return true;
    case VK_BACK:     
      rawCode = CSKEY_BACKSPACE;
      cookedCode = CSKEY_BACKSPACE;
      return true;
    case VK_TAB:
      rawCode = CSKEY_TAB;
      cookedCode = CSKEY_TAB;
      return true;
    case VK_ESCAPE:
      rawCode = CSKEY_ESC;
      cookedCode = CSKEY_ESC;
      return true;
    case VK_F1:
      rawCode = CSKEY_F1;
      cookedCode = CSKEY_F1;
      return true;
    case VK_F2:
      rawCode = CSKEY_F2;
      cookedCode = CSKEY_F2;
      return true;
    case VK_F3:
      rawCode = CSKEY_F3;
      cookedCode = CSKEY_F3;
      return true;
    case VK_F4:
      rawCode = CSKEY_F4;
      cookedCode = CSKEY_F4;
      return true;
    case VK_F5:
      rawCode = CSKEY_F5;
      cookedCode = CSKEY_F5;
      return true;
    case VK_F6:
      rawCode = CSKEY_F6;
      cookedCode = CSKEY_F6;
      return true;
    case VK_F7:
      rawCode = CSKEY_F7;
      cookedCode = CSKEY_F7;
      return true;
    case VK_F8:
      rawCode = CSKEY_F8;
      cookedCode = CSKEY_F8;
      return true;
    case VK_F9:
      rawCode = CSKEY_F9;
      cookedCode = CSKEY_F9;
      return true;
    case VK_F10:
      rawCode = CSKEY_F10;
      cookedCode = CSKEY_F10;
      return true;
    case VK_F11:
      rawCode = CSKEY_F11;
      cookedCode = CSKEY_F11;
      return true;
    case VK_F12:
      rawCode = CSKEY_F12;
      cookedCode = CSKEY_F12;
      return true;
    case VK_NUMLOCK:
      rawCode = CSKEY_PADNUM;
      cookedCode = CSKEY_PADNUM;
      return true;
    case VK_PAUSE:
      rawCode = CSKEY_PAUSE;
      cookedCode = CSKEY_PAUSE;
      return true;
    case VK_APPS:
      rawCode = CSKEY_CONTEXT;
      cookedCode = CSKEY_CONTEXT;
      return true;
    case VK_CAPITAL:
      rawCode = CSKEY_CAPSLOCK;
      cookedCode = CSKEY_CAPSLOCK;
      return true;
    case VK_SNAPSHOT:
      rawCode = CSKEY_PRINTSCREEN;
      cookedCode = CSKEY_PRINTSCREEN;
      return true;
    case VK_SCROLL:
      rawCode = CSKEY_SCROLLLOCK;
      cookedCode = CSKEY_SCROLLLOCK;
      return true;
    default:
      {
	WCHAR wCh[2];
	int ret;
	uint8 keystate[256];
	/*
	  For a number of keys, emit a special raw code,
	  but a processed cooked code.
	 */
	switch (vKey)
	{
	  case VK_ADD:
	    rawCode = CSKEY_PADPLUS;
	    break;
	  case VK_SUBTRACT:
	    rawCode = CSKEY_PADMINUS;
	    break;
	  case VK_MULTIPLY:
	    rawCode = CSKEY_PADMULT;
	    break;
	  case VK_DECIMAL:
	    rawCode = CSKEY_PADDECIMAL;
	    break;
	  case VK_DIVIDE:
	    if (keyFlags & 0x01000000)
	    {
	      rawCode = CSKEY_PADDIV;
	      break;
	    }
	    // No extended key?... fall through
	  default:
	    {
	      // First, calculate the raw code. This is done by retrieving the 
	      // key's character without any pressed modifiers.
	      memset (&keystate, 0, sizeof (keystate));
	      if (cswinIsWinNT ())
	      {
		ret = ToUnicode (vKey, keyFlags, keystate, wCh, 2, 0);
	      }
	      else
	      {
		char outCh[2];
		ret = ToAscii (vKey, keyFlags, keystate, (PWORD)&outCh, 0);
		if (ret != 0)
		{
		  MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, outCh, 
		    (ret > 0) ? ret : 0, wCh,  sizeof (wCh) / sizeof (WCHAR));
		}
	      }
	      /*
		In case 2 chars were emitted (couldn't compose from dead key),
		put the last one into 'cooked'.
	      */
	      if (ret > 0)
	      {
		rawCode = wCh[(ret == 1) ? 0 : 1];
	      }
	      else if (ret < 0)
	      {
		// Dead char
		rawCode = wCh[0];
	      }
	      else
	      {
		// Couldn't get a code for that key
		rawCode = cookedCode = 0;
		return false;
	      }
	    }
	}

	// Now, the cooked code

	// Set up modifiers
	ModifiersToKeyState (modifiersState, keystate);
        /* "Inverse ALtGr quirk": to get proper AltGr combos, Ctrl+Alt must be 
         * "pressed", too */
        if (modifiersState.modifiers[csKeyModifierTypeAlt] & 
          (1 << csKeyModifierNumRight))
        {
          keystate[VK_CONTROL] |= 0x80;
        }

	if (cswinIsWinNT ())
	{
	  ret = ToUnicode (vKey, keyFlags, keystate, wCh, 2, 0);
	}
	else
	{
	  char outCh[2];
	  ret = ToAscii (vKey, keyFlags, keystate, (PWORD)&outCh, 0);
	  if (ret != 0)
	  {
	    MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, outCh, 
	      (ret > 0) ? ret : 0, wCh,  sizeof (wCh) / sizeof (WCHAR));
	  }
	}
	if (ret > 0)
	{
	  cookedCode = wCh[(ret == 1) ? 0 : 1];
	  // Composed or normal char. (Composed shouldn't happen.)
	}
  	else if (ret < 0)
	{
	  cookedCode = wCh[0];
	  // Dead char
	  charType = csKeyCharTypeDead;
	}
	else
	{
	  cookedCode = 0;
	}

	return true;
      }
  }
#undef DISTINGUISH_EXTENDED

  return false;
}

const char* csWin32KeyboardDriver::GetVKName (LONG vKey)
{
#ifdef CS_KEY_DEBUG_ENABLE
  const char* vkName = Win32VKeyNames.StringForIdent (vKey);
  if (vkName != 0) return vkName;

  static char genName[13];
  if (((vKey >= 'A') && (vKey <= 'Z')) || ((vKey >= '0') && (vKey <= '9')))
  {
    genName[0] = vKey; genName[1] = 0;
  }
  else
  {
    scsPrintf (genName, "[%ld]", vKey);
  }
  return genName;
#endif
  return 0;
}
