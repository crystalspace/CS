/*
    Crystal Space Windowing System: Windowing System Component
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

#include <stddef.h>
#include <ctype.h>

#define SYSDEF_CASE
#include "cssysdef.h"
#include "csengine/cspixmap.h"
#include "cssys/csinput.h"
#include "cssys/csendian.h"
#include "qint.h"
#include "igraph2d.h"
#include "csws/cscomp.h"
#include "csws/csapp.h"
#include "csws/csmouse.h"
#include "csws/cswsutil.h"
#include "csws/cstheme.h"
#include "csws/csttlbar.h"
#include "csutil/hashmap.h"
#include "csutil/csmd5.h"

#define TITLEBAR_TEXTURE_NAME	"csws::TitlebarButtons"

//--//--//--//--//--//--//--//--//--//--//--//--/ The csTheme class --//--//

csTheme::csTheme(csApp * napp)
{
  g_app = napp;
  themeComponents = new csHashMap();

  AddThemeComponent(new csThemeComponent(this));
  AddThemeComponent(new csThemeWindow(this));
  AddThemeComponent(new csThemeButton(this));
};

csHashKey csTheme::HashName(char * name)
{
  csMD5::Digest digest = csMD5::Encode(name);

  unsigned int a, b, c, d, e;

  // Convert the hash into four unsigned integers
  a = convert_endian(*(REINTERPRET_CAST(int*)(digest.data +  0)));
  b = convert_endian(*(REINTERPRET_CAST(int*)(digest.data +  4)));
  c = convert_endian(*(REINTERPRET_CAST(int*)(digest.data +  8)));
  d = convert_endian(*(REINTERPRET_CAST(int*)(digest.data + 12)));

  // Return a "mesh" of the 4 values
  e = (a * b * c * d) & 0x7fffffff;
  return e;
}

void csTheme::AddThemeComponent(csThemeComponent * comp)
{
  themeComponents->Put(HashName(comp->GetName()),comp);
}

csThemeComponent * csTheme::GetThemeComponent(char * name)
{
  return (csThemeComponent *) themeComponents->Get(HashName(name));
}

void csTheme::DrawBorder(csComponent &comp,int FrameStyle,int &bw, int &bh, int li, int di)
{
  GetBorderSize(comp,FrameStyle,bw,bh);
  switch (FrameStyle)
  {
    case csthfsOblique:
      if (comp.bound.Height () >= 6)
      {
        int aw = comp.bound.Height () / 3;
        comp.ObliqueRect3D (0, 0, comp.bound.Width (), comp.bound.Height (), aw,li,di);
        comp.ObliqueRect3D (1, 1, comp.bound.Width () - 1, comp.bound.Height () - 1, aw - 1, di, li);
        comp.ObliqueRect3D (2, 2, comp.bound.Width () - 2, comp.bound.Height () - 2, aw - 2, di, li);
        break;
      } // otherwise fallback to a smaller frame.
    case csthfsThickRect:
      if (comp.bound.Height () >= 6)
      {
        comp.Rect3D (0, 0, comp.bound.Width (), comp.bound.Height (),li,di);
        comp.Rect3D (1, 1, comp.bound.Width () - 1, comp.bound.Height () - 1, di, li);
        comp.Rect3D (2, 2, comp.bound.Width () - 2, comp.bound.Height () - 2, di, li);
        break;
      } // otherwise fallback to a smaller frame
    case csthfsThinRect:
      if (comp.bound.Height () >= 4)
      {
        comp.Rect3D (0, 0, comp.bound.Width (), comp.bound.Height (),li,di);
        comp.Rect3D (1, 1, comp.bound.Width () - 1, comp.bound.Height () - 1, di, li);
        break;
      } // otherwise fallback to a smaller frame
    case csthfsThin:
      if (comp.bound.Height () >= 2)
      {
        comp.Rect3D (0, 0, comp.bound.Width (), comp.bound.Height (),di,di);
        break;
      } // otherwise fallback to no frame.
    case csthfsNone:
      break;
  } /* endswitch */
}

void csTheme::BroadcastThemeChange(csThemeComponent *tcomp)
{
  csEvent * bmsg = NULL;
  if (tcomp == NULL)
    bmsg = new csEvent(0,csevBroadcast,cscmdThemeChange,this);
  else
    bmsg = new csEvent(0,csevBroadcast,cscmdThemeComponentChange,tcomp);
  g_app->PutEvent(bmsg);
}

csComponent * csThemeWindow::GetCloseButton(csComponent * parent)
{
  int tx,ty,tw,th;

  if (bmpClosen == NULL)
  {
    FindCFGBitmap (theme->GetApp()->System, *(theme->GetApp()->titlebardefs), "CLOSEN", &tx, &ty, &tw, &th);
    bmpClosen = new csPixmap (theme->GetApp()->GetTexture (
      TITLEBAR_TEXTURE_NAME), tx, ty, tw, th);
  }
  if (bmpClosep == NULL)
  {
    FindCFGBitmap (theme->GetApp()->System, *(theme->GetApp()->titlebardefs), "CLOSEP", &tx, &ty, &tw, &th);
    bmpClosep = new csPixmap (theme->GetApp()->GetTexture (
      TITLEBAR_TEXTURE_NAME), tx, ty, tw, th);
  }
  csButton *bt = new csButton (parent, cscmdClose, 0, csbfsNone);
  bt->SetBitmap(new csPixmap(*bmpClosen),new csPixmap(*bmpClosep));
  return bt;
}

csComponent * csThemeWindow::GetHideButton(csComponent * parent)
{
  int tx,ty,tw,th;

  if (bmpHiden == NULL)
  {
    FindCFGBitmap (theme->GetApp()->System, *(theme->GetApp()->titlebardefs), "HIDEN", &tx, &ty, &tw, &th);
    bmpHiden = new csPixmap (theme->GetApp()->GetTexture (
      TITLEBAR_TEXTURE_NAME), tx, ty, tw, th);
  }
  if (bmpHidep == NULL)
  {
    FindCFGBitmap (theme->GetApp()->System, *(theme->GetApp()->titlebardefs), "HIDEP", &tx, &ty, &tw, &th);
    bmpHidep = new csPixmap (theme->GetApp()->GetTexture (
      TITLEBAR_TEXTURE_NAME), tx, ty, tw, th);
  }

  csButton *bt = new csButton (parent, cscmdHide, 0, csbfsNone);
  bt->SetBitmap(new csPixmap(*bmpHiden),new csPixmap(*bmpHidep));
  return bt;
}

csComponent * csThemeWindow::GetMaximizeButton(csComponent * parent)
{
  int tx,ty,tw,th;

  if (bmpMaximizen == NULL)
  {
    FindCFGBitmap (theme->GetApp()->System, *(theme->GetApp()->titlebardefs), "MAXN", &tx, &ty, &tw, &th);
    bmpMaximizen = new csPixmap (theme->GetApp()->GetTexture (
      TITLEBAR_TEXTURE_NAME), tx, ty, tw, th);
  }
  if (bmpMaximizep == NULL)
  {
    FindCFGBitmap (theme->GetApp()->System, *(theme->GetApp()->titlebardefs), "MAXP", &tx, &ty, &tw, &th);
    bmpMaximizep = new csPixmap (theme->GetApp()->GetTexture (
      TITLEBAR_TEXTURE_NAME), tx, ty, tw, th);
  }

  csButton *bt = new csButton (parent, cscmdMaximize, 0, csbfsNone);
  bt->SetBitmap(new csPixmap(*bmpMaximizen),new csPixmap(*bmpMaximizep));
  return bt;
}

csComponent * csThemeWindow::GetTitleBar(csComponent *parent, char *iTitle)
{
  csComponent *tb = new csTitleBar (parent, iTitle);
  return tb;
}

int csThemeWindow::GetMenuHeight()
{
  return 16;
}

int csThemeWindow::GetTitleBarHeight()
{
  return 16;
}

void csTheme::GetBorderSize(csComponent &comp,int FrameStyle,int &bw, int &bh)
{
  switch (FrameStyle)
  {
    case csthfsOblique:
      if (comp.bound.Height () >= 6)
      {
        bh=bw=3;
        break;
      } // otherwise fallback to a smaller frame.
    case csthfsThickRect:
      if (comp.bound.Height () >= 6)
      {
        bh=bw=3;
        break;
      } // otherwise fallback to a smaller frame
    case csthfsThinRect:
      if (comp.bound.Height () >= 4)
      {
        bh=bw=2;
        break;
      } // otherwise fallback to a smaller frame
    case csthfsThin:
      if (comp.bound.Height () >= 2)
      {
        bw = bh = 1;
        break;
      } // otherwise fallback to no frame.
    case csthfsNone:
      bw = bh = 0;
      break;
  } /* endswitch */
}


csThemeComponent::csThemeComponent(csTheme * ntheme)
{
  theme = ntheme;
  name = "csComponent";
}

void csThemeComponent::BroadcastThemeChange()
{
  theme->BroadcastThemeChange(this);
}


csThemeWindow::csThemeWindow(csTheme * ntheme) : csThemeComponent(ntheme)
{
  name = "csWindow";
  BorderLightColor=CSPAL_WINDOW_2LIGHT3D;
  BorderDarkColor=CSPAL_WINDOW_2DARK3D;
  BackgroundColor=CSPAL_WINDOW_BORDER;
  BorderTexture=NULL;
  bmpClosep = NULL;
  bmpClosen = NULL;
  bmpHidep = NULL;
  bmpHiden = NULL;
  bmpMaximizep = NULL;
  bmpMaximizen = NULL;
}

csThemeButton::csThemeButton(csTheme * ntheme) : csThemeComponent(ntheme)
{
  name = "csButton";
  BorderLightColor=CSPAL_BUTTON_LIGHT3D;
  BorderDarkColor=CSPAL_BUTTON_DARK3D;
  BackgroundColor=CSPAL_BUTTON_BACKGROUND;
  BorderTexture=NULL;
}
