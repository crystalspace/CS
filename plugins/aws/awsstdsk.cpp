#include "cssysdef.h"
#include "aws/awsstdsk.h"
#include "aws/awscomp.h"

#include <stdio.h>


void 
awsStandardSink::Hide(void *sink, iAwsSource *source)
{
  awsComponent *c = (awsComponent *)source;

  c->Hide();
}
 
void 
awsStandardSink::Show(void *sink, iAwsSource *source)
{
  awsComponent *c = (awsComponent *)source;

  c->Show();
}

void 
awsStandardSink::HideWindow(void *sink, iAwsSource *source)
{
  awsComponent *c = (awsComponent *)source;

  if (strcmp(c->Type(), "Window")==0)
  {
    c->Hide();
  }
  else
  { 
    printf("hiding window, %s\n", c->Window()->Type());
    //if (c->Window()) c->Window()->Hide();
  }
}

void 
awsStandardSink::Invalidate(void *sink, iAwsSource *source)
{
  awsComponent *c = (awsComponent *)source;

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
