#include "cssysdef.h"
#include "awsscrbr.h"
#include "aws3dfrm.h"
#include "awskcfct.h"
#include "awsslot.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

SCF_IMPLEMENT_IBASE(awsScrollBar)
SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const int awsScrollBar::signalChanged=0x1;

const int awsScrollBar::fsVertical =0x0;
const int awsScrollBar::fsHorizontal =0x1;

awsScrollBar::awsScrollBar():is_down(false), mouse_is_over(false),
was_down(false), tex(NULL),
frame_style(0), alpha_level(92),
decVal(NULL), incVal(NULL), knob(NULL), timer (NULL),
sink(NULL), dec_slot(NULL), inc_slot(NULL), knob_slot(NULL), tick_slot(NULL),
value(0), max(1), min(0), amntvis(0),
value_delta(0.1), value_page_delta(0.25)
{
  SetFlag(AWSF_CMP_ALWAYSERASE);
  captured = false;
}

awsScrollBar::~awsScrollBar()
{
  if (dec_slot)
    dec_slot->Disconnect(decVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("DecValue"));
  if (inc_slot)
    inc_slot->Disconnect(incVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("IncValue"));
  if (knob_slot)
    knob_slot->Disconnect(knob, awsCmdButton::signalClicked, sink, sink->GetTriggerID("KnobTick"));
  if (tick_slot)
    tick_slot->Disconnect(timer, awsTimer::signalTick, sink, sink->GetTriggerID("TickTock"));

  SCF_DEC_REF(incVal);
  SCF_DEC_REF(decVal);
  SCF_DEC_REF(knob);
  SCF_DEC_REF(sink);
  SCF_DEC_REF(inc_slot);
  SCF_DEC_REF(dec_slot);
  SCF_DEC_REF(knob_slot);
  SCF_DEC_REF(tick_slot);
  SCF_DEC_REF (timer);

  if (captured)
    WindowManager ()->ReleaseMouse ();
    
}

char *
awsScrollBar::Type()
{
  return "Scroll Bar";
}

bool
awsScrollBar::Setup(iAws *_wmgr, awsComponentNode *settings)
{
  if (!awsComponent::Setup(_wmgr, settings)) return false;

  iAwsPrefManager *pm=WindowManager()->GetPrefMgr();

  pm->LookupIntKey("OverlayTextureAlpha", alpha_level); // global get
  pm->GetInt(settings, "Style", frame_style);
  pm->GetInt(settings, "Alpha", alpha_level);          // local overrides, if present.

  tex=pm->GetTexture("Texture");

  // Setup embedded buttons
  incVal = new awsSliderButton;
  decVal = new awsSliderButton;
  knob   = new awsSliderButton;
  timer = new awsTimer (WindowManager ()->GetObjectRegistry (), this);

  awsKeyFactory incinfo, decinfo, knobinfo;

  decinfo.Initialize(new scfString("decVal"), new scfString("Slider Button"));
  incinfo.Initialize(new scfString("incVal"), new scfString("Slider Button"));
  knobinfo.Initialize(new scfString("knob"), new scfString("Slider Button"));

  decinfo.AddIntKey(new scfString("Style"), awsCmdButton::fsToolbar);
  incinfo.AddIntKey(new scfString("Style"), awsCmdButton::fsToolbar);
  knobinfo.AddIntKey(new scfString("Style"), awsCmdButton::fsToolbar);

  switch (frame_style)
  {
  case fsVertical:
    {

      incimg = pm->GetTexture("ScrollBarDn");
      decimg = pm->GetTexture("ScrollBarUp");

      // Abort if the images are not found
      if (!incimg || !decimg)
        return false;

      int img_w, img_h;

      incimg->GetOriginalDimensions(img_w, img_h);

      decinfo.AddRectKey(new scfString("Frame"),
                         csRect(0, 0,
                                Frame().Width(), img_h));

      incinfo.AddRectKey(new scfString("Frame"),
                         csRect(0, Frame().Height()-img_h,
                                Frame().Width(), Frame().Height()));

      knobinfo.AddRectKey(new scfString("Frame"),
                         csRect(0, img_h+1,
                                Frame().Width(), 2*img_h+1));
    } break;


  default:
    {

      incimg = pm->GetTexture("ScrollBarRt");
      decimg = pm->GetTexture("ScrollBarLt");

      // Abort if the images are not found
      if (!incimg || !decimg)
        return false;

      int img_w, img_h;

      incimg->GetOriginalDimensions(img_w, img_h);

      decinfo.AddRectKey(new scfString("Frame"),
                         csRect(Frame().xmin, Frame().ymin,
                                Frame().xmin+img_w+5, Frame().ymax));

      incinfo.AddRectKey(new scfString("Frame"),
                         csRect(Frame().xmax-img_w-5, Frame().ymin,
                                Frame().xmax, Frame().ymax));

      knobinfo.AddRectKey(new scfString("Frame"),
                         csRect(Frame().xmin+img_w+6, Frame().ymin,
                                Frame().xmin+2*img_w+6, Frame().ymax));
    } break;
  } // end switch framestyle
  //  DEBUG_BREAK;
  decVal->SetWindow(Window());
  incVal->SetWindow(Window());
  knob->SetWindow(Window());

  decVal->SetParent(this);
  incVal->SetParent(this);
  knob->SetParent(this);

  decVal->Setup(_wmgr, decinfo.GetThisNode());
  incVal->Setup(_wmgr, incinfo.GetThisNode());
  knob->Setup(_wmgr, knobinfo.GetThisNode());

  decVal->SetProperty("Image", decimg);
  incVal->SetProperty("Image", incimg);
  csTicks t=(csTicks)10;
  incVal->SetProperty("TicksPerSecond", (void*)&t);
  decVal->SetProperty("TicksPerSecond", (void*)&t);
  knob->SetProperty("TicksPerSecond", (void*)&t);

  sink = new awsSink(this);

  sink->RegisterTrigger("DecValue", &DecClicked);
  sink->RegisterTrigger("IncValue", &IncClicked);
  sink->RegisterTrigger("TickTock", &TickTock);
  sink->RegisterTrigger("KnobTick", &KnobTick);

  dec_slot = new awsSlot();
  inc_slot = new awsSlot();
  tick_slot = new awsSlot();
  knob_slot = new awsSlot();

  dec_slot->Connect(decVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("DecValue"));
  inc_slot->Connect(incVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("IncValue"));
  knob_slot->Connect(knob, awsCmdButton::signalClicked, sink, sink->GetTriggerID("KnobTick"));

  tick_slot->Connect(timer, awsTimer::signalTick, sink, sink->GetTriggerID("TickTock"));

  return true;
}

bool
awsScrollBar::GetProperty(char *name, void **parm)
{
  if (awsComponent::GetProperty(name, parm)) return true;

  if (strcmp("Value", name)==0)
  {
    *parm = (void *)&value;
    return true;
  }


  return false;
}

bool
awsScrollBar::SetProperty(char *name, void *parm)
{
  if (awsComponent::SetProperty(name, parm)) return true;

  if (strcmp("Change", name)==0)
  {
    value_delta = *(int *)parm;
    return true;
  }
  else if (strcmp("BigChange", name)==0)
  {
    value_page_delta = *(int *)parm;
    return true;
  }
  else if (strcmp("Min", name)==0)
  {
    min = *(int *)parm;

    // Fix value in case it's out of range
    value = ( value < min ? min :
               ( value > max ? max : value));

    Invalidate();
    return true;
  }
  else if (strcmp("Max", name)==0)
  {
    max = *(int *)parm;

    // Fix the page size
    if (amntvis>max)
      amntvis=max;

    // Fix value in case it's out of range
    value = ( value < min ? min :
               ( value > max ? max : value));

    Invalidate();
    return true;
  }
  else if (strcmp("PageSize", name)==0)
  {
    amntvis = *(int *)parm;

    // Fix the page size
    if (amntvis>max)
      amntvis=max;

    Invalidate();
    return true;
  }

  return false;
}

void
awsScrollBar::KnobTick(void *sk, iAwsSource *)
{
  // adjust position of knob and scrollbar value
  awsScrollBar *sb = (awsScrollBar *)sk;

  if (sb->frame_style==fsVertical)
  {
    int height=10;
    csRect f(sb->Frame());

    // Get the bar height
    f.ymin+=sb->decVal->Frame().Height()+1;
    f.ymax-=sb->incVal->Frame().Height()+1;

    // Get the knob height
    if (sb->amntvis==0)
      sb->WindowManager()->GetPrefMgr()->LookupIntKey("ScrollBarHeight", height);
    else
      height = (int)((sb->amntvis*f.Height())/sb->max);


    // Get the actual height that we can traverse with the knob
    int bh = f.Height()-height;

    sb->value = (sb->knob->last_y - sb->decVal->Frame().ymax) * sb->max / bh;
  }
  else if (sb->frame_style==fsHorizontal)
  {
    int width=10;
    csRect f(sb->Frame());

    f.xmin+=sb->decVal->Frame().Width()+1;
    f.xmax-=sb->incVal->Frame().Width()+1;

    if (sb->amntvis==0)
      sb->WindowManager()->GetPrefMgr()->LookupIntKey("ScrollBarWidth", width);
    else
      width=(int)((sb->amntvis*f.Width())/sb->max);

    // Get the actual height that we can traverse with the knob
    int bw = f.Width()-width;

    sb->value = (sb->knob->last_x - sb->decVal->Frame().xmax) * sb->max / bw;

  }
  else
    return;

  sb->value = (sb->value < sb->min ? sb->min :
                ( sb->value > sb->max ? sb->max : sb->value));

  sb->Broadcast(signalChanged);
  sb->Invalidate();
}

void
awsScrollBar::TickTock(void *sk, iAwsSource *)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  if (sb->frame_style==fsVertical)
  {
    if (sb->last_y < sb->knob->Frame ().ymin)
      sb->value-=sb->amntvis;
    else if (sb->last_y > sb->knob->Frame ().ymax)
      sb->value+=sb->amntvis;
    else
      return;
  }
  else
  {
    if (sb->last_x < sb->knob->Frame ().xmin)
      sb->value-=sb->amntvis;
    else if (sb->last_x > sb->knob->Frame ().xmax)
      sb->value+=sb->amntvis;
    else
      return;
  }

  sb->value = (sb->value < sb->min ? sb->min :
                ( sb->value > sb->max ? sb->max : sb->value));

  sb->Broadcast(signalChanged);
  sb->Invalidate();
}

void
awsScrollBar::IncClicked(void *sk, iAwsSource *)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  sb->value+=sb->value_delta;

  /// Check floor and ceiling
  sb->value = ( sb->value < sb->min ? sb->min :
                ( sb->value > sb->max ? sb->max : sb->value));

  sb->Broadcast(signalChanged);
  sb->Invalidate();
}

void
awsScrollBar::DecClicked(void *sk, iAwsSource *)
{
  awsScrollBar *sb = (awsScrollBar *)sk;

  sb->value-=sb->value_delta;

  /// Check floor and ceiling
  sb->value = ( sb->value < sb->min ? sb->min :
                ( sb->value > sb->max ? sb->max : sb->value));

  sb->Broadcast(signalChanged);
  sb->Invalidate();

}

void
awsScrollBar::OnDraw(csRect clip)
{
  aws3DFrame frame3d;
  int height=10, width=10;


  csRect f(Frame());

  if (frame_style==fsVertical)
  {
    // Get the bar height
    f.ymin+=decVal->Frame().Height()+1;
    f.ymax-=incVal->Frame().Height()+1;

    // Get the knob height
    if (amntvis==0)
      WindowManager()->GetPrefMgr()->LookupIntKey("ScrollBarHeight", height);
    else
      height = (int)((amntvis*f.Height())/max);


    // Get the actual height that we can traverse with the knob
    int bh = f.Height()-height;

    // Get the knob's position
    int ky = (int)((value*bh)/max);

    f.ymin+=ky;
    f.ymax=f.ymin+height;

    if (f.ymax>incVal->Frame().ymin-1)
      f.ymax=incVal->Frame().ymin-1;

  }
  else
  {
    f.xmin+=decVal->Frame().Width()+1;
    f.xmax-=incVal->Frame().Width()+1;

    if (amntvis==0)
      WindowManager()->GetPrefMgr()->LookupIntKey("ScrollBarWidth", width);
    else
      width=(int)((amntvis*f.Width())/max);

    // Get the actual height that we can traverse with the knob
    int bw = f.Width()-width;

    // Get the knob's position
    int kx = (int)((value*bw)/max);

    f.xmin+=kx;
    f.xmax=f.xmin+width;

    if (f.xmax>incVal->Frame().xmin-1)
      f.xmax=incVal->Frame().xmin-1;
  }

  knob->Frame () = f;

  //  frame3d.Draw(WindowManager(), Window(), f, aws3DFrame::fsRaised, tex, alpha_level);
}

bool
awsScrollBar::OnMouseDown(int btn,int x,int y)
{
  if (btn == 1 && !captured)
  {
    WindowManager ()->CaptureMouse (this);
    captured = true;
    timer->SetTimer (100);
    timer->Start ();
    last_x = x;
    last_y = y;
    return true;
  }
  return false;
}

bool
awsScrollBar::OnMouseUp(int btn,int ,int )
{
  if (captured && btn==1)
  {
    WindowManager ()->ReleaseMouse ();
    captured = false;
    timer->Stop ();
  }
  return true;
}

bool
awsScrollBar::OnMouseMove(int ,int x,int y)
{
  if (captured)
  {
    last_x = x;
    last_y = y;
    return true;
  }
  return false;
}

bool
awsScrollBar::OnMouseClick(int btn,int x,int y)
{
  return HandleClicking (btn,x,y);
}

bool
awsScrollBar::OnMouseDoubleClick(int btn,int x,int y)
{
  return HandleClicking (btn,x,y);
}

bool
awsScrollBar::HandleClicking (int btn,int x,int y)
{
  if (btn==1)
  {
    if (captured)
      WindowManager ()->ReleaseMouse ();
    if (frame_style==fsVertical)
    {
      if (y < knob->Frame().ymin && y > decVal->Frame ().ymax)
        value-=amntvis;
      else if (y > knob->Frame().ymax && y < incVal->Frame ().ymin)
        value+=amntvis;
    }
    else
    {
      if (x < knob->Frame().xmin && x > decVal->Frame().xmax)
        value-=amntvis;
      else if (x > knob->Frame().xmax  && x < incVal->Frame().xmin)
        value+=amntvis;
    }
    
    // Check floor and ceiling
    value = ( value < min ? min :
                  ( value > max ? max : value));

    Broadcast(signalChanged);
    Invalidate();
    return true;
  }
  return false;
}

bool
awsScrollBar::OnMouseExit()
{
  mouse_is_over=false;
  Invalidate();

  if (is_down)
    is_down=false;

  return true;
}

bool
awsScrollBar::OnMouseEnter()
{
  mouse_is_over=true;
  Invalidate();
  return true;
}

bool
awsScrollBar::OnKeypress(int ,int )
{
  return false;
}

bool
awsScrollBar::OnLostFocus()
{
  return false;
}

bool
awsScrollBar::OnGainFocus()
{
  return false;
}

void
awsScrollBar::OnAdded()
{
  AddChild(incVal);
  AddChild(decVal);
  AddChild(knob);
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsScrollBarFactory)
SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsScrollBarFactory::awsScrollBarFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  Register("Scroll Bar");
  RegisterConstant("sbfsVertical",  awsScrollBar::fsVertical);
  RegisterConstant("sbfsHorizontal", awsScrollBar::fsHorizontal);

  RegisterConstant("signalScrollBarChanged",  awsScrollBar::signalChanged);
}

awsScrollBarFactory::~awsScrollBarFactory()
{
  // empty
}

iAwsComponent *
awsScrollBarFactory::Create()
{
  return new awsScrollBar;
}



/************************************* Slider Button ****************/

SCF_IMPLEMENT_IBASE_EXT (awsSliderButton)
SCF_IMPLEMENT_IBASE_EXT_END

awsSliderButton::awsSliderButton() : timer(NULL), captured (false), nTicks ((csTicks)0), sink(NULL), tick_slot(NULL)
{
}

awsSliderButton::~awsSliderButton()
{
  if (tick_slot)
    tick_slot->Disconnect(timer, awsTimer::signalTick, sink, sink->GetTriggerID("TickTock"));

  if (captured)
    WindowManager ()->ReleaseMouse ();

  SCF_DEC_REF(tick_slot);
  SCF_DEC_REF (timer);

}


bool awsSliderButton::Setup(iAws *wmgr, awsComponentNode *settings)
{
  if (!awsCmdButton::Setup (wmgr, settings)) return false;
  
  timer = new awsTimer (WindowManager ()->GetObjectRegistry (), this);
  sink = new awsSink(this);

  sink->RegisterTrigger("TickTock", &TickTock);

  tick_slot = new awsSlot();

  tick_slot->Connect(timer, awsTimer::signalTick, sink, sink->GetTriggerID("TickTock"));
  return true;
}

bool awsSliderButton::GetProperty(char *name, void **parm)
{
  if (awsCmdButton::GetProperty(name, parm)) return true;

  if (strcmp("TicksPerSecond", name)==0)
  {
    *parm = (void *)&nTicks;
    return true;
  }
  return false;
}

bool awsSliderButton::SetProperty(char *name, void *parm)
{
  if (awsCmdButton::SetProperty(name, parm)) return true;
 
  if (strcmp("TicksPerSecond", name)==0)
  {
    csTicks n = *(csTicks *)parm;
    if (n<=0)
      nTicks = (csTicks)0;
    else
      nTicks = (csTicks)(1000 / n);
    timer->SetTimer (nTicks);

    return true;
  }
  return false;
}

char *awsSliderButton::Type()
{
  return "Slider Button";
}

void awsSliderButton::TickTock(void *sk, iAwsSource *)
{
  awsSliderButton *sb = (awsSliderButton *)sk;
  sb->Broadcast(signalClicked);
}

bool awsSliderButton::OnMouseDown(int btn, int x, int y)
{
  bool succ = awsCmdButton::OnMouseDown (btn, x, y);

  if (!is_switch && btn==1 && nTicks != 0 && !captured)
  {
    timer->Start ();
    WindowManager ()->CaptureMouse (this);
    last_x = x;
    last_y = y;
    captured = true;
  }
  return succ;
}

bool awsSliderButton::OnMouseUp(int btn, int x, int y)
{
  bool succ = awsCmdButton::OnMouseUp (btn, x, y);
  if (!is_switch && captured)
  {
    timer->Stop ();
    WindowManager ()->ReleaseMouse ();
    captured = false;
  }

  return succ;
}

bool awsSliderButton::OnMouseMove(int , int x, int y)
{
  if (captured)
    last_x = x, last_y=y;

  return false;
}

bool awsSliderButton::OnMouseClick(int ,int ,int )
{
  if (captured)
  {
    timer->Stop ();
    WindowManager ()->ReleaseMouse ();
    captured = false;
  }
  return false;
}

bool awsSliderButton::OnMouseDoubleClick(int ,int ,int )
{
  if (captured)
  {
    timer->Stop ();
    WindowManager ()->ReleaseMouse ();
    captured = false;
  }
  return false;
}


/************************************* Slider Button Factory ****************/

SCF_IMPLEMENT_IBASE_EXT(awsSliderButtonFactory)
SCF_IMPLEMENT_IBASE_EXT_END

awsSliderButtonFactory::awsSliderButtonFactory(iAws *wmgr):awsCmdButtonFactory(wmgr)
{
  Register("Slider Button");
}

awsSliderButtonFactory::~awsSliderButtonFactory()
{
 // empty
}

iAwsComponent *
awsSliderButtonFactory::Create()
{
 return new awsSliderButton;
}
