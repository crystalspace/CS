#include "cssysdef.h"
#include "awsimgvw.h"
#include "aws3dfrm.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

SCF_IMPLEMENT_IBASE(awsImageView)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const int awsImageView::signalClicked=0x1;
const int awsImageView::signalMouseDown=0x2;
const int awsImageView::signalMouseUp=0x3;
const int awsImageView::signalMouseMoved=0x4;

const int awsImageView::fsBump =0x0;
const int awsImageView::fsSimple =0x1;
const int awsImageView::fsRaised =0x2;
const int awsImageView::fsSunken =0x3;


awsImageView::awsImageView():is_down(false), mouse_is_over(false), 
                             was_down(false), img(NULL),
                             frame_style(0), alpha_level(92)
                             
{ 
}

awsImageView::~awsImageView()
{
}

char *
awsImageView::Type() 
{ return "Image View"; }

bool
awsImageView::Setup(iAws *_wmgr, awsComponentNode *settings)
{
if (!awsComponent::Setup(_wmgr, settings)) return false;

 iAwsPrefManager *pm=WindowManager()->GetPrefMgr();
 
 pm->LookupIntKey("OverlayTextureAlpha", alpha_level); // global get
 pm->GetInt(settings, "Style", frame_style);
 pm->GetInt(settings, "Alpha", alpha_level);          // local overrides, if present.
 img=pm->GetTexture("Texture");
 
 return true;
}

bool 
awsImageView::GetProperty(char *name, void **parm)
{
  if (awsComponent::GetProperty(name, parm)) return true;

  return false;
}

bool 
awsImageView::SetProperty(char *name, void *parm)
{
  if (awsComponent::SetProperty(name, parm)) return true;
  
  return false;
}

void 
awsImageView::OnDraw(csRect clip)
{
  aws3DFrame frame3d;

  frame3d.Draw(WindowManager(), Window(), Frame(), frame_style, img, alpha_level);
}

bool 
awsImageView::OnMouseDown(int , int , int )
{
  Broadcast(signalMouseDown);

  was_down=is_down;

  if (is_down==false)
    is_down=true;
  
  Invalidate();
  return true;
}
    
bool 
awsImageView::OnMouseUp(int ,int ,int )
{
  Broadcast(signalMouseUp);

  if (is_down)
  {
    Broadcast(signalClicked);
    is_down=false;
  }
  
  Invalidate();
  return true;
}
    
bool
awsImageView::OnMouseMove(int ,int ,int )
{
  Broadcast(signalMouseMoved);
  return false;
}

bool
awsImageView::OnMouseClick(int ,int ,int )
{
  return false;
}

bool
awsImageView::OnMouseDoubleClick(int ,int ,int )
{
  return false;
}

bool 
awsImageView::OnMouseExit()
{
  mouse_is_over=false;
  Invalidate();

  if (is_down)
    is_down=false;
  
  return true;
}

bool
awsImageView::OnMouseEnter()
{
  mouse_is_over=true;
  Invalidate();
  return true;
}

bool
awsImageView::OnKeypress(int ,int )
{
  return false;
}
    
bool
awsImageView::OnLostFocus()
{
  return false;
}

bool 
awsImageView::OnGainFocus()
{
  return false;
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsImageViewFactory)
  SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsImageViewFactory::awsImageViewFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  Register("Command Button");
  RegisterConstant("ivfsBump",  awsImageView::fsBump);
  RegisterConstant("ivfsSimple", awsImageView::fsSimple);
  RegisterConstant("ivfsRaised",  awsImageView::fsRaised);
  RegisterConstant("ivfsSunken",  awsImageView::fsSunken);

  RegisterConstant("signalImageViewClicked",  awsImageView::signalClicked);
  RegisterConstant("signalImageViewMouseUp",  awsImageView::signalMouseUp);
  RegisterConstant("signalImageViewMouseDown",  awsImageView::signalMouseDown);
  RegisterConstant("signalImageViewMouseMoved",  awsImageView::signalMouseMoved);
}

awsImageViewFactory::~awsImageViewFactory()
{
 // empty
}

iAwsComponent *
awsImageViewFactory::Create()
{
 return new awsImageView; 
}

