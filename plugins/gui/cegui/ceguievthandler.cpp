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
  md = new csMouseDriver (reg);
}

csCEGUIEventHandler::~csCEGUIEventHandler()
{
  delete md;
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

bool csCEGUIEventHandler::OnMouseDown (iEvent &event)
{
  switch (csMouseEventHelper::GetButton (&event))
  {
    case csmbLeft:
      CEGUI::System::getSingletonPtr()->injectMouseButtonDown(CEGUI::LeftButton);
      break;
    case csmbMiddle:
      CEGUI::System::getSingletonPtr()->injectMouseButtonDown(CEGUI::MiddleButton);
      break;
    case csmbRight:
      CEGUI::System::getSingletonPtr()->injectMouseButtonDown(CEGUI::RightButton);
      break;
    case csmbWheelUp:
      CEGUI::System::getSingletonPtr()->injectMouseWheelChange (1.0f);
      break;
    case csmbWheelDown:
      CEGUI::System::getSingletonPtr()->injectMouseWheelChange (-1.0f);
      break;
  }
  return true;
}

bool csCEGUIEventHandler::OnMouseMove (iEvent &event)
{
  CEGUI::System::getSingletonPtr()->injectMousePosition(csMouseEventHelper::GetX(&event), 
    csMouseEventHelper::GetY(&event));
  return true;
}

bool csCEGUIEventHandler::OnMouseUp (iEvent &event)
{
  switch (csMouseEventHelper::GetButton (&event))
  {
    case csmbLeft:
      CEGUI::System::getSingletonPtr()->injectMouseButtonUp(CEGUI::LeftButton);
      break;
    case csmbMiddle:
      CEGUI::System::getSingletonPtr()->injectMouseButtonUp(CEGUI::MiddleButton);
      break;
    case csmbRight:
      CEGUI::System::getSingletonPtr()->injectMouseButtonUp(CEGUI::RightButton);
      break;
  }
  return true;
}

bool csCEGUIEventHandler::OnKeyboard (iEvent &event) 
{
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&event);
  
  if (eventtype == csKeyEventTypeDown)
  {
    utf32_char code = csKeyEventHelper::GetCookedCode(&event);
    switch (code)
    {
      case CSKEY_ESC:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::Escape);
        return true;
      case CSKEY_BACKSPACE:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::Backspace);
        return true;
      case CSKEY_DEL:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::Delete);
        return true;
      case CSKEY_ENTER:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::Return);
        return true;
      case CSKEY_DOWN:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::ArrowDown);
        return true;
      case CSKEY_UP:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::ArrowUp);
        return true;
      case CSKEY_LEFT:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::ArrowLeft);
        return true;
      case CSKEY_RIGHT:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::ArrowRight);
        return true;
      case CSKEY_PGUP:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::PageDown);
        return true;
      case CSKEY_PGDN:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::PageUp);
        return true;
      case CSKEY_HOME:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::Home);
        return true;
      case CSKEY_END:
        CEGUI::System::getSingletonPtr()->injectKeyDown(CEGUI::Key::End);
        return true;
    }

    CEGUI::System::getSingletonPtr()->injectChar(static_cast<CEGUI::utf32>(code));
    return true;
  }

  return false;
}
