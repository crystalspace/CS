#include "cssysdef.h"
#include "awsstdsk.h"
#include "awscomp.h"
#include "iaws/awsdefs.h"

#include <stdio.h>

void awsStandardSink::Hide (void *sink, iAwsSource *source)
{
  (void)sink;

  iAwsComponent *c = source->GetComponent ();

  c->Hide ();
}

void awsStandardSink::Show (void *sink, iAwsSource *source)
{
  (void)sink;

  iAwsComponent *c = source->GetComponent ();

  c->Show ();
}

void awsStandardSink::HideWindow (void *sink, iAwsSource *source)
{
  sink = (awsStandardSink *)sink;
  iAwsComponent *c = source->GetComponent ();
  c->Window()->Hide ();
}

void awsStandardSink::MaximizeWindow (void *sink, iAwsSource *source)
{
  sink = (awsStandardSink *)sink;
  iAwsComponent *c = source->GetComponent ();
  c->Window ()->Maximize ();
}

void awsStandardSink::UnMaximizeWindow (void *sink, iAwsSource *source)
{
  sink = (awsStandardSink *)sink;
  iAwsComponent *c = source->GetComponent ();
  c->Window ()->UnMaximize ();
}

void awsStandardSink::WindowSlideOutLeft (void *_sink, iAwsSource *source)
{
  awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsComponent *win = c->Window();

  if (c == win)
    return;
  else
    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_LEFT);
}

void awsStandardSink::WindowSlideOutRight (void *_sink, iAwsSource *source)
{
   awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsComponent *win = c->Window();

  if (c == win)
    return;
  else
    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_RIGHT);
}

void awsStandardSink::WindowSlideOutUp (void *_sink, iAwsSource *source)
{
   awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsComponent *win = c->Window();

  if (c == win)
    return;
  else
    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_UP);
}

void awsStandardSink::WindowSlideOutDown (void *_sink, iAwsSource *source)
{
   awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsComponent *win = c->Window();

  if (c == win)
    return;
  else
    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_DOWN);
}


void awsStandardSink::Invalidate (void *sink, iAwsSource *source)
{
  (void)sink;

  iAwsComponent *c = source->GetComponent ();

  c->Invalidate ();
}

awsStandardSink::awsStandardSink (iAws *_wmgr) :
  awsSink(), wmgr(_wmgr)
{
  SetParm (this);
  RegisterTrigger ("Show", Show);
  RegisterTrigger ("Hide", Hide);
  RegisterTrigger ("HideWindow", HideWindow);
  RegisterTrigger ("Invalidate", Invalidate);
  RegisterTrigger ("SlideOutLeft", WindowSlideOutLeft);
  RegisterTrigger ("SlideOutRight", WindowSlideOutRight);
  RegisterTrigger ("SlideOutUp", WindowSlideOutUp);
  RegisterTrigger ("SlideOutDown", WindowSlideOutDown);
  RegisterTrigger ("MaximizeWindow", MaximizeWindow);
  RegisterTrigger ("UnMaximizeWindow", UnMaximizeWindow);
}

awsStandardSink::~awsStandardSink ()
{
}
