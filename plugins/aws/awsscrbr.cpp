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
decVal(NULL), incVal(NULL), 
sink(NULL), dec_slot(NULL), inc_slot(NULL),
value(0), max(1), min(0), amntvis(0),
value_delta(0.1), value_page_delta(0.25)
{
}

awsScrollBar::~awsScrollBar()
{
  dec_slot->Disconnect(decVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("DecValue"));
  inc_slot->Disconnect(incVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("IncValue"));

  SCF_DEC_REF(incVal);
  SCF_DEC_REF(decVal);
  SCF_DEC_REF(sink);
  SCF_DEC_REF(inc_slot);
  SCF_DEC_REF(dec_slot);
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
  incVal = new awsCmdButton;
  decVal = new awsCmdButton;

  awsKeyFactory incinfo, decinfo;

  decinfo.Initialize(new scfString("decVal"), new scfString("Command Button"));
  incinfo.Initialize(new scfString("incVal"), new scfString("Command Button"));

  decinfo.AddIntKey(new scfString("Style"), awsCmdButton::fsToolbar);
  incinfo.AddIntKey(new scfString("Style"), awsCmdButton::fsToolbar);

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
    } break;
  } // end switch framestyle

  decVal->SetWindow(Window());
  incVal->SetWindow(Window());

  decVal->SetParent(this);
  incVal->SetParent(this);

  decVal->Setup(_wmgr, decinfo.GetThisNode());
  incVal->Setup(_wmgr, incinfo.GetThisNode());

  decVal->SetProperty("Image", decimg);
  incVal->SetProperty("Image", incimg);

  sink = new awsSink(this);

  sink->RegisterTrigger("DecValue", &DecClicked);
  sink->RegisterTrigger("IncValue", &IncClicked);

  dec_slot = new awsSlot();
  inc_slot = new awsSlot();

  dec_slot->Connect(decVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("DecValue"));
  inc_slot->Connect(incVal, awsCmdButton::signalClicked, sink, sink->GetTriggerID("IncValue"));

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
awsScrollBar::IncClicked(void *sk, iAwsSource *source)
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
awsScrollBar::DecClicked(void *sk, iAwsSource *source)
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
  
  frame3d.Draw(WindowManager(), Window(), f, aws3DFrame::fsRaised, tex, alpha_level);
}

bool 
awsScrollBar::OnMouseDown(int button , int x , int y)
{
  return false;
}

bool 
awsScrollBar::OnMouseUp(int button, int x, int y)
{
  return false;
}

bool
awsScrollBar::OnMouseMove(int button, int x, int y)
{
  return false;
}

bool
awsScrollBar::OnMouseClick(int ,int ,int )
{
  return false;
}

bool
awsScrollBar::OnMouseDoubleClick(int ,int ,int )
{
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


