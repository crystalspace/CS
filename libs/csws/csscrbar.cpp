/*
    Crystal Space Windowing System: scroll bar class
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

#include "cssysdef.h"
#include "csws/csscrbar.h"
#include "csws/csapp.h"
#include "csutil/event.h"

// Minimal scroll button size
#define CSSB_MINIMAL_KNOBSIZE	(3+3+8)
/// Minimal scroll bar size
#define CSSB_MINIMAL_SIZE	(2+2+7)

// Scrolling state
#define SCROLL_UL			1001	// scroll up or left (h/v scrollbars)
#define SCROLL_DR			1002	// scroll down or right
#define SCROLL_PAGE_UL		1003	// scroll up or left by pages
#define SCROLL_PAGE_DR		1004	// scroll down or right by pages

// Period of time to wait before scroll autorepeat
#define SCROLL_START_INTERVAL   500
// Scroll time interval in milliseconds
#define SCROLL_REPEAT_INTERVAL  100

#define SCROLLBAR_TEXTURE_NAME  "csws::ScrollBar"

csPixmap *csScrollBar::sprarrows[12] =
{ 0, 0, 0, 0, 0, 0, 0, 0 };

csPixmap *csScrollBar::sprscroller[2] =
{ 0, 0 };

static int scrbarref = 0;

csScrollBar::csScrollBar (csComponent *iParent, csScrollBarFrameStyle iFrameStyle)
  : csComponent (iParent)
{
  active_button = status.value = status.maxvalue = status.size = status.maxsize
    = status.step = status.pagestep = 0;

  scrbarref++;
  if (app && !sprscroller[0])
  {
    // Load arrow and scroller images
    iTextureHandle *scrolltex = app->GetTexture (SCROLLBAR_TEXTURE_NAME);
	int i;
    for (i = 0; i < 12; i++)
      sprarrows [i] = new csSimplePixmap (scrolltex, i * 9, 0, 9, 9);
    sprscroller [0] = new csSimplePixmap (scrolltex, 12 * 9 + 0, 0, 7, 8);
    sprscroller [1] = new csSimplePixmap (scrolltex, 12 * 9 + 7, 0, 8, 7);
  } /* endif */

  TrackScroller = false;
  FrameStyle = iFrameStyle;
  SetPalette (CSPAL_SCROLLBAR);

  // create both scroll buttons
  csButtonFrameStyle bfs = (FrameStyle == cssfsThickRect ? csbfsThickRect : csbfsThinRect);
  const int bs = CSBS_NOMOUSEFOCUS | CSBS_NODEFAULTBORDER;
  scroller = new csButton (this, cscmdNothing, bs, bfs);
  topleft  = new csButton (this, cscmdNothing, bs, bfs);
  topleft->id = SCROLL_UL;
  botright = new csButton (this, cscmdNothing, bs, bfs);
  botright->id = SCROLL_DR;

  // create repeat timer
  timer = new csTimer (this, SCROLL_REPEAT_INTERVAL);

  ApplySkin (GetSkin ());
}

csScrollBar::~csScrollBar ()
{
  if (--scrbarref == 0)
  {
	int i;
    for (i = 0; i < 12; i++)
    {
      delete sprarrows [i];
      sprarrows [i] = 0;
    } /* endfor */
    delete sprscroller [0]; sprscroller[0] = 0;
    delete sprscroller [1]; sprscroller[1] = 0;
  } /* endif */
}

bool csScrollBar::HandleEvent (iEvent &Event)
{
  switch (Event.Type)
  {
    case csevMouseMove:
      // track mouse motion when scroller button is pressed
      if (TrackScroller)
      {
        int x = Event.Mouse.x - scroller->bound.xmin;
        int y = Event.Mouse.y - scroller->bound.ymin;
        if ((x != scrollerdx)
         || (y != scrollerdy))
        {
          int delta = (IsHorizontal ? x - scrollerdx : y - scrollerdy)
             * status.maxvalue / activepixlen;
          SetValue (status.value + delta);
        } /* endif */
        return true;
      } /* endif */
      if (app->MouseOwner == this)
      {
        if (!bound.ContainsRel (Event.Mouse.x, Event.Mouse.y))
        {
          active_button = 0;
          Invalidate ();
        } else if (active_button == 0)
          goto pagescroll;
        return true;
      } /* endif */
      break;
    case csevMouseDown:
      if (GetState (CSS_DISABLED))
        return true;
      if (csComponent::HandleEvent (Event))
      {
        // Switch mouse owner to us if scroller captured it
        if (app->MouseOwner == scroller)
        {
          app->CaptureMouse (this);
          TrackScroller = true;
        } /* endif */
        return true;
      } /* endif */
      if ((Event.Mouse.Button == 1)
       && bound.ContainsRel (Event.Mouse.x, Event.Mouse.y)
       && (status.maxvalue > 0))
      {
pagescroll:
        int cmp;

        if (scroller->bound.IsEmpty ())
          if (IsHorizontal)
            cmp = bound.Width () / 2;
          else
            cmp = bound.Height () / 2;
        else
          if (IsHorizontal)
            cmp = scroller->bound.xmin;
          else
            cmp = scroller->bound.ymin;

        if ((IsHorizontal ? Event.Mouse.x : Event.Mouse.y) <= cmp)
          active_button = SCROLL_PAGE_UL;
        else
          active_button = SCROLL_PAGE_DR;
        Invalidate ();

        if (app->MouseOwner != this)
          app->CaptureMouse (this);
        // Emulate timeout
        timer->Pause (SCROLL_START_INTERVAL);
        SendCommand (cscmdTimerPulse, (intptr_t)timer);
        return true;
      } else
      {
        active_button = 0;
        Invalidate ();
      } /* endif */
      return true;
    case csevMouseUp:
      if (Event.Mouse.Button == 1)
      {
        if (TrackScroller)
        {
          app->CaptureMouse (scroller);
          TrackScroller = false;
          return scroller->HandleEvent (Event);
        } /* endif */
        if (app->MouseOwner == this)
        {
          app->CaptureMouse (0);
          active_button = 0;
          Invalidate ();
        } /* endif */
      } /* endif */
      break;
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdButtonUp:
          active_button = 0;
          Invalidate ();
          return true;
        case cscmdButtonDown:
          if (Event.Command.Info == (intptr_t)scroller)
          {
            scroller->GetMousePosition (scrollerdx, scrollerdy);
            return true;
          } /* endif */
          active_button = ((csComponent *)Event.Command.Info)->id;
          timer->Pause (SCROLL_START_INTERVAL);
          // fallback to timer pulse
          goto pulse;
        case cscmdTimerPulse:
          if (Event.Command.Info == (intptr_t)timer)
          {
pulse:      if (active_button == SCROLL_UL)
              SetValue (status.value - status.step);
            else if (active_button == SCROLL_DR)
              SetValue (status.value + status.step);
            else if (active_button == SCROLL_PAGE_UL)
              SetValue (status.value - status.pagestep);
            else if (active_button == SCROLL_PAGE_DR)
              SetValue (status.value + status.pagestep);
            return true;
          }
          break;
        case cscmdScrollBarSet:
        {
          status = *((csScrollBarStatus *)Event.Command.Info);
          if (status.size > status.maxsize)
            status.size = status.maxsize;
          int oldvalue = status.value;
          status.value = -1;
          SetValue (oldvalue);
          Event.Command.Info = 0;
          return true;
        }
        case cscmdScrollBarGetStatus:
          *((csScrollBarStatus *)Event.Command.Info) = status;
          Event.Command.Info = 0;
          return true;
        case cscmdScrollBarQueryValue:
          Event.Command.Info = status.value;
          return true;
        case cscmdScrollBarSetValue:
          SetValue (int (Event.Command.Info));
          return true;
      } /* endswitch */
      break;
    case csevKeyboard:
      if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case CSKEY_UP:
	  case CSKEY_LEFT:
	  case CSKEY_DOWN:
	  case CSKEY_RIGHT:
	    if (csKeyEventHelper::GetModifiersBits (&Event) & 
	      (CSMASK_ALT | CSMASK_SHIFT))
	      break;
	    if (IsHorizontal != 
	      ((csKeyEventHelper::GetCookedCode (&Event) == CSKEY_LEFT) || 
	      (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_RIGHT)))
	      break;
	    if (!app->MouseOwner)
	    {
	      if (!app->KeyboardOwner)
		app->CaptureKeyboard (this);

	      int delta = (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_UP) || 
		(csKeyEventHelper::GetCookedCode (&Event) == CSKEY_LEFT) ? -1 : +1;
	      delta *= (csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_CTRL) ? 
		status.pagestep : status.step;

	      SetValue (status.value + delta);
	    } /* endif */
	    return true;
	} /* endswitch */
      }
      else
      {
	switch (csKeyEventHelper::GetCookedCode (&Event))
	{
	  case CSKEY_UP:
	  case CSKEY_LEFT:
	  case CSKEY_DOWN:
	  case CSKEY_RIGHT:
	    if (IsHorizontal != 
	      ((csKeyEventHelper::GetCookedCode (&Event) == CSKEY_LEFT) || 
	      (csKeyEventHelper::GetCookedCode (&Event) == CSKEY_RIGHT)))
	      break;
	    if (app->KeyboardOwner == this)
	      app->CaptureKeyboard (0);
	    return true;
	} /* endswitch */
      }
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

bool csScrollBar::SetRect (int xmin, int ymin, int xmax, int ymax)
{
  if (csComponent::SetRect (xmin, ymin, xmax, ymax))
  {
    int w = bound.Width ();
    int h = bound.Height ();
    if (w > h)
    {
      // horizontal scroll bar
      IsHorizontal = true;

      if ((w / 2 < h) || (h < CSSB_MINIMAL_SIZE))
      {
        // make buttons invisible
        if (topleft)
          topleft->SetRect (0, 0, -1, h);
        if (botright)
          botright->SetRect (w, 0, w, h);
      } else
      {
        if (topleft)
          topleft->SetRect (0, 0, h, h);
        if (botright)
          botright->SetRect (w - h, 0, w, h);
      } /* endif */
    } else
    {
      // vertical scroll bar
      IsHorizontal = false;
      if ((h / 2 < w) || (w < CSSB_MINIMAL_SIZE))
      {
        // make buttons invisible
        if (topleft)
          topleft->SetRect (0, 0, w, -1);
        if (botright)
          botright->SetRect (0, h, w, h);
      } else
      {
        if (topleft)
          topleft->SetRect (0, 0, w, w);
        if (botright)
          botright->SetRect (0, h - w, w, h);
      } /* endif */
    } /* endif */
    SetValue (status.value);
    return true;
  } else
    return false;
}

void csScrollBar::SetValue (int iValue)
{
  if (iValue < 0)
    iValue = 0;
  if (iValue > status.maxvalue)
    iValue = status.maxvalue;
  if (status.value == iValue)
    return;
  status.value = iValue;

  bool disable = GetState (CSS_DISABLED) ? true : false;
  scroller->SetState (CSS_DISABLED, disable);

  if (disable || (status.maxvalue <= 0))
  {
noscrollbut:
    if (IsHorizontal)
      scroller->SetRect (bound.Width () / 2, 0, bound.Width () / 2, -1);
    else
      scroller->SetRect (0, bound.Height () / 2, -1, bound.Height () / 2);
  }
  else
  {
    int pixmin, pixmax;

    if (IsHorizontal)
    {
      pixmin = topleft->bound.xmax;
      pixmax = botright->bound.xmin;
    } else
    {
      pixmin = topleft->bound.ymax;
      pixmax = botright->bound.ymin;
    } /* endif */

    activepixlen = pixmax - pixmin;
    if ((activepixlen + 2 < CSSB_MINIMAL_KNOBSIZE) || (status.maxsize <= 0))
      goto noscrollbut;

    int pixsize = (status.size * activepixlen) / status.maxsize;
    if (pixsize < CSSB_MINIMAL_KNOBSIZE)
      pixsize = CSSB_MINIMAL_KNOBSIZE;
    if (pixsize >= activepixlen)
      goto noscrollbut;

    pixmin += pixsize / 2;
    pixmax -= ((pixsize + 1) / 2);
    activepixlen = pixmax - pixmin;
    int spix = pixmin + (status.value * activepixlen) / status.maxvalue
      - pixsize / 2;
    int epix = spix + pixsize;
    if (IsHorizontal)
      scroller->SetRect (spix, 0, epix, bound.Height ());
    else
      scroller->SetRect (0, spix, bound.Width (), epix);
  } /* endif */

  // Set up arrows on scroll buttons
  if (IsHorizontal)
  {
    scroller->SetBitmap (sprscroller[1], 0, false);
    if (disable || (status.maxvalue <= 0) || (status.value <= 0))
    {
      topleft->SetBitmap (sprarrows[8], 0, false);
      topleft->SetState (CSS_DISABLED, true);
    } else
    {
      topleft->SetBitmap (sprarrows[6], sprarrows[10], false);
      topleft->SetState (CSS_DISABLED, false);
    } /* endif */
    if (disable || (status.maxvalue <= 0) || (status.value >= status.maxvalue))
    {
      botright->SetBitmap (sprarrows[9], 0, false);
      botright->SetState (CSS_DISABLED, true);
    } else
    {
      botright->SetBitmap (sprarrows[7], sprarrows[11], false);
      botright->SetState (CSS_DISABLED, false);
    } /* endif */
  }
  else
  {
    scroller->SetBitmap (sprscroller[0], 0, false);
    if (disable || (status.maxvalue <= 0) || (status.value <= 0))
    {
      topleft->SetBitmap (sprarrows[2], 0, false);
      topleft->SetState (CSS_DISABLED, true);
    } else
    {
      topleft->SetBitmap (sprarrows[0], sprarrows[4], false);
      topleft->SetState (CSS_DISABLED, false);
    } /* endif */
    if (disable || (status.maxvalue <= 0) || (status.value >= status.maxvalue))
    {
      botright->SetBitmap (sprarrows[3], 0, false);
      botright->SetState (CSS_DISABLED, true);
    } else
    {
      botright->SetBitmap (sprarrows[1], sprarrows[5], false);
      botright->SetState (CSS_DISABLED, false);
    } /* endif */
  } /* endif */

  if (parent)
    parent->SendCommand (cscmdScrollBarValueChanged, (intptr_t)this);
}

void csScrollBar::SetState (int mask, bool enable)
{
  int oldstate = state;
  csComponent::SetState (mask, enable);
  if ((oldstate ^ state) & CSS_DISABLED)
  {
    int oldval = status.value--;
    SetValue (oldval);
  }
}
