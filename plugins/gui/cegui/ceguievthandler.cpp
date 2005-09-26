/*
    Copyright (C) 2005 Dan Hardfeldt and Seth Yastrov

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

#include "iutil/objreg.h"
#include "csutil/event.h"
#include "csutil/csinput.h"

#include "CEGUI.h"

#include "ceguievthandler.h"
#include "ceguirenderer.h"

csCEGUIEventHandler::csCEGUIEventHandler (iObjectRegistry *reg, 
  csCEGUIRenderer* owner) 
{
  obj_reg = reg;
  renderer = owner;
  csRef<iKeyboardDriver> kbd = csQueryRegistry<iKeyboardDriver> (reg);
  compose = kbd->CreateKeyComposer ();
}

csCEGUIEventHandler::~csCEGUIEventHandler()
{
}

bool csCEGUIEventHandler::Initialize ()
{
  RegisterQueue (obj_reg, CSMASK_MouseMove | CSMASK_MouseDown |
    CSMASK_MouseUp | CSMASK_Keyboard | CSMASK_Broadcast);
  return true;
}

bool csCEGUIEventHandler::OnUnhandledEvent (iEvent &event) 
{
  if (csCommandEventHelper::GetCode (&event) == cscmdContextResize)
  {
    csRef<iGraphics2D> g2d = CS_QUERY_REGISTRY (obj_reg, iGraphics2D);
    renderer->setDisplaySize (CEGUI::Size (g2d->GetWidth (), g2d->GetHeight ()));
    return true;
  }

  return false;
}

CEGUI::MouseButton csCEGUIEventHandler::CSMBtoCEMB (uint button)
{
  CEGUI::MouseButton cemb = CEGUI::NoButton;
  switch (button)
  {
    case csmbLeft:
      cemb = CEGUI::LeftButton;
      break;
    case csmbMiddle:
      cemb = CEGUI::MiddleButton;
      break;
    case csmbRight:
      cemb = CEGUI::RightButton;
      break;
    case csmbExtra1:
      cemb = CEGUI::X1Button;
      break;
    case csmbExtra2:
      cemb = CEGUI::X2Button;
      break;
  }
  return cemb;
}

bool csCEGUIEventHandler::OnMouseDown (iEvent &event)
{
  const uint csmb = csMouseEventHelper::GetButton (&event);
  CEGUI::MouseButton cemb = CSMBtoCEMB (csmb);
  if (cemb != CEGUI::NoButton)
    return CEGUI::System::getSingletonPtr()->injectMouseButtonDown (cemb);
  else
  {
    switch (csMouseEventHelper::GetButton (&event))
    {
      case csmbWheelUp:
	return CEGUI::System::getSingletonPtr()->injectMouseWheelChange (1.0f);
      case csmbWheelDown:
	return CEGUI::System::getSingletonPtr()->injectMouseWheelChange (-1.0f);
    }
  }
  return false;
}

bool csCEGUIEventHandler::OnMouseMove (iEvent &event)
{
  return CEGUI::System::getSingletonPtr()->injectMousePosition (
    csMouseEventHelper::GetX(&event), 
    csMouseEventHelper::GetY(&event));
}

bool csCEGUIEventHandler::OnMouseUp (iEvent &event)
{
  CEGUI::MouseButton cemb = CSMBtoCEMB (
    csMouseEventHelper::GetButton (&event));
  if (cemb != CEGUI::NoButton)
    return CEGUI::System::getSingletonPtr()->injectMouseButtonUp (cemb);
  return false;
}

bool csCEGUIEventHandler::OnKeyboard (iEvent &event) 
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&event);
  
  utf32_char code = csKeyEventHelper::GetRawCode (&event);
  if (CSKEY_IS_PAD_KEY (code))
    /* CEGUI seems to ignore Numpad* keys for e.g. textinput, so chew
     * those keys for it. */
    code = csKeyEventHelper::GetCookedCode (&event);
  uint ceCode = 0;
#define TRANSLATE(CScode, CEcode) \
  case CSKEY_ ## CScode: ceCode = CEGUI::Key:: CEcode; break

  switch (code)
  {
    TRANSLATE(ESC,		Escape);
    TRANSLATE(BACKSPACE,	Backspace);
    TRANSLATE(TAB,		Tab);
    TRANSLATE(SPACE,		Space);
    TRANSLATE(INS,		Insert);
    TRANSLATE(DEL,		Delete);
    TRANSLATE(ENTER,		Return);
    TRANSLATE(DOWN,		ArrowDown);
    TRANSLATE(UP,		ArrowUp);
    TRANSLATE(LEFT,		ArrowLeft);
    TRANSLATE(RIGHT,		ArrowRight);
    TRANSLATE(PGUP,		PageUp);
    TRANSLATE(PGDN,		PageDown);
    TRANSLATE(HOME,		Home);
    TRANSLATE(END,		End);
    TRANSLATE(CONTEXT,		AppMenu);
    TRANSLATE(PRINTSCREEN,	SysRq);
    TRANSLATE(PAUSE,		Pause);
    TRANSLATE(F1,		F1);
    TRANSLATE(F2,		F2);
    TRANSLATE(F3,		F3);
    TRANSLATE(F4,		F4);
    TRANSLATE(F5,		F5);
    TRANSLATE(F6,		F6);
    TRANSLATE(F7,		F7);
    TRANSLATE(F8,		F8);
    TRANSLATE(F9,		F9);
    TRANSLATE(F10,		F10);
    TRANSLATE(F11,		F11);
    TRANSLATE(F12,		F12);
    TRANSLATE(PAD0,		Numpad0);
    TRANSLATE(PAD1,		Numpad1);
    TRANSLATE(PAD2,		Numpad2);
    TRANSLATE(PAD3,		Numpad3);
    TRANSLATE(PAD4,		Numpad4);
    TRANSLATE(PAD5,		Numpad5);
    TRANSLATE(PAD6,		Numpad6);
    TRANSLATE(PAD7,		Numpad7);
    TRANSLATE(PAD8,		Numpad8);
    TRANSLATE(PAD9,		Numpad9);
    TRANSLATE(PADDECIMAL,	Decimal);
    TRANSLATE(PADDIV,		Divide);
    TRANSLATE(PADMULT,		Multiply);
    TRANSLATE(PADMINUS,		Subtract);
    TRANSLATE(PADPLUS,		Add);
    TRANSLATE(PADENTER,		NumpadEnter);
    TRANSLATE(PADNUM,		NumLock);
    TRANSLATE(CAPSLOCK,		Capital);
    TRANSLATE(SCROLLLOCK,	ScrollLock);
    TRANSLATE(SHIFT_LEFT,	LeftShift);
    TRANSLATE(SHIFT_RIGHT,	RightShift);
    TRANSLATE(CTRL_LEFT,	LeftControl);
    TRANSLATE(CTRL_RIGHT,	RightControl);
    TRANSLATE(ALT_LEFT,		LeftAlt);
    TRANSLATE(ALT_RIGHT,	RightAlt);
    default:
      {
	if (CSKEY_IS_MODIFIER(code))
	{
	  if (CSKEY_MODIFIER_TYPE(code) == csKeyModifierTypeShift)
	    ceCode = CEGUI::Key::LeftShift;
	  else if (CSKEY_MODIFIER_TYPE(code) == csKeyModifierTypeCtrl)
	    ceCode = CEGUI::Key::LeftControl;
	  else if (CSKEY_MODIFIER_TYPE(code) == csKeyModifierTypeAlt)
	    ceCode = CEGUI::Key::LeftAlt;
	}
      }
  }
#undef TRANSLATE

  if (ceCode != 0)
  {
    if (eventtype == csKeyEventTypeDown)
      return CEGUI::System::getSingletonPtr()->injectKeyDown (ceCode);
    else
      return CEGUI::System::getSingletonPtr()->injectKeyUp (ceCode);
  }
  else
  {
    if (eventtype == csKeyEventTypeDown)
    {
      // Only send chars on down event to avoid double characters
      csKeyEventData keyData;
      csKeyEventHelper::GetEventData (&event, keyData);
      utf32_char buf[2];
      int num;
      csKeyComposeResult compRes =
	compose->HandleKey (keyData, buf, sizeof (buf) / sizeof (utf32_char),
	  &num);
      bool handle = false;
      if (compRes != csComposeNoChar)
      {
	for (int i = 0; i < num; i++)
	  handle |= CEGUI::System::getSingletonPtr()->injectChar (
	    static_cast<CEGUI::utf32> (buf[i]));
      }
      else
	// Bit of a hack :P
	handle = CEGUI::System::getSingletonPtr()->injectChar (
	  static_cast<CEGUI::utf32> (0));
      if (handle)
	/* If CEGUI handled a character event, remember the key so we can
         * properly catch the corresponding key up event */
	caughtCharKeys.Add (code);
      return handle;
    }
    else if (caughtCharKeys.In (code))
    {
      // Catch "up" event for character down events we caught.
      caughtCharKeys.Delete (code);
      return true;
    }
  }

  return false;
}
