/*
    Crystal Space Windowing System: Dwfault window skin
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
#include "csws/cswindow.h"
#include "csws/csapp.h"
#include "csws/cswstex.h"
#include "csws/cswsutil.h"
#include "csws/sdefault.h"
#include "ivaria/reporter.h"
#include "qint.h"

#define TITLEBAR_TEXTURE_NAME	"csws::TitlebarButtons"

void csDefaultWindowSkin::Initialize (csApp *iApp, csSkin *Parent)
{
  Skin = Parent;
  if (ButtonTex)
    ButtonTex->DecRef ();
  ButtonTex = iApp->GetTexture (TITLEBAR_TEXTURE_NAME);
  if (ButtonTex)
    ButtonTex->IncRef ();
  else
  {
    iApp->Printf (CS_REPORTER_SEVERITY_ERROR,
      "Cannot find texture %s for default window skin\n", TITLEBAR_TEXTURE_NAME);
    return;	// @@@ Return value?
  }

  Parent->Load (Back, "Window", "Background");
}

void csDefaultWindowSkin::Deinitialize ()
{
  if (ButtonTex)
  {
    ButtonTex->DecRef ();
    ButtonTex = 0;
  }
  Back.Free ();
}

csButton *csDefaultWindowSkin::CreateButton (csWindow &This, int ButtonID)
{
  int command;
  const char *defid;
  switch (ButtonID)
  {
    case CSWID_BUTSYSMENU:
      command = cscmdWindowSysMenu;
      defid = "SysMenu";
      break;
    case CSWID_BUTCLOSE:
      command = cscmdClose;
      defid = "Close";
      break;
    case CSWID_BUTHIDE:
      command = cscmdHide;
      defid = "Hide";
      break;
    case CSWID_BUTMAXIMIZE:
      command = cscmdMaximize;
      defid = "Maximize";
      break;
//  case CSWID_BUTMINIMIZE:
//    command = cscmdMinimize;
//    defid = "Minimize";
//    break;
    default:
      return 0;
  }

  csButton *bt = new csButton (&This, command, 0, csbfsNone);
  SetButtBitmap (bt, defid);
  bt->id = ButtonID;

  return bt;
}

void csDefaultWindowSkin::SetButtBitmap (csButton *button, const char *id)
{
  if (!button || !ButtonTex)
    return;

  char id_n [16], id_p [16];
  strcat (strcpy (id_n, id), ".N");
  strcat (strcpy (id_p, id), ".P");

  int tx,ty,tw,th;
  ParseConfigBitmap (button->app, Skin->Prefix, "Window", id_n, tx, ty, tw, th);
  csPixmap *bmpn = new csSimplePixmap (ButtonTex, tx, ty, tw, th);
  ParseConfigBitmap (button->app, Skin->Prefix, "Window", id_p, tx, ty, tw, th);
  csPixmap *bmpp = new csSimplePixmap (ButtonTex, tx, ty, tw, th);
  button->SetBitmap (bmpn, bmpp);
}

void csDefaultWindowSkin::PlaceGadgets (csWindow &This)
{
  int bw, bh;
  This.GetBorderSize (bw, bh);
  int th = This.GetTitlebarHeight ();
  int d_xmin = bw, d_xmax = This.bound.Width () - bw;
  int d_ymin = bh, d_ymax = bh + th;
  int butsize = th;
  int ws = This.GetWindowStyle ();

  // Place system menu button
  csComponent *c = This.GetChild (CSWID_BUTSYSMENU);
  if (c)
  {
    c->SetRect (d_xmin, d_ymin, d_xmin + butsize, d_ymax);
    d_xmin += butsize;
  } /* endif */

  // Place close button
  c = This.GetChild (CSWID_BUTCLOSE);
  if (c)
  {
    c->SetRect (d_xmax - butsize, d_ymin, d_xmax, d_ymax);
    d_xmax -= butsize;
  } /* endif */

  // Place maximize button
  c = This.GetChild (CSWID_BUTMAXIMIZE);
  if (c)
  {
    c->SetRect (d_xmax - butsize, d_ymin, d_xmax, d_ymax);
    d_xmax -= butsize;
  } /* endif */

  // Place hide button
  c = This.GetChild (CSWID_BUTHIDE);
  if (c)
  {
    c->SetRect (d_xmax - butsize, d_ymin, d_xmax, d_ymax);
    d_xmax -= butsize;
  } /* endif */

  // Place title bar
  c = This.GetChild (CSWID_TITLEBAR);
  if (c) c->SetRect (d_xmin, d_ymin, d_xmax, d_ymax);

  // Now distribute the client rectangle
  d_xmin = bw;
  d_xmax = This.bound.Width () - bw;
  if (ws & (CSWS_TITLEBAR | CSWID_BUTCLOSE | CSWID_BUTMAXIMIZE | CSWID_BUTHIDE))
    d_ymin = d_ymax;
  else
    d_ymin = bh;
  d_ymax = This.bound.Height () - bh;

  // Place system menu (hidden by default)
  c = This.GetChild (CSWID_SYSMENU);
  if (c)
    c->SetRect (d_xmin, d_ymin, d_xmin + c->bound.Width (),
      d_ymin + c->bound.Height ());

  // Place the menu bar
  c = This.GetChild (CSWID_MENUBAR);
  if (c)
  {
    int y = d_ymin + This.GetMenuHeight ();
    c->SetRect (d_xmin, d_ymin, d_xmax, y);
    d_ymin = y;
  } /* endif */

  // Place the tool bar
  c = This.GetChild (CSWID_TOOLBAR);
  if (c)
  {
    if ((ws & CSWS_TBPOS_MASK) == CSWS_TBPOS_TOP
     || (ws & CSWS_TBPOS_MASK) == CSWS_TBPOS_BOTTOM)
      c->SetSize (d_xmax - d_xmin, 0);
    else
      c->SetSize (0, d_ymax - d_ymin);
    int ToolbarW, ToolbarH;
    c->SuggestSize (ToolbarW, ToolbarH);
    switch (ws & CSWS_TBPOS_MASK)
    {
      case CSWS_TBPOS_TOP:
      {
        int tmp = d_ymin + ToolbarH;
        c->SetRect (d_xmin, d_ymin, d_xmax, tmp);
        d_ymin = tmp;
        break;
      }
      case CSWS_TBPOS_BOTTOM:
      {
        int tmp = d_ymax - ToolbarH;
        c->SetRect (d_xmin, tmp, d_xmax, d_ymax);
        d_ymax = tmp;
        break;
      }
      case CSWS_TBPOS_LEFT:
      {
        int tmp = d_xmin + ToolbarW;
        c->SetRect (d_xmin, d_ymin, tmp, d_ymax);
        d_xmin = tmp;
        break;
      }
      case CSWS_TBPOS_RIGHT:
      {
        int tmp = d_xmax - ToolbarW;
        c->SetRect (tmp, d_ymin, d_xmax, d_ymax);
        d_xmax = tmp;
        break;
      }
    } /* endswitch */
  } /* endif */

  // Place the client window
  c = This.GetChild (CSWID_CLIENT);
  if (c)
  {
    if (ws & CSWS_CLIENTBORDER)
    {
      d_xmin++; d_xmax--;
      d_ymin++; d_ymax--;
    } /* endif */
    c->SetRect (d_xmin, d_ymin, d_xmax, d_ymax);
  } /* endif */
}

void csDefaultWindowSkin::Draw (csComponent &This)
{
#define This ((csWindow &)This)
  int bw = 0, bh = 0;
  int wbw, wbh;
  This.GetBorderSize (wbw, wbh);
  switch (This.GetFrameStyle ())
  {
    case cswfsNone:
      break;
    case cswfsThin:
      This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
        CSPAL_WINDOW_2DARK3D, CSPAL_WINDOW_2DARK3D);
      bw = bh = 1;
      break;
    case cswfs3D:
    {
      if ((wbw > 0) && (wbh > 0))
      {
        This.Rect3D (0, 0, This.bound.Width (), This.bound.Height (),
          CSPAL_WINDOW_2DARK3D, CSPAL_WINDOW_2LIGHT3D);
        bw++; bh++;
      } /* endif */
      if ((wbw > 1) && (wbh > 1))
      {
        This.Rect3D (1, 1, This.bound.Width () - 1, This.bound.Height () - 1,
          CSPAL_WINDOW_DARK3D, CSPAL_WINDOW_LIGHT3D);
        bw++; bh++;
      } /* endif */
      break;
    }
  } /* endswitch */

  if ((Back.GetType () == csbgNone)
   && (Back.GetColor () != CSPAL_WINDOW_BORDER))
  {
    float r, g, b;
    csGetRGB (This.GetColor (CSPAL_WINDOW_BORDER), This.app, r, g, b);
    csRGBcolor rgb (QInt (r * 255.9), QInt (g * 255.9), QInt (b * 255.9));
    Back.SetColor (CSPAL_WINDOW_BORDER);
    Back.SetColor (0, rgb);
    Back.SetColor (1, rgb);
    Back.SetColor (2, rgb);
    Back.SetColor (3, rgb);
    Back.SetType (csbgNone);
  }

  int x1 = This.bound.Width ()  - wbw, x2 = This.bound.Width ()  - bw;
  int y1 = This.bound.Height () - wbh, y2 = This.bound.Height () - bh;
  This.Box (bw, bh, wbw, y2, CSPAL_WINDOW_BORDER);
  This.Box (x1, bh, x2, y2, CSPAL_WINDOW_BORDER);
  This.Box (wbw, bh, x1, wbh, CSPAL_WINDOW_BORDER);
  This.Box (wbw, y1, x1, y2, CSPAL_WINDOW_BORDER);
  Back.Draw (This, wbw, wbh, This.bound.Width () - 2 * wbw,
    This.bound.Height () - 2 * wbh, 0, 0, This.GetAlpha ());

  if (This.GetWindowStyle () & CSWS_CLIENTBORDER)
  {
    csComponent *c = This.GetChild (CSWID_CLIENT);
    if (c)
      This.Rect3D (c->bound.xmin - 1, c->bound.ymin - 1, c->bound.xmax + 1,
        c->bound.ymax + 1, CSPAL_WINDOW_LIGHT3D, CSPAL_WINDOW_DARK3D);
  } /* endif */
#undef This
}

void csDefaultWindowSkin::SetState (csWindow &This, int Which, bool State)
{
  if (Which == CSS_MAXIMIZED)
  {
    csButton *bt = (csButton *)This.GetChild (CSWID_BUTMAXIMIZE);
    if (!bt) return;
    if (State)
      SetButtBitmap (bt, "Restore");
    else
      SetButtBitmap (bt, "Maximize");
  }
}

void csDefaultWindowSkin::SetBorderSize (csWindow &This)
{
  int bs = This.GetFrameStyle () == cswfsNone ? 0 :
           This.GetFrameStyle () == cswfsThin ? 3 : 4;
  This.SetBorderSize (bs, bs);
}
