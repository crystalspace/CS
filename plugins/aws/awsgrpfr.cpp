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
#include "awsgrpfr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"
#include "aws3dfrm.h"

const int awsGroupFrame::signalClicked = 0x1;

awsGroupFrame::awsGroupFrame ()
  : caption (0)
{
}

awsGroupFrame::~awsGroupFrame ()
{
}

const char *awsGroupFrame::Type ()
{
  return "Group Frame";
}

bool awsGroupFrame::Setup (iAws *_wmgr, iAwsComponentNode *settings)
{
  if (!awsPanel::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->GetString (settings, "Caption", caption);

  return true;
}

bool awsGroupFrame::GetProperty (const char *name, intptr_t *parm)
{
  if (awsPanel::GetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    const char *st = 0;

    if (caption) st = caption->GetData ();

    iString *s = new scfString (st);
    *parm = (intptr_t)s;
    return true;
  }
  return false;
}

bool awsGroupFrame::SetProperty (const char *name, intptr_t parm)
{
  if (awsPanel::SetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    iString *s = (iString *) (parm);

    if (s && s->Length ())
    {
      if (caption) caption->DecRef ();
      caption = s;
      caption->IncRef ();
      Invalidate ();
    }
    else
    {
      if (caption) caption->DecRef ();
      caption = 0;
    }
    return true;
  }
  return false;
}

void awsGroupFrame::OnDraw (csRect clip)
{
  awsPanel::OnDraw (clip);

  iGraphics2D *g2d = WindowManager ()->G2D ();

  /// Draw the caption, if there is one.
  if (caption)
  {
    int tw, th, tx, ty;

    /// Get the size of the text.
    WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
      caption->GetData (),
      tw,
      th);

    /// Calculate the center.
    tx = 10;  // (Frame ().Width () >> 1) -  (tw >> 1);
    ty = 8;   // (Frame ().Height () >> 1) - (th >> 1);

    /// Draw the text.
    g2d->Write (
        WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
        Frame ().xmin + tx,
        Frame ().ymin + ty,
        WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
        -1,
        caption->GetData ());
  }
}

awsGroupFrameFactory::awsGroupFrameFactory (iAws *wmgr)
  : awsComponentFactory (wmgr)
{
  Register ("Group Frame");
  RegisterConstant ("signalGroupFrameClicked", awsGroupFrame::signalClicked);
}

awsGroupFrameFactory::~awsGroupFrameFactory ()
{
  // Empty.
}

iAwsComponent *awsGroupFrameFactory::Create ()
{
  return new awsGroupFrame;
}
