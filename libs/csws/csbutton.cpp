/*
    Crystal Space Windowing System: button class
    Copyright (C) 1998 by Jorrit Tyberghein
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

#include <string.h>

#define SYSDEF_CASE
#include "sysdef.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"
#include "csws/csbutton.h"
#include "csws/csstatic.h"

csButton::csButton (csComponent *iParent, int iCommandCode,
  int iButtonStyle, csButtonFrameStyle iFrameStyle) : csComponent (iParent),
  ImageNormal (NULL), ImagePressed (NULL), delImages (false),
  CommandCode (iCommandCode), underline_pos (-1),
  ButtonStyle (iButtonStyle), FrameStyle (iFrameStyle),
  Pressed (false)
{
  SetPalette (CSPAL_BUTTON);
  if (ButtonStyle & CSBS_SELECTABLE)
    SetState (CSS_SELECTABLE, true);
  if (FrameStyle == csbfsOblique)
    SetState (CSS_TRANSPARENT, TRUE);
  id = iCommandCode;
}

csButton::~csButton ()
{
  FreeBitmaps ();
}

void csButton::SetBitmap (csSprite2D *iNormal, csSprite2D *iPressed,
  bool iDelete)
{
  FreeBitmaps ();
  ImageNormal = iNormal;
  if (iPressed)
    ImagePressed = iPressed;
  else
    ImagePressed = iNormal;
  delImages = iDelete;
  Invalidate ();
}

void csButton::GetBitmap (csSprite2D **iNormal, csSprite2D **iPressed)
{
  if (iNormal)
    *iNormal = ImageNormal;
  if (iPressed)
    *iPressed = ImagePressed;
}

void csButton::FreeBitmaps ()
{
  if (delImages)
  {
    if (ImageNormal)
      CHKB (delete ImageNormal);
    if (ImagePressed && ImagePressed != ImageNormal)
      CHKB (delete ImagePressed);
  } /* endif */
  delImages = false;
  ImageNormal = NULL;
  ImagePressed = NULL;
}

void csButton::Draw ()
{
  int li, di;                   // light and dark colors
  int areaw, areah;             // drawing area width and height

  if (Pressed)
  {
    di = CSPAL_BUTTON_LIGHT3D;
    li = CSPAL_BUTTON_DARK3D;
  } else
  {
    di = CSPAL_BUTTON_DARK3D;
    li = CSPAL_BUTTON_LIGHT3D;
  } /* endif */

  DefaultBorder = ((ButtonStyle & CSBS_NODEFAULTBORDER) == 0)
    && ((parent->GetDefault () == this));

  // Draw the frame
  switch (FrameStyle)
  {
    case csbfsNone:
      if (!GetState (CSS_TRANSPARENT))
        Box (0, 0, bound.Width (), bound.Height (), CSPAL_BUTTON_BACKGROUND);
      areaw = bound.Width (); areah = bound.Height ();
      break;
    case csbfsOblique:
      if (bound.Height () >= 6)
      {
        int aw = bound.Height () / 3;
        if (DefaultBorder)
          ObliqueRect3D (0, 0, bound.Width (), bound.Height (), aw,
            CSPAL_BUTTON_DEFFRAME, CSPAL_BUTTON_DEFFRAME);
        else
          ObliqueRect3D (0, 0, bound.Width (), bound.Height (), aw,
            CSPAL_BUTTON_LIGHT3D, CSPAL_BUTTON_DARK3D);
        ObliqueRect3D (1, 1, bound.Width () - 1, bound.Height () - 1, aw - 1, di, li);
        ObliqueRect3D (2, 2, bound.Width () - 2, bound.Height () - 2, aw - 2, di, li);
        int rvy = bound.Height ();
        // Fill the button background
        int dx = aw - 1;
        rvy = bound.Height () - 1;
        for (int i = 3; i < aw; i++, dx--)
        {
          Line (dx, i, bound.Width () - 4, i, CSPAL_BUTTON_BACKGROUND);
          Line (3, rvy - i, bound.Width () - dx - 1, rvy - i, CSPAL_BUTTON_BACKGROUND);
        } /* endfor */
        Box (3, aw, bound.Width () - 3, bound.Height () - aw, CSPAL_BUTTON_BACKGROUND);
        areaw = bound.Width () - 6; areah = bound.Height () - 6;
        break;
      } // otherwise fallback to rectangular frame
    case csbfsThickRect:
      if (DefaultBorder)
        Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_BUTTON_DEFFRAME, CSPAL_BUTTON_DEFFRAME);
      else
        Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_BUTTON_LIGHT3D, CSPAL_BUTTON_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1, di, li);
      Rect3D (2, 2, bound.Width () - 2, bound.Height () - 2, di, li);
      Box (3, 3, bound.Width () - 3, bound.Height () - 3, CSPAL_BUTTON_BACKGROUND);
      areaw = bound.Width () - 6; areah = bound.Height () - 6;
      break;
    case csbfsThinRect:
      if (DefaultBorder)
        Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_BUTTON_DEFFRAME, CSPAL_BUTTON_DEFFRAME);
      else
        Rect3D (0, 0, bound.Width (), bound.Height (),
          CSPAL_BUTTON_LIGHT3D, CSPAL_BUTTON_DARK3D);
      Rect3D (1, 1, bound.Width () - 1, bound.Height () - 1, di, li);
      Box (2, 2, bound.Width () - 2, bound.Height () - 2, CSPAL_BUTTON_BACKGROUND);
      areaw = bound.Width () - 4; areah = bound.Height () - 4;
      break;
    case csbfsVeryThinRect:
      if (Pressed)
      {
        Rect3D (0, 0, bound.Width (), bound.Height (), di, li);
        areaw = bound.Width () - 2; areah = bound.Height () - 2;
        Box (1, 1, bound.Width () - 1, bound.Height () - 1, CSPAL_BUTTON_BACKGROUND);
      }
      else
      {
        areaw = bound.Width (); areah = bound.Height ();
        Box (0, 0, bound.Width (), bound.Height (), CSPAL_BUTTON_BACKGROUND);
      } /* endif */
      break;
    default:
      return;
  } /* endswitch */

  // Calculate image position
  int imgx = 0, imgy = 0, imgw = 0, imgh = 0;
  csSprite2D *img = Pressed ? ImagePressed : ImageNormal;
  if (img)
  {
    imgw = img->Width (); imgh = img->Height ();
    if (imgw > areaw) imgw = areaw;
    if (imgh > areah) imgh = areah;
    imgx = (bound.Width () - imgw) / 2;
    imgy = (bound.Height () - imgh) / 2;
    if ((ButtonStyle & CSBS_SHIFT) && Pressed)
    { imgx++; imgy++; }
  } /* endif */

  // Calculate text position
  int txtx = 0, txty = 0;
  if (text)
  {
    txtx = (bound.Width () - TextWidth (text)) / 2;
    if (img)
      switch (ButtonStyle & CSBS_TEXTPLACEMENT)
      {
        case CSBS_TEXTABOVE:
          imgy += TextHeight () / 2;
          txty = imgy - TextHeight () - 1;
          break;
        case CSBS_TEXTBELOW:
          imgy -= TextHeight () / 2;
          txty = imgy + img->Height () + 1;
          break;
      } /* endswitch */
    else
      txty = (bound.Height () - TextHeight ()) / 2;
    if ((ButtonStyle & CSBS_SHIFT) && Pressed)
    { txtx++; txty++; }
  }

  // Draw image
  if (img)
    Sprite2D (img, imgx, imgy, imgw, imgh);
  // Draw text
  if (text)
  {
    Text (txtx, txty, GetState (CSS_DISABLED) ? CSPAL_BUTTON_DTEXT
      : CSPAL_BUTTON_TEXT, -1, text);
    if (!GetState (CSS_DISABLED))
      DrawUnderline (txtx, txty, text, underline_pos, CSPAL_BUTTON_TEXT);
  }

  csComponent::Draw ();
}

bool csButton::HandleEvent (csEvent &Event)
{
  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdAreYouDefault:
          if (GetState (CSS_FOCUSED) || (ButtonStyle & CSBS_DEFAULT))
            Event.Command.Info = this;
          return true;
        case cscmdButtonDeselect:
          if ((ButtonStyle & CSBS_MULTICHOOSE)
           && Pressed)
            SetPressed (false);
          return true;
        case cscmdActivate:
          Event.Command.Info = this;
          Press ();
          return true;
        case cscmdStaticHotKeyEvent:
        {
          csEvent *ev = (csEvent *)Event.Command.Info;
          ev->Key.Code = ' ';
          return csButton::HandleEvent (*ev);
        }
        case cscmdStaticMouseEvent:
        {
          csEvent *ev = (csEvent *)Event.Command.Info;
          if (app->MouseOwner)
          {
            int dX = 0, dY = 0;
            app->MouseOwner->LocalToGlobal (dX, dY);
            GlobalToLocal (dX, dY);
            // release mouse ownership so that csButton::HandleEvent can capture it
            app->CaptureMouse (NULL);
            if ((ev->Type == csevMouseMove)
             && app->MouseOwner->bound.ContainsRel (ev->Mouse.x, ev->Mouse.y))
              ev->Mouse.x = ev->Mouse.y = 0;
            else
            {
              ev->Mouse.x -= dX;
              ev->Mouse.y -= dY;
            } /* endif */
          } /* endif */
          return csButton::HandleEvent (*ev);
        }
      } /* endswitch */
      break;
    case csevMouseDown:
    case csevMouseDoubleClick:
      switch (Event.Mouse.Button)
      {
        case 1:
          if (GetState (CSS_DISABLED)
           || app->MouseOwner)
            return true;
          app->CaptureMouse (this);
          SetPressed (true);
          if ((ButtonStyle & CSBS_NOMOUSEFOCUS) == 0)
            csComponent::HandleEvent (Event);
          return true;
        case 2:
          if (parent)
            parent->SendCommand (cscmdButtonRightClick, (void *)this);
          return true;
      }
      break;
    case csevMouseUp:
      if (Event.Mouse.Button == 1)
      {
        if (app->MouseOwner == this)
        {
          app->CaptureMouse (NULL);
          if (Pressed)
          {
            if (!(ButtonStyle & CSBS_MULTICHOOSE))
              SetPressed (false);
            Press ();
          }
        }
        return true;
      }
      break;
    case csevMouseMove:
      if ((app->MouseOwner == this)
       && !(ButtonStyle & CSBS_MULTICHOOSE))
      {
        bool inside = bound.ContainsRel (Event.Mouse.x, Event.Mouse.y);
        if (inside != Pressed)
          SetPressed (inside);
        return true;
      } /* endif */
      return true;
    case csevKeyDown:
      return HandleKeyPress (Event);
    case csevKeyUp:
      if (app->KeyboardOwner)
        if (((underline_pos >= 0)
          && (UPPERCASE (Event.Key.Code) == UPPERCASE (text [underline_pos])))
         || ((Event.Key.Code == ' ')
          && (GetState (CSS_FOCUSED))))
        {
          if (!(ButtonStyle & CSBS_MULTICHOOSE))
            SetPressed (false);
          Invalidate ();
          app->CaptureKeyboard (NULL);
          Press ();
          return true;
        }
      return false;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

bool csButton::PostHandleEvent (csEvent &Event)
{
  if (parent->GetState (CSS_FOCUSED)
   && GetState (CSS_VISIBLE)
   && (Event.Type == csevKeyDown))
    return HandleKeyPress (Event);
  return csComponent::PostHandleEvent (Event);
}

bool csButton::HandleKeyPress (csEvent &Event)
{
  // Check hot key
  if (((underline_pos >= 0)
    && ((Event.Key.ShiftKeys & CSMASK_CTRL) == 0)
    && (Event.Key.ShiftKeys & CSMASK_FIRST)
    && (UPPERCASE (Event.Key.Code) == UPPERCASE (text [underline_pos]))
    && (app->KeyboardOwner == NULL))
   || ((GetState (CSS_FOCUSED))
    && (Event.Key.Code == ' ')
    && (Event.Key.ShiftKeys & CSMASK_FIRST)
    && (!(Event.Key.ShiftKeys & (CSMASK_ALLSHIFTS - CSMASK_ALT)))))
  {
    if (!app->KeyboardOwner)
      app->CaptureKeyboard (this);
    SetPressed (true);
    if ((ButtonStyle & CSBS_NOKEYBOARDFOCUS) == 0)
      Select ();
    return true;
  }
  return false;
}

void csButton::DeselectNeighbours ()
{
  csComponent *c = this;
  while (((c = c->next) != this)
     && !c->GetState (CSS_GROUP))
    c->SendCommand (cscmdButtonDeselect, this);
  if ((c != this) && (!GetState (CSS_GROUP)))
  {
    c = this;
    while ((c = c->prev) != this)
    {
      c->SendCommand (cscmdButtonDeselect, this);
      if (c->GetState (CSS_GROUP))
        break;
    } /* endwhile */
  } /* endif */
}

void csButton::SetPressed (bool state)
{
  if ((ButtonStyle & CSBS_MULTICHOOSE)
   && state)
    DeselectNeighbours ();

  Pressed = state;
  if (parent)
    parent->SendCommand (Pressed ? cscmdButtonDown : cscmdButtonUp,
      (void *)this);

  Invalidate ();
}

void csButton::Press ()
{
  if (ButtonStyle & CSBS_DISMISS)
    app->Dismiss (CommandCode);
  if (parent && CommandCode)
    parent->SendCommand (CommandCode, (void *)this);
}

void csButton::SetState (int mask, bool enable)
{
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if (((oldstate ^ state) & CSS_FOCUSED)
   || ((oldstate ^ state) & CSS_DISABLED))
  {
    if (((state & CSS_FOCUSED) == 0)
     && (app->KeyboardOwner == this))
    {
      SetPressed (false);
      if (app->KeyboardOwner == this)
        app->CaptureKeyboard (NULL);
    } /* endif */
    if ((state & CSS_DISABLED)
     && (app->MouseOwner))
    {
      if (app->MouseOwner == this)
        app->CaptureMouse (NULL);
      if (Pressed)
        if (!(ButtonStyle & CSBS_MULTICHOOSE))
          SetPressed (false);
    } /* endif */
    Invalidate ();
  }
}

void csButton::SuggestSize (int &w, int &h)
{
  w = 0; h = 0;
  if (ImagePressed)
  {
    w = ImagePressed->Width ();
    h = ImagePressed->Height ();
  } /* endif */
  if (ImageNormal)
  {
    if (w < ImageNormal->Width ())
      w = ImageNormal->Width ();
    if (h < ImageNormal->Height ())
      h = ImageNormal->Height ();
  } /* endif */

  if (text)
  {
    int tw = TextWidth (text) + 8;
    int th = TextHeight () + 4;
    if (tw > w) w = tw;
    h += th;
  } /* endif */

  switch (FrameStyle)
  {
    case csbfsOblique:
      h += 6;
      w += (h / 3) * 2;
      break;
    case csbfsThickRect:
      w += 6;
      h += 6;
      break;
    case csbfsThinRect:
      w += 4;
      h += 4;
      break;
    case csbfsVeryThinRect:
      w += 1;
      h += 1;
      break;
    case csbfsNone:
    default:
      break;
  } /* endswitch */
}
