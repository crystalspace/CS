/*
    Crystal Space input library
    Copyright (C) 1998,2000 by Jorrit Tyberghein
    Copyright (C) 2001 by Eric Sunshine <sunshine@sunshineco.com>
    Written by Andrew Zabolotny <bit@eltech.ru>

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
#include "csutil/cfgacc.h"
#include "csutil/csevent.h"
#include "csutil/csinput.h"
#include "csutil/sysfunc.h"
#include "csutil/event.h"
#include "csutil/csuctransform.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"

#ifdef CS_KEY_DEBUG_ENABLE
#include "cskey_identstrs.h"
#endif

#undef NOP
#define NOP ((char)-1)
// This array defines first 32..128 character codes with SHIFT key applied
static char ShiftedKey [128-32] =
{
' ', '!', '"', '#', '$', '%', '&', '"', '(', ')', '*', '+', '<', '_', '>', '?',
')', '!', '@', '#', '$', '%', '^', '&', '*', '(', ':', ':', '<', '+', '>', '?',
'@', NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP,
NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, '{', '|', '}', '^', '_',
'~', NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP,
NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, NOP, '{', '|', '}', '~', NOP
};
#undef NOP

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Input driver <--//--//--

csInputDriver::csInputDriver(iObjectRegistry* r) :
  Registered(false), Registry(r), Listener(0)
{
}

csInputDriver::~csInputDriver()
{
  StopListening();
}

csPtr<iEventQueue> csInputDriver::GetEventQueue()
{
  return CS_QUERY_REGISTRY(Registry, iEventQueue);
}

void csInputDriver::StartListening()
{
  if (Listener != 0 && !Registered)
  {
    csRef<iEventQueue> q (GetEventQueue());
    if (q != 0)
    {
      q->RegisterListener(Listener, CSMASK_Broadcast);
      Registered = true;
    }
  }
}

void csInputDriver::StopListening()
{
  if (Listener != 0 && Registered)
  {
    csRef<iEventQueue> q (GetEventQueue());
    if (q != 0)
      q->RemoveListener(Listener);
  }
  Registered = false;
}

void csInputDriver::Post(iEvent* e)
{
  StartListening(); // If this failed at construction, try again.
  csRef<iEventQueue> q (GetEventQueue());
  if (q != 0)
    q->Post(e);
}

bool csInputDriver::HandleEvent(iEvent& e)
{
  const bool focusChanged = ((e.Type == csevBroadcast)
    && (csCommandEventHelper::GetCode(&e) == cscmdFocusChanged));
  if (focusChanged) 
  {
    if (!csCommandEventHelper::GetInfo(&e)) // Application lost focus.
      LostFocus();
    if (csCommandEventHelper::GetInfo(&e)) // Application gained focus.
      GainFocus();
  }
  return focusChanged;
}

//-----------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE(csKeyComposer)
  SCF_IMPLEMENTS_INTERFACE(iKeyComposer)
SCF_IMPLEMENT_IBASE_END

csKeyComposer::csKeyComposer ()
{
  SCF_CONSTRUCT_IBASE(0);
  lastDead = 0;
}

csKeyComposer::~csKeyComposer ()
{
  SCF_DESTRUCT_IBASE ();
}

/*
  Default dead-key handling.
  Combines a subset of the Unicode diacritical marks(or "dead chars") with a 
  choice of latin letters.
 */

/*
  List of supported diacritical marks.
  Should be sorted ascending.
 */
const utf32_char marks[] = {
  0x005e, // accent circumflex
  0x0060, // accent grave
  0x007e, // tilde
  0x00a8, // diaresis
  0x00b4, // accent acute
  0x02da  // ring
};

const int numMarks = sizeof (marks) / sizeof (utf32_char);

/*
  List of supported latin letters.
  Should be sorted ascending.
 */
const utf32_char latinLetters[] = {
  ' ',
  'A', 'E', 'I', 'O', 'U', 'Y',
  'a', 'e', 'i', 'o', 'u', 'y'
};

const int numLetters = sizeof (latinLetters) / sizeof (utf32_char);

/// A combination is not available.
#undef NOPE
#define NOPE ((utf32_char)-1)

/*
  All combinations of marks and letters we support.
 */
const utf32_char combinedChars[numMarks][numLetters] = {
  {0x005e,
   0x00c2, 0x00ca, 0x00ce, 0x00d4, 0x00db, 0x0176,
   0x00e2, 0x00ea, 0x00ee, 0x00f4, 0x00fb, 0x0177},
  {0x0060,
   0x00c0, 0x00c8, 0x00cc, 0x00d2, 0x00d9, NOPE,
   0x00e0, 0x00e8, 0x00ec, 0x00f2, 0x00f9, NOPE},
  {0x007e,
   0x00c3, NOPE,   0x0128, 0x00d5, 0x0168, NOPE,
   0x00e3, NOPE,   0x0129, 0x00f5, 0x0169, NOPE},
  {0x00a8,
   0x00c4, 0x00cb, 0x00cf, 0x00d6, 0x00dc, 0x00ff,
   0x00e4, 0x00eb, 0x00ef, 0x00f6, 0x00fc, 0x0178},
  {0x00b4,
   0x00c1, 0x00c9, 0x00cd, 0x00d3, 0x00da, 0x00dd,
   0x00e2, 0x00e9, 0x00ed, 0x00f3, 0x00fa, 0x00fd},
  {0x02da,
   0x00c5, NOPE,   NOPE,   NOPE,   0x016e, NOPE,
   0x00e5, NOPE,   NOPE,   NOPE,   0x016f, NOPE}
};

csKeyComposeResult csKeyComposer::HandleKey (
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

  if (lastDead != 0)
  {
    utf32_char dead = lastDead;
    lastDead = 0;

    int p = -1;
    int l = 0, r = numMarks - 1;

    while (l <= r)
    {
      int m = (l + r) / 2;
      if (marks[m] == dead)
      {
	p = m;
	break;
      }
      else if (marks[m] > dead)
	r = m - 1;
      else
	l = m + 1;
    }
    if (p == -1)
    {
      RETURN2(csComposeUncomposeable, dead, keyEventData.codeCooked)
    }
    else
    {
      const utf32_char* converted = combinedChars[p];
      p = -1; l = 0; r = numLetters;
      
      while (l <= r)
      {
	int m = (l + r) / 2;
	if (latinLetters[m] == keyEventData.codeCooked)
	{
	  p = m;
	  break;
	}
	else if (latinLetters[m] > keyEventData.codeCooked)
	  r = m - 1;
	else
	  l = m + 1;
      }
      if (p == -1)
      {
	RETURN2(csComposeUncomposeable, dead, keyEventData.codeCooked)
      }
      else
      {
	utf32_char combined = converted[p];
	if (combined == NOPE)
	  RETURN2(csComposeUncomposeable, dead, keyEventData.codeCooked)
	else
	  RETURN1(csComposeComposedChar, converted[p])
      }
    }
  }
  else
  {
    if (keyEventData.charType == csKeyCharTypeDead)
    {
      lastDead = keyEventData.codeCooked;
      RETURN0(csComposeNoChar)
    }
    else
      RETURN1(csComposeNormalChar, keyEventData.codeCooked)
  }

#undef RETURN
#undef RETURN0
#undef RETURN1
#undef RETURN2
}

#undef NOPE

void csKeyComposer::ResetState ()
{
  lastDead = 0;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Keyboard driver <--//--/

SCF_IMPLEMENT_IBASE(csKeyboardDriver)
  SCF_IMPLEMENTS_INTERFACE(iKeyboardDriver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(csKeyboardDriver::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csKeyboardDriver::csKeyboardDriver (iObjectRegistry* r) :
  csInputDriver(r)
{
  SCF_CONSTRUCT_IBASE(0);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);

  memset (&modifiersState, 0, sizeof (modifiersState));

  keyDebugChecked = false;

  Listener = &scfiEventHandler;
  StartListening();
}

csKeyboardDriver::~csKeyboardDriver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_IBASE();
}

void csKeyboardDriver::Reset ()
{
  memset (&modifiersState, 0, sizeof (modifiersState));

  csHash<bool, utf32_char>::GlobalIterator keyIter =
    keyStates.GetIterator ();
  
  while (keyIter.HasNext ())
  {
    utf32_char rawCode;
    bool isDown = keyIter.Next (rawCode);
    if (isDown)
    {
      DoKey (rawCode, 0, false); // @@@ Hmmm... synthesizes cooked all the time
    }
  }
}

void csKeyboardDriver::RestoreKeys ()
{
  /*
    This should set the 'key down' flags and emit KeyDown events for all
    actually pressed keys. However, retrieving the pressed keys is highly 
    platform-dependent, so this method needs to overridden or alternatively
    DoKey() should be called for each pressed key by some other object with 
    knowledge about the keyboard, e.g. the canvas.
   */
}

void csKeyboardDriver::DoKey (utf32_char codeRaw, utf32_char codeCooked, 
			      bool iDown, bool autoRepeat, 
			      csKeyCharType charType)
{
  if (codeCooked == 0)
  {
    SynthesizeCooked (codeRaw, modifiersState, codeCooked);
  }

  if (IsKeyboardDebugging())
  {
    csPrintf ("raw: %s cooked: %s %s%s\n", GetKeycodeString (codeRaw), 
      GetKeycodeString (codeCooked), iDown ? "down" : "up", 
      autoRepeat ? " autoRepeat" : "");
  }

  /*
    Set state before the event if the key is released, otherwise
    after. This is mainly for modifier keys, so if you press Alt
    you won't get an Alt modifier, neither in the up nor the
    down event. 
   */
  if (!iDown) SetKeyState (codeRaw, iDown, autoRepeat);

  // @@@ Pooled events, somehow?
  csRef<iEvent> ev;
  ev.AttachNew (new csEvent ());
  ev->Type = csevKeyboard;
  ev->Category = ev->SubCategory = ev->Flags = 0;
  ev->Add ("keyEventType", 
    (uint8)(iDown ? csKeyEventTypeDown : csKeyEventTypeUp));
  ev->Add ("keyCodeRaw", (uint32)codeRaw);
  ev->Add ("keyCodeCooked", (uint32)codeCooked);
  ev->Add ("keyModifiers", &modifiersState, sizeof (modifiersState));
  // For auto-repeat, set a flag
  ev->Add ("keyAutoRepeat", autoRepeat);
  ev->Add ("keyCharType", (uint8)charType);
  ev->Time = csGetTicks ();
  Post (ev);

  if (iDown) SetKeyState (codeRaw, iDown, autoRepeat);
}

void csKeyboardDriver::SynthesizeCooked (utf32_char codeRaw, 
					 const csKeyModifiers& modifiers, 
					 utf32_char& codeCooked)
{
  if (CSKEY_IS_SPECIAL (codeRaw))
  {
    if (CSKEY_IS_MODIFIER (codeRaw))
      codeCooked = CSKEY_MODIFIER (CSKEY_MODIFIER_TYPE (codeRaw), 
	csKeyModifierNumAny);
    else if (CSKEY_IS_PAD_KEY (codeRaw))
      codeCooked = CSKEY_PAD_TO_NORMAL (codeRaw);
    else
      codeCooked = codeRaw;
  }
  else
  {
    if (modifiers.modifiers[csKeyModifierTypeAlt] != 0)
      codeCooked = 0;
    else if (modifiers.modifiers[csKeyModifierTypeCtrl] != 0)
    {
      if ((codeRaw >= 'A') && (codeRaw <= 'Z'))
	codeCooked = codeRaw - 'A' + 1;
      else if ((codeRaw >= 'a') && (codeRaw <= 'z'))
	codeCooked = codeRaw - 'a' + 1;
      else
	codeCooked = 0;
    }
    else if (modifiers.modifiers[csKeyModifierTypeShift] != 0)
    {
      char newCode;
      if ((codeRaw < 32) || (codeRaw > 127)
        || ((newCode = ShiftedKey [codeRaw - 32]) == (char)-1))
      {
        csUnicodeTransform::MapToUpper(codeRaw, &codeCooked, 1, csUcMapSimple);
      }
      else
        codeCooked = newCode;
    }
    else
      codeCooked = 0;
  }
}

csEventError csKeyboardDriver::SynthesizeCooked (iEvent *ev)
{
  utf32_char codeRaw, codeCooked;
  csKeyModifiers mods;

  csEventError err = ev->Retrieve ("keyCodeRaw", codeRaw);
  if (err != csEventErrNone) return err;

  SynthesizeCooked (codeRaw, mods, codeCooked);

  ev->Add ("keyCodeCooked", codeCooked);
  ev->Add ("keyModifiers", (void *) & mods, sizeof (mods));

  return csEventErrNone;
}

// This really should be inside csKeyboardDriver::GetKeycodeString, but Cygwin
// crashes on exit if functions have local static variables with complex types
static csString genName;

const char* csKeyboardDriver::GetKeycodeString (utf32_char code)
{
#ifdef CS_KEY_DEBUG_ENABLE
  const char* str = KeyCodeNames.StringForIdent (code);
  if (str != 0) return str;

  genName.Format ("[%" PRIu32 "]", code);
  return genName;
#endif
  return 0;
}

bool csKeyboardDriver::IsKeyboardDebugging ()
{
#ifndef CS_KEY_DEBUG_ENABLE
  return false;
#endif
  // Delay checking b/c at construction time the config manager is prolly
  // not set up yet.
  // @@@ Should probably controlled via "--verbose".
  if (!keyDebugChecked)
  {
    csConfigAccess cfgAccess (Registry);
    keyDebug = cfgAccess->GetBool ("KeyboardDriver.EnableDebugging");
    keyDebugChecked = true;
  }
  return keyDebug;
}

void csKeyboardDriver::SetKeyState (utf32_char codeRaw, bool iDown,
				    bool autoRepeat)
{
  if (CSKEY_IS_MODIFIER (codeRaw))
  {
    int modType = CSKEY_MODIFIER_TYPE(codeRaw);
    int modNum = CSKEY_MODIFIER_NUM(codeRaw);

    if (modType >= csKeyModifierTypeLast) return;

    // Caps/Scroll/NumLock are treated specially
    if ((modType == csKeyModifierTypeCapsLock) ||
      (modType == csKeyModifierTypeNumLock) ||
      (modType == csKeyModifierTypeScrollLock))
    {
      // Toggle their state when the key is pressed down.
      if (iDown && !autoRepeat)
      {
        bool state = modifiersState.modifiers[modType] != 0;
        modifiersState.modifiers[modType] = state ? 0 : 1;
      }

      keyStates.PutUnique (codeRaw, iDown);
    }
    else
    {
      if (modNum == csKeyModifierNumAny)
      {
	modifiersState.modifiers[modType] = iDown ? 0xffffffff : 0;
      }
      else
      {
	if (iDown)
	  modifiersState.modifiers[modType] |= (1 << modNum);
	else
	  modifiersState.modifiers[modType] &= ~(1 << modNum);

	keyStates.PutUnique (codeRaw, iDown);
      }
    }
  }
  else
    keyStates.PutUnique (codeRaw, iDown);
}

bool csKeyboardDriver::GetKeyState (utf32_char codeRaw) const
{
  if (CSKEY_IS_MODIFIER (codeRaw) && 
    (CSKEY_MODIFIER_NUM(codeRaw) == csKeyModifierNumAny))
  {
    return (GetModifierState (codeRaw) != 0);
  }
  else
    return keyStates.Get (codeRaw, false);
}

uint32 csKeyboardDriver::GetModifierState (utf32_char codeRaw) const
{
  if (CSKEY_IS_MODIFIER (codeRaw))
  {
    int modType = CSKEY_MODIFIER_TYPE(codeRaw);
    int modNum = CSKEY_MODIFIER_NUM(codeRaw);

    if (modType >= csKeyModifierTypeLast) return 0;

    if (modNum == csKeyModifierNumAny)
    {
      return modifiersState.modifiers[modType];
    }
    else
    {
      return (modifiersState.modifiers[modType] & (1 << modNum));
    }
  }
  return 0;
}

csPtr<iKeyComposer> csKeyboardDriver::CreateKeyComposer ()
{
  return csPtr<iKeyComposer> (new csKeyComposer ());
}

//--//--//--//--//--//--//--//--//--//--//--//--//--//--> Mouse driver <--//--/

SCF_IMPLEMENT_IBASE(csMouseDriver)
  SCF_IMPLEMENTS_INTERFACE(iMouseDriver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(csMouseDriver::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE(iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csMouseDriver::csMouseDriver (iObjectRegistry* r) :
  csInputDriver(r)
{
  SCF_CONSTRUCT_IBASE(0);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  Listener = &scfiEventHandler;
  StartListening();
  for (int iter=0 ; iter<CS_MAX_MOUSE_COUNT ; iter++) {
    memset (Button[iter], 0, sizeof(bool) * CS_MAX_MOUSE_BUTTONS);
    memset (Last[iter], 0, sizeof(int) * CS_MAX_MOUSE_AXES);
  }
  memset (Axes, 0, sizeof(Axes));
  Reset();

  csConfigAccess cfg;
  cfg.AddConfig(Registry, "/config/mouse.cfg");
  SetDoubleClickTime (
    cfg->GetInt ("MouseDriver.DoubleClickTime", 300),
    cfg->GetInt ("MouseDriver.DoubleClickDist", 2));
}

csMouseDriver::~csMouseDriver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_IBASE();
}

void csMouseDriver::Reset ()
{
  for (int i = 0; i < CS_MAX_MOUSE_COUNT; i++) {
    for (int j = 0; j < CS_MAX_MOUSE_BUTTONS; j++)
      if (Button [i][j])
	DoButton (i + 1, j + 1, false, Last[i], Axes[i]);
    LastClickButton[i] = (uint)-1;
  }
}

iKeyboardDriver* csMouseDriver::GetKeyboardDriver()
{
  if (Keyboard == 0)
    Keyboard = CS_QUERY_REGISTRY(Registry, iKeyboardDriver);
  return Keyboard;
}

/**
 * Try to post a new mouse button event.
 * 
 * \todo Building the key-modifiers mask is broken, needs \see iKeyboardDriver 
 *   support to fix.
 */
void csMouseDriver::DoButton (uint number, uint button, bool down, 
                              const int32 *axes, uint numAxes)
{
  if (number <= 0 || number > CS_MAX_MOUSE_COUNT)
    return;

  if (memcmp(Last[number - 1], axes, numAxes * sizeof(int))!=0)
    DoMotion (number, axes, numAxes);

  if (button <= 0 || button > CS_MAX_MOUSE_BUTTONS)
    return;

  iKeyboardDriver* k = GetKeyboardDriver();
  int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
            | (k->GetKeyState (CSKEY_ALT  ) ? CSMASK_ALT   : 0)
            | (k->GetKeyState (CSKEY_CTRL ) ? CSMASK_CTRL  : 0);

  Button [number - 1][button - 1] = down;

  csRef<iEvent> ev;
  csTicks evtime = csGetTicks();
  ev.AttachNew (new csEvent (evtime,
			     down ? csevMouseDown : csevMouseUp, 
			     number, axes, numAxes, 0, 
			     button, smask));
  Post(ev);

  if ((button == LastClickButton[number - 1])
      && (evtime - LastClickTime[number - 1] <= DoubleClickTime)) {
    for (uint iter=0 ; iter<Axes[number - 1] ; iter++)
      if (unsigned (ABS (axes[iter] - LastClick[number - 1][iter])) >
	  DoubleClickDist)
	goto mousedown;
    csRef<iEvent> ev;
    ev.AttachNew (new csEvent (evtime,
			       down ? csevMouseDoubleClick : csevMouseClick, 
			       number, axes, numAxes, 0, button, smask));
    Post (ev);
    // Don't allow for sequential double click events
    if (down)
      LastClickButton [number - 1] = (uint)-1;
  }
  else if (down)
  {
  mousedown:
    // Remember the coordinates/button/position of last mousedown event
    LastClickButton[number - 1] = button;
    LastClickTime[number - 1] = evtime;
    for (uint iter=0; iter<Axes[number - 1]; iter++)
      LastClick[number - 1][iter] = axes[iter];
  }
}

/**
 * Try to post a new mouse motion event.
 * 
 * \todo Building the key-modifiers mask is broken, needs \see iKeyboardDriver 
 *   support to fix.
 */
void csMouseDriver::DoMotion (uint number, const int32 *axes, uint numAxes)
{
  uint32 cflags = 0;
  if (number <= 0 || number > CS_MAX_MOUSE_COUNT)
    return;

  for (uint iter=0; iter<numAxes ; iter++)
    if (Last [number - 1][iter] != axes[iter])
      cflags |= (1 << iter);

  if (cflags==0)
    return; /* no change to report */

  iKeyboardDriver* k = GetKeyboardDriver();
  int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
    | (k->GetKeyState (CSKEY_ALT  ) ? CSMASK_ALT   : 0)
    | (k->GetKeyState (CSKEY_CTRL ) ? CSMASK_CTRL  : 0);
  memcpy(Last [number - 1], axes, numAxes * sizeof(int));
  Axes [number - 1] = numAxes;

  csRef<iEvent> ev;
  ev.AttachNew (new csEvent (csGetTicks (), csevMouseMove, 
			     number, axes, numAxes, cflags, 0, smask));
  Post (ev);
}

void csMouseDriver::SetDoubleClickTime (int iTime, size_t iDist)
{
  DoubleClickTime = iTime;
  DoubleClickDist = iDist;
}

//--//--//--//--//--//--//--//--//--//--//--//--//--/> Joystick driver <--//--/

SCF_IMPLEMENT_IBASE(csJoystickDriver)
  SCF_IMPLEMENTS_INTERFACE(iJoystickDriver)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iEventHandler)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csJoystickDriver::eiEventHandler)
  SCF_IMPLEMENTS_INTERFACE (iEventHandler)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csJoystickDriver::csJoystickDriver (iObjectRegistry* r) :
  csInputDriver(r)
{
  SCF_CONSTRUCT_IBASE(0);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  Listener = &scfiEventHandler;
  StartListening();
  for (int iter=0 ; iter<CS_MAX_JOYSTICK_COUNT ; iter++) {
    memset (Button[iter], 0, sizeof(bool) * CS_MAX_JOYSTICK_BUTTONS);
    memset (Last[iter], 0, sizeof(int) * CS_MAX_JOYSTICK_AXES);
  }
  memset (Axes, 0, sizeof(Axes));
}

csJoystickDriver::~csJoystickDriver ()
{
  SCF_DESTRUCT_EMBEDDED_IBASE(scfiEventHandler);
  SCF_DESTRUCT_IBASE();
}

void csJoystickDriver::Reset ()
{
  for (int i = 0; i < CS_MAX_JOYSTICK_COUNT; i++)
    for (int j = 0; j < CS_MAX_JOYSTICK_BUTTONS; j++)
      if (Button [i][j])
        DoButton (i + 1, j + 1, false, Last [i], Axes [i]);
}

iKeyboardDriver* csJoystickDriver::GetKeyboardDriver()
{
  if (Keyboard == 0)
    Keyboard = CS_QUERY_REGISTRY(Registry, iKeyboardDriver);
  return Keyboard;
}

/**
 * Try to post a new joystick button event.
 * 
 * \todo Building the key-modifiers mask is broken, needs \see iKeyboardDriver 
 *  support to fix.
 */
void csJoystickDriver::DoButton (uint number, uint button, bool down,
				 const int32 *axes, uint numAxes)
{
  if (number <= 0 || number > CS_MAX_JOYSTICK_COUNT)
    return;

  if (memcmp(Last[number - 1], axes, numAxes * sizeof(int))!=0)
    DoMotion(number, axes, numAxes);

  if (button <= 0 || button > CS_MAX_JOYSTICK_BUTTONS)
    return;

  iKeyboardDriver* k = GetKeyboardDriver();
  int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
            | (k->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
            | (k->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);

  Button [number - 1][button - 1] = down;

  csRef<iEvent> ev;
  ev.AttachNew (new csEvent (csGetTicks (),
			     down ? csevJoystickDown : csevJoystickUp, number, 
			     axes, numAxes, 0, button, smask));
  Post(ev);
}

/**
 * Try to post a new joystick motion event.
 * 
 * \todo Building the key-modifiers mask is broken, needs \see iKeyboardDriver 
 *  support to fix.
 */
void csJoystickDriver::DoMotion (uint number, const int32 *axes, uint numAxes)
{
  uint32 cflags = 0;
  if (number <= 0 || number > CS_MAX_JOYSTICK_COUNT)
    return;

  for (uint iter=0 ; iter<numAxes ; iter++)
    if (Last[number - 1][iter] != axes[iter])
      cflags |= (1 << iter);

  if (cflags==0)
    return; /* no change to report */

  iKeyboardDriver* k = GetKeyboardDriver();
  int smask = (k->GetKeyState (CSKEY_SHIFT) ? CSMASK_SHIFT : 0)
    | (k->GetKeyState (CSKEY_ALT)   ? CSMASK_ALT   : 0)
    | (k->GetKeyState (CSKEY_CTRL)  ? CSMASK_CTRL  : 0);
  memcpy(Last [number - 1], axes, numAxes * sizeof(int));
  Axes [number - 1] = numAxes;

  csRef<iEvent> ev;
  ev.AttachNew (new csEvent(csGetTicks(), csevJoystickMove, 
			    number, axes, numAxes, cflags, 0, smask));
  Post(ev);
}
