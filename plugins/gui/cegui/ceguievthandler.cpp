/*
    Copyright (C) 2005 Dan Härdfeldt and Seth Yastrov

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "cssysdef.h"

#include "iutil/objreg.h"
#include "csutil/event.h"
#include "csutil/csinput.h"

#include "ceguievthandler.h"
#include "ceguirenderer.h"

csCEGUIEventHandler::csCEGUIEventHandler (iObjectRegistry *reg, csCEGUIRenderer* owner, CEGUI::System* system) 
{
  obj_reg = reg;
  renderer = owner;
  md = new csMouseDriver (reg);
  ceguisystem = system;
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
    iGraphics2D* g2d = (iGraphics2D*) csCommandEventHelper::GetInfo (&event);
    renderer->setDisplaySize (CEGUI::Size (g2d->GetWidth (), g2d->GetHeight ()));
    return true;
  }
  if (csCommandEventHelper::GetCode (&event) == cscmdSystemClose)
  {
    delete renderer->GetSystem().getSingletonPtr();
    renderer->destroyAllTextures();
    return true;
  }

  return false;
}

bool csCEGUIEventHandler::OnMouseDown (iEvent &event)
{
  switch (csMouseEventHelper::GetButton (&event))
  {
    case csmbLeft:
      ceguisystem->injectMouseButtonDown(CEGUI::LeftButton);
      break;
    case csmbMiddle:
      ceguisystem->injectMouseButtonDown(CEGUI::MiddleButton);
      break;
    case csmbRight:
      ceguisystem->injectMouseButtonDown(CEGUI::RightButton);
      break;
    case csmbWheelUp:
      ceguisystem->injectMouseWheelChange (1.0f);
      break;
    case csmbWheelDown:
      ceguisystem->injectMouseWheelChange (-1.0f);
      break;
  }
  return true;
}

bool csCEGUIEventHandler::OnMouseMove (iEvent &event)
{
  ceguisystem->injectMousePosition(csMouseEventHelper::GetX(&event), csMouseEventHelper::GetY(&event));
  return true;
}

bool csCEGUIEventHandler::OnMouseUp (iEvent &event)
{
  switch (csMouseEventHelper::GetButton (&event))
  {
    case csmbLeft:
      ceguisystem->injectMouseButtonUp(CEGUI::LeftButton);
      break;
    case csmbMiddle:
      ceguisystem->injectMouseButtonUp(CEGUI::MiddleButton);
      break;
    case csmbRight:
      ceguisystem->injectMouseButtonUp(CEGUI::RightButton);
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
        ceguisystem->injectKeyDown(CEGUI::Key::Escape);
        return true;
      case CSKEY_BACKSPACE:
        ceguisystem->injectKeyDown(CEGUI::Key::Backspace);
        return true;
      case CSKEY_DEL:
        ceguisystem->injectKeyDown(CEGUI::Key::Delete);
        return true;
      case CSKEY_ENTER:
        ceguisystem->injectKeyDown(CEGUI::Key::Return);
        return true;
      case CSKEY_DOWN:
        ceguisystem->injectKeyDown(CEGUI::Key::ArrowDown);
        return true;
      case CSKEY_UP:
        ceguisystem->injectKeyDown(CEGUI::Key::ArrowUp);
        return true;
      case CSKEY_LEFT:
        ceguisystem->injectKeyDown(CEGUI::Key::ArrowLeft);
        return true;
      case CSKEY_RIGHT:
        ceguisystem->injectKeyDown(CEGUI::Key::ArrowRight);
        return true;
      case CSKEY_PGUP:
        ceguisystem->injectKeyDown(CEGUI::Key::PageDown);
        return true;
      case CSKEY_PGDN:
        ceguisystem->injectKeyDown(CEGUI::Key::PageUp);
        return true;
      case CSKEY_HOME:
        ceguisystem->injectKeyDown(CEGUI::Key::Home);
        return true;
      case CSKEY_END:
        ceguisystem->injectKeyDown(CEGUI::Key::End);
        return true;
    }

    ceguisystem->injectChar(static_cast<CEGUI::utf32>(code));
    return true;
  }

  return false;
}
