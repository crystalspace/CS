#include "cssysdef.h"
#include "awsbarct.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"
#include "aws3dfrm.h"

#include <stdio.h>

SCF_IMPLEMENT_IBASE(awsBarChart)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const int awsBarChart:: fsBump = 0x0;
const int awsBarChart:: fsSimple = 0x1;
const int awsBarChart:: fsRaised = 0x2;
const int awsBarChart:: fsSunken = 0x3;
const int awsBarChart:: fsFlat = 0x4;
const int awsBarChart:: fsNone = 0x5;

const int awsBarChart:: coRolling=0x1;
const int awsBarChart:: coRollLeft=0x0;
const int awsBarChart:: coRollRight=0x2;
const int awsBarChart:: coVertGridLines=0x4;
const int awsBarChart:: coHorzGridLines=0x8;
const int awsBarChart:: coVerticalChart=0x10;

const int awsBarChart:: signalClicked = 0x1;

awsBarChart::awsBarChart () :
  frame_style(0),
  inner_frame_style(fsNone),
  chart_options(0),
  alpha_level(96),
  bkg(NULL),
  caption(NULL),
  yText(NULL),
  xText(NULL),
  max_items(0)
{
  SCF_CONSTRUCT_IBASE (NULL);
}

awsBarChart::~awsBarChart ()
{
}

char *awsBarChart::Type ()
{
  return "Group Frame";
}

bool awsBarChart::Setup (iAws *_wmgr, awsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;

  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  pm->LookupIntKey ("OverlayTextureAlpha", alpha_level);
  pm->GetInt (settings, "Style", frame_style);
  pm->GetInt (settings, "InnerStyle", inner_frame_style);
  pm->GetInt (settings, "Options", chart_options);
  pm->GetInt (settings, "MaxItems", max_items);
  pm->GetString (settings, "Caption", caption);
  pm->GetString (settings, "XLegend", xText);
  pm->GetString (settings, "YLegend", yText);

  bkg = pm->GetTexture ("Texture");

  return true;
}

bool awsBarChart::GetProperty (char *name, void **parm)
{
  if (awsComponent::GetProperty (name, parm)) return true;

  if (strcmp ("Caption", name) == 0)
  {
    char *st = NULL;

    if (caption) st = caption->GetData ();

    iString *s = new scfString (st);
    *parm = (void *)s;
    return true;
  }

  return false;
}

bool awsBarChart::SetProperty (char *name, void *parm)
{
  if (awsComponent::SetProperty (name, parm)) return true;

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
      caption = NULL;
    }

    return true;
  }

  return false;
}

bool awsBarChart::Execute (char *action, iAwsParmList &parmlist)
{
  if (awsComponent::Execute (action, parmlist)) return true;

  if (strcmp(action, "AddItem")==0)
  {
    BarItem *i = new BarItem;
    BarItem *old=NULL;

    parmlist.GetFloat("value", &i->value);
    parmlist.GetString("label", &i->label);

    if (chart_options & coRolling)
    {
      if (chart_options & coRollRight)
      {
	if (items.Length() >= max_items)
	{
	  old=(BarItem *)items[items.Length()-1];
	  items.Delete(old);
	  delete old;
	}

	items.Push(i);
	  
      } // end if chart should roll right
      else
      {
	if (items.Length() >= max_items)
	{
	  old=(BarItem *)items.Pop();
	  delete old;
        }

        items.Insert(items.Length(), i);
  
      } // end else chart rolls left
    } // end if the chart rolls
    else
    {
      items.Push(i);      
    } // end else chart grows.

    Invalidate();
    
    return true;

   } // end if action is "AddItem"

  return false;
}

void awsBarChart::OnDraw (csRect clip)
{
  iGraphics2D *g2d = WindowManager ()->G2D ();

  csRect insets;
  csRect inner_frame(Frame());

  aws3DFrame frame3d;

  frame3d.Draw (
      WindowManager (),
      Window (),
      Frame (),
      frame_style,
      bkg,
      alpha_level);

  // Get the normal inset for this item.
  insets=getInsets();

  // Draw the caption, if there is one
  if (caption)
  {
    int tw, th, tx, ty;

    // Get the size of the text
    WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
        caption->GetData (),
        tw, 
        th);

    // Calculate the center
    tx = insets.xmin+5;  //(Frame().Width()>>1) -  (tw>>1);
    ty = insets.ymin+(th>>1);   //(Frame().Height()>>1) - (th>>1);

    insets.ymin+=th;

    // Draw the text
    g2d->Write (
        WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
        Frame ().xmin + tx,
        Frame ().ymin + ty,
        WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
        -1,
        caption->GetData ());
  }

  // Draw the x legend, if there is one
  if (xText)
  {
    int tw, th, tx, ty;

    // Get the size of the text
    WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
        xText->GetData (),
        tw, 
        th);

    // Calculate the center
    tx = (Frame().Width()>>1) -  (tw>>1);
    ty = Frame().Height() - (th>>1);

    insets.ymax+=th;

    // Draw the text
    g2d->Write (
        WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
        Frame ().xmin + tx,
        Frame ().ymin + ty,
        WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
        -1,
        xText->GetData ());
  }

  inner_frame.xmin+=insets.xmin+2;
  inner_frame.ymin+=insets.ymin+2;
  inner_frame.xmax-=insets.xmax+2;
  inner_frame.ymax-=insets.ymax+2;

  frame3d.Draw (
      WindowManager (),
      Window (),
      inner_frame,
      inner_frame_style,
      bkg,
      alpha_level);

  // Now draw chart!
  int bw = inner_frame.Width() / items.Length();
  int i;
  float max=0.0001;

  for(i=0; i<items.Length(); ++i)
  {
    BarItem *bi = (BarItem *)items[i];

    if (max < bi->value) max=bi->value;
  }

  for(i=0; i<items.Length(); ++i)
  {
    BarItem *bi = (BarItem *)items[i];

    int x  = inner_frame.xmin + insets.xmin + (i*bw) + 1;
    int sy = inner_frame.ymin + insets.ymin;
    int ey = inner_frame.ymax - insets.ymax;

    float vp = bi->value / max;

    if (vp<1.0)
     sy = sy + (int)((float)(ey-sy) * vp);

    g2d->DrawBox(x, sy, bw-1, ey-sy, WindowManager ()->GetPrefMgr ()->GetColor (AC_RED)); 
  }
}

bool awsBarChart::OnMouseDown (int, int, int)
{
  return false;
}

bool awsBarChart::OnMouseUp (int, int, int)
{
  return false;
}

bool awsBarChart::OnMouseMove (int, int, int)
{
  return false;
}

bool awsBarChart::OnMouseClick (int, int, int)
{
  return false;
}

bool awsBarChart::OnMouseDoubleClick (int, int, int)
{
  return false;
}

bool awsBarChart::OnMouseExit ()
{
  return false;
}

bool awsBarChart::OnMouseEnter ()
{
  return false;
}

bool awsBarChart::OnKeypress (int, int)
{
  return false;
}

bool awsBarChart::OnLostFocus ()
{
  return false;
}

bool awsBarChart::OnGainFocus ()
{
  return false;
}

csRect awsBarChart::getInsets()
{
  switch(frame_style)
  {
  case fsBump:
    return csRect(4,4,4,4);

  case fsFlat:
  case fsSimple:
    return csRect(1,1,1,1);

  case fsRaised:
  case fsSunken:
    return csRect(2,2,2,2);
  
  case fsNone:
  default:
    return csRect(0,0,0,0);
  }
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsBarChartFactory)
  SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsBarChartFactory::awsBarChartFactory (
  iAws *wmgr) :
    awsComponentFactory(wmgr)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Register ("Bar Chart");
  RegisterConstant ("bcsBump", awsBarChart::fsBump);
  RegisterConstant ("bcsSimple", awsBarChart::fsSimple);
  RegisterConstant ("bcsSunken", awsBarChart::fsSunken);
  RegisterConstant ("bcsRaised", awsBarChart::fsRaised);
  RegisterConstant ("bcsFlat", awsBarChart::fsFlat);
  RegisterConstant ("bcsNone", awsBarChart::fsNone);

  RegisterConstant ("signalBarChartClicked", awsBarChart::signalClicked);
}

awsBarChartFactory::~awsBarChartFactory ()
{
  // empty
}

iAwsComponent *awsBarChartFactory::Create ()
{
  return new awsBarChart;
}
