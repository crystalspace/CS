/*
    Crystal Space Windowing System: button class
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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
#include "cssysdef.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"
#include "csws/csbutton.h"
#include "csws/csstatic.h"
#include "csws/csskin.h"

#define SKIN ((csButtonSkin *)skinslice)

csButton::csButton (csComponent *iParent, int iCommandCode,
  int iButtonStyle, csButtonFrameStyle iFrameStyle) : csComponent (iParent),
  ImageNormal (NULL), ImagePressed (NULL),
  FrameNormal (NULL), FramePressed (NULL), FrameHighlighted (NULL),
  delImages (false), delFrameImages(false),
  CommandCode (iCommandCode), underline_pos (-1),
  ButtonStyle (iButtonStyle), FrameStyle (iFrameStyle),
  ButtonAlpha(0), Pressed (false), Highlighted (false) 
{
  SetPalette (CSPAL_BUTTON);
  if (ButtonStyle & CSBS_SELECTABLE)
    SetState (CSS_SELECTABLE, true);
  if (FrameStyle == csbfsOblique || FrameStyle == csbfsTextured || FrameStyle == csbfsBitmap)
    SetState (CSS_TRANSPARENT, true);
  id = iCommandCode;
  ApplySkin (GetSkin ());
}

csButton::~csButton ()
{
  FreeBitmaps ();
  FreeFrameBitmaps();
}

void csButton::SetBitmap (csPixmap *iNormal, csPixmap *iPressed,
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

void csButton::GetBitmap (csPixmap **iNormal, csPixmap **iPressed)
{
  if (iNormal)
    *iNormal = ImageNormal;
  if (iPressed)
    *iPressed = ImagePressed;
}

void csButton::GetFrameBitmaps(csPixmap **iNormal, csPixmap **iPressed, csPixmap **iHighlighted)
{
  if (iNormal)
  	*iNormal = FrameNormal;
  	
  if (iPressed)
  	*iPressed = FramePressed;
  	
  if (iHighlighted)
  	*iHighlighted = FrameHighlighted;
}

void csButton::SetFrameBitmaps(csPixmap *iNormal, csPixmap *iPressed, csPixmap *iHighlighted, bool iDelete)
{
	FreeFrameBitmaps();
	
	FrameNormal = iNormal;
	if (iPressed)
		FramePressed = iPressed;
	else
		FramePressed = iNormal;
		
	if (iHighlighted)
		FrameHighlighted = iHighlighted;
	else
		FrameHighlighted = iNormal;
		
	delFrameImages = iDelete;
	
	Invalidate();
}

void csButton::SetButtonTexture(csPixmap *iNormal, csPixmap *iPressed, bool iDelete)
{
	FreeFrameBitmaps();
	
	FrameNormal = iNormal;
	if (iPressed)
		FramePressed = iPressed;
	else
		FramePressed = iNormal;
		
	FrameHighlighted = NULL;
		
	delFrameImages = iDelete;
	
	Invalidate();
}

void csButton::SetAlpha(uint8 iAlpha)
{
	ButtonAlpha=iAlpha;
}

void csButton::SetTextureOrigin(int iOrgX, int iOrgY)
{
 TexOrgX = iOrgX;
 TexOrgY = iOrgY;
}

void 
csButton::GetTextureOrigin(int *iOrgX, int *iOrgY)
{
 if (iOrgX)
 	*iOrgX = TexOrgX;
 	
 if (iOrgY)
 	*iOrgY = TexOrgY;
}

void csButton::FreeBitmaps ()
{
  if (delImages)
  {
    if (ImageNormal)
      delete ImageNormal;
    if (ImagePressed && ImagePressed != ImageNormal)
      delete ImagePressed;
  } /* endif */
  delImages = false;
  ImageNormal = NULL;
  ImagePressed = NULL;
}

void csButton::FreeFrameBitmaps ()
{
  if (delFrameImages)
  {
  	if (FramePressed && FramePressed != FrameNormal)
      delete FramePressed;
      
   	if (FrameHighlighted && FrameHighlighted != FrameNormal)
      delete FrameHighlighted;

    if (FrameNormal)
      delete FrameNormal;
  } /* endif */
      
  delFrameImages = false;
  FrameNormal = NULL;
  FramePressed = NULL;
  FrameHighlighted = NULL;
}


bool csButton::HandleEvent (iEvent &Event)
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
          iEvent *ev = (iEvent *)Event.Command.Info;
          ev->Key.Code = CSKEY_SPACE;
          return csButton::HandleEvent (*ev);
        }
        case cscmdStaticMouseEvent:
        {
          iEvent *ev = (iEvent *)Event.Command.Info;
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
          if (parent && !GetState (CSS_DISABLED))
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
      break;
      
    case csevMouseEnter:
    	Highlighted=true;
    	Invalidate();
    	if (GetState(CSS_TRANSPARENT)) parent->Invalidate();
    	return true;
    	break;
    	
    case csevMouseExit:
    	Highlighted=false;
    	Invalidate();
    	if (GetState(CSS_TRANSPARENT)) parent->Invalidate();
    	return true;
    	break;
    	
    case csevKeyDown:
      return HandleKeyPress (Event);
    case csevKeyUp:
      if (app->KeyboardOwner)
        if (((underline_pos >= 0)
          && (UPPERCASE (Event.Key.Char) == UPPERCASE (text [underline_pos])))
         || ((Event.Key.Code == CSKEY_SPACE)
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

bool csButton::PostHandleEvent (iEvent &Event)
{
  if (parent->GetState (CSS_FOCUSED)
   && GetState (CSS_VISIBLE)
   && (Event.Type == csevKeyDown))
    return HandleKeyPress (Event);
  return csComponent::PostHandleEvent (Event);
}

bool csButton::HandleKeyPress (iEvent &Event)
{
  // Check hot key
  if (!GetState (CSS_DISABLED))
    if (((underline_pos >= 0)
      && (app->KeyboardOwner == NULL)
      && CheckHotKey (Event, text [underline_pos]))
     || ((GetState (CSS_FOCUSED))
      && (Event.Key.Code == CSKEY_SPACE)
      && (Event.Key.Modifiers & CSMASK_FIRST)
      && (!(Event.Key.Modifiers & (CSMASK_ALLSHIFTS - CSMASK_ALT)))))
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
  {
    parent->SendCommand (Pressed ? cscmdButtonDown : cscmdButtonUp,
      (void *)this);
   
    if (GetState(CSS_TRANSPARENT)) parent->Invalidate();
      
  }
      
      

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
  SKIN->SuggestSize (*this, w, h);
}
