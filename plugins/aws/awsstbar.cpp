/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "awsstbar.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"
#include "csutil/snprintf.h"
#include "aws3dfrm.h"

#include <stdio.h>

//static iAwsSink *chart_sink = 0;

CS_IMPLEMENT_STATIC_VAR (GetChartSlot, awsSlot,())
static awsSlot *chart_slot = 0;


awsStatusBar::awsStatusBar () :
  frame_style(0),
  alpha_level(96),
  bkg(0),
  barimg(0),
  bar_color(0),
  status(0.0)
{
  chart_slot = GetChartSlot ();
}

awsStatusBar::~awsStatusBar ()
{
  
}

const char *awsStatusBar::Type ()
{
  return "Status Bar";
}

bool awsStatusBar::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;


  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  unsigned char r=0, g=0, b=0;
  int stat=0;

  iString *bartxt=0;

  pm->LookupIntKey ("OverlayTextureAlpha", alpha_level);
  pm->GetInt (settings, "Style", frame_style);
  pm->GetInt (settings, "BarTextureAlpha", bar_alpha_level);
  pm->GetInt (settings, "Status", stat);
  pm->GetString (settings, "BarTexture", bartxt);
  
  pm->LookupRGBKey("StatusBarColor", r, g, b);

  bar_color = pm->FindColor(r,g,b);

  bkg = pm->GetTexture ("Texture");

  if (bartxt)
    barimg = pm->GetTexture(bartxt->GetData(), bartxt->GetData());

  if (stat)
    status = (float)stat/100.0;

  return true;
}

bool awsStatusBar::GetProperty (const char *name, void **parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  if (strcmp ("Status", name) == 0)
  {
    *((float **)parm) = &status;

    return true;
  }

  return false;
}

bool awsStatusBar::SetProperty (const char *name, void *parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

  if (strcmp ("Status", name) == 0)
  {
    status = *(float *)parm;

    return true;
  }

  return false;
}

bool awsStatusBar::Execute (const char *action, iAwsParmList* parmlist)
{
  if (awsComponent::Execute (action, parmlist)) return true;
 
  return false;
}

void awsStatusBar::OnDraw (csRect /*clip*/)
{
  iGraphics2D *g2d = WindowManager ()->G2D ();

  csRect insets;
  csRect inner(Frame());

  aws3DFrame frame3d;

  frame3d.Setup(WindowManager(),bkg, alpha_level);
  frame3d.Draw (
      Frame (),
      frame_style,
      Window()->Frame());

  if (status==0.0) return;

  // Get the normal inset for this item.
  insets=getInsets();
  
  inner.xmin+=insets.xmin;
  inner.xmax-=insets.xmax;
  inner.ymin+=insets.ymin;
  inner.ymax-=insets.ymax;

  // Figure out how much we should draw
  int width = (int)(status * (float)inner.Width());

  if (barimg)
  {
    iGraphics3D *g3d = WindowManager ()->G3D ();

    int tw, th;
    barimg->GetOriginalDimensions(tw ,th);

    // Figure out how much we should draw
    int twidth = (int)(status * (float)tw);

    if (twidth<1) return;

    g3d->DrawPixmap(barimg, 
		    inner.xmin, inner.ymin, width, inner.Height(),
		    0,0,twidth,th, 0);
  }
  else 
    g2d->DrawBox(inner.xmin, inner.ymin, width, inner.Height(), bar_color);
  

}

csRect awsStatusBar::getInsets()
{
  switch(frame_style)
  {
  case fsBump:
    return csRect(4,4,4,4);

  case fsFlat:
  case fsSimple:
    return csRect(1,1,1,1);

  case fsRaised:
    return csRect(1,1,3,3);
  case fsSunken:
    return csRect(3,3,1,1);
  
  case fsNone:
  default:
    return csRect(0,0,0,0);
  }
}


/************************************* Command Button Factory ****************/
awsStatusBarFactory::awsStatusBarFactory (
  iAws *wmgr) :
    awsComponentFactory(wmgr)
{
  Register ("Status Bar");
  RegisterConstant ("sbBump", awsStatusBar::fsBump);
  RegisterConstant ("sbSimple", awsStatusBar::fsSimple);
  RegisterConstant ("sbSunken", awsStatusBar::fsSunken);
  RegisterConstant ("sbRaised", awsStatusBar::fsRaised);
  RegisterConstant ("sbFlat", awsStatusBar::fsFlat);
  RegisterConstant ("sbNone", awsStatusBar::fsNone);

  RegisterConstant ("signalStatusBarClicked", awsStatusBar::signalClicked);
}

awsStatusBarFactory::~awsStatusBarFactory ()
{
  // empty
}

iAwsComponent *awsStatusBarFactory::Create ()
{
  return new awsStatusBar;
}
