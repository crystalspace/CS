/*
    Crystal Space Windowing System: static control class
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

#include <ctype.h>

#include "csws/csstatic.h"
#include "csws/csapp.h"
#include "csws/cswsaux.h"
#include "csutil/event.h"

csStatic::csStatic (csComponent *iParent, csComponent *iLink, const char *iText,
  csStaticStyle iStyle) : csComponent (iParent)
{
  Init (iStyle);
  SetText (iText);
  link = iLink;
  SetSuggestedSize (0, 0);
  CheckUp ();
}

csStatic::csStatic (csComponent *iParent, csStaticStyle iStyle)
  : csComponent (iParent)
{
  Init (iStyle);
  SetSuggestedSize (0, 0);
}

csStatic::csStatic (csComponent *iParent, csPixmap *iBitmap)
  : csComponent (iParent)
{
  Init (csscsBitmap);
  Bitmap = iBitmap;
  SetSuggestedSize (0, 0);
}

csStatic::~csStatic ()
{
  delete Bitmap;
}

void csStatic::Init (csStaticStyle iStyle)
{
  Bitmap = 0;
  underline_pos = (size_t)-1;
  link = 0;
  SetPalette (CSPAL_STATIC);
  style = iStyle;
  if (style != csscsRectangle)
    state |= CSS_TRANSPARENT;
  TextAlignment = CSSTA_LEFT | CSSTA_VCENTER;
  linkactive = false;
  linkdisabled = false;
}

void csStatic::Draw ()
{
  int disabled = link ? linkdisabled : false;
  int textcolor = disabled ? CSPAL_STATIC_DTEXT :
    linkactive ? CSPAL_STATIC_ATEXT : CSPAL_STATIC_ITEXT;
  int fonth;
  GetTextSize ("", &fonth);
  switch (style)
  {
    case csscsEmpty:
      break;
    case csscsLabel:
      DrawUnderline (0, 0, text, underline_pos, textcolor);
      break;
    case csscsFrameLabel:
    {
      int txtx = GetTextSize ("///");
      int fryt = (fonth - 1) / 2;
      int fryb = bound.Height () - fryt;
      int txtw = GetTextSize (text);

      Line (0, fryt, txtx, fryt, CSPAL_STATIC_DARK3D);
      Line (txtx + txtw, fryt, bound.Width (), fryt, CSPAL_STATIC_DARK3D);
      Line (bound.Width () - 2, fryt, bound.Width () - 2, fryb, CSPAL_STATIC_DARK3D);
      Line (bound.Width () - 2, fryb - 1, 1, fryb - 1, CSPAL_STATIC_DARK3D);
      Line (0, fryb + 1, 0, fryt, CSPAL_STATIC_DARK3D);

      Line (1, fryt + 1, txtx, fryt + 1, CSPAL_STATIC_LIGHT3D);
      Line (txtx + txtw, fryt + 1, bound.Width () - 1, fryt + 1, CSPAL_STATIC_LIGHT3D);
      Line (bound.Width () - 1, fryt + 1, bound.Width () - 1, fryb, CSPAL_STATIC_LIGHT3D);
      Line (bound.Width (), fryb, 1, fryb, CSPAL_STATIC_LIGHT3D);
      Line (1, fryb, 1, fryt + 1, CSPAL_STATIC_LIGHT3D);

      Text (txtx, 0, textcolor, -1, text);
      DrawUnderline (txtx, 0, text, underline_pos, textcolor);
      break;
    }
    case csscsBitmap:
      if (Bitmap && Bitmap->GetTextureHandle ())
      {
        Pixmap (Bitmap, 0, 0, bound.Width (), bound.Height ());
        break;
      }
    case csscsRectangle:
      Clear (CSPAL_STATIC_BACKGROUND);
      break;
    case csscsText:
      break;
  } /* endswitch */

  if (text && (style != csscsFrameLabel))
  {
    int x, y;
    switch (TextAlignment & CSSTA_HALIGNMASK)
    {
      case CSSTA_LEFT:    x = 0; break;
      case CSSTA_RIGHT:   x = bound.Width () - GetTextSize (text); break;
      case CSSTA_HCENTER: x = (bound.Width () - GetTextSize (text)) / 2; break;
      default:            return;
    } /* endswitch */
    switch (TextAlignment & CSSTA_VALIGNMASK)
    {
      case CSSTA_TOP:     y = 0; break;
      case CSSTA_BOTTOM:  y = bound.Height () - fonth; break;
      case CSSTA_VCENTER: y = (bound.Height () - fonth) / 2; break;
      default:            return;
    } /* endswitch */
    if (!(TextAlignment & CSSTA_WRAPMASK))
      Text (x, y, textcolor, -1, text);
    else
    {
      int lines_avail = bound.Height () / fonth;
      // remember where the lines start
      char **starts = new char * [strlen (text)];
      // count the lines
      char s[2], *t, *p = csStrNew(text);
      int i;
      int rowstart = 0, rowend = 0; // area within bound we gonna write in
      int line = 0; // runs through the lines we write
      int lines = 0; // total number of lines in text
      int len;
      t = strtok (p, "\n");
      s[1] = '\0';
      while (t)
      {
        starts [lines] = t;
        lines++;
        while (*t)
        {
          s [0] = *t;
          len = 0;
          while ((len += GetTextSize (s)) <= bound.Width () && *++t)
            s [0] = *t;
          if (*t)
          {
            starts [lines] = t;
            lines++;
          }
        }
        t = strtok (0, "\n");
      }

      // determine the starting line
      switch (TextAlignment & CSSTA_VALIGNMASK)
      {
        case CSSTA_TOP:
          rowstart = 0; rowend = MIN (lines, lines_avail);
          line = rowend - 1;
          break;
        case CSSTA_BOTTOM:
          rowstart = lines_avail - MIN (lines_avail, lines); rowend = lines_avail;
          line = lines - 1;
          break;
        case CSSTA_VCENTER:
          rowstart = MAX (0, (lines_avail - lines) / 2);
          rowend   = MIN (lines_avail, rowstart + lines);
          line = lines - 1 - MAX (0, lines - lines_avail) / 2;
          break;
      } /* endswitch */

      strcpy (p, text);
      // now draw every line
      for (i = rowend - 1; i >= rowstart; i--, line--)
      {
        if (line < lines - 1)
          if (*(starts [line + 1] - 1) == '\n')
            *(starts [line + 1] - 1) = '\0';
	 else
	   *(starts [line + 1]) = '\0';
        t = starts [line];

        switch (TextAlignment & CSSTA_HALIGNMASK)
        {
          case CSSTA_LEFT:    x = 0; break;
          case CSSTA_RIGHT:   x = bound.Width () - GetTextSize (t); break;
          case CSSTA_HCENTER: x = (bound.Width () - GetTextSize (t)) / 2; break;
        } /* endswitch */
        y = i * fonth;
        Text (x, y, textcolor, -1, t);
      }

      delete [] p;
      delete [] starts;
    }
  }
  csComponent::Draw ();
}

bool csStatic::IsHotKey (iEvent &Event)
{
  return ((underline_pos != (size_t)-1)
    && ((csKeyEventHelper::GetModifiersBits (&Event) & CSMASK_CTRL) == 0)
       && (toupper (csKeyEventHelper::GetCookedCode (&Event)) == 
       toupper (text [underline_pos])));
}

bool csStatic::HandleEvent (iEvent &Event)
{
  CheckUp ();

  switch (Event.Type)
  {
    case csevCommand:
      switch (Event.Command.Code)
      {
        case cscmdStaticGetBitmap:
          Event.Command.Info = (intptr_t)Bitmap;
          break;
        case cscmdStaticSetBitmap:
          if (style == csscsBitmap)
          {
            Bitmap = (csPixmap *)Event.Command.Info;
            Event.Command.Info = 0;
            Invalidate ();
          }
          break;
      } /* endswitch */
      break;
    case csevMouseDown:
      if ((style == csscsLabel)
       || (style == csscsFrameLabel))
      {
        // for frames, check if mouse is within label text
        if (style == csscsFrameLabel)
        {
          int fh, xmin = GetTextSize ("///", &fh);
          csRect r (xmin, 0, xmin + GetTextSize (text), fh);
          if (!r.Contains (Event.Mouse.x, Event.Mouse.y))
            break;
        }
        if (!app->MouseOwner
         && link)
        {
          app->CaptureMouse (this);
          link->SendCommand (cscmdStaticMouseEvent, (intptr_t)&Event);
          // if link did not captured the mouse, release it
          if (app->MouseOwner == this)
            app->CaptureMouse (0);
          CheckUp ();
          return true;
        } /* endif */
      } /* endif */
      break;
  } /* endswitch */
  return csComponent::HandleEvent (Event);
}

bool csStatic::PostHandleEvent (iEvent &Event)
{
  CheckUp ();
  if ((style == csscsLabel)
   || (style == csscsFrameLabel))
    switch (Event.Type)
    {
      case csevKeyboard:
	if (csKeyEventHelper::GetEventType (&Event) == csKeyEventTypeDown)
	{
	  if (!app->KeyboardOwner
	  && parent->GetState (CSS_FOCUSED)
	  && IsHotKey (Event)
	  && link)
	  {
	    link->Select ();
	    oldKO = app->CaptureKeyboard (this);
	    link->SendCommand (cscmdStaticHotKeyEvent, (intptr_t)&Event);
	    CheckUp ();
	    return true;
	  }
	}
	else
	{
	  if (app->KeyboardOwner
	  && IsHotKey (Event)
	  && link)
	  {
	    link->SendCommand (cscmdStaticHotKeyEvent, (intptr_t)&Event);
	    app->CaptureKeyboard (oldKO);
	    CheckUp ();
	    return true;
	  }
	}
        break;
    } /* endswitch */
  return csComponent::PostHandleEvent (Event);
}

void csStatic::SuggestSize (int &w, int &h)
{
  w = 0; h = 0;
  switch (style)
  {
    case csscsEmpty:
      break;
    case csscsRectangle:
      break;
    case csscsLabel:
    case csscsFrameLabel:
    case csscsText:
      if (text)
        w = GetTextSize (text, &h);
      break;
    case csscsBitmap:
      if (Bitmap)
      {
        w = Bitmap->Width ();
        h = Bitmap->Height ();
      } /* endif */
      break;
  } /* endswitch */
  if (style == csscsLabel)
    h++;
}

void csStatic::SetText (const char *iText)
{
  if (style == csscsText)
    csComponent::SetText (iText);
  else
  {
    PrepareLabel (iText, text, underline_pos);
    Invalidate ();
  }
}

void csStatic::CheckUp ()
{
  if (!link)
    return;
  bool newlinkactive = !!link->GetState (CSS_FOCUSED);
  bool newlinkdisabled = !!link->GetState (CSS_DISABLED);
  if ((linkactive != newlinkactive) || (linkdisabled != newlinkdisabled))
  {
    linkdisabled = newlinkdisabled;
    linkactive = newlinkactive;
    Invalidate ();
  }
}
