#include "cssysdef.h"
#include "awssink.h"
#include "csutil/scfstr.h"
#include "csutil/csstring.h"

#include "iaws/awsparm.h"

#include <stdio.h>

enum radiostate {none,speed,accel,position,rectangle,numparticle,particletime,addage};
static radiostate r_state = none;
enum inputstate {nostate,textbox,scrollbar};
static inputstate i_state = nostate;

awsSink * awsSink::asink = NULL;

awsSink::awsSink() : wmgr(0), getinput1(0), getinput2(0), 
  getinput3(0), getinput4(0), test(0)
{
	asink = this;
	valueX = new float[5];
	valueY = new float[5];
	valueZ = new float[5];
	valueOther = new float[5];
	valueX[0] = 0.0;
	valueY[0] = 0.0;
	valueZ[0] = 0.0;
	valueOther[0] = 0.0;
	setinput = new char[255];
	updatestate = true;
}

awsSink::~awsSink()
{
}

void awsSink::SetSink(iAwsSink *s)
{
  sink = s;

  if (sink) {
    sink->RegisterTrigger("SetScrollBarX", &SetScrollBarX);
	sink->RegisterTrigger("SetScrollBarY", &SetScrollBarY);
	sink->RegisterTrigger("SetScrollBarZ", &SetScrollBarZ);
	sink->RegisterTrigger("SetScrollBarOther", &SetScrollBarOther);
    sink->RegisterTrigger("SetInput1",&SetInput1);
	sink->RegisterTrigger("SetInput2",&SetInput2);
	sink->RegisterTrigger("SetInput3",&SetInput3);
	sink->RegisterTrigger("SetInput4",&SetInput4);
    sink->RegisterTrigger("SetSpeed",&SetSpeed);
	sink->RegisterTrigger("SetAccel",&SetAccel);
	sink->RegisterTrigger("SetPosition",&SetPosition);
	sink->RegisterTrigger("SetRectangle",&SetRectangle);
	sink->RegisterTrigger("SetNumParticle",&SetNumParticle);
	sink->RegisterTrigger("SetParticleTime",&SetParticleTime);
	sink->RegisterTrigger("SetAddAge",&SetAddAge);
	sink->RegisterTrigger("RegisterInput1",&RegisterInput1);
	sink->RegisterTrigger("RegisterInput2",&RegisterInput2);
	sink->RegisterTrigger("RegisterInput3",&RegisterInput3);
	sink->RegisterTrigger("RegisterInput4",&RegisterInput4);
	sink->RegisterTrigger("RegisterLabel1",&RegisterLabel1);
	sink->RegisterTrigger("RegisterLabel2",&RegisterLabel2);
	sink->RegisterTrigger("RegisterLabel3",&RegisterLabel3);
	sink->RegisterTrigger("RegisterLabel4",&RegisterLabel4);
	sink->RegisterTrigger("RegisterFrame",&RegisterFrame);
  }
}


void awsSink::SetTestWin(iAwsWindow *testwin)
{
  test=testwin;
}

void awsSink::SetWindowManager(iAws *_wmgr)
{
  wmgr=_wmgr;
}

void awsSink::SetScrollBarX(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Value", (void**)&sink->valueX);

  sink->updatestate = true;
  i_state = scrollbar;
	//if(sink->valueX != NULL)
		//printf("Value: %f\n",sink->valueX[0]);
}
void awsSink::SetScrollBarY(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Value", (void**)&sink->valueY);

  sink->updatestate = true;
  i_state = scrollbar;
	//if(sink->valueY != NULL)
		//printf("Value: %f\n",sink->valueY[0]);
}
void awsSink::SetScrollBarZ(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Value", (void**)&sink->valueZ);

  sink->updatestate = true;
  i_state = scrollbar;
	//if(sink->valueZ != NULL)
		//printf("Value: %f\n",sink->valueZ[0]);
}
void awsSink::SetScrollBarOther(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  iAwsComponent *comp = source->GetComponent();
  if(comp->GetProperty("Value",(void**)&sink->valueOther))
	  printf("Value: %f\n",sink->valueOther[0]);

  sink->updatestate = true;
  i_state = scrollbar;
  //comp->GetProperty("Value", (void**)&sink->valueOther);
	//if(sink->valueOther != NULL)
		
}
void awsSink::SetInput1(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;
  csRef<iString>tmp = csPtr<iString> (new scfString);
  if (sink->getinput1) sink->getinput1->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (void**)&sink->getinput1);
  if(sink->getinput1) {
	  sink->valueX[0] = atof(sink->getinput1->GetData());
	  sink->updatestate = true;
	  i_state = textbox;
	  printf("GetInput1: %f\n",sink->valueX[0]);
  }
  return;
}
void awsSink::SetInput2(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;
  csRef<iString>tmp = csPtr<iString> (new scfString);
  if (sink->getinput2) sink->getinput2->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (void**)&sink->getinput2);
  if(sink->getinput2) {
	  sink->valueY[0] = atof(sink->getinput2->GetData());
	  sink->updatestate = true;
	  i_state = textbox;
  }
  return;
}
void awsSink::SetInput3(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;
  csRef<iString>tmp = csPtr<iString> (new scfString);
  if (sink->getinput3) sink->getinput3->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (void**)&sink->getinput3);
  if(sink->getinput3) {
	  sink->valueZ[0] = atof(sink->getinput3->GetData());
	  sink->updatestate = true;
	  i_state = textbox;
  }
  return;
}
void awsSink::SetInput4(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;
  csRef<iString>tmp = csPtr<iString> (new scfString);
  if (sink->getinput4) sink->getinput4->DecRef();

  iAwsComponent *comp = source->GetComponent();
  comp->GetProperty("Text", (void**)&sink->getinput4);
  if(sink->getinput4) {
	  sink->valueOther[0] = atof(sink->getinput4->GetData());
	  sink->updatestate = true;
	  i_state = textbox;
	  printf("GetInput4: %f\n",sink->valueOther[0]);
  }
  return;
}
void awsSink::RegisterInput1(void *sk, iAwsSource *source)
{
	asink->textbox1 = source->GetComponent();
	return;
}
void awsSink::RegisterInput2(void *sk, iAwsSource *source)
{
	asink->textbox2 = source->GetComponent();
	return;
}
void awsSink::RegisterInput3(void *sk, iAwsSource *source)
{
	asink->textbox3 = source->GetComponent();
	return;
}
void awsSink::RegisterInput4(void *sk, iAwsSource *source)
{
	asink->textbox4 = source->GetComponent();
	return;
}
void awsSink::RegisterLabel1(void *sk, iAwsSource *source)
{
	asink->label1 = source->GetComponent();
	return;
}
void awsSink::RegisterLabel2(void *sk, iAwsSource *source)
{
	asink->label2 = source->GetComponent();
	return;
}
void awsSink::RegisterLabel3(void *sk, iAwsSource *source)
{
	asink->label3 = source->GetComponent();
	return;
}
void awsSink::RegisterLabel4(void *sk, iAwsSource *source)
{
	asink->label4 = source->GetComponent();
	return;
}
void awsSink::RegisterFrame(void *sk, iAwsSource *source)
{
	asink->frame = source->GetComponent();
	return;
}

void awsSink::UpdateInput(float value,int textboxnum)
{
  char txt[20];
  gcvt (value,6,txt);

  csRef<iString> tmp (csPtr<iString> (new scfString(txt)));

  switch(textboxnum) 
  {
    case 1: 
		asink->textbox1->SetProperty("Text",(void*)tmp);
		break;
	case 2:
		asink->textbox2->SetProperty("Text",(void*)tmp);
		break;
	case 3:
		asink->textbox3->SetProperty("Text",(void*)tmp);
		break;
	case 4:
		asink->textbox4->SetProperty("Text",(void*)tmp);
		break;
	
  }
  return;
}
void awsSink::UpdateLabel(csString txt,int textboxnum)
{
  csRef<iString> tmp (csPtr<iString> (new scfString(txt)));
 
  
  switch(textboxnum) 
  {
    case 1: 
		asink->label1->SetProperty("Caption",(void*)tmp);
		break;
	case 2:
		asink->label2->SetProperty("Caption",(void*)tmp);
		break;
	case 3:
		asink->label3->SetProperty("Caption",(void*)tmp);
		break;
	case 4:
		asink->label4->SetProperty("Caption",(void*)tmp);
		break;
  }
}
void awsSink::SetSpeed(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  r_state=speed;
  sink->updatestate = true;
}
void awsSink::SetAccel(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  r_state=accel;
  sink->updatestate = true;
}
void awsSink::SetPosition(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  r_state=position;
  sink->updatestate = true;
}
void awsSink::SetRectangle(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  r_state=rectangle;
  sink->updatestate = true;
}
void awsSink::SetNumParticle(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  r_state=numparticle;
  sink->updatestate = true;
}
void awsSink::SetParticleTime(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  r_state=particletime;
  sink->updatestate = true;
}
void awsSink::SetAddAge(void *sk, iAwsSource *source)
{
  awsSink *sink = (awsSink *)sk;

  r_state=addage;
  sink->updatestate = true;
}
float awsSink::GetValueX(float scale,float phase)
{
  float temp;

  switch(i_state)
  {
    case nostate:
		temp = 0.0; 
		break;
	case textbox:
		temp = valueX[0];
        break;
	case scrollbar:
		temp = (valueX[0]+phase) * scale;
		break;
  }
  return temp;
}
float awsSink::GetValueY(float scale,float phase)
{
  float temp;

  switch(i_state)
  {
    case nostate:
		temp = 0.0; 
		break;
	case textbox:
		temp = valueY[0];
        break;
	case scrollbar:
		temp = (valueY[0]+phase) * scale;
		break;
  }
  return temp;
}
float awsSink::GetValueZ(float scale,float phase)
{
  float temp;

  switch(i_state)
  {
    case nostate:
		temp = 0.0; 
		break;
	case textbox:
		temp = valueZ[0];
        break;
	case scrollbar:
		temp = (valueZ[0]+phase) * scale;
		break;
  }
  return temp;
}
float awsSink::GetValueOther(float scale)
{
  float temp;

  switch(i_state)
  {
    case nostate:
		temp = 0.0; 
		break;
	case textbox:
		temp = valueOther[0];
        break;
	case scrollbar:
		temp = valueOther[0] * scale;
		break;
  }
  return temp;
}
int awsSink::GetState()
{
	return r_state;
}
bool awsSink::GetUpdateState()
{
	return updatestate;
}
void awsSink::SetUpdateState(bool state)
{
	updatestate = state;
}




