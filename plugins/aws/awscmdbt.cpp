#include "cssysdef.h"
#include "awscmdbt.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"

#include <stdio.h>

SCF_IMPLEMENT_IBASE(awsCmdButton)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const unsigned int awsCmdButton::fsNormal =0x0;
const unsigned int awsCmdButton::fsToolbar=0x1;
const unsigned int awsCmdButton::fsBitmap =0x2;

awsCmdButton::awsCmdButton():is_down(false), mouse_is_over(false), frame_style(0), alpha_level(0)
{
  tex[0]=tex[1]=tex[2]=NULL;
}

awsCmdButton::~awsCmdButton()
{
}

bool
awsCmdButton::Setup(iAws *_wmgr, awsComponentNode *settings)
{
 if (!awsComponent::Setup(_wmgr, settings)) return false;

 return true;
}

void 
awsCmdButton::OnDraw(csRect clip)
{

  iGraphics2D *g2d = WindowManager()->G2D();
  iGraphics3D *g3d = WindowManager()->G3D();

  int hi    = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT);
  int hi2   = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT2);
  int lo    = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW);
  int lo2   = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW2);
  int fill  = WindowManager()->GetPrefMgr()->GetColor(AC_FILL);
  int black = WindowManager()->GetPrefMgr()->GetColor(AC_BLACK);

  switch(frame_style)
  {
  case fsNormal:
  case fsToolbar:
    
    g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-1, Frame().ymin+1, hi);
    g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmin+1, Frame().ymax-1, hi);
    g2d->DrawLine(Frame().xmin+1, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, lo);
    g2d->DrawLine(Frame().xmax-1, Frame().ymin+1, Frame().xmax-1, Frame().ymax-1, lo);

    g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmax-2, Frame().ymin+2, hi2);
    g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmin+2, Frame().ymax-2, hi2);
    g2d->DrawLine(Frame().xmin+2, Frame().ymax-2, Frame().xmax-2, Frame().ymax-2, lo2);
    g2d->DrawLine(Frame().xmax-2, Frame().ymin+2, Frame().xmax-2, Frame().ymax-2, lo2);

    break;

  case fsBitmap:
    {
      int texindex;
      int w,h;

      if (is_down) texindex=2;
      else if (mouse_is_over) texindex=1;
      else texindex=0;

      tex[texindex]->GetOriginalDimensions(w, h);

      g3d->DrawPixmap(tex[texindex], 0,0, w, h, Frame().xmin, Frame().ymin, w, h, alpha_level);
    }
    break;
  }
 
}

bool 
awsCmdButton::OnMouseDown(int button, int x, int y)
{
  return false;
}
    
bool 
awsCmdButton::OnMouseUp(int button, int x, int y)
{
  return false;
}
    
bool
awsCmdButton::OnMouseMove(int button, int x, int y)
{
  return false;
}

bool
awsCmdButton::OnMouseClick(int button, int x, int y)
{
  return false;
}

bool
awsCmdButton::OnMouseDoubleClick(int button, int x, int y)
{
  return false;
}

bool 
awsCmdButton::OnMouseExit()
{
  mouse_is_over=false;
  return true;
}

bool
awsCmdButton::OnMouseEnter()
{
  mouse_is_over=true;
  return true;
}

bool
awsCmdButton::OnKeypress(int key, int modifiers)
{
  return false;
}
    
bool
awsCmdButton::OnLostFocus()
{
  return false;
}

bool 
awsCmdButton::OnGainFocus()
{
  return false;
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsCmdButtonFactory)
  SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsCmdButtonFactory::awsCmdButtonFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  Register("Command Button");
  RegisterConstant("bfsNormal",  0x0);
  RegisterConstant("bfsToolbar", 0x1);
  RegisterConstant("bfsBitmap",  0x2);
}

awsCmdButtonFactory::~awsCmdButtonFactory()
{
 // empty
}

awsCmdButton *
awsCmdButtonFactory::Create()
{
 return new awsCmdButton; 
}

