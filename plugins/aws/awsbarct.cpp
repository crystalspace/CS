#include "cssysdef.h"
#include "awsbarct.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"
#include "csutil/snprintf.h"
#include "aws3dfrm.h"

#include <stdio.h>

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
const int awsBarChart:: signalTimer = 0x2;


static iAwsSink *chart_sink = 0;

CS_IMPLEMENT_STATIC_VAR (GetChartSlot, awsSlot,())

static awsSlot *chart_slot = NULL;

static void DriveTimer (void *parm, iAwsSource *source)
{
  iAwsComponent *comp = source->GetComponent ();

  comp->Broadcast(awsBarChart::signalTimer);
}

awsBarChart::awsBarChart () :
  frame_style(0),
  inner_frame_style(fsNone),
  chart_options(0),
  alpha_level(96),
  bkg(NULL),
  caption(NULL),
  yText(NULL),
  xText(NULL),
  items(NULL),
  count_items(0),
  items_buffer_size(0),
  max_items(0),
  bar_color(0)
{
  chart_slot = GetChartSlot ();
}

awsBarChart::~awsBarChart ()
{
  if (update_timer != 0)
  {
    chart_slot->Disconnect (
        update_timer,
        awsTimer::signalTick,
        chart_sink,
        chart_sink->GetTriggerID ("Tick"));
    delete update_timer;
  }
}

char *awsBarChart::Type ()
{
  return "Bar Chart";
}

bool awsBarChart::Setup (iAws *_wmgr, awsComponentNode *settings)
{
  if (!awsComponent::Setup (_wmgr, settings)) return false;


  iAwsPrefManager *pm = WindowManager ()->GetPrefMgr ();

  unsigned char r=0, g=0, b=0;
  int timer_interval=1000;

  pm->LookupIntKey ("OverlayTextureAlpha", alpha_level);
  pm->GetInt (settings, "Style", frame_style);
  pm->GetInt (settings, "InnerStyle", inner_frame_style);
  pm->GetInt (settings, "Options", chart_options);
  pm->GetInt (settings, "MaxItems", max_items);
  pm->GetInt (settings, "UpdateInterval", timer_interval);
  pm->GetString (settings, "Caption", caption);
  pm->GetString (settings, "XLegend", xText);
  pm->GetString (settings, "YLegend", yText);

  pm->LookupRGBKey("ChartBarColor", r, g, b);

  bar_color = pm->FindColor(r,g,b);

  bkg = pm->GetTexture ("Texture");

  if (chart_options & coRolling)
  {
    // Setup blink event handling
    if (chart_sink == 0)
    {
      chart_sink = WindowManager ()->GetSinkMgr ()->CreateSink (NULL);
      chart_sink->RegisterTrigger ("Tick", &DriveTimer);
    }

    update_timer = new awsTimer (WindowManager ()->GetObjectRegistry (), this);
    update_timer->SetTimer (timer_interval);
    update_timer->Start ();

    chart_slot->Connect (
      update_timer,
      awsTimer::signalTick,
      chart_sink,
      chart_sink->GetTriggerID ("Tick"));
  }

  if (max_items)
  {
    items = new BarItem[max_items+1];
    items_buffer_size=max_items+1;
  }

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
    BarItem i;
    
    parmlist.GetFloat("value", &i.value);
    parmlist.GetString("label", &i.label);

    if (chart_options & coRolling)
    {
      if (chart_options & coRollRight)
      {
	if (count_items >= max_items)
	  Pop(false);
	
	Push(i, false);
	  
      } // end if chart should roll right
      else
      {
	if (count_items >= max_items)
	  Pop();
       
	Push(i);
         
      } // end else chart rolls left
    } // end if the chart rolls
    else
    {
     Push(i);      
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
 
  if (count_items<1) return;

  // Now draw chart!
  int tw=0, th=0;
  int i, maxtxtlen=0;
  float max=0.0001;
  char buf[32];

  for(i=0; i<count_items; ++i)
  {
    BarItem *bi = &items[i];

    if (max < bi->value) 
    {
      max=bi->value;
      cs_snprintf(buf, 32, "%0.2f", max);
    }
  }

  WindowManager ()->GetPrefMgr ()->GetDefaultFont ()->GetDimensions (
        buf,
        tw, 
        th);

   
  if (!(chart_options & coVerticalChart))
  {    
    inner_frame.xmin += tw+4;

    int x = inner_frame.xmin + insets.xmin+1;
    int sy = inner_frame.ymin + insets.ymin+1;
    int ey = inner_frame.ymax - insets.ymax+1;
    int dh = (th + (th>>1)) + 2;

    float dv = (dh * max) / inner_frame.Height();
    float cv = max;
    
    g2d->DrawLine(x, sy,  x, ey, 0);
    
    for(i=sy; i<ey; i+=dh, cv-=dv)
    {
      cs_snprintf(buf, 32, "%0.2f", cv);
      g2d->DrawLine(x-3, i,  x+1, i, 0);
      g2d->Write(WindowManager ()->GetPrefMgr ()->GetDefaultFont (),
        x - tw - 5,
        i,
        WindowManager ()->GetPrefMgr ()->GetColor (AC_TEXTFORE),
        -1,
        buf);
      
    }

    inner_frame.xmin+=2;
  }

   frame3d.Draw (
      WindowManager (),
      Window (),
      inner_frame,
      inner_frame_style,
      bkg,
      alpha_level);

  // Setup some variables
  int bw = inner_frame.Width() /  (max_items==0 ? count_items : max_items);
  int bh = inner_frame.Height() / (max_items==0 ? count_items : max_items);

  if (bw<1) bw=1;
  if (bh<1) bh=1;

  for(i=count_items-1; i>=0; --i)
  {
    BarItem *bi = &items[i];

    if (chart_options & coVerticalChart)
    {
      int y  = inner_frame.ymin + insets.ymin + (i*bh) + 1;
      int sx = inner_frame.xmin + insets.xmin;
      int ex = inner_frame.xmax - insets.xmax;

      float vp = bi->value / max;

      if (vp<1.0)
	ex = ex - (int)((float)(ex-sx) * vp);

      g2d->DrawBox(sx, y, ex-sx, bh-1, bar_color); 
    }
    else
    {
      int x  = inner_frame.xmin + insets.xmin + (i*bw) + 1;
      int sy = inner_frame.ymin + insets.ymin;
      int ey = inner_frame.ymax - insets.ymax;

      float vp = bi->value / max;

      if (vp<1.0)
	sy = sy + (int)((float)(ey-sy) * (1.0-vp));

      g2d->DrawBox(x, sy, bw-1, ey-sy, bar_color); 
    }
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


void 
awsBarChart::Push(BarItem &i, bool normal)
{
  if (items_buffer_size <= count_items+1)
  {
    BarItem *tmp = new BarItem[items_buffer_size+16];
    if (items)
    {
      if (!normal)
      {
	// Leave zeroth hole open for new insert to front
	memcpy(tmp+1, items, items_buffer_size*sizeof(BarItem));
	tmp[0]=i;	
      }
      else
      {
	// Insert new item on back
	memcpy(tmp, items, items_buffer_size*sizeof(BarItem));
	tmp[count_items]=i;
      }

      delete items;
      items=tmp;
      items_buffer_size+=16;
      count_items++;
    } // end if items
    else
    {
      items=tmp;
      items[0]=i;
    }
  } // end if not enough space
  else
  {
    if (!normal)
      {
	// Leave zeroth hole open for new insert to front
	memmove(items+1, items, count_items*sizeof(BarItem));
	items[0]=i;	
	count_items++;
      }
      else
      {
	// Insert new item on back
	items[count_items++]=i;
      }
  } // end else enough space
}

void 
awsBarChart::Pop(bool normal)
{
  if (!normal)
    --count_items;
  else
  {
    // Copy all to the bottom.  count_items decremented as a side-effect.
    memmove(items, items+1, (--count_items)*sizeof(BarItem));    
  }
}


/************************************* Command Button Factory ****************/
awsBarChartFactory::awsBarChartFactory (
  iAws *wmgr) :
    awsComponentFactory(wmgr)
{
  Register ("Bar Chart");
  RegisterConstant ("bcsBump", awsBarChart::fsBump);
  RegisterConstant ("bcsSimple", awsBarChart::fsSimple);
  RegisterConstant ("bcsSunken", awsBarChart::fsSunken);
  RegisterConstant ("bcsRaised", awsBarChart::fsRaised);
  RegisterConstant ("bcsFlat", awsBarChart::fsFlat);
  RegisterConstant ("bcsNone", awsBarChart::fsNone);

  RegisterConstant ("bcoRolling", awsBarChart::coRolling);
  RegisterConstant ("bcoRollLeft", awsBarChart::coRollLeft);
  RegisterConstant ("bcoRollRight", awsBarChart::coRollRight);
  RegisterConstant ("bcoVertGridLines", awsBarChart::coVertGridLines);
  RegisterConstant ("bcoHorzGridLines", awsBarChart::coHorzGridLines);
  RegisterConstant ("bcoVerticalChart", awsBarChart::coVerticalChart);

  RegisterConstant ("signalBarChartClicked", awsBarChart::signalClicked);
  RegisterConstant ("signalBarChartTimer", awsBarChart::signalTimer);
}

awsBarChartFactory::~awsBarChartFactory ()
{
  // empty
}

iAwsComponent *awsBarChartFactory::Create ()
{
  return new awsBarChart;
}
