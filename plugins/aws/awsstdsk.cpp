#include "cssysdef.h"
#include "awsstdsk.h"
#include "awscomp.h"

#include <stdio.h>


void 
awsStandardSink::Hide(void *sink, iAwsSource *source)
{
  (void) sink;
  iAwsComponent *c = source->GetComponent();

  c->Hide();
}
 
void 
awsStandardSink::Show(void *sink, iAwsSource *source)
{
  (void) sink;
  iAwsComponent *c = source->GetComponent();

  c->Show();
}

void 
awsStandardSink::HideWindow(void *sink, iAwsSource *source)
{
  (void) sink;
  iAwsComponent *c = source->GetComponent();

  if (strcmp(c->Type(), "Window")==0)
  {
    c->Hide();
  }
  else
  { 
    if (c->Window()) c->Window()->Hide();
  }
}

void 
awsStandardSink::Invalidate(void *sink, iAwsSource *source)
{
  (void) sink;
  iAwsComponent *c = source->GetComponent();

  c->Invalidate();
}

awsStandardSink::awsStandardSink():awsSink(NULL)
{
  RegisterTrigger("Show", Show);
  RegisterTrigger("Hide", Hide);
  RegisterTrigger("HideWindow", HideWindow);
  RegisterTrigger("Invalidate", Invalidate);
}

awsStandardSink::~awsStandardSink()
{

}
