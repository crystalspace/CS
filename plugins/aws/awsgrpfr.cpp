#include "cssysdef.h"
#include "awsgrpfr.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"
#include "csutil/csevent.h"
#include "iutil/evdefs.h"

#include <stdio.h>

SCF_IMPLEMENT_IBASE(awsGroupFrame)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const int awsGroupFrame::fsBump =0x0;
const int awsGroupFrame::fsSimple =0x1;
const int awsGroupFrame::fsRaised =0x2;
const int awsGroupFrame::fsSunken =0x3;

const int awsGroupFrame::signalClicked=0x1;

awsGroupFrame::awsGroupFrame():frame_style(0), alpha_level(96), bkg(NULL), caption(NULL) 
{}

awsGroupFrame::~awsGroupFrame()
{}

char *
awsGroupFrame::Type() 
{ return "Group Frame"; }

bool
awsGroupFrame::Setup(iAws *_wmgr, awsComponentNode *settings)
{
 if (!awsComponent::Setup(_wmgr, settings)) return false;

 iAwsPrefManager *pm=WindowManager()->GetPrefMgr();
 
 pm->LookupIntKey("OverlayTextureAlpha", alpha_level);
 pm->GetInt(settings, "Style", frame_style);
 pm->GetString(settings, "Caption", caption);

 bkg=pm->GetTexture("Texture");
  
 return true;
}

bool 
awsGroupFrame::GetProperty(char *name, void **parm)
{
  if (awsComponent::GetProperty(name, parm)) return true;

  if (strcmp("Caption", name)==0)
  {
    char *st = NULL;

    if (caption) st=caption->GetData();

    iString *s = new scfString(st);
    *parm = (void *)s;
    return true;
  }

  return false;
}

bool 
awsGroupFrame::SetProperty(char *name, void *parm)
{
  if (awsComponent::SetProperty(name, parm)) return true;

  if (strcmp("Caption", name)==0)
  {
    iString *s = (iString *)(parm);
    
    if (s)
    {
      if (caption) caption->DecRef();
      caption=s;
      caption->IncRef();
      Invalidate();
    }
    
    return true;
  }
  
  return false;
}

void 
awsGroupFrame::OnDraw(csRect clip)
{

  iGraphics2D *g2d = WindowManager()->G2D();
  iGraphics3D *g3d = WindowManager()->G3D();

  int hi    = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT);
  int hi2   = WindowManager()->GetPrefMgr()->GetColor(AC_HIGHLIGHT2);
  int lo    = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW);
  int lo2   = WindowManager()->GetPrefMgr()->GetColor(AC_SHADOW2);
  int  fill = WindowManager()->GetPrefMgr()->GetColor(AC_FILL);
  int dfill = WindowManager()->GetPrefMgr()->GetColor(AC_DARKFILL);
  int black = WindowManager()->GetPrefMgr()->GetColor(AC_BLACK);
    
  switch(frame_style)
  {
  case fsBump:
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmax-1, Frame().ymin+0, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmin+0, Frame().ymax-1, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmax-1, Frame().ymin+0, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-0, Frame().xmax-0, Frame().ymax-0, black);
      g2d->DrawLine(Frame().xmax-0, Frame().ymin+1, Frame().xmax-0, Frame().ymax-0, black);

      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-2, Frame().ymin+1, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmin+1, Frame().ymax-2, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-2, Frame().xmax-2, Frame().ymax-2, lo2);
      g2d->DrawLine(Frame().xmax-2, Frame().ymin+1, Frame().xmax-2, Frame().ymax-2, lo2);

      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmax-3, Frame().ymin+2, lo2);
      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmin+2, Frame().ymax-3, lo2);
      g2d->DrawLine(Frame().xmin+2, Frame().ymax-3, Frame().xmax-3, Frame().ymax-3, hi2);
      g2d->DrawLine(Frame().xmax-3, Frame().ymin+2, Frame().xmax-3, Frame().ymax-3, hi2);

      g2d->DrawLine(Frame().xmin+3, Frame().ymin+3, Frame().xmax-4, Frame().ymin+3, black);
      g2d->DrawLine(Frame().xmin+3, Frame().ymin+3, Frame().xmin+3, Frame().ymax-4, black);
      break;

  case fsSunken:
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmax-1, Frame().ymin+0, lo2);
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmin+0, Frame().ymax-1, lo2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-0, Frame().ymin+1, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmin+1, Frame().ymax-0, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-0, Frame().xmax-0, Frame().ymax-0, hi);
      g2d->DrawLine(Frame().xmax-0, Frame().ymin+1, Frame().xmax-0, Frame().ymax-0, hi);

      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmax-1, Frame().ymin+2, black);
      g2d->DrawLine(Frame().xmin+2, Frame().ymin+2, Frame().xmin+2, Frame().ymax-1, black);
      g2d->DrawLine(Frame().xmin+2, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, hi2);
      g2d->DrawLine(Frame().xmax-1, Frame().ymin+2, Frame().xmax-1, Frame().ymax-1, hi2);

      g2d->DrawBox(Frame().xmin+3, Frame().ymin+3, Frame().Width()-3, Frame().Height()-3, dfill);

      if (bkg)
       g3d->DrawPixmap(bkg, Frame().xmin, Frame().ymin, Frame().Width()+1, Frame().Height()+1, 0, 0, Frame().Width()+1, Frame().Height()+1, alpha_level);

      break;
      
  case fsRaised:
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmax-1, Frame().ymin+0, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymin+0, Frame().xmin+0, Frame().ymax-1, hi);
      g2d->DrawLine(Frame().xmin+0, Frame().ymax-1, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmax-1, Frame().ymin+0, Frame().xmax-1, Frame().ymax-1, lo);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-0, Frame().xmax-0, Frame().ymax-0, black);
      g2d->DrawLine(Frame().xmax-0, Frame().ymin+1, Frame().xmax-0, Frame().ymax-0, black);

      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmax-2, Frame().ymin+1, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymin+1, Frame().xmin+1, Frame().ymax-2, hi2);
      g2d->DrawLine(Frame().xmin+1, Frame().ymax-2, Frame().xmax-2, Frame().ymax-2, lo2);
      g2d->DrawLine(Frame().xmax-2, Frame().ymin+1, Frame().xmax-2, Frame().ymax-2, lo2);

      g2d->DrawBox(Frame().xmin+2, Frame().ymin+2, Frame().Width()-3, Frame().Height()-3, fill);

      if (bkg)
       g3d->DrawPixmap(bkg, Frame().xmin, Frame().ymin, Frame().Width()+1, Frame().Height()+1, 0, 0, Frame().Width()+1, Frame().Height()+1, alpha_level);

      break;

    case fsSimple:
      g2d->DrawBox(Frame().xmin, Frame().ymin, Frame().Width(), Frame().Height(), black);
      break;
  }


  // Draw the caption, if there is one 
  if (caption)
  {     
    int tw, th, tx, ty;
    
    // Get the size of the text
    WindowManager()->GetPrefMgr()->GetDefaultFont()->GetDimensions(caption->GetData(), tw, th);

    // Calculate the center
    tx = 10; //(Frame().Width()>>1) -  (tw>>1);
    ty = 8;   //(Frame().Height()>>1) - (th>>1);

    // Draw the text
    g2d->Write(WindowManager()->GetPrefMgr()->GetDefaultFont(),
               Frame().xmin+tx,
               Frame().ymin+ty,
               WindowManager()->GetPrefMgr()->GetColor(AC_TEXTFORE),
               -1,
               caption->GetData());
    
  }
}

bool 
awsGroupFrame::OnMouseDown(int button, int x, int y)
{
  return false;
}
    
bool 
awsGroupFrame::OnMouseUp(int button, int x, int y)
{  
  return false;
}
    
bool
awsGroupFrame::OnMouseMove(int button, int x, int y)
{
  return false;
}

bool
awsGroupFrame::OnMouseClick(int button, int x, int y)
{
  return false;
}

bool
awsGroupFrame::OnMouseDoubleClick(int button, int x, int y)
{
  return false;
}

bool 
awsGroupFrame::OnMouseExit()
{
  return false;
}

bool
awsGroupFrame::OnMouseEnter()
{
  return false;
}

bool
awsGroupFrame::OnKeypress(int key, int modifiers)
{
  return false;
}
    
bool
awsGroupFrame::OnLostFocus()
{
  return false;
}

bool 
awsGroupFrame::OnGainFocus()
{
  return false;
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsGroupFrameFactory)
  SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsGroupFrameFactory::awsGroupFrameFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  Register("Group Frame");
  RegisterConstant("gfsBump",  awsGroupFrame::fsBump);
  RegisterConstant("gfsSimple", awsGroupFrame::fsSimple);
  RegisterConstant("gfsSunken",  awsGroupFrame::fsSunken);
  RegisterConstant("gfsRaised", awsGroupFrame::fsRaised);

  RegisterConstant("signalGroupFrameClicked",  awsGroupFrame::signalClicked);
}

awsGroupFrameFactory::~awsGroupFrameFactory()
{
 // empty
}

iAwsComponent *
awsGroupFrameFactory::Create()
{
 return new awsGroupFrame; 
}

