#include "cssysdef.h"
#include "iutil/vfs.h"
#include "csutil/scfstr.h"
#include "csutil/csstring.h"
#include "iengine/engine.h"
#include "awssink.h"

#include "iaws/awsparm.h"

#include <stdio.h>


/*
 *  Component reference  (iawscomponent_NAME)
 *  Data storage (somestruct.state.somevalue)
 *  Updated flag (somestruct.state_changed)
 *
 *  Register Creation Trigger in constructor  (sink->RegisterTrigger("text",&function) )
 *  Register Input Trigger in constructor (sink->RegisterTrigger("text",&function) )
 *
 *  Registration function (static)
 *  Input function (static)
 *  Update function
 *
 *
 */


////
// Some helper macros that make adding components and windows a bit less tedious
////

// This macro implements a static callback function which records the pointer to the iAwsComponent that calls the trigger
// The function named here should be attached to the creation trigger of the aws component
#define IMPLEMENT_REGISTER_FUNCTION(function,componentvar)  \
void awsSink::function(void *sk, iAwsSource *source) \
{ \
  asink->componentvar=source->GetComponent(); \
} 


// This macro implements a static callback function which pulls the value out of a textbox control
// converts it to a float, stores the float in a given variable beneath the global pointer 'asink'
// and sets a boolean variable beneath 'asink' to true to signal that a data update has occurred
#define IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(function,floatvar,invalidate_flag,update) \
void awsSink::function(void *sk, iAwsSource *source) \
{ \
  iString *textvalue; \
  if (source->GetComponent()->GetProperty("Text",(void **)&textvalue) && textvalue->Length()) \
  { \
    asink->floatvar=atof(textvalue->GetData()); \
    asink->invalidate_flag=true; \
  } \
  else \
    asink->update(); \
}


// This macro implements a static callback function which pulls the value out of a textbox control
// converts it to a integer, stores the integer in a given variable beneath the global pointer 'asink'
// and sets a boolean variable beneath 'asink' to true to signal that a data update has occurred
#define IMPLEMENT_COMPONENT_TEXTBOX_TO_INT(function,intvar,invalidate_flag,update) \
void awsSink::function(void *sk, iAwsSource *source) \
{ \
  iString *textvalue; \
  if (source->GetComponent()->GetProperty("Text",(void **)&textvalue) && textvalue->Length()) \
  { \
    asink->intvar=strtol(textvalue->GetData(),NULL,10); \
    asink->invalidate_flag=true; \
  } \
  else \
    asink->update(); \
}


// This macro implements a static callback function which checks the state of a checkbox or radio button
// and stores the result (on/off) in a given boolean variable beneath the global pointer 'asink'
// It also sets a different boolean variable beneath 'asink' to true to signal that a data update has occurred.
#define IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(function,boolvar,invalidate_flag,update) \
void awsSink::function(void *sk, iAwsSource *source) \
{ \
  bool *p_bvalue; \
  if (source->GetComponent()->GetProperty("State",(void **)&p_bvalue)) \
  { \
    if (*p_bvalue) \
      asink->boolvar=true; \
    else \
      asink->boolvar=false; \
    asink->invalidate_flag=true; \
  } \
  else \
    asink->update(); \
}


// This macro can be used to set the contents of a textbox given the iAwsComponent * of the text box
// and an integer.  value must be defined as a csRef<iString> prior to this macro being used in a
// function.
#define SET_TEXTBOX_INT(component,intval) \
    value = csPtr<iString> (new scfString()); \
    value->Format("%d",intval); \
    component->SetProperty("Text",value);

// This macro can be used to set the contents of a textbox given the iAwsComponent * of the text box
// and a float.  value must be defined as a csRef<iString> prior to this macro being used in a
// function.
#define SET_TEXTBOX_FLOAT(component,floatval) \
    value = csPtr<iString> (new scfString()); \
    value->Format("%f",floatval); \
    component->SetProperty("Text",value);

// This macro can be used to set the contents of a checkbox or radio button given the iAwsComponent * 
// of the checkbox or radio button and a statement that can evaluate to true/false.  
// boolval must be defined as a bool prior to this macro being used in a function.
#define SET_CHECKBOX_BOOL(component,boolval) \
  if (boolval) \
    bvalue=true; \
  else \
    bvalue=false; \
  component->SetProperty("State",&bvalue);




awsSink * awsSink::asink = NULL;

awsSink::awsSink() : wmgr(0) 
{
	asink = this;

  for (int i=0;i<(int)SECTION_COUNT;i++)
    SectionState[i]=false;

  // Initialize GraphicSelection Data
  GraphicSelectionData.iawscomponent_GraphicFileList=0;
  GraphicSelectionData.iawscomponent_GraphicFilter=0;
  GraphicSelectionData.iawscomponent_GraphicSelection=0;
  GraphicSelectionData.currentdirectory=new scfString();
  GraphicSelectionData.currentfilepath=new scfString();
  GraphicSelectionData.filter=new scfString();

  EmitterStateData.iawscomponent_EmitterState=0;
  EmitterStateData.settings_changed=false;

  InitialPositionData.iawscomponent_InitialPosition=0;
  InitialPositionData.settings_changed=false;
}

awsSink::~awsSink()
{
}

void awsSink::SetSink(iAwsSink *s)
{
  sink = s;

  if (sink) 
  {
    sink->RegisterTrigger("RegisterSectionSelection",&RegisterSectionSelection);
    sink->RegisterTrigger("FillSectionList",&FillSectionList);
    sink->RegisterTrigger("SectionListSelectionChanged",&SectionListSelectionChanged);


    sink->RegisterTrigger("RegisterGraphicSelection",&RegisterGraphicSelection);
    sink->RegisterTrigger("RegisterGraphicFileList",&RegisterGraphicFileList);
    sink->RegisterTrigger("RegisterGraphicFilter",&RegisterGraphicFilter);
    sink->RegisterTrigger("SetGraphicFilter",&AwsSetGraphicFilter);
    sink->RegisterTrigger("GraphicFileSelected",&AwsGraphicFileSelected);

    // Emitter State Callbacks
    sink->RegisterTrigger("RegisterEmitterState",&RegisterEmitterState);
    sink->RegisterTrigger("RegisterParticleCount",&RegisterParticleCount);
    sink->RegisterTrigger("RegisterParticleMaxAge",&RegisterParticleMaxAge);
    sink->RegisterTrigger("RegisterLighting",&RegisterLighting);
    sink->RegisterTrigger("RegisterAlphaBlend",&RegisterAlphaBlend);
    sink->RegisterTrigger("RegisterRectParticlesRadio",&RegisterRectParticlesRadio);
    sink->RegisterTrigger("RegisterRegParticlesRadio",&RegisterRegParticlesRadio);
    sink->RegisterTrigger("RegisterRectParticlesWidth",&RegisterRectParticlesWidth);
    sink->RegisterTrigger("RegisterRectParticlesHeight",&RegisterRectParticlesHeight);
    sink->RegisterTrigger("RegisterRegParticlesNumber",&RegisterRegParticlesNumber);
    sink->RegisterTrigger("RegisterRegParticlesRadius",&RegisterRegParticlesRadius);
    sink->RegisterTrigger("RegisterUseBoundingBox",&RegisterUseBoundingBox);
    sink->RegisterTrigger("RegisterBBoxMinX",&RegisterBBoxMinX);
    sink->RegisterTrigger("RegisterBBoxMinY",&RegisterBBoxMinY);
    sink->RegisterTrigger("RegisterBBoxMinZ",&RegisterBBoxMinZ);
    sink->RegisterTrigger("RegisterBBoxMaxX",&RegisterBBoxMaxX);
    sink->RegisterTrigger("RegisterBBoxMaxY",&RegisterBBoxMaxY);
    sink->RegisterTrigger("RegisterBBoxMaxZ",&RegisterBBoxMaxZ);
    sink->RegisterTrigger("SetParticleCount",&AwsSetParticleCount);
    sink->RegisterTrigger("SetParticleMaxAge",&AwsSetParticleMaxAge);
    sink->RegisterTrigger("SetLighting",&AwsSetLighting);
    sink->RegisterTrigger("SetAlphaBlend",&AwsSetAlphaBlend);
    sink->RegisterTrigger("SetParticleType",&AwsSetParticleType);
    sink->RegisterTrigger("SetRectangularWidth",&AwsSetRectangularWidth);
    sink->RegisterTrigger("SetRectangularHeight",&AwsSetRectangularHeight);
    sink->RegisterTrigger("SetRegularNumber",&AwsSetRegularNumber);
    sink->RegisterTrigger("SetRegularRadius",&AwsSetRegularRadius);
    sink->RegisterTrigger("SetUseBoundingBox",&AwsSetUseBoundingBox);
    sink->RegisterTrigger("SetBBoxMinX",&AwsSetBBoxMinX);
    sink->RegisterTrigger("SetBBoxMinY",&AwsSetBBoxMinY);
    sink->RegisterTrigger("SetBBoxMinZ",&AwsSetBBoxMinZ);
    sink->RegisterTrigger("SetBBoxMaxX",&AwsSetBBoxMaxX);
    sink->RegisterTrigger("SetBBoxMaxY",&AwsSetBBoxMaxY);
    sink->RegisterTrigger("SetBBoxMaxZ",&AwsSetBBoxMaxZ);

    // Initial Position Options Callbacks
    sink->RegisterTrigger("RegisterInitialPosition",&RegisterInitialPosition);
    sink->RegisterTrigger("RegisterIPFPX",&RegisterIPFPX);
    sink->RegisterTrigger("RegisterIPFPY",&RegisterIPFPY);
    sink->RegisterTrigger("RegisterIPFPZ",&RegisterIPFPZ);
    sink->RegisterTrigger("RegisterIPFWeight",&RegisterIPFWeight);
    sink->RegisterTrigger("RegisterIPLSX",&RegisterIPLSX);
    sink->RegisterTrigger("RegisterIPLSY",&RegisterIPLSY);
    sink->RegisterTrigger("RegisterIPLSZ",&RegisterIPLSZ);
    sink->RegisterTrigger("RegisterIPLEX",&RegisterIPLEX);
    sink->RegisterTrigger("RegisterIPLEY",&RegisterIPLEY);
    sink->RegisterTrigger("RegisterIPLEZ",&RegisterIPLEZ);
    sink->RegisterTrigger("RegisterIPLWeight",&RegisterIPLWeight);
    sink->RegisterTrigger("RegisterIPBMX",&RegisterIPBMX);
    sink->RegisterTrigger("RegisterIPBMY",&RegisterIPBMY);
    sink->RegisterTrigger("RegisterIPBMZ",&RegisterIPBMZ);
    sink->RegisterTrigger("RegisterIPBXX",&RegisterIPBXX);
    sink->RegisterTrigger("RegisterIPBXY",&RegisterIPBXY);
    sink->RegisterTrigger("RegisterIPBXZ",&RegisterIPBXZ);
    sink->RegisterTrigger("RegisterIPBWeight",&RegisterIPBWeight);
    sink->RegisterTrigger("RegisterIPSCX",&RegisterIPSCX);
    sink->RegisterTrigger("RegisterIPSCY",&RegisterIPSCY);
    sink->RegisterTrigger("RegisterIPSCZ",&RegisterIPSCZ);
    sink->RegisterTrigger("RegisterIPSMin",&RegisterIPSMin);
    sink->RegisterTrigger("RegisterIPSMax",&RegisterIPSMax);
    sink->RegisterTrigger("RegisterIPSWeight",&RegisterIPSWeight);
    sink->RegisterTrigger("RegisterIPCNOX",&RegisterIPCNOX);
    sink->RegisterTrigger("RegisterIPCNOY",&RegisterIPCNOY);
    sink->RegisterTrigger("RegisterIPCNOZ",&RegisterIPCNOZ);
    sink->RegisterTrigger("RegisterIPCNElev",&RegisterIPCNElev);
    sink->RegisterTrigger("RegisterIPCNAzim",&RegisterIPCNAzim);
    sink->RegisterTrigger("RegisterIPCNAper",&RegisterIPCNAper);
    sink->RegisterTrigger("RegisterIPCNMin",&RegisterIPCNMin);
    sink->RegisterTrigger("RegisterIPCNMax",&RegisterIPCNMax);
    sink->RegisterTrigger("RegisterIPCNWeight",&RegisterIPCNWeight);
    sink->RegisterTrigger("RegisterIPCYSX",&RegisterIPCYSX);
    sink->RegisterTrigger("RegisterIPCYSY",&RegisterIPCYSY);
    sink->RegisterTrigger("RegisterIPCYSZ",&RegisterIPCYSZ);
    sink->RegisterTrigger("RegisterIPCYEX",&RegisterIPCYEX);
    sink->RegisterTrigger("RegisterIPCYEY",&RegisterIPCYEY);
    sink->RegisterTrigger("RegisterIPCYEZ",&RegisterIPCYEZ);
    sink->RegisterTrigger("RegisterIPCYMin",&RegisterIPCYMin);
    sink->RegisterTrigger("RegisterIPCYMax",&RegisterIPCYMax);
    sink->RegisterTrigger("RegisterIPCYWeight",&RegisterIPCYWeight);
    sink->RegisterTrigger("RegisterIPSTCX",&RegisterIPSTCX);
    sink->RegisterTrigger("RegisterIPSTCY",&RegisterIPSTCY);
    sink->RegisterTrigger("RegisterIPSTCZ",&RegisterIPSTCZ);
    sink->RegisterTrigger("RegisterIPSTMin",&RegisterIPSTMin);
    sink->RegisterTrigger("RegisterIPSTMax",&RegisterIPSTMax);
    sink->RegisterTrigger("RegisterIPSTWeight",&RegisterIPSTWeight);
    sink->RegisterTrigger("RegisterIPCYTSX",&RegisterIPCYTSX);
    sink->RegisterTrigger("RegisterIPCYTSY",&RegisterIPCYTSY);
    sink->RegisterTrigger("RegisterIPCYTSZ",&RegisterIPCYTSZ);
    sink->RegisterTrigger("RegisterIPCYTEX",&RegisterIPCYTEX);
    sink->RegisterTrigger("RegisterIPCYTEY",&RegisterIPCYTEY);
    sink->RegisterTrigger("RegisterIPCYTEZ",&RegisterIPCYTEZ);
    sink->RegisterTrigger("RegisterIPCYTMin",&RegisterIPCYTMin);
    sink->RegisterTrigger("RegisterIPCYTMax",&RegisterIPCYTMax);
    sink->RegisterTrigger("RegisterIPCYTWeight",&RegisterIPCYTWeight);



    sink->RegisterTrigger("SetIPFPositionX",&AwsSetIPFPositionX);
    sink->RegisterTrigger("SetIPFPositionY",&AwsSetIPFPositionY);
    sink->RegisterTrigger("SetIPFPositionZ",&AwsSetIPFPositionZ);
    sink->RegisterTrigger("SetIPFWeight",&AwsSetIPFWeight);
    sink->RegisterTrigger("SetIPLStartX",&AwsSetIPLStartX);
    sink->RegisterTrigger("SetIPLStartY",&AwsSetIPLStartY);
    sink->RegisterTrigger("SetIPLStartZ",&AwsSetIPLStartZ);
    sink->RegisterTrigger("SetIPLEndX",&AwsSetIPLEndX);
    sink->RegisterTrigger("SetIPLEndY",&AwsSetIPLEndY);
    sink->RegisterTrigger("SetIPLEndZ",&AwsSetIPLEndZ);
    sink->RegisterTrigger("SetIPLWeight",&AwsSetIPLWeight);
    sink->RegisterTrigger("SetIPBMinX",&AwsSetIPBMinX);
    sink->RegisterTrigger("SetIPBMinY",&AwsSetIPBMinY);
    sink->RegisterTrigger("SetIPBMinZ",&AwsSetIPBMinZ);
    sink->RegisterTrigger("SetIPBMaxX",&AwsSetIPBMaxX);
    sink->RegisterTrigger("SetIPBMaxY",&AwsSetIPBMaxY);
    sink->RegisterTrigger("SetIPBMaxZ",&AwsSetIPBMaxZ);
    sink->RegisterTrigger("SetIPBWeight",&AwsSetIPBWeight);
    sink->RegisterTrigger("SetIPSCenterX",&AwsSetIPSCenterX);
    sink->RegisterTrigger("SetIPSCenterY",&AwsSetIPSCenterY);
    sink->RegisterTrigger("SetIPSCenterZ",&AwsSetIPSCenterZ);
    sink->RegisterTrigger("SetIPSMin",&AwsSetIPSMin);
    sink->RegisterTrigger("SetIPSMax",&AwsSetIPSMax);
    sink->RegisterTrigger("SetIPSWeight",&AwsSetIPSWeight);
    sink->RegisterTrigger("SetIPCNOriginX",&AwsSetIPCNOriginX);
    sink->RegisterTrigger("SetIPCNOriginY",&AwsSetIPCNOriginY);
    sink->RegisterTrigger("SetIPCNOriginZ",&AwsSetIPCNOriginZ);
    sink->RegisterTrigger("SetIPCNElev",&AwsSetIPCNElev);
    sink->RegisterTrigger("SetIPCNAzim",&AwsSetIPCNAzim);
    sink->RegisterTrigger("SetIPCNAper",&AwsSetIPCNAper);
    sink->RegisterTrigger("SetIPCNMin",&AwsSetIPCNMin);
    sink->RegisterTrigger("SetIPCNMax",&AwsSetIPCNMax);
    sink->RegisterTrigger("SetIPCNWeight",&AwsSetIPCNWeight);
    sink->RegisterTrigger("SetIPCYStartX",&AwsSetIPCYStartX);
    sink->RegisterTrigger("SetIPCYStartY",&AwsSetIPCYStartY);
    sink->RegisterTrigger("SetIPCYStartZ",&AwsSetIPCYStartZ);
    sink->RegisterTrigger("SetIPCYEndX",&AwsSetIPCYEndX);
    sink->RegisterTrigger("SetIPCYEndY",&AwsSetIPCYEndY);
    sink->RegisterTrigger("SetIPCYEndZ",&AwsSetIPCYEndZ);
    sink->RegisterTrigger("SetIPCYMin",&AwsSetIPCYMin);
    sink->RegisterTrigger("SetIPCYMax",&AwsSetIPCYMax);
    sink->RegisterTrigger("SetIPCYWeight",&AwsSetIPCYWeight);
    sink->RegisterTrigger("SetIPSTCenterX",&AwsSetIPSTCenterX);
    sink->RegisterTrigger("SetIPSTCenterY",&AwsSetIPSTCenterY);
    sink->RegisterTrigger("SetIPSTCenterZ",&AwsSetIPSTCenterZ);
    sink->RegisterTrigger("SetIPSTMin",&AwsSetIPSTMin);
    sink->RegisterTrigger("SetIPSTMax",&AwsSetIPSTMax);
    sink->RegisterTrigger("SetIPSTWeight",&AwsSetIPSTWeight);
    sink->RegisterTrigger("SetIPCYTStartX",&AwsSetIPCYTStartX);
    sink->RegisterTrigger("SetIPCYTStartY",&AwsSetIPCYTStartY);
    sink->RegisterTrigger("SetIPCYTStartZ",&AwsSetIPCYTStartZ);
    sink->RegisterTrigger("SetIPCYTEndX",&AwsSetIPCYTEndX);
    sink->RegisterTrigger("SetIPCYTEndY",&AwsSetIPCYTEndY);
    sink->RegisterTrigger("SetIPCYTEndZ",&AwsSetIPCYTEndZ);
    sink->RegisterTrigger("SetIPCYTMin",&AwsSetIPCYTMin);
    sink->RegisterTrigger("SetIPCYTMax",&AwsSetIPCYTMax);
    sink->RegisterTrigger("SetIPCYTWeight",&AwsSetIPCYTWeight);


    // Attractor Options Callbacks
    sink->RegisterTrigger("RegisterAttractor",&RegisterAttractor);
    sink->RegisterTrigger("RegisterATForce",&RegisterATForce);
    sink->RegisterTrigger("RegisterATFPX",&RegisterATFPX);
    sink->RegisterTrigger("RegisterATFPY",&RegisterATFPY);
    sink->RegisterTrigger("RegisterATFPZ",&RegisterATFPZ);
    sink->RegisterTrigger("RegisterATFWeight",&RegisterATFWeight);
    sink->RegisterTrigger("RegisterATLSX",&RegisterATLSX);
    sink->RegisterTrigger("RegisterATLSY",&RegisterATLSY);
    sink->RegisterTrigger("RegisterATLSZ",&RegisterATLSZ);
    sink->RegisterTrigger("RegisterATLEX",&RegisterATLEX);
    sink->RegisterTrigger("RegisterATLEY",&RegisterATLEY);
    sink->RegisterTrigger("RegisterATLEZ",&RegisterATLEZ);
    sink->RegisterTrigger("RegisterATLWeight",&RegisterATLWeight);
    sink->RegisterTrigger("RegisterATBMX",&RegisterATBMX);
    sink->RegisterTrigger("RegisterATBMY",&RegisterATBMY);
    sink->RegisterTrigger("RegisterATBMZ",&RegisterATBMZ);
    sink->RegisterTrigger("RegisterATBXX",&RegisterATBXX);
    sink->RegisterTrigger("RegisterATBXY",&RegisterATBXY);
    sink->RegisterTrigger("RegisterATBXZ",&RegisterATBXZ);
    sink->RegisterTrigger("RegisterATBWeight",&RegisterATBWeight);
    sink->RegisterTrigger("RegisterATSCX",&RegisterATSCX);
    sink->RegisterTrigger("RegisterATSCY",&RegisterATSCY);
    sink->RegisterTrigger("RegisterATSCZ",&RegisterATSCZ);
    sink->RegisterTrigger("RegisterATSMin",&RegisterATSMin);
    sink->RegisterTrigger("RegisterATSMax",&RegisterATSMax);
    sink->RegisterTrigger("RegisterATSWeight",&RegisterATSWeight);
    sink->RegisterTrigger("RegisterATCNOX",&RegisterATCNOX);
    sink->RegisterTrigger("RegisterATCNOY",&RegisterATCNOY);
    sink->RegisterTrigger("RegisterATCNOZ",&RegisterATCNOZ);
    sink->RegisterTrigger("RegisterATCNElev",&RegisterATCNElev);
    sink->RegisterTrigger("RegisterATCNAzim",&RegisterATCNAzim);
    sink->RegisterTrigger("RegisterATCNAper",&RegisterATCNAper);
    sink->RegisterTrigger("RegisterATCNMin",&RegisterATCNMin);
    sink->RegisterTrigger("RegisterATCNMax",&RegisterATCNMax);
    sink->RegisterTrigger("RegisterATCNWeight",&RegisterATCNWeight);
    sink->RegisterTrigger("RegisterATCYSX",&RegisterATCYSX);
    sink->RegisterTrigger("RegisterATCYSY",&RegisterATCYSY);
    sink->RegisterTrigger("RegisterATCYSZ",&RegisterATCYSZ);
    sink->RegisterTrigger("RegisterATCYEX",&RegisterATCYEX);
    sink->RegisterTrigger("RegisterATCYEY",&RegisterATCYEY);
    sink->RegisterTrigger("RegisterATCYEZ",&RegisterATCYEZ);
    sink->RegisterTrigger("RegisterATCYMin",&RegisterATCYMin);
    sink->RegisterTrigger("RegisterATCYMax",&RegisterATCYMax);
    sink->RegisterTrigger("RegisterATCYWeight",&RegisterATCYWeight);
    sink->RegisterTrigger("RegisterATSTCX",&RegisterATSTCX);
    sink->RegisterTrigger("RegisterATSTCY",&RegisterATSTCY);
    sink->RegisterTrigger("RegisterATSTCZ",&RegisterATSTCZ);
    sink->RegisterTrigger("RegisterATSTMin",&RegisterATSTMin);
    sink->RegisterTrigger("RegisterATSTMax",&RegisterATSTMax);
    sink->RegisterTrigger("RegisterATSTWeight",&RegisterATSTWeight);
    sink->RegisterTrigger("RegisterATCYTSX",&RegisterATCYTSX);
    sink->RegisterTrigger("RegisterATCYTSY",&RegisterATCYTSY);
    sink->RegisterTrigger("RegisterATCYTSZ",&RegisterATCYTSZ);
    sink->RegisterTrigger("RegisterATCYTEX",&RegisterATCYTEX);
    sink->RegisterTrigger("RegisterATCYTEY",&RegisterATCYTEY);
    sink->RegisterTrigger("RegisterATCYTEZ",&RegisterATCYTEZ);
    sink->RegisterTrigger("RegisterATCYTMin",&RegisterATCYTMin);
    sink->RegisterTrigger("RegisterATCYTMax",&RegisterATCYTMax);
    sink->RegisterTrigger("RegisterATCYTWeight",&RegisterATCYTWeight);



    sink->RegisterTrigger("SetATForce",&AwsSetATForce);
    sink->RegisterTrigger("SetATFPositionX",&AwsSetATFPositionX);
    sink->RegisterTrigger("SetATFPositionY",&AwsSetATFPositionY);
    sink->RegisterTrigger("SetATFPositionZ",&AwsSetATFPositionZ);
    sink->RegisterTrigger("SetATFWeight",&AwsSetATFWeight);
    sink->RegisterTrigger("SetATLStartX",&AwsSetATLStartX);
    sink->RegisterTrigger("SetATLStartY",&AwsSetATLStartY);
    sink->RegisterTrigger("SetATLStartZ",&AwsSetATLStartZ);
    sink->RegisterTrigger("SetATLEndX",&AwsSetATLEndX);
    sink->RegisterTrigger("SetATLEndY",&AwsSetATLEndY);
    sink->RegisterTrigger("SetATLEndZ",&AwsSetATLEndZ);
    sink->RegisterTrigger("SetATLWeight",&AwsSetATLWeight);
    sink->RegisterTrigger("SetATBMinX",&AwsSetATBMinX);
    sink->RegisterTrigger("SetATBMinY",&AwsSetATBMinY);
    sink->RegisterTrigger("SetATBMinZ",&AwsSetATBMinZ);
    sink->RegisterTrigger("SetATBMaxX",&AwsSetATBMaxX);
    sink->RegisterTrigger("SetATBMaxY",&AwsSetATBMaxY);
    sink->RegisterTrigger("SetATBMaxZ",&AwsSetATBMaxZ);
    sink->RegisterTrigger("SetATBWeight",&AwsSetATBWeight);
    sink->RegisterTrigger("SetATSCenterX",&AwsSetATSCenterX);
    sink->RegisterTrigger("SetATSCenterY",&AwsSetATSCenterY);
    sink->RegisterTrigger("SetATSCenterZ",&AwsSetATSCenterZ);
    sink->RegisterTrigger("SetATSMin",&AwsSetATSMin);
    sink->RegisterTrigger("SetATSMax",&AwsSetATSMax);
    sink->RegisterTrigger("SetATSWeight",&AwsSetATSWeight);
    sink->RegisterTrigger("SetATCNOriginX",&AwsSetATCNOriginX);
    sink->RegisterTrigger("SetATCNOriginY",&AwsSetATCNOriginY);
    sink->RegisterTrigger("SetATCNOriginZ",&AwsSetATCNOriginZ);
    sink->RegisterTrigger("SetATCNElev",&AwsSetATCNElev);
    sink->RegisterTrigger("SetATCNAzim",&AwsSetATCNAzim);
    sink->RegisterTrigger("SetATCNAper",&AwsSetATCNAper);
    sink->RegisterTrigger("SetATCNMin",&AwsSetATCNMin);
    sink->RegisterTrigger("SetATCNMax",&AwsSetATCNMax);
    sink->RegisterTrigger("SetATCNWeight",&AwsSetATCNWeight);
    sink->RegisterTrigger("SetATCYStartX",&AwsSetATCYStartX);
    sink->RegisterTrigger("SetATCYStartY",&AwsSetATCYStartY);
    sink->RegisterTrigger("SetATCYStartZ",&AwsSetATCYStartZ);
    sink->RegisterTrigger("SetATCYEndX",&AwsSetATCYEndX);
    sink->RegisterTrigger("SetATCYEndY",&AwsSetATCYEndY);
    sink->RegisterTrigger("SetATCYEndZ",&AwsSetATCYEndZ);
    sink->RegisterTrigger("SetATCYMin",&AwsSetATCYMin);
    sink->RegisterTrigger("SetATCYMax",&AwsSetATCYMax);
    sink->RegisterTrigger("SetATCYWeight",&AwsSetATCYWeight);
    sink->RegisterTrigger("SetATSTCenterX",&AwsSetATSTCenterX);
    sink->RegisterTrigger("SetATSTCenterY",&AwsSetATSTCenterY);
    sink->RegisterTrigger("SetATSTCenterZ",&AwsSetATSTCenterZ);
    sink->RegisterTrigger("SetATSTMin",&AwsSetATSTMin);
    sink->RegisterTrigger("SetATSTMax",&AwsSetATSTMax);
    sink->RegisterTrigger("SetATSTWeight",&AwsSetATSTWeight);
    sink->RegisterTrigger("SetATCYTStartX",&AwsSetATCYTStartX);
    sink->RegisterTrigger("SetATCYTStartY",&AwsSetATCYTStartY);
    sink->RegisterTrigger("SetATCYTStartZ",&AwsSetATCYTStartZ);
    sink->RegisterTrigger("SetATCYTEndX",&AwsSetATCYTEndX);
    sink->RegisterTrigger("SetATCYTEndY",&AwsSetATCYTEndY);
    sink->RegisterTrigger("SetATCYTEndZ",&AwsSetATCYTEndZ);
    sink->RegisterTrigger("SetATCYTMin",&AwsSetATCYTMin);
    sink->RegisterTrigger("SetATCYTMax",&AwsSetATCYTMax);
    sink->RegisterTrigger("SetATCYTWeight",&AwsSetATCYTWeight);


  }
}

void awsSink::SetWindowManager(iAws *_wmgr)
{
  wmgr=_wmgr;
}



void awsSink::RegisterSectionSelection(void *sk, iAwsSource *source)
{
  asink->iawscomponent_SectionSelection = source->GetComponent();
  return;
}


void awsSink::FillSectionList(void *sk, iAwsSource *source)
{
  asink->iawscomponent_SectionList = source->GetComponent();
  iAwsParmList *pl=0;
  pl=asink->wmgr->CreateParmList();
  asink->iawscomponent_SectionList->Execute("ClearList");

  pl->Clear();
  pl->AddString("text0","Graphic");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Emitter State");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Initial Position");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Initial Speed");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Initial Acceleration");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Field Speed");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Field Acceleration");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Attractor");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Aging Moments");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->Clear();
  pl->AddString("text0","Load/Save State");
  pl->AddBool("selectable",false);
  pl->AddBool("stateful0",true);
  pl->AddBool("state0",false);
  asink->iawscomponent_SectionList->Execute("InsertItem",pl);

  pl->DecRef();

}

void awsSink::SectionListSelectionChanged(void *sk, iAwsSource *source)
{
  int i;
  iAwsParmList *pl=0;
  pl=asink->wmgr->CreateParmList();

  // Figure out which state changed
  for (i=0;i<(int)SECTION_COUNT;i++)
  {
    bool state;
    pl->Clear();
    pl->AddInt("row",i);
    pl->AddBool("state0",true);
    asink->iawscomponent_SectionList->Execute("GetItem",pl);
    pl->GetBool("state0",&state);


    if (state != asink->SectionState[i])
      break;
  }
  if (i>=(int)SECTION_COUNT)
    return; // None changed

  asink->SectionState[i]=!asink->SectionState[i];

  switch(i)
  {
    case SECTION_GRAPHIC:
      if (asink->SectionState[i])
      {
        asink->GraphicSelectionData.iawscomponent_GraphicSelection->Show();
        asink->GraphicSelectionData.iawscomponent_GraphicSelection->Raise();
      }
      else
        asink->GraphicSelectionData.iawscomponent_GraphicSelection->Hide();
      break;
    case SECTION_EMITTER_STATE:
      if (asink->SectionState[i])
      {
        asink->EmitterStateData.iawscomponent_EmitterState->Show();
        asink->EmitterStateData.iawscomponent_EmitterState->Raise();
      }
      else
        asink->EmitterStateData.iawscomponent_EmitterState->Hide();
      break;
    case SECTION_INIT_POSITION:
      if (asink->SectionState[i])
      {
        asink->InitialPositionData.iawscomponent_InitialPosition->Show();
        asink->InitialPositionData.iawscomponent_InitialPosition->Raise();
      }
      else
        asink->InitialPositionData.iawscomponent_InitialPosition->Hide();
      break;
    case SECTION_ATTRACTOR:
      if (asink->SectionState[i])
      {
        asink->AttractorData.iawscomponent_Attractor->Show();
        asink->AttractorData.iawscomponent_Attractor->Raise();
      }
      else
        asink->AttractorData.iawscomponent_Attractor->Hide();
      break;
    case SECTION_INIT_SPEED:
    case SECTION_INIT_ACCELERATION:
    case SECTION_FIELD_SPEED:
    case SECTION_FIELD_ACCELERATION:
    case SECTION_AGING_MOMENTS:
    default:
      printf("Unhandled section state change %d.\n",i);
      break;
  }


}






////
// Graphic Selection Functions
////

// Registration
IMPLEMENT_REGISTER_FUNCTION(RegisterGraphicSelection,GraphicSelectionData.iawscomponent_GraphicSelection)
IMPLEMENT_REGISTER_FUNCTION(RegisterGraphicFilter,GraphicSelectionData.iawscomponent_GraphicFilter)
IMPLEMENT_REGISTER_FUNCTION(RegisterGraphicFileList,GraphicSelectionData.iawscomponent_GraphicFileList)





const char *awsSink::GetGraphicCWD()
{
  return GraphicSelectionData.currentdirectory->GetData();
}

void awsSink::SetGraphicCWD(const char *cwd)
{
  GraphicSelectionData.currentdirectory->Clear();
  GraphicSelectionData.currentdirectory->Append(cwd);
  FillGraphicFileList();
}

const char *awsSink::GetGraphicFile()
{
  return GraphicSelectionData.currentfilepath->GetData();
}

void awsSink::SetGraphicFile(const char *filestr)
{
  GraphicSelectionData.currentfilepath->Clear();
  GraphicSelectionData.currentfilepath->Append(GraphicSelectionData.currentdirectory);
  GraphicSelectionData.currentfilepath->Append(filestr);
}


const char *awsSink::GetGraphicFilter()
{
  return GraphicSelectionData.filter->GetData();
}

void awsSink::SetGraphicFilter(const char *filterstr)
{
  GraphicSelectionData.filter->Clear();
  GraphicSelectionData.filter->Append(filterstr);
  FillGraphicFileList();
}

/// Static callback to handle graphic file working directory change.
void awsSink::AwsSetGraphicFilter(void *sk, iAwsSource *source)
{
  iString *cwd;
  csRef<iString> path=new scfString();
  csRef<iString> filter=new scfString();

  if (asink->GraphicSelectionData.iawscomponent_GraphicFilter->GetProperty("Text",(void **)&cwd) && cwd->Length())
  {
    size_t position=cwd->FindLast('/');
    if (position != (size_t) -1)
    {
      // Filter contains a path
      cwd->SubString(path,0,position+1);
      cwd->SubString(filter,position+1,cwd->Length()-position-1);
      asink->GraphicSelectionData.iawscomponent_GraphicFilter->SetProperty("Text",filter);
    }
    else
    {
      filter->Replace(cwd);
    }
  }

  // Handle filter first
  asink->GraphicSelectionData.filter->Replace(filter);

  // Handle path
  if (path->Length())
  {
    if (!strcmp(path->GetData(),"../"))
    {
      if (strcmp(asink->GraphicSelectionData.currentdirectory->GetData(),"/"))
      {
        size_t next_slash=asink->GraphicSelectionData.currentdirectory->FindLast('/',asink->GraphicSelectionData.currentdirectory->Length()-2);
        if (next_slash != (size_t) -1)
          asink->GraphicSelectionData.currentdirectory->Truncate(next_slash+1);
      }
    }
    else
      asink->GraphicSelectionData.currentdirectory->Append(path);
  }

  // Update file list
  asink->FillGraphicFileList();
}

  /// Static callback to handle graphic file selection change
void awsSink::AwsGraphicFileSelected(void *sk, iAwsSource *source)
{
  // Read the selected value
  iString *filename;
  iAwsParmList *pl=0;
  pl=asink->wmgr->CreateParmList();
  pl->Clear();
  pl->AddString("text0","");
  
  asink->GraphicSelectionData.iawscomponent_GraphicFileList->Execute("GetSelectedItem",pl);
  pl->GetString("text0",&filename);

  if ((*filename)[filename->Length()-1]=='/')
  {
    // If terminates with / , then this is a directory
    if (!strcmp(filename->GetData(),"../"))
    {
      // If ../  then we need to go up one directory
      if (strcmp(asink->GraphicSelectionData.currentdirectory->GetData(),"/"))
      {
        size_t next_slash=asink->GraphicSelectionData.currentdirectory->FindLast('/',asink->GraphicSelectionData.currentdirectory->Length()-2);
        if (next_slash != (size_t) -1)
          asink->GraphicSelectionData.currentdirectory->Truncate(next_slash+1);
      }
    }
    else
    {
      // Otherwise add to current directory
      asink->GraphicSelectionData.currentdirectory->Append(filename);
    }
    // Update File List
    asink->FillGraphicFileList();
  }
  else
  {
    // Else set current selected file
    asink->GraphicSelectionData.currentfilepath->Clear();
    asink->GraphicSelectionData.currentfilepath->Append(asink->GraphicSelectionData.currentdirectory);
    asink->GraphicSelectionData.currentfilepath->Append(filename->GetData());
  }
}


void awsSink::FillGraphicFileList()
{
  if (GraphicSelectionData.iawscomponent_GraphicFileList != 0 && vfs != 0)
  {
    csRef<iStringArray> files;
    int length,i;
    const char *filename;
    iAwsParmList *pl=0;
    pl=wmgr->CreateParmList();

    // Update the path display
    GraphicSelectionData.iawscomponent_GraphicFileList->SetProperty("Column0Caption",GraphicSelectionData.currentdirectory->GetData());

    // Clear out list
    GraphicSelectionData.iawscomponent_GraphicFileList->Execute("ClearList");


    // Include .. if this isn't the root
    if (strcmp(GraphicSelectionData.currentdirectory->GetData(),"/"))
    {
      pl->Clear();
      pl->AddString("text0","../");
      GraphicSelectionData.iawscomponent_GraphicFileList->Execute("InsertItem",pl);
    }

    // Read directories first.
    files=vfs->FindFiles(GraphicSelectionData.currentdirectory->GetData());
    if (files != 0 )
    {
      length=files->Length();
      for (i=0;i<length;i++)
      {
        filename=files->Get(i);
        if (filename[strlen(filename)-1]=='/' && strlen(filename)>GraphicSelectionData.currentdirectory->Length())
        {
          pl->Clear();
          pl->AddString("text0",filename+GraphicSelectionData.currentdirectory->Length());
          GraphicSelectionData.iawscomponent_GraphicFileList->Execute("InsertItem",pl);
        }
      }
    }

    // Read filtered files next
    if (GraphicSelectionData.filter->Length()>0)
    {
      scfString merged(GraphicSelectionData.currentdirectory->GetData());
      merged.Append(GraphicSelectionData.filter);
      files=vfs->FindFiles(merged.GetData());
    }

    if (files != 0 )
    {
      length=files->Length();
      for (i=0;i<length;i++)
      {
        filename=files->Get(i);
        if (filename[strlen(filename)-1]!='/' && strlen(filename)>GraphicSelectionData.currentdirectory->Length())
        {
          pl->Clear();
          pl->AddString("text0",filename+GraphicSelectionData.currentdirectory->Length());
          GraphicSelectionData.iawscomponent_GraphicFileList->Execute("InsertItem",pl);
        }
      }
    }
  }
}



////
//  Emitter State functions
////

// Registration
IMPLEMENT_REGISTER_FUNCTION(RegisterEmitterState,EmitterStateData.iawscomponent_EmitterState)
IMPLEMENT_REGISTER_FUNCTION(RegisterParticleCount,EmitterStateData.iawscomponent_ParticleCount)
IMPLEMENT_REGISTER_FUNCTION(RegisterParticleMaxAge,EmitterStateData.iawscomponent_ParticleMaxAge)
IMPLEMENT_REGISTER_FUNCTION(RegisterLighting,EmitterStateData.iawscomponent_Lighting)
IMPLEMENT_REGISTER_FUNCTION(RegisterAlphaBlend,EmitterStateData.iawscomponent_AlphaBlend)
IMPLEMENT_REGISTER_FUNCTION(RegisterRectParticlesRadio,EmitterStateData.iawscomponent_RectParticlesRadio)
IMPLEMENT_REGISTER_FUNCTION(RegisterRegParticlesRadio,EmitterStateData.iawscomponent_RegParticlesRadio)
IMPLEMENT_REGISTER_FUNCTION(RegisterRectParticlesWidth,EmitterStateData.iawscomponent_RectParticlesWidth)
IMPLEMENT_REGISTER_FUNCTION(RegisterRectParticlesHeight,EmitterStateData.iawscomponent_RectParticlesHeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterRegParticlesNumber,EmitterStateData.iawscomponent_RegParticlesNumber)
IMPLEMENT_REGISTER_FUNCTION(RegisterRegParticlesRadius,EmitterStateData.iawscomponent_RegParticlesRadius)
IMPLEMENT_REGISTER_FUNCTION(RegisterUseBoundingBox,EmitterStateData.iawscomponent_UseBoundingBox)
IMPLEMENT_REGISTER_FUNCTION(RegisterBBoxMinX,EmitterStateData.iawscomponent_BBoxMinX)
IMPLEMENT_REGISTER_FUNCTION(RegisterBBoxMinY,EmitterStateData.iawscomponent_BBoxMinY)
IMPLEMENT_REGISTER_FUNCTION(RegisterBBoxMinZ,EmitterStateData.iawscomponent_BBoxMinZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterBBoxMaxX,EmitterStateData.iawscomponent_BBoxMaxX)
IMPLEMENT_REGISTER_FUNCTION(RegisterBBoxMaxY,EmitterStateData.iawscomponent_BBoxMaxY)
IMPLEMENT_REGISTER_FUNCTION(RegisterBBoxMaxZ,EmitterStateData.iawscomponent_BBoxMaxZ)




IMPLEMENT_COMPONENT_TEXTBOX_TO_INT(AwsSetParticleCount,
								   EmitterStateData.state.particle_count,EmitterStateData.settings_changed,
								   UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_INT(AwsSetParticleMaxAge,
								   EmitterStateData.state.particle_max_age,EmitterStateData.settings_changed,
								   UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(AwsSetLighting,
									 EmitterStateData.state.lighting,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(AwsSetAlphaBlend,
									 EmitterStateData.state.alpha_blend,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
// Although this is a radio button, it only has 2 options, so we can treat it like a checkbox
IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(AwsSetParticleType,
									 EmitterStateData.state.rectangular_particles,
									 EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetRectangularWidth,
									 EmitterStateData.state.rect_w,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetRectangularHeight,
									 EmitterStateData.state.rect_h,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetRegularNumber,
									 EmitterStateData.state.reg_number,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetRegularRadius,
									 EmitterStateData.state.reg_radius,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(AwsSetUseBoundingBox,
									 EmitterStateData.state.using_bounding_box,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetBBoxMinX,
									 EmitterStateData.state.bbox_minx,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetBBoxMinY,
									 EmitterStateData.state.bbox_miny,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetBBoxMinZ,
									 EmitterStateData.state.bbox_minz,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetBBoxMaxX,
									 EmitterStateData.state.bbox_maxx,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetBBoxMaxY,
									 EmitterStateData.state.bbox_maxy,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetBBoxMaxZ,
									 EmitterStateData.state.bbox_maxz,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)






bool awsSink::EmitterStateChanged()
{
  return EmitterStateData.settings_changed;
}

void awsSink::ClearEmitterStateChanged()
{
  EmitterStateData.settings_changed=false;
}

EmitterState *awsSink::GetEmitterState()
{
  return &(EmitterStateData.state);
}

void awsSink::SetEmitterState(EmitterState *source)
{

  memcpy(&(EmitterStateData.state),source,sizeof(EmitterState));
  UpdateEmitterStateDisplay();
  ClearEmitterStateChanged();
}





void awsSink::UpdateEmitterStateDisplay()
{
  bool bvalue;
  csRef<iString> value;

  SET_TEXTBOX_INT(EmitterStateData.iawscomponent_ParticleCount,EmitterStateData.state.particle_count);
  SET_TEXTBOX_INT(EmitterStateData.iawscomponent_ParticleMaxAge,EmitterStateData.state.particle_max_age);
  SET_CHECKBOX_BOOL(EmitterStateData.iawscomponent_Lighting,EmitterStateData.state.lighting);
  SET_CHECKBOX_BOOL(EmitterStateData.iawscomponent_AlphaBlend,EmitterStateData.state.alpha_blend);
  SET_CHECKBOX_BOOL(EmitterStateData.iawscomponent_RectParticlesRadio,EmitterStateData.state.rectangular_particles);
  SET_CHECKBOX_BOOL(EmitterStateData.iawscomponent_RegParticlesRadio,!EmitterStateData.state.rectangular_particles);
  SET_CHECKBOX_BOOL(EmitterStateData.iawscomponent_UseBoundingBox,EmitterStateData.state.using_bounding_box);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_RectParticlesWidth,EmitterStateData.state.rect_w);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_RectParticlesHeight,EmitterStateData.state.rect_h);
  SET_TEXTBOX_INT(EmitterStateData.iawscomponent_RegParticlesNumber,EmitterStateData.state.reg_number);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_RegParticlesRadius,EmitterStateData.state.reg_radius);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_BBoxMinX,EmitterStateData.state.bbox_minx);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_BBoxMinY,EmitterStateData.state.bbox_miny);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_BBoxMinZ,EmitterStateData.state.bbox_minz);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_BBoxMaxX,EmitterStateData.state.bbox_maxx);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_BBoxMaxY,EmitterStateData.state.bbox_maxy);
  SET_TEXTBOX_FLOAT(EmitterStateData.iawscomponent_BBoxMaxZ,EmitterStateData.state.bbox_maxz);
}




////
//  Initial Position Display
////
IMPLEMENT_REGISTER_FUNCTION(RegisterInitialPosition,InitialPositionData.iawscomponent_InitialPosition)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPFPX,InitialPositionData.iawscomponent_IPFPX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPFPY,InitialPositionData.iawscomponent_IPFPY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPFPZ,InitialPositionData.iawscomponent_IPFPZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPFWeight,InitialPositionData.iawscomponent_IPFWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPLSX,InitialPositionData.iawscomponent_IPLSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPLSY,InitialPositionData.iawscomponent_IPLSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPLSZ,InitialPositionData.iawscomponent_IPLSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPLEX,InitialPositionData.iawscomponent_IPLEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPLEY,InitialPositionData.iawscomponent_IPLEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPLEZ,InitialPositionData.iawscomponent_IPLEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPLWeight,InitialPositionData.iawscomponent_IPLWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPBMX,InitialPositionData.iawscomponent_IPBMX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPBMY,InitialPositionData.iawscomponent_IPBMY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPBMZ,InitialPositionData.iawscomponent_IPBMZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPBXX,InitialPositionData.iawscomponent_IPBXX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPBXY,InitialPositionData.iawscomponent_IPBXY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPBXZ,InitialPositionData.iawscomponent_IPBXZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPBWeight,InitialPositionData.iawscomponent_IPBWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSCX,InitialPositionData.iawscomponent_IPSCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSCY,InitialPositionData.iawscomponent_IPSCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSCZ,InitialPositionData.iawscomponent_IPSCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSMin,InitialPositionData.iawscomponent_IPSMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSMax,InitialPositionData.iawscomponent_IPSMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSWeight,InitialPositionData.iawscomponent_IPSWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNOX,InitialPositionData.iawscomponent_IPCNOX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNOY,InitialPositionData.iawscomponent_IPCNOY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNOZ,InitialPositionData.iawscomponent_IPCNOZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNElev,InitialPositionData.iawscomponent_IPCNElevation)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNAzim,InitialPositionData.iawscomponent_IPCNAzimuth)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNAper,InitialPositionData.iawscomponent_IPCNAperture)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNMin,InitialPositionData.iawscomponent_IPCNMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNMax,InitialPositionData.iawscomponent_IPCNMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCNWeight,InitialPositionData.iawscomponent_IPCNWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYSX,InitialPositionData.iawscomponent_IPCYSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYSY,InitialPositionData.iawscomponent_IPCYSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYSZ,InitialPositionData.iawscomponent_IPCYSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYEX,InitialPositionData.iawscomponent_IPCYEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYEY,InitialPositionData.iawscomponent_IPCYEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYEZ,InitialPositionData.iawscomponent_IPCYEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYMin,InitialPositionData.iawscomponent_IPCYMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYMax,InitialPositionData.iawscomponent_IPCYMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYWeight,InitialPositionData.iawscomponent_IPCYWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSTCX,InitialPositionData.iawscomponent_IPSTCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSTCY,InitialPositionData.iawscomponent_IPSTCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSTCZ,InitialPositionData.iawscomponent_IPSTCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSTMin,InitialPositionData.iawscomponent_IPSTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSTMax,InitialPositionData.iawscomponent_IPSTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPSTWeight,InitialPositionData.iawscomponent_IPSTWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTSX,InitialPositionData.iawscomponent_IPCYTSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTSY,InitialPositionData.iawscomponent_IPCYTSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTSZ,InitialPositionData.iawscomponent_IPCYTSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTEX,InitialPositionData.iawscomponent_IPCYTEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTEY,InitialPositionData.iawscomponent_IPCYTEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTEZ,InitialPositionData.iawscomponent_IPCYTEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTMin,InitialPositionData.iawscomponent_IPCYTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTMax,InitialPositionData.iawscomponent_IPCYTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIPCYTWeight,InitialPositionData.iawscomponent_IPCYTWeight)



// Fixed Position
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPFPositionX,
									 InitialPositionData.state.fixed_position.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPFPositionY,
									 InitialPositionData.state.fixed_position.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPFPositionZ,
									 InitialPositionData.state.fixed_position.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPFWeight,
									 InitialPositionData.state.fixed_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
// Line
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPLStartX,
									 InitialPositionData.state.line_start.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPLStartY,
									 InitialPositionData.state.line_start.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPLStartZ,
									 InitialPositionData.state.line_start.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPLEndX,
									 InitialPositionData.state.line_end.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPLEndY,
									 InitialPositionData.state.line_end.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPLEndZ,
									 InitialPositionData.state.line_end.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPLWeight,
									 InitialPositionData.state.line_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)

// Box
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPBMinX,
									 InitialPositionData.state.box_min.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPBMinY,
									 InitialPositionData.state.box_min.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPBMinZ,
									 InitialPositionData.state.box_min.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPBMaxX,
									 InitialPositionData.state.box_max.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPBMaxY,
									 InitialPositionData.state.box_max.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPBMaxZ,
									 InitialPositionData.state.box_max.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPBWeight,
									 InitialPositionData.state.box_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)

// Sphere
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSCenterX,
									 InitialPositionData.state.sphere_center.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSCenterY,
									 InitialPositionData.state.sphere_center.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSCenterZ,
									 InitialPositionData.state.sphere_center.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSMin,
									 InitialPositionData.state.sphere_min,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSMax,
									 InitialPositionData.state.sphere_max,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSWeight,
									 InitialPositionData.state.sphere_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)

// Cone
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNOriginX,
									 InitialPositionData.state.cone_origin.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNOriginY,
									 InitialPositionData.state.cone_origin.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNOriginZ,
									 InitialPositionData.state.cone_origin.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNElev,
									 InitialPositionData.state.cone_elevation,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNAzim,
									 InitialPositionData.state.cone_azimuth,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNAper,
									 InitialPositionData.state.cone_aperture,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNMin,
									 InitialPositionData.state.cone_min,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNMax,
									 InitialPositionData.state.cone_max,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCNWeight,
									 InitialPositionData.state.cone_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)

// Cylinder
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYStartX,
									 InitialPositionData.state.cylinder_start.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYStartY,
									 InitialPositionData.state.cylinder_start.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYStartZ,
									 InitialPositionData.state.cylinder_start.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYEndX,
									 InitialPositionData.state.cylinder_end.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYEndY,
									 InitialPositionData.state.cylinder_end.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYEndZ,
									 InitialPositionData.state.cylinder_end.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYMin,
									 InitialPositionData.state.cylinder_min,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYMax,
									 InitialPositionData.state.cylinder_max,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYWeight,
									 InitialPositionData.state.cylinder_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)

// Sphere Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSTCenterX,
									 InitialPositionData.state.spheretangent_center.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSTCenterY,
									 InitialPositionData.state.spheretangent_center.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSTCenterZ,
									 InitialPositionData.state.spheretangent_center.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSTMin,
									 InitialPositionData.state.spheretangent_min,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSTMax,
									 InitialPositionData.state.spheretangent_max,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPSTWeight,
									 InitialPositionData.state.spheretangent_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)

// Cylinder Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTStartX,
									 InitialPositionData.state.cylindertangent_start.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTStartY,
									 InitialPositionData.state.cylindertangent_start.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTStartZ,
									 InitialPositionData.state.cylindertangent_start.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTEndX,
									 InitialPositionData.state.cylindertangent_end.x,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTEndY,
									 InitialPositionData.state.cylindertangent_end.y,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTEndZ,
									 InitialPositionData.state.cylindertangent_end.z,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTMin,
									 InitialPositionData.state.cylindertangent_min,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTMax,
									 InitialPositionData.state.cylindertangent_max,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIPCYTWeight,
									 InitialPositionData.state.cylindertangent_weight,InitialPositionData.settings_changed,
									 UpdateInitialPositionStateDisplay)






bool awsSink::InitialPositionStateChanged()
{
  return InitialPositionData.settings_changed;
}

void awsSink::ClearInitialPositionStateChanged()
{
  InitialPositionData.settings_changed=false;
}

Emitter3DState *awsSink::GetInitialPositionState()
{
  return &(InitialPositionData.state);
}

void awsSink::SetInitialPositionState(Emitter3DState *source)
{
  memcpy(&(InitialPositionData.state),source,sizeof(Emitter3DState));
  ClearInitialPositionStateChanged();
  UpdateInitialPositionStateDisplay();
}

void awsSink::UpdateInitialPositionStateDisplay()
{
  csRef<iString> value;

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPFPX,InitialPositionData.state.fixed_position.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPFPY,InitialPositionData.state.fixed_position.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPFPZ,InitialPositionData.state.fixed_position.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPFWeight,InitialPositionData.state.fixed_weight);

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPLSX,InitialPositionData.state.line_start.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPLSY,InitialPositionData.state.line_start.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPLSZ,InitialPositionData.state.line_start.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPLEX,InitialPositionData.state.line_end.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPLEY,InitialPositionData.state.line_end.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPLEZ,InitialPositionData.state.line_end.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPLWeight,InitialPositionData.state.line_weight);

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPBMX,InitialPositionData.state.box_min.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPBMY,InitialPositionData.state.box_min.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPBMZ,InitialPositionData.state.box_min.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPBXX,InitialPositionData.state.box_max.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPBXY,InitialPositionData.state.box_max.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPBXZ,InitialPositionData.state.box_max.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPBWeight,InitialPositionData.state.box_weight);

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSCX,InitialPositionData.state.sphere_center.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSCY,InitialPositionData.state.sphere_center.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSCZ,InitialPositionData.state.sphere_center.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSMin,InitialPositionData.state.sphere_min);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSMax,InitialPositionData.state.sphere_max);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSWeight,InitialPositionData.state.sphere_weight);

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNOX,InitialPositionData.state.cone_origin.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNOY,InitialPositionData.state.cone_origin.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNOZ,InitialPositionData.state.cone_origin.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNElevation,InitialPositionData.state.cone_elevation);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNAzimuth,InitialPositionData.state.cone_azimuth);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNAperture,InitialPositionData.state.cone_aperture);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNMin,InitialPositionData.state.cone_min);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNMax,InitialPositionData.state.cone_max);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCNWeight,InitialPositionData.state.cone_weight);

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYSX,InitialPositionData.state.cylinder_start.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYSY,InitialPositionData.state.cylinder_start.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYSZ,InitialPositionData.state.cylinder_start.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYEX,InitialPositionData.state.cylinder_end.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYEY,InitialPositionData.state.cylinder_end.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYEZ,InitialPositionData.state.cylinder_end.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYMin,InitialPositionData.state.cylinder_min);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYMax,InitialPositionData.state.cylinder_max);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYWeight,InitialPositionData.state.cylinder_weight);

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSTCX,InitialPositionData.state.spheretangent_center.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSTCY,InitialPositionData.state.spheretangent_center.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSTCZ,InitialPositionData.state.spheretangent_center.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSTMin,InitialPositionData.state.spheretangent_min);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSTMax,InitialPositionData.state.spheretangent_max);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPSTWeight,InitialPositionData.state.spheretangent_weight);

  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTSX,InitialPositionData.state.cylindertangent_start.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTSY,InitialPositionData.state.cylindertangent_start.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTSZ,InitialPositionData.state.cylindertangent_start.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTEX,InitialPositionData.state.cylindertangent_end.x);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTEY,InitialPositionData.state.cylindertangent_end.y);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTEZ,InitialPositionData.state.cylindertangent_end.z);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTMin,InitialPositionData.state.cylindertangent_min);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTMax,InitialPositionData.state.cylindertangent_max);
  SET_TEXTBOX_FLOAT(InitialPositionData.iawscomponent_IPCYTWeight,InitialPositionData.state.cylindertangent_weight);


}






////
//  Attractor Display
////
IMPLEMENT_REGISTER_FUNCTION(RegisterAttractor,AttractorData.iawscomponent_Attractor)
IMPLEMENT_REGISTER_FUNCTION(RegisterATForce,AttractorData.iawscomponent_ATForce)
IMPLEMENT_REGISTER_FUNCTION(RegisterATFPX,AttractorData.iawscomponent_ATFPX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATFPY,AttractorData.iawscomponent_ATFPY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATFPZ,AttractorData.iawscomponent_ATFPZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATFWeight,AttractorData.iawscomponent_ATFWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterATLSX,AttractorData.iawscomponent_ATLSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATLSY,AttractorData.iawscomponent_ATLSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATLSZ,AttractorData.iawscomponent_ATLSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATLEX,AttractorData.iawscomponent_ATLEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATLEY,AttractorData.iawscomponent_ATLEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATLEZ,AttractorData.iawscomponent_ATLEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATLWeight,AttractorData.iawscomponent_ATLWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterATBMX,AttractorData.iawscomponent_ATBMX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATBMY,AttractorData.iawscomponent_ATBMY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATBMZ,AttractorData.iawscomponent_ATBMZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATBXX,AttractorData.iawscomponent_ATBXX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATBXY,AttractorData.iawscomponent_ATBXY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATBXZ,AttractorData.iawscomponent_ATBXZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATBWeight,AttractorData.iawscomponent_ATBWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSCX,AttractorData.iawscomponent_ATSCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSCY,AttractorData.iawscomponent_ATSCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSCZ,AttractorData.iawscomponent_ATSCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSMin,AttractorData.iawscomponent_ATSMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSMax,AttractorData.iawscomponent_ATSMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSWeight,AttractorData.iawscomponent_ATSWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNOX,AttractorData.iawscomponent_ATCNOX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNOY,AttractorData.iawscomponent_ATCNOY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNOZ,AttractorData.iawscomponent_ATCNOZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNElev,AttractorData.iawscomponent_ATCNElevation)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNAzim,AttractorData.iawscomponent_ATCNAzimuth)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNAper,AttractorData.iawscomponent_ATCNAperture)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNMin,AttractorData.iawscomponent_ATCNMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNMax,AttractorData.iawscomponent_ATCNMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCNWeight,AttractorData.iawscomponent_ATCNWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYSX,AttractorData.iawscomponent_ATCYSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYSY,AttractorData.iawscomponent_ATCYSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYSZ,AttractorData.iawscomponent_ATCYSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYEX,AttractorData.iawscomponent_ATCYEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYEY,AttractorData.iawscomponent_ATCYEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYEZ,AttractorData.iawscomponent_ATCYEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYMin,AttractorData.iawscomponent_ATCYMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYMax,AttractorData.iawscomponent_ATCYMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYWeight,AttractorData.iawscomponent_ATCYWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSTCX,AttractorData.iawscomponent_ATSTCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSTCY,AttractorData.iawscomponent_ATSTCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSTCZ,AttractorData.iawscomponent_ATSTCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSTMin,AttractorData.iawscomponent_ATSTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSTMax,AttractorData.iawscomponent_ATSTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterATSTWeight,AttractorData.iawscomponent_ATSTWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTSX,AttractorData.iawscomponent_ATCYTSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTSY,AttractorData.iawscomponent_ATCYTSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTSZ,AttractorData.iawscomponent_ATCYTSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTEX,AttractorData.iawscomponent_ATCYTEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTEY,AttractorData.iawscomponent_ATCYTEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTEZ,AttractorData.iawscomponent_ATCYTEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTMin,AttractorData.iawscomponent_ATCYTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTMax,AttractorData.iawscomponent_ATCYTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterATCYTWeight,AttractorData.iawscomponent_ATCYTWeight)



IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATForce,
									 AttractorData.state.force,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)


// Fixed Position
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATFPositionX,
									 AttractorData.state.e3d_state.fixed_position.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATFPositionY,
									 AttractorData.state.e3d_state.fixed_position.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATFPositionZ,
									 AttractorData.state.e3d_state.fixed_position.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATFWeight,
									 AttractorData.state.e3d_state.fixed_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
// Line
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATLStartX,
									 AttractorData.state.e3d_state.line_start.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATLStartY,
									 AttractorData.state.e3d_state.line_start.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATLStartZ,
									 AttractorData.state.e3d_state.line_start.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATLEndX,
									 AttractorData.state.e3d_state.line_end.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATLEndY,
									 AttractorData.state.e3d_state.line_end.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATLEndZ,
									 AttractorData.state.e3d_state.line_end.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATLWeight,
									 AttractorData.state.e3d_state.line_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)

// Box
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATBMinX,
									 AttractorData.state.e3d_state.box_min.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATBMinY,
									 AttractorData.state.e3d_state.box_min.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATBMinZ,
									 AttractorData.state.e3d_state.box_min.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATBMaxX,
									 AttractorData.state.e3d_state.box_max.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATBMaxY,
									 AttractorData.state.e3d_state.box_max.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATBMaxZ,
									 AttractorData.state.e3d_state.box_max.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATBWeight,
									 AttractorData.state.e3d_state.box_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)

// Sphere
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSCenterX,
									 AttractorData.state.e3d_state.sphere_center.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSCenterY,
									 AttractorData.state.e3d_state.sphere_center.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSCenterZ,
									 AttractorData.state.e3d_state.sphere_center.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSMin,
									 AttractorData.state.e3d_state.sphere_min,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSMax,
									 AttractorData.state.e3d_state.sphere_max,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSWeight,
									 AttractorData.state.e3d_state.sphere_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)

// Cone
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNOriginX,
									 AttractorData.state.e3d_state.cone_origin.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNOriginY,
									 AttractorData.state.e3d_state.cone_origin.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNOriginZ,
									 AttractorData.state.e3d_state.cone_origin.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNElev,
									 AttractorData.state.e3d_state.cone_elevation,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNAzim,
									 AttractorData.state.e3d_state.cone_azimuth,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNAper,
									 AttractorData.state.e3d_state.cone_aperture,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNMin,
									 AttractorData.state.e3d_state.cone_min,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNMax,
									 AttractorData.state.e3d_state.cone_max,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCNWeight,
									 AttractorData.state.e3d_state.cone_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)

// Cylinder
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYStartX,
									 AttractorData.state.e3d_state.cylinder_start.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYStartY,
									 AttractorData.state.e3d_state.cylinder_start.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYStartZ,
									 AttractorData.state.e3d_state.cylinder_start.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYEndX,
									 AttractorData.state.e3d_state.cylinder_end.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYEndY,
									 AttractorData.state.e3d_state.cylinder_end.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYEndZ,
									 AttractorData.state.e3d_state.cylinder_end.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYMin,
									 AttractorData.state.e3d_state.cylinder_min,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYMax,
									 AttractorData.state.e3d_state.cylinder_max,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYWeight,
									 AttractorData.state.e3d_state.cylinder_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)

// Sphere Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSTCenterX,
									 AttractorData.state.e3d_state.spheretangent_center.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSTCenterY,
									 AttractorData.state.e3d_state.spheretangent_center.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSTCenterZ,
									 AttractorData.state.e3d_state.spheretangent_center.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSTMin,
									 AttractorData.state.e3d_state.spheretangent_min,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSTMax,
									 AttractorData.state.e3d_state.spheretangent_max,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATSTWeight,
									 AttractorData.state.e3d_state.spheretangent_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)

// Cylinder Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTStartX,
									 AttractorData.state.e3d_state.cylindertangent_start.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTStartY,
									 AttractorData.state.e3d_state.cylindertangent_start.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTStartZ,
									 AttractorData.state.e3d_state.cylindertangent_start.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTEndX,
									 AttractorData.state.e3d_state.cylindertangent_end.x,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTEndY,
									 AttractorData.state.e3d_state.cylindertangent_end.y,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTEndZ,
									 AttractorData.state.e3d_state.cylindertangent_end.z,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTMin,
									 AttractorData.state.e3d_state.cylindertangent_min,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTMax,
									 AttractorData.state.e3d_state.cylindertangent_max,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetATCYTWeight,
									 AttractorData.state.e3d_state.cylindertangent_weight,AttractorData.settings_changed,
									 UpdateAttractorStateDisplay)






bool awsSink::AttractorStateChanged()
{
  return AttractorData.settings_changed;
}

void awsSink::ClearAttractorStateChanged()
{
  AttractorData.settings_changed=false;
}

AttractorState *awsSink::GetAttractorState()
{
  return &(AttractorData.state);
}

void awsSink::SetAttractorState(AttractorState *source)
{
  memcpy(&(AttractorData.state),source,sizeof(AttractorState));
  ClearAttractorStateChanged();
  UpdateAttractorStateDisplay();
}

void awsSink::UpdateAttractorStateDisplay()
{
  csRef<iString> value;

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATForce,AttractorData.state.force);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATFPX,AttractorData.state.e3d_state.fixed_position.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATFPY,AttractorData.state.e3d_state.fixed_position.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATFPZ,AttractorData.state.e3d_state.fixed_position.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATFWeight,AttractorData.state.e3d_state.fixed_weight);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATLSX,AttractorData.state.e3d_state.line_start.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATLSY,AttractorData.state.e3d_state.line_start.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATLSZ,AttractorData.state.e3d_state.line_start.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATLEX,AttractorData.state.e3d_state.line_end.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATLEY,AttractorData.state.e3d_state.line_end.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATLEZ,AttractorData.state.e3d_state.line_end.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATLWeight,AttractorData.state.e3d_state.line_weight);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATBMX,AttractorData.state.e3d_state.box_min.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATBMY,AttractorData.state.e3d_state.box_min.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATBMZ,AttractorData.state.e3d_state.box_min.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATBXX,AttractorData.state.e3d_state.box_max.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATBXY,AttractorData.state.e3d_state.box_max.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATBXZ,AttractorData.state.e3d_state.box_max.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATBWeight,AttractorData.state.e3d_state.box_weight);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSCX,AttractorData.state.e3d_state.sphere_center.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSCY,AttractorData.state.e3d_state.sphere_center.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSCZ,AttractorData.state.e3d_state.sphere_center.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSMin,AttractorData.state.e3d_state.sphere_min);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSMax,AttractorData.state.e3d_state.sphere_max);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSWeight,AttractorData.state.e3d_state.sphere_weight);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNOX,AttractorData.state.e3d_state.cone_origin.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNOY,AttractorData.state.e3d_state.cone_origin.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNOZ,AttractorData.state.e3d_state.cone_origin.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNElevation,AttractorData.state.e3d_state.cone_elevation);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNAzimuth,AttractorData.state.e3d_state.cone_azimuth);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNAperture,AttractorData.state.e3d_state.cone_aperture);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNMin,AttractorData.state.e3d_state.cone_min);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNMax,AttractorData.state.e3d_state.cone_max);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCNWeight,AttractorData.state.e3d_state.cone_weight);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYSX,AttractorData.state.e3d_state.cylinder_start.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYSY,AttractorData.state.e3d_state.cylinder_start.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYSZ,AttractorData.state.e3d_state.cylinder_start.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYEX,AttractorData.state.e3d_state.cylinder_end.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYEY,AttractorData.state.e3d_state.cylinder_end.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYEZ,AttractorData.state.e3d_state.cylinder_end.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYMin,AttractorData.state.e3d_state.cylinder_min);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYMax,AttractorData.state.e3d_state.cylinder_max);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYWeight,AttractorData.state.e3d_state.cylinder_weight);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSTCX,AttractorData.state.e3d_state.spheretangent_center.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSTCY,AttractorData.state.e3d_state.spheretangent_center.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSTCZ,AttractorData.state.e3d_state.spheretangent_center.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSTMin,AttractorData.state.e3d_state.spheretangent_min);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSTMax,AttractorData.state.e3d_state.spheretangent_max);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATSTWeight,AttractorData.state.e3d_state.spheretangent_weight);

  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTSX,AttractorData.state.e3d_state.cylindertangent_start.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTSY,AttractorData.state.e3d_state.cylindertangent_start.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTSZ,AttractorData.state.e3d_state.cylindertangent_start.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTEX,AttractorData.state.e3d_state.cylindertangent_end.x);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTEY,AttractorData.state.e3d_state.cylindertangent_end.y);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTEZ,AttractorData.state.e3d_state.cylindertangent_end.z);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTMin,AttractorData.state.e3d_state.cylindertangent_min);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTMax,AttractorData.state.e3d_state.cylindertangent_max);
  SET_TEXTBOX_FLOAT(AttractorData.iawscomponent_ATCYTWeight,AttractorData.state.e3d_state.cylindertangent_weight);


}






void awsSink::SetVFS(csRef<iVFS> newvfs)
{
  vfs=newvfs;
}

csRef<iVFS> awsSink::GetVFS()
{
  return vfs;
}








