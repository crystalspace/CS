#include "cssysdef.h"
#include "awslabel.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/fontserv.h"
#include "csutil/scfstr.h"

#include <stdio.h>

SCF_IMPLEMENT_IBASE(awsLabel)
  SCF_IMPLEMENTS_INTERFACE(awsComponent)
SCF_IMPLEMENT_IBASE_END

const int awsLabel::signalClicked=0x1;

const int awsLabel::alignLeft=0x0;
const int awsLabel::alignRight=0x1;
const int awsLabel::alignCenter=0x2;

awsLabel::awsLabel():is_down(false), mouse_is_over(false), alignment(0),
                     caption(NULL)
{
  SetFlag(AWSF_CMP_ALWAYSERASE);
}

awsLabel::~awsLabel()
{ }

char *
awsLabel::Type()
{ return "Label"; }

bool
awsLabel::Setup(iAws *_wmgr, awsComponentNode *settings)
{
 if (!awsComponent::Setup(_wmgr, settings)) return false;

 iAwsPrefManager *pm=WindowManager()->GetPrefMgr();

 pm->GetString(settings, "Caption", caption);
 pm->GetInt(settings, "Align", alignment);

 return true;
}

bool
awsLabel::GetProperty(char *name, void **parm)
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
awsLabel::SetProperty(char *name, void *parm)
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
awsLabel::OnDraw(csRect clip)
{
  iGraphics2D *g2d = WindowManager()->G2D();

   // Draw the caption, if there is one
   if (caption)
    {
      int tw, th, tx, ty, mcc;

      mcc = WindowManager()->GetPrefMgr()->GetDefaultFont()->GetLength(caption->GetData(), Frame().Width());

      scfString tmp(caption->GetData());
      tmp.Truncate(mcc);

      // Get the size of the text
      WindowManager()->GetPrefMgr()->GetDefaultFont()->GetDimensions(tmp.GetData(), tw, th);

      // Calculate the center
      ty = (Frame().Height()>>1) - (th>>1);

      switch(alignment)
      {
      case alignRight:
        tx = Frame().Width()-tw;
        break;

      case alignCenter:
        tx = (Frame().Width()>>1) -  (tw>>1);
        break;

      default:
        tx = 0;
        break;
      }

      // Draw the text
      g2d->Write(WindowManager()->GetPrefMgr()->GetDefaultFont(),
                 Frame().xmin+tx+is_down,
                 Frame().ymin+ty+is_down,
                 WindowManager()->GetPrefMgr()->GetColor(AC_TEXTFORE),
                 -1,
                 tmp.GetData());

    }
}

bool
awsLabel::OnMouseDown(int ,int ,int )
{
  is_down=true;
  //Invalidate();
  return false;
}

bool
awsLabel::OnMouseUp(int ,int ,int )
{
  if (is_down)
    Broadcast(signalClicked);

  is_down=false;
  //Invalidate();
  return false;
}

bool
awsLabel::OnMouseMove(int ,int ,int )
{
  return false;
}

bool
awsLabel::OnMouseClick(int ,int ,int )
{
  return false;
}

bool
awsLabel::OnMouseDoubleClick(int ,int ,int )
{
  return false;
}

bool
awsLabel::OnMouseExit()
{
  mouse_is_over=false;
  //Invalidate();

  if (is_down)
    is_down=false;

  return true;
}

bool
awsLabel::OnMouseEnter()
{
  mouse_is_over=true;
  //Invalidate();
  return true;
}

bool
awsLabel::OnKeypress(int ,int )
{
  return false;
}

bool
awsLabel::OnLostFocus()
{
  return false;
}

bool
awsLabel::OnGainFocus()
{
  return false;
}

/************************************* Command Button Factory ****************/
SCF_IMPLEMENT_IBASE(awsLabelFactory)
  SCF_IMPLEMENTS_INTERFACE(iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

awsLabelFactory::awsLabelFactory(iAws *wmgr):awsComponentFactory(wmgr)
{
  SCF_CONSTRUCT_IBASE (NULL);
  Register("Label");
  RegisterConstant("signalLabelClicked",  awsLabel::signalClicked);

  RegisterConstant("lblAlignLeft",  awsLabel::alignLeft);
  RegisterConstant("lblAlignRight",  awsLabel::alignRight);
  RegisterConstant("lblAlignCenter",  awsLabel::alignCenter);
}

awsLabelFactory::~awsLabelFactory()
{
 // empty
}

iAwsComponent *
awsLabelFactory::Create()
{
 return new awsLabel;
}

