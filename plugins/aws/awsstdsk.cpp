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

  if (strcmp (c->Type (), "Window") == 0)
  {
    c->Hide ();
  }
  else
  {
    if (c->Window ()) c->Window ()->Hide ();
  }
}

void awsStandardSink::WindowSlideOutLeft (void *_sink, iAwsSource *source)
{
  awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsWindow    *win=NULL;

  if (strcmp (c->Type (), "Window") == 0)
  {
    return;
  }
  else
  {
    if (c->Window ()) win=c->Window ();
    else return;

    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_LEFT, 0.05);

  }
}

void awsStandardSink::WindowSlideOutRight (void *_sink, iAwsSource *source)
{
   awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsWindow    *win=NULL;

  if (strcmp (c->Type (), "Window") == 0)
  {
    return;
  }
  else
  {
    if (c->Window ()) win=c->Window ();
    else return;

    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_RIGHT, 0.05);

  }
}

void awsStandardSink::WindowSlideOutUp (void *_sink, iAwsSource *source)
{
   awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsWindow    *win=NULL;

  if (strcmp (c->Type (), "Window") == 0)
  {
    return;
  }
  else
  {
    if (c->Window ()) win=c->Window ();
    else return;

    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_UP, 0.05);

  }
}

void awsStandardSink::WindowSlideOutDown (void *_sink, iAwsSource *source)
{
   awsStandardSink *sink = (awsStandardSink *)_sink;

  iAwsComponent *c = source->GetComponent ();
  iAwsWindow    *win=NULL;

  if (strcmp (c->Type (), "Window") == 0)
  {
    return;
  }
  else
  {
    if (c->Window ()) win=c->Window ();
    else return;

    sink->wmgr->CreateTransition(win, AWS_TRANSITION_SLIDE_OUT_DOWN, 0.05);

  }
}


void awsStandardSink::Invalidate (void *sink, iAwsSource *source)
{
  (void)sink;

  iAwsComponent *c = source->GetComponent ();

  c->Invalidate ();
}

awsStandardSink::awsStandardSink (iAws *_wmgr) :
  awsSink(this), wmgr(_wmgr)
{
  RegisterTrigger ("Show", Show);
  RegisterTrigger ("Hide", Hide);
  RegisterTrigger ("HideWindow", HideWindow);
  RegisterTrigger ("Invalidate", Invalidate);
  RegisterTrigger ("SlideOutLeft", WindowSlideOutLeft);
  RegisterTrigger ("SlideOutRight", WindowSlideOutRight);
  RegisterTrigger ("SlideOutUp", WindowSlideOutUp);
  RegisterTrigger ("SlideOutDown", WindowSlideOutDown);
}

awsStandardSink::~awsStandardSink ()
{
}
