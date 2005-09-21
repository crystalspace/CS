/*
    Copyright (C) 2004 by Andrew Mann

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "csutil/csstring.h"
#include "csutil/scfstr.h"
#include "csutil/sysfunc.h"
#include "iaws/awsparm.h"
#include "iengine/engine.h"
#include "iutil/vfs.h"
#include "awssink.h"

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
void PartEditSink::function(intptr_t sk, iAwsSource *source) \
{ \
  asink->componentvar=source->GetComponent(); \
} 


// This macro implements a static callback function which pulls the value out of a textbox control
// converts it to a float, stores the float in a given variable beneath the global pointer 'asink'
// and sets a boolean variable beneath 'asink' to true to signal that a data update has occurred
#define IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(function,floatvar,invalidate_flag,update) \
void PartEditSink::function(intptr_t sk, iAwsSource *source) \
{ \
  iString *textvalue; \
  if (source->GetComponent()->GetProperty("Text",(intptr_t*)&textvalue) && textvalue->Length()) \
  { \
    asink->floatvar=atof(textvalue->GetData()); \
    asink->invalidate_flag=true; \
  } \
  else \
    asink->update(); \
  FreeScrollSetComponent(true,(intptr_t)&(asink->floatvar),source->GetComponent(),&(asink->invalidate_flag)); \
}


// This macro implements a static callback function which pulls the value out of a textbox control
// converts it to a integer, stores the integer in a given variable beneath the global pointer 'asink'
// and sets a boolean variable beneath 'asink' to true to signal that a data update has occurred
#define IMPLEMENT_COMPONENT_TEXTBOX_TO_INT(function,intvar,invalidate_flag,update) \
void PartEditSink::function(intptr_t sk, iAwsSource *source) \
{ \
  iString *textvalue; \
  if (source->GetComponent()->GetProperty("Text",(intptr_t*)&textvalue) && textvalue->Length()) \
  { \
    asink->intvar=strtol(textvalue->GetData(),0,10); \
    asink->invalidate_flag=true; \
  } \
  else \
    asink->update(); \
  FreeScrollSetComponent(false,(intptr_t)&(asink->intvar),source->GetComponent(),&(asink->invalidate_flag)); \
}


// This macro implements a static callback function which checks the state of a checkbox or radio button
// and stores the result (on/off) in a given boolean variable beneath the global pointer 'asink'
// It also sets a different boolean variable beneath 'asink' to true to signal that a data update has occurred.
#define IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(function,boolvar,invalidate_flag,update) \
void PartEditSink::function(intptr_t sk, iAwsSource *source) \
{ \
  bool *p_bvalue; \
  if (source->GetComponent()->GetProperty("State",(intptr_t*)&p_bvalue)) \
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
    component->SetProperty("Text",(intptr_t)(iString*)value);

// This macro can be used to set the contents of a textbox given the iAwsComponent * of the text box
// and a float.  value must be defined as a csRef<iString> prior to this macro being used in a
// function.
#define SET_TEXTBOX_FLOAT(component,floatval) \
    value = csPtr<iString> (new scfString()); \
    value->Format("%f",floatval); \
    component->SetProperty("Text",(intptr_t)(iString*)value);

// This macro can be used to set the contents of a checkbox or radio button given the iAwsComponent * 
// of the checkbox or radio button and a statement that can evaluate to true/false.  
// boolval must be defined as a bool prior to this macro being used in a function.
#define SET_CHECKBOX_BOOL(component,boolval) \
  if (boolval) \
    bvalue=true; \
  else \
    bvalue=false; \
  component->SetProperty("State",(intptr_t)&bvalue);


PartEditSink * PartEditSink::asink = 0;

PartEditSink::PartEditSink() : wmgr(0) 
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

PartEditSink::~PartEditSink()
{
}

void PartEditSink::SetSink(iAwsSink *s)
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

     // Initial Speed Options Callbacks
    sink->RegisterTrigger("RegisterInitialSpeed",&RegisterInitialSpeed);
    sink->RegisterTrigger("RegisterISFPX",&RegisterISFPX);
    sink->RegisterTrigger("RegisterISFPY",&RegisterISFPY);
    sink->RegisterTrigger("RegisterISFPZ",&RegisterISFPZ);
    sink->RegisterTrigger("RegisterISFWeight",&RegisterISFWeight);
    sink->RegisterTrigger("RegisterISLSX",&RegisterISLSX);
    sink->RegisterTrigger("RegisterISLSY",&RegisterISLSY);
    sink->RegisterTrigger("RegisterISLSZ",&RegisterISLSZ);
    sink->RegisterTrigger("RegisterISLEX",&RegisterISLEX);
    sink->RegisterTrigger("RegisterISLEY",&RegisterISLEY);
    sink->RegisterTrigger("RegisterISLEZ",&RegisterISLEZ);
    sink->RegisterTrigger("RegisterISLWeight",&RegisterISLWeight);
    sink->RegisterTrigger("RegisterISBMX",&RegisterISBMX);
    sink->RegisterTrigger("RegisterISBMY",&RegisterISBMY);
    sink->RegisterTrigger("RegisterISBMZ",&RegisterISBMZ);
    sink->RegisterTrigger("RegisterISBXX",&RegisterISBXX);
    sink->RegisterTrigger("RegisterISBXY",&RegisterISBXY);
    sink->RegisterTrigger("RegisterISBXZ",&RegisterISBXZ);
    sink->RegisterTrigger("RegisterISBWeight",&RegisterISBWeight);
    sink->RegisterTrigger("RegisterISSCX",&RegisterISSCX);
    sink->RegisterTrigger("RegisterISSCY",&RegisterISSCY);
    sink->RegisterTrigger("RegisterISSCZ",&RegisterISSCZ);
    sink->RegisterTrigger("RegisterISSMin",&RegisterISSMin);
    sink->RegisterTrigger("RegisterISSMax",&RegisterISSMax);
    sink->RegisterTrigger("RegisterISSWeight",&RegisterISSWeight);
    sink->RegisterTrigger("RegISCNOX",&RegisterISCNOX);
    sink->RegisterTrigger("RegisterISCNOY",&RegisterISCNOY);
    sink->RegisterTrigger("RegisterISCNOZ",&RegisterISCNOZ);
    sink->RegisterTrigger("RegisterISCNElev",&RegisterISCNElev);
    sink->RegisterTrigger("RegisterISCNAzim",&RegisterISCNAzim);
    sink->RegisterTrigger("RegisterISCNAper",&RegisterISCNAper);
    sink->RegisterTrigger("RegisterISCNMin",&RegisterISCNMin);
    sink->RegisterTrigger("RegisterISCNMax",&RegisterISCNMax);
    sink->RegisterTrigger("RegisterISCNWeight",&RegisterISCNWeight);
    sink->RegisterTrigger("RegisterISCYSX",&RegisterISCYSX);
    sink->RegisterTrigger("RegisterISCYSY",&RegisterISCYSY);
    sink->RegisterTrigger("RegisterISCYSZ",&RegisterISCYSZ);
    sink->RegisterTrigger("RegisterISCYEX",&RegisterISCYEX);
    sink->RegisterTrigger("RegisterISCYEY",&RegisterISCYEY);
    sink->RegisterTrigger("RegisterISCYEZ",&RegisterISCYEZ);
    sink->RegisterTrigger("RegisterISCYMin",&RegisterISCYMin);
    sink->RegisterTrigger("RegisterISCYMax",&RegisterISCYMax);
    sink->RegisterTrigger("RegisterISCYWeight",&RegisterISCYWeight);
    sink->RegisterTrigger("RegisterISSTCX",&RegisterISSTCX);
    sink->RegisterTrigger("RegisterISSTCY",&RegisterISSTCY);
    sink->RegisterTrigger("RegisterISSTCZ",&RegisterISSTCZ);
    sink->RegisterTrigger("RegisterISSTMin",&RegisterISSTMin);
    sink->RegisterTrigger("RegisterISSTMax",&RegisterISSTMax);
    sink->RegisterTrigger("RegisterISSTWeight",&RegisterISSTWeight);
    sink->RegisterTrigger("RegisterISCYTSX",&RegisterISCYTSX);
    sink->RegisterTrigger("RegisterISCYTSY",&RegisterISCYTSY);
    sink->RegisterTrigger("RegisterISCYTSZ",&RegisterISCYTSZ);
    sink->RegisterTrigger("RegisterISCYTEX",&RegisterISCYTEX);
    sink->RegisterTrigger("RegisterISCYTEY",&RegisterISCYTEY);
    sink->RegisterTrigger("RegisterISCYTEZ",&RegisterISCYTEZ);
    sink->RegisterTrigger("RegisterISCYTMin",&RegisterISCYTMin);
    sink->RegisterTrigger("RegisterISCYTMax",&RegisterISCYTMax);
    sink->RegisterTrigger("RegisterISCYTWeight",&RegisterISCYTWeight);

    sink->RegisterTrigger("SetISFPositionX",&AwsSetISFPositionX);
    sink->RegisterTrigger("SetISFPositionY",&AwsSetISFPositionY);
    sink->RegisterTrigger("SetISFPositionZ",&AwsSetISFPositionZ);
    sink->RegisterTrigger("SetISFWeight",&AwsSetISFWeight);
    sink->RegisterTrigger("SetISLStartX",&AwsSetISLStartX);
    sink->RegisterTrigger("SetISLStartY",&AwsSetISLStartY);
    sink->RegisterTrigger("SetISLStartZ",&AwsSetISLStartZ);
    sink->RegisterTrigger("SetISLEndX",&AwsSetISLEndX);
    sink->RegisterTrigger("SetISLEndY",&AwsSetISLEndY);
    sink->RegisterTrigger("SetISLEndZ",&AwsSetISLEndZ);
    sink->RegisterTrigger("SetISLWeight",&AwsSetISLWeight);
    sink->RegisterTrigger("SetISBMinX",&AwsSetISBMinX);
    sink->RegisterTrigger("SetISBMinY",&AwsSetISBMinY);
    sink->RegisterTrigger("SetISBMinZ",&AwsSetISBMinZ);
    sink->RegisterTrigger("SetISBMaxX",&AwsSetISBMaxX);
    sink->RegisterTrigger("SetISBMaxY",&AwsSetISBMaxY);
    sink->RegisterTrigger("SetISBMaxZ",&AwsSetISBMaxZ);
    sink->RegisterTrigger("SetISBWeight",&AwsSetISBWeight);
    sink->RegisterTrigger("SetISSCenterX",&AwsSetISSCenterX);
    sink->RegisterTrigger("SetISSCenterY",&AwsSetISSCenterY);
    sink->RegisterTrigger("SetISSCenterZ",&AwsSetISSCenterZ);
    sink->RegisterTrigger("SetISSMin",&AwsSetISSMin);
    sink->RegisterTrigger("SetISSMax",&AwsSetISSMax);
    sink->RegisterTrigger("SetISSWeight",&AwsSetISSWeight);
    sink->RegisterTrigger("SetISCNOriginX",&AwsSetISCNOriginX);
    sink->RegisterTrigger("SetISCNOriginY",&AwsSetISCNOriginY);
    sink->RegisterTrigger("SetISCNOriginZ",&AwsSetISCNOriginZ);
    sink->RegisterTrigger("SetISCNElev",&AwsSetISCNElev);
    sink->RegisterTrigger("SetISCNAzim",&AwsSetISCNAzim);
    sink->RegisterTrigger("SetISCNAper",&AwsSetISCNAper);
    sink->RegisterTrigger("SetISCNMin",&AwsSetISCNMin);
    sink->RegisterTrigger("SetISCNMax",&AwsSetISCNMax);
    sink->RegisterTrigger("SetISCNWeight",&AwsSetISCNWeight);
    sink->RegisterTrigger("SetISCYStartX",&AwsSetISCYStartX);
    sink->RegisterTrigger("SetISCYStartY",&AwsSetISCYStartY);
    sink->RegisterTrigger("SetISCYStartZ",&AwsSetISCYStartZ);
    sink->RegisterTrigger("SetISCYEndX",&AwsSetISCYEndX);
    sink->RegisterTrigger("SetISCYEndY",&AwsSetISCYEndY);
    sink->RegisterTrigger("SetISCYEndZ",&AwsSetISCYEndZ);
    sink->RegisterTrigger("SetISCYMin",&AwsSetISCYMin);
    sink->RegisterTrigger("SetISCYMax",&AwsSetISCYMax);
    sink->RegisterTrigger("SetISCYWeight",&AwsSetISCYWeight);
    sink->RegisterTrigger("SetISSTCenterX",&AwsSetISSTCenterX);
    sink->RegisterTrigger("SetISSTCenterY",&AwsSetISSTCenterY);
    sink->RegisterTrigger("SetISSTCenterZ",&AwsSetISSTCenterZ);
    sink->RegisterTrigger("SetISSTMin",&AwsSetISSTMin);
    sink->RegisterTrigger("SetISSTMax",&AwsSetISSTMax);
    sink->RegisterTrigger("SetISSTWeight",&AwsSetISSTWeight);
    sink->RegisterTrigger("SetISCYTStartX",&AwsSetISCYTStartX);
    sink->RegisterTrigger("SetISCYTStartY",&AwsSetISCYTStartY);
    sink->RegisterTrigger("SetISCYTStartZ",&AwsSetISCYTStartZ);
    sink->RegisterTrigger("SetISCYTEndX",&AwsSetISCYTEndX);
    sink->RegisterTrigger("SetISCYTEndY",&AwsSetISCYTEndY);
    sink->RegisterTrigger("SetISCYTEndZ",&AwsSetISCYTEndZ);
    sink->RegisterTrigger("SetISCYTMin",&AwsSetISCYTMin);
    sink->RegisterTrigger("SetISCYTMax",&AwsSetISCYTMax);
    sink->RegisterTrigger("SetISCYTWeight",&AwsSetISCYTWeight);

    // Initial Acceleration Options Callbacks
    sink->RegisterTrigger("RegisterInitialAcceleration",&RegisterInitialAcceleration);
    sink->RegisterTrigger("RegisterIAFPX",&RegisterIAFPX);
    sink->RegisterTrigger("RegisterIAFPY",&RegisterIAFPY);
    sink->RegisterTrigger("RegisterIAFPZ",&RegisterIAFPZ);
    sink->RegisterTrigger("RegisterIAFWeight",&RegisterIAFWeight);
    sink->RegisterTrigger("RegisterIALSX",&RegisterIALSX);
    sink->RegisterTrigger("RegisterIALSY",&RegisterIALSY);
    sink->RegisterTrigger("RegisterIALSZ",&RegisterIALSZ);
    sink->RegisterTrigger("RegisterIALEX",&RegisterIALEX);
    sink->RegisterTrigger("RegisterIALEY",&RegisterIALEY);
    sink->RegisterTrigger("RegisterIALEZ",&RegisterIALEZ);
    sink->RegisterTrigger("RegisterIALWeight",&RegisterIALWeight);
    sink->RegisterTrigger("RegisterIABMX",&RegisterIABMX);
    sink->RegisterTrigger("RegisterIABMY",&RegisterIABMY);
    sink->RegisterTrigger("RegisterIABMZ",&RegisterIABMZ);
    sink->RegisterTrigger("RegisterIABXX",&RegisterIABXX);
    sink->RegisterTrigger("RegisterIABXY",&RegisterIABXY);
    sink->RegisterTrigger("RegisterIABXZ",&RegisterIABXZ);
    sink->RegisterTrigger("RegisterIABWeight",&RegisterIABWeight);
    sink->RegisterTrigger("RegisterIASCX",&RegisterIASCX);
    sink->RegisterTrigger("RegisterIASCY",&RegisterIASCY);
    sink->RegisterTrigger("RegisterIASCZ",&RegisterIASCZ);
    sink->RegisterTrigger("RegisterIASMin",&RegisterIASMin);
    sink->RegisterTrigger("RegisterIASMax",&RegisterIASMax);
    sink->RegisterTrigger("RegisterIASWeight",&RegisterIASWeight);
    sink->RegisterTrigger("RegisterIACNOX",&RegisterIACNOX);
    sink->RegisterTrigger("RegisterIACNOY",&RegisterIACNOY);
    sink->RegisterTrigger("RegisterIACNOZ",&RegisterIACNOZ);
    sink->RegisterTrigger("RegisterIACNElev",&RegisterIACNElev);
    sink->RegisterTrigger("RegisterIACNAzim",&RegisterIACNAzim);
    sink->RegisterTrigger("RegisterIACNAper",&RegisterIACNAper);
    sink->RegisterTrigger("RegisterIACNMin",&RegisterIACNMin);
    sink->RegisterTrigger("RegisterIACNMax",&RegisterIACNMax);
    sink->RegisterTrigger("RegisterIACNWeight",&RegisterIACNWeight);
    sink->RegisterTrigger("RegisterIACYSX",&RegisterIACYSX);
    sink->RegisterTrigger("RegisterIACYSY",&RegisterIACYSY);
    sink->RegisterTrigger("RegisterIACYSZ",&RegisterIACYSZ);
    sink->RegisterTrigger("RegisterIACYEX",&RegisterIACYEX);
    sink->RegisterTrigger("RegisterIACYEY",&RegisterIACYEY);
    sink->RegisterTrigger("RegisterIACYEZ",&RegisterIACYEZ);
    sink->RegisterTrigger("RegisterIACYMin",&RegisterIACYMin);
    sink->RegisterTrigger("RegisterIACYMax",&RegisterIACYMax);
    sink->RegisterTrigger("RegisterIACYWeight",&RegisterIACYWeight);
    sink->RegisterTrigger("RegisterIASTCX",&RegisterIASTCX);
    sink->RegisterTrigger("RegisterIASTCY",&RegisterIASTCY);
    sink->RegisterTrigger("RegisterIASTCZ",&RegisterIASTCZ);
    sink->RegisterTrigger("RegisterIASTMin",&RegisterIASTMin);
    sink->RegisterTrigger("RegisterIASTMax",&RegisterIASTMax);
    sink->RegisterTrigger("RegisterIASTWeight",&RegisterIASTWeight);
    sink->RegisterTrigger("RegisterIACYTSX",&RegisterIACYTSX);
    sink->RegisterTrigger("RegisterIACYTSY",&RegisterIACYTSY);
    sink->RegisterTrigger("RegisterIACYTSZ",&RegisterIACYTSZ);
    sink->RegisterTrigger("RegisterIACYTEX",&RegisterIACYTEX);
    sink->RegisterTrigger("RegisterIACYTEY",&RegisterIACYTEY);
    sink->RegisterTrigger("RegisterIACYTEZ",&RegisterIACYTEZ);
    sink->RegisterTrigger("RegisterIACYTMin",&RegisterIACYTMin);
    sink->RegisterTrigger("RegisterIACYTMax",&RegisterIACYTMax);
    sink->RegisterTrigger("RegisterIACYTWeight",&RegisterIACYTWeight);

    sink->RegisterTrigger("SetIAFPositionX",&AwsSetIAFPositionX);
    sink->RegisterTrigger("SetIAFPositionY",&AwsSetIAFPositionY);
    sink->RegisterTrigger("SetIAFPositionZ",&AwsSetIAFPositionZ);
    sink->RegisterTrigger("SetIAFWeight",&AwsSetIAFWeight);
    sink->RegisterTrigger("SetIALStartX",&AwsSetIALStartX);
    sink->RegisterTrigger("SetIALStartY",&AwsSetIALStartY);
    sink->RegisterTrigger("SetIALStartZ",&AwsSetIALStartZ);
    sink->RegisterTrigger("SetIALEndX",&AwsSetIALEndX);
    sink->RegisterTrigger("SetIALEndY",&AwsSetIALEndY);
    sink->RegisterTrigger("SetIALEndZ",&AwsSetIALEndZ);
    sink->RegisterTrigger("SetIALWeight",&AwsSetIALWeight);
    sink->RegisterTrigger("SetIABMinX",&AwsSetIABMinX);
    sink->RegisterTrigger("SetIABMinY",&AwsSetIABMinY);
    sink->RegisterTrigger("SetIABMinZ",&AwsSetIABMinZ);
    sink->RegisterTrigger("SetIABMaxX",&AwsSetIABMaxX);
    sink->RegisterTrigger("SetIABMaxY",&AwsSetIABMaxY);
    sink->RegisterTrigger("SetIABMaxZ",&AwsSetIABMaxZ);
    sink->RegisterTrigger("SetIABWeight",&AwsSetIABWeight);
    sink->RegisterTrigger("SetIASCenterX",&AwsSetIASCenterX);
    sink->RegisterTrigger("SetIASCenterY",&AwsSetIASCenterY);
    sink->RegisterTrigger("SetIASCenterZ",&AwsSetIASCenterZ);
    sink->RegisterTrigger("SetIASMin",&AwsSetIASMin);
    sink->RegisterTrigger("SetIASMax",&AwsSetIASMax);
    sink->RegisterTrigger("SetIASWeight",&AwsSetIASWeight);
    sink->RegisterTrigger("SetIACNOriginX",&AwsSetIACNOriginX);
    sink->RegisterTrigger("SetIACNOriginY",&AwsSetIACNOriginY);
    sink->RegisterTrigger("SetIACNOriginZ",&AwsSetIACNOriginZ);
    sink->RegisterTrigger("SetIACNElev",&AwsSetIACNElev);
    sink->RegisterTrigger("SetIACNAzim",&AwsSetIACNAzim);
    sink->RegisterTrigger("SetIACNAper",&AwsSetIACNAper);
    sink->RegisterTrigger("SetIACNMin",&AwsSetIACNMin);
    sink->RegisterTrigger("SetIACNMax",&AwsSetIACNMax);
    sink->RegisterTrigger("SetIACNWeight",&AwsSetIACNWeight);
    sink->RegisterTrigger("SetIACYStartX",&AwsSetIACYStartX);
    sink->RegisterTrigger("SetIACYStartY",&AwsSetIACYStartY);
    sink->RegisterTrigger("SetIACYStartZ",&AwsSetIACYStartZ);
    sink->RegisterTrigger("SetIACYEndX",&AwsSetIACYEndX);
    sink->RegisterTrigger("SetIACYEndY",&AwsSetIACYEndY);
    sink->RegisterTrigger("SetIACYEndZ",&AwsSetIACYEndZ);
    sink->RegisterTrigger("SetIACYMin",&AwsSetIACYMin);
    sink->RegisterTrigger("SetIACYMax",&AwsSetIACYMax);
    sink->RegisterTrigger("SetIACYWeight",&AwsSetIACYWeight);
    sink->RegisterTrigger("SetIASTCenterX",&AwsSetIASTCenterX);
    sink->RegisterTrigger("SetIASTCenterY",&AwsSetIASTCenterY);
    sink->RegisterTrigger("SetIASTCenterZ",&AwsSetIASTCenterZ);
    sink->RegisterTrigger("SetIASTMin",&AwsSetIASTMin);
    sink->RegisterTrigger("SetIASTMax",&AwsSetIASTMax);
    sink->RegisterTrigger("SetIASTWeight",&AwsSetIASTWeight);
    sink->RegisterTrigger("SetIACYTStartX",&AwsSetIACYTStartX);
    sink->RegisterTrigger("SetIACYTStartY",&AwsSetIACYTStartY);
    sink->RegisterTrigger("SetIACYTStartZ",&AwsSetIACYTStartZ);
    sink->RegisterTrigger("SetIACYTEndX",&AwsSetIACYTEndX);
    sink->RegisterTrigger("SetIACYTEndY",&AwsSetIACYTEndY);
    sink->RegisterTrigger("SetIACYTEndZ",&AwsSetIACYTEndZ);
    sink->RegisterTrigger("SetIACYTMin",&AwsSetIACYTMin);
    sink->RegisterTrigger("SetIACYTMax",&AwsSetIACYTMax);
    sink->RegisterTrigger("SetIACYTWeight",&AwsSetIACYTWeight);

    // Field Speed Options Callbacks
    sink->RegisterTrigger("RegisterFieldSpeed",&RegisterFieldSpeed);
    sink->RegisterTrigger("RegisterFSActive",&RegisterFSActive);
    sink->RegisterTrigger("RegisterFSFPX",&RegisterFSFPX);
    sink->RegisterTrigger("RegisterFSFPY",&RegisterFSFPY);
    sink->RegisterTrigger("RegisterFSFPZ",&RegisterFSFPZ);
    sink->RegisterTrigger("RegisterFSFWeight",&RegisterFSFWeight);
    sink->RegisterTrigger("RegisterFSLSX",&RegisterFSLSX);
    sink->RegisterTrigger("RegisterFSLSY",&RegisterFSLSY);
    sink->RegisterTrigger("RegisterFSLSZ",&RegisterFSLSZ);
    sink->RegisterTrigger("RegisterFSLEX",&RegisterFSLEX);
    sink->RegisterTrigger("RegFSLEY",&RegisterFSLEY);
    sink->RegisterTrigger("RegFSLEZ",&RegisterFSLEZ);
    sink->RegisterTrigger("RegisterFSLWeight",&RegisterFSLWeight);
    sink->RegisterTrigger("RegisterFSBMX",&RegisterFSBMX);
    sink->RegisterTrigger("RegisterFSBMY",&RegisterFSBMY);
    sink->RegisterTrigger("RegisterFSBMZ",&RegisterFSBMZ);
    sink->RegisterTrigger("RegisterFSBXX",&RegisterFSBXX);
    sink->RegisterTrigger("RegisterFSBXY",&RegisterFSBXY);
    sink->RegisterTrigger("RegisterFSBXZ",&RegisterFSBXZ);
    sink->RegisterTrigger("RegisterFSBWeight",&RegisterFSBWeight);
    sink->RegisterTrigger("RegFSSCX",&RegisterFSSCX);
    sink->RegisterTrigger("RegisterFSSCY",&RegisterFSSCY);
    sink->RegisterTrigger("RegisterFSSCZ",&RegisterFSSCZ);
    sink->RegisterTrigger("RegisterFSSMin",&RegisterFSSMin);
    sink->RegisterTrigger("RegisterFSSMax",&RegisterFSSMax);
    sink->RegisterTrigger("RegisterFSSWeight",&RegisterFSSWeight);
    sink->RegisterTrigger("RegisterFSCNOX",&RegisterFSCNOX);
    sink->RegisterTrigger("RegisterFSCNOY",&RegisterFSCNOY);
    sink->RegisterTrigger("RegisterFSCNOZ",&RegisterFSCNOZ);
    sink->RegisterTrigger("RegisterFSCNElev",&RegisterFSCNElev);
    sink->RegisterTrigger("RegisterFSCNAzim",&RegisterFSCNAzim);
    sink->RegisterTrigger("RegisterFSCNAper",&RegisterFSCNAper);
    sink->RegisterTrigger("RegisterFSCNMin",&RegisterFSCNMin);
    sink->RegisterTrigger("RegisterFSCNMax",&RegisterFSCNMax);
    sink->RegisterTrigger("RegisterFSCNWeight",&RegisterFSCNWeight);
    sink->RegisterTrigger("RegisterFSCYSX",&RegisterFSCYSX);
    sink->RegisterTrigger("RegisterFSCYSY",&RegisterFSCYSY);
    sink->RegisterTrigger("RegisterFSCYSZ",&RegisterFSCYSZ);
    sink->RegisterTrigger("RegisterFSCYEX",&RegisterFSCYEX);
    sink->RegisterTrigger("RegisterFSCYEY",&RegisterFSCYEY);
    sink->RegisterTrigger("RegisterFSCYEZ",&RegisterFSCYEZ);
    sink->RegisterTrigger("RegisterFSCYMin",&RegisterFSCYMin);
    sink->RegisterTrigger("RegisterFSCYMax",&RegisterFSCYMax);
    sink->RegisterTrigger("RegisterFSCYWeight",&RegisterFSCYWeight);
    sink->RegisterTrigger("RegisterFSSTCX",&RegisterFSSTCX);
    sink->RegisterTrigger("RegisterFSSTCY",&RegisterFSSTCY);
    sink->RegisterTrigger("RegisterFSSTCZ",&RegisterFSSTCZ);
    sink->RegisterTrigger("RegisterFSSTMin",&RegisterFSSTMin);
    sink->RegisterTrigger("RegisterFSSTMax",&RegisterFSSTMax);
    sink->RegisterTrigger("RegisterFSSTWeight",&RegisterFSSTWeight);
    sink->RegisterTrigger("RegisterFSCYTSX",&RegisterFSCYTSX);
    sink->RegisterTrigger("RegisterFSCYTSY",&RegisterFSCYTSY);
    sink->RegisterTrigger("RegisterFSCYTSZ",&RegisterFSCYTSZ);
    sink->RegisterTrigger("RegisterFSCYTEX",&RegisterFSCYTEX);
    sink->RegisterTrigger("RegisterFSCYTEY",&RegisterFSCYTEY);
    sink->RegisterTrigger("RegisterFSCYTEZ",&RegisterFSCYTEZ);
    sink->RegisterTrigger("RegisterFSCYTMin",&RegisterFSCYTMin);
    sink->RegisterTrigger("RegisterFSCYTMax",&RegisterFSCYTMax);
    sink->RegisterTrigger("RegisterFSCYTWeight",&RegisterFSCYTWeight);

    sink->RegisterTrigger("SetFSActive",&AwsSetFSActive);
    sink->RegisterTrigger("SetFSFPositionX",&AwsSetFSFPositionX);
    sink->RegisterTrigger("SetFSFPositionY",&AwsSetFSFPositionY);
    sink->RegisterTrigger("SetFSFPositionZ",&AwsSetFSFPositionZ);
    sink->RegisterTrigger("SetFSFWeight",&AwsSetFSFWeight);
    sink->RegisterTrigger("SetFSLStartX",&AwsSetFSLStartX);
    sink->RegisterTrigger("SetFSLStartY",&AwsSetFSLStartY);
    sink->RegisterTrigger("SetFSLStartZ",&AwsSetFSLStartZ);
    sink->RegisterTrigger("SetFSLEndX",&AwsSetFSLEndX);
    sink->RegisterTrigger("SetFSLEndY",&AwsSetFSLEndY);
    sink->RegisterTrigger("SetFSLEndZ",&AwsSetFSLEndZ);
    sink->RegisterTrigger("SetFSLWeight",&AwsSetFSLWeight);
    sink->RegisterTrigger("SetFSBMinX",&AwsSetFSBMinX);
    sink->RegisterTrigger("SetFSBMinY",&AwsSetFSBMinY);
    sink->RegisterTrigger("SetFSBMinZ",&AwsSetFSBMinZ);
    sink->RegisterTrigger("SetFSBMaxX",&AwsSetFSBMaxX);
    sink->RegisterTrigger("SetFSBMaxY",&AwsSetFSBMaxY);
    sink->RegisterTrigger("SetFSBMaxZ",&AwsSetFSBMaxZ);
    sink->RegisterTrigger("SetFSBWeight",&AwsSetFSBWeight);
    sink->RegisterTrigger("SetFSSCenterX",&AwsSetFSSCenterX);
    sink->RegisterTrigger("SetFSSCenterY",&AwsSetFSSCenterY);
    sink->RegisterTrigger("SetFSSCenterZ",&AwsSetFSSCenterZ);
    sink->RegisterTrigger("SetFSSMin",&AwsSetFSSMin);
    sink->RegisterTrigger("SetFSSMax",&AwsSetFSSMax);
    sink->RegisterTrigger("SetFSSWeight",&AwsSetFSSWeight);
    sink->RegisterTrigger("SetFSCNOriginX",&AwsSetFSCNOriginX);
    sink->RegisterTrigger("SetFSCNOriginY",&AwsSetFSCNOriginY);
    sink->RegisterTrigger("SetFSCNOriginZ",&AwsSetFSCNOriginZ);
    sink->RegisterTrigger("SetFSCNElev",&AwsSetFSCNElev);
    sink->RegisterTrigger("SetFSCNAzim",&AwsSetFSCNAzim);
    sink->RegisterTrigger("SetFSCNAper",&AwsSetFSCNAper);
    sink->RegisterTrigger("SetFSCNMin",&AwsSetFSCNMin);
    sink->RegisterTrigger("SetFSCNMax",&AwsSetFSCNMax);
    sink->RegisterTrigger("SetFSCNWeight",&AwsSetFSCNWeight);
    sink->RegisterTrigger("SetFSCYStartX",&AwsSetFSCYStartX);
    sink->RegisterTrigger("SetFSCYStartY",&AwsSetFSCYStartY);
    sink->RegisterTrigger("SetFSCYStartZ",&AwsSetFSCYStartZ);
    sink->RegisterTrigger("SetFSCYEndX",&AwsSetFSCYEndX);
    sink->RegisterTrigger("SetFSCYEndY",&AwsSetFSCYEndY);
    sink->RegisterTrigger("SetFSCYEndZ",&AwsSetFSCYEndZ);
    sink->RegisterTrigger("SetFSCYMin",&AwsSetFSCYMin);
    sink->RegisterTrigger("SetFSCYMax",&AwsSetFSCYMax);
    sink->RegisterTrigger("SetFSCYWeight",&AwsSetFSCYWeight);
    sink->RegisterTrigger("SetFSSTCenterX",&AwsSetFSSTCenterX);
    sink->RegisterTrigger("SetFSSTCenterY",&AwsSetFSSTCenterY);
    sink->RegisterTrigger("SetFSSTCenterZ",&AwsSetFSSTCenterZ);
    sink->RegisterTrigger("SetFSSTMin",&AwsSetFSSTMin);
    sink->RegisterTrigger("SetFSSTMax",&AwsSetFSSTMax);
    sink->RegisterTrigger("SetFSSTWeight",&AwsSetFSSTWeight);
    sink->RegisterTrigger("SetFSCYTStartX",&AwsSetFSCYTStartX);
    sink->RegisterTrigger("SetFSCYTStartY",&AwsSetFSCYTStartY);
    sink->RegisterTrigger("SetFSCYTStartZ",&AwsSetFSCYTStartZ);
    sink->RegisterTrigger("SetFSCYTEndX",&AwsSetFSCYTEndX);
    sink->RegisterTrigger("SetFSCYTEndY",&AwsSetFSCYTEndY);
    sink->RegisterTrigger("SetFSCYTEndZ",&AwsSetFSCYTEndZ);
    sink->RegisterTrigger("SetFSCYTMin",&AwsSetFSCYTMin);
    sink->RegisterTrigger("SetFSCYTMax",&AwsSetFSCYTMax);
    sink->RegisterTrigger("SetFSCYTWeight",&AwsSetFSCYTWeight);

    // Field Acceleration Options Callbacks
    sink->RegisterTrigger("RegisterFieldAccel",&RegisterFieldAccel);
    sink->RegisterTrigger("RegisterFAActive",&RegisterFAActive);
    sink->RegisterTrigger("RegisterFAFPX",&RegisterFAFPX);
    sink->RegisterTrigger("RegisterFAFPY",&RegisterFAFPY);
    sink->RegisterTrigger("RegisterFAFPZ",&RegisterFAFPZ);
    sink->RegisterTrigger("RegisterFAFWeight",&RegisterFAFWeight);
    sink->RegisterTrigger("RegisterFALSX",&RegisterFALSX);
    sink->RegisterTrigger("RegisterFALSY",&RegisterFALSY);
    sink->RegisterTrigger("RegisterFALSZ",&RegisterFALSZ);
    sink->RegisterTrigger("RegisterFALEX",&RegisterFALEX);
    sink->RegisterTrigger("RegFALEY",&RegisterFALEY);
    sink->RegisterTrigger("RegFALEZ",&RegisterFALEZ);
    sink->RegisterTrigger("RegisterFALWeight",&RegisterFALWeight);
    sink->RegisterTrigger("RegisterFABMX",&RegisterFABMX);
    sink->RegisterTrigger("RegisterFABMY",&RegisterFABMY);
    sink->RegisterTrigger("RegisterFABMZ",&RegisterFABMZ);
    sink->RegisterTrigger("RegisterFABXX",&RegisterFABXX);
    sink->RegisterTrigger("RegisterFABXY",&RegisterFABXY);
    sink->RegisterTrigger("RegisterFABXZ",&RegisterFABXZ);
    sink->RegisterTrigger("RegisterFABWeight",&RegisterFABWeight);
    sink->RegisterTrigger("RegFASCX",&RegisterFASCX);
    sink->RegisterTrigger("RegisterFASCY",&RegisterFASCY);
    sink->RegisterTrigger("RegisterFASCZ",&RegisterFASCZ);
    sink->RegisterTrigger("RegisterFASMin",&RegisterFASMin);
    sink->RegisterTrigger("RegisterFASMax",&RegisterFASMax);
    sink->RegisterTrigger("RegisterFASWeight",&RegisterFASWeight);
    sink->RegisterTrigger("RegisterFACNOX",&RegisterFACNOX);
    sink->RegisterTrigger("RegisterFACNOY",&RegisterFACNOY);
    sink->RegisterTrigger("RegisterFACNOZ",&RegisterFACNOZ);
    sink->RegisterTrigger("RegisterFACNElev",&RegisterFACNElev);
    sink->RegisterTrigger("RegisterFACNAzim",&RegisterFACNAzim);
    sink->RegisterTrigger("RegisterFACNAper",&RegisterFACNAper);
    sink->RegisterTrigger("RegisterFACNMin",&RegisterFACNMin);
    sink->RegisterTrigger("RegisterFACNMax",&RegisterFACNMax);
    sink->RegisterTrigger("RegisterFACNWeight",&RegisterFACNWeight);
    sink->RegisterTrigger("RegisterFACYSX",&RegisterFACYSX);
    sink->RegisterTrigger("RegisterFACYSY",&RegisterFACYSY);
    sink->RegisterTrigger("RegisterFACYSZ",&RegisterFACYSZ);
    sink->RegisterTrigger("RegisterFACYEX",&RegisterFACYEX);
    sink->RegisterTrigger("RegisterFACYEY",&RegisterFACYEY);
    sink->RegisterTrigger("RegisterFACYEZ",&RegisterFACYEZ);
    sink->RegisterTrigger("RegisterFACYMin",&RegisterFACYMin);
    sink->RegisterTrigger("RegisterFACYMax",&RegisterFACYMax);
    sink->RegisterTrigger("RegisterFACYWeight",&RegisterFACYWeight);
    sink->RegisterTrigger("RegisterFASTCX",&RegisterFASTCX);
    sink->RegisterTrigger("RegisterFASTCY",&RegisterFASTCY);
    sink->RegisterTrigger("RegisterFASTCZ",&RegisterFASTCZ);
    sink->RegisterTrigger("RegisterFASTMin",&RegisterFASTMin);
    sink->RegisterTrigger("RegisterFASTMax",&RegisterFASTMax);
    sink->RegisterTrigger("RegisterFASTWeight",&RegisterFASTWeight);
    sink->RegisterTrigger("RegisterFACYTSX",&RegisterFACYTSX);
    sink->RegisterTrigger("RegisterFACYTSY",&RegisterFACYTSY);
    sink->RegisterTrigger("RegisterFACYTSZ",&RegisterFACYTSZ);
    sink->RegisterTrigger("RegisterFACYTEX",&RegisterFACYTEX);
    sink->RegisterTrigger("RegisterFACYTEY",&RegisterFACYTEY);
    sink->RegisterTrigger("RegisterFACYTEZ",&RegisterFACYTEZ);
    sink->RegisterTrigger("RegisterFACYTMin",&RegisterFACYTMin);
    sink->RegisterTrigger("RegisterFACYTMax",&RegisterFACYTMax);
    sink->RegisterTrigger("RegisterFACYTWeight",&RegisterFACYTWeight);

    sink->RegisterTrigger("SetFAActive",&AwsSetFAActive);
    sink->RegisterTrigger("SetFAFPositionX",&AwsSetFAFPositionX);
    sink->RegisterTrigger("SetFAFPositionY",&AwsSetFAFPositionY);
    sink->RegisterTrigger("SetFAFPositionZ",&AwsSetFAFPositionZ);
    sink->RegisterTrigger("SetFAFWeight",&AwsSetFAFWeight);
    sink->RegisterTrigger("SetFALStartX",&AwsSetFALStartX);
    sink->RegisterTrigger("SetFALStartY",&AwsSetFALStartY);
    sink->RegisterTrigger("SetFALStartZ",&AwsSetFALStartZ);
    sink->RegisterTrigger("SetFALEndX",&AwsSetFALEndX);
    sink->RegisterTrigger("SetFALEndY",&AwsSetFALEndY);
    sink->RegisterTrigger("SetFALEndZ",&AwsSetFALEndZ);
    sink->RegisterTrigger("SetFALWeight",&AwsSetFALWeight);
    sink->RegisterTrigger("SetFABMinX",&AwsSetFABMinX);
    sink->RegisterTrigger("SetFABMinY",&AwsSetFABMinY);
    sink->RegisterTrigger("SetFABMinZ",&AwsSetFABMinZ);
    sink->RegisterTrigger("SetFABMaxX",&AwsSetFABMaxX);
    sink->RegisterTrigger("SetFABMaxY",&AwsSetFABMaxY);
    sink->RegisterTrigger("SetFABMaxZ",&AwsSetFABMaxZ);
    sink->RegisterTrigger("SetFABWeight",&AwsSetFABWeight);
    sink->RegisterTrigger("SetFASCenterX",&AwsSetFASCenterX);
    sink->RegisterTrigger("SetFASCenterY",&AwsSetFASCenterY);
    sink->RegisterTrigger("SetFASCenterZ",&AwsSetFASCenterZ);
    sink->RegisterTrigger("SetFASMin",&AwsSetFASMin);
    sink->RegisterTrigger("SetFASMax",&AwsSetFASMax);
    sink->RegisterTrigger("SetFASWeight",&AwsSetFASWeight);
    sink->RegisterTrigger("SetFACNOriginX",&AwsSetFACNOriginX);
    sink->RegisterTrigger("SetFACNOriginY",&AwsSetFACNOriginY);
    sink->RegisterTrigger("SetFACNOriginZ",&AwsSetFACNOriginZ);
    sink->RegisterTrigger("SetFACNElev",&AwsSetFACNElev);
    sink->RegisterTrigger("SetFACNAzim",&AwsSetFACNAzim);
    sink->RegisterTrigger("SetFACNAper",&AwsSetFACNAper);
    sink->RegisterTrigger("SetFACNMin",&AwsSetFACNMin);
    sink->RegisterTrigger("SetFACNMax",&AwsSetFACNMax);
    sink->RegisterTrigger("SetFACNWeight",&AwsSetFACNWeight);
    sink->RegisterTrigger("SetFACYStartX",&AwsSetFACYStartX);
    sink->RegisterTrigger("SetFACYStartY",&AwsSetFACYStartY);
    sink->RegisterTrigger("SetFACYStartZ",&AwsSetFACYStartZ);
    sink->RegisterTrigger("SetFACYEndX",&AwsSetFACYEndX);
    sink->RegisterTrigger("SetFACYEndY",&AwsSetFACYEndY);
    sink->RegisterTrigger("SetFACYEndZ",&AwsSetFACYEndZ);
    sink->RegisterTrigger("SetFACYMin",&AwsSetFACYMin);
    sink->RegisterTrigger("SetFACYMax",&AwsSetFACYMax);
    sink->RegisterTrigger("SetFACYWeight",&AwsSetFACYWeight);
    sink->RegisterTrigger("SetFASTCenterX",&AwsSetFASTCenterX);
    sink->RegisterTrigger("SetFASTCenterY",&AwsSetFASTCenterY);
    sink->RegisterTrigger("SetFASTCenterZ",&AwsSetFASTCenterZ);
    sink->RegisterTrigger("SetFASTMin",&AwsSetFASTMin);
    sink->RegisterTrigger("SetFASTMax",&AwsSetFASTMax);
    sink->RegisterTrigger("SetFASTWeight",&AwsSetFASTWeight);
    sink->RegisterTrigger("SetFACYTStartX",&AwsSetFACYTStartX);
    sink->RegisterTrigger("SetFACYTStartY",&AwsSetFACYTStartY);
    sink->RegisterTrigger("SetFACYTStartZ",&AwsSetFACYTStartZ);
    sink->RegisterTrigger("SetFACYTEndX",&AwsSetFACYTEndX);
    sink->RegisterTrigger("SetFACYTEndY",&AwsSetFACYTEndY);
    sink->RegisterTrigger("SetFACYTEndZ",&AwsSetFACYTEndZ);
    sink->RegisterTrigger("SetFACYTMin",&AwsSetFACYTMin);
    sink->RegisterTrigger("SetFACYTMax",&AwsSetFACYTMax);
    sink->RegisterTrigger("SetFACYTWeight",&AwsSetFACYTWeight);

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
    sink->RegisterTrigger("RegATCYEX",&RegisterATCYEX);
    sink->RegisterTrigger("RegisterATCYEY",&RegisterATCYEY);
    sink->RegisterTrigger("RegisterATCYEZ",&RegisterATCYEZ);
    sink->RegisterTrigger("RegisterATCYMin",&RegisterATCYMin);
    sink->RegisterTrigger("RegisterATCYMax",&RegisterATCYMax);
    sink->RegisterTrigger("RegisterATCYWeight",&RegisterATCYWeight);
    sink->RegisterTrigger("RegATSTCX",&RegisterATSTCX);
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

    sink->RegisterTrigger("RegisterFSWindow",&RegisterFSWindow);
    sink->RegisterTrigger("RegisterFSLabel",&RegisterFSLabel);
    sink->RegisterTrigger("RegisterFSTextBox",&RegisterFSTextBox);
    sink->RegisterTrigger("RegisterFSScrollBar",&RegisterFSScrollBar);
    sink->RegisterTrigger("SetFSScrollBar",&AwsSetFSScrollBar);
    sink->RegisterTrigger("SetFSTextBox",&AwsSetFSTextBox);

    sink->RegisterTrigger("RegisterAgingMoments",&RegisterAgingMoments);
    sink->RegisterTrigger("RegisterLoadSaveSelection",&RegisterLoadSave);
  }
  FreeScrollData.iawscomponent_AssociatedTextBox=0;
}

void PartEditSink::SetWindowManager(iAws *_wmgr)
{
  wmgr=_wmgr;
}



void PartEditSink::RegisterSectionSelection(intptr_t sk, iAwsSource *source)
{
  asink->iawscomponent_SectionSelection = source->GetComponent();
  return;
}


void PartEditSink::FillSectionList(intptr_t sk, iAwsSource *source)
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

void PartEditSink::SectionListSelectionChanged(intptr_t sk, iAwsSource *source)
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
    case SECTION_INIT_SPEED:
      if (asink->SectionState[i])
      {
        asink->InitialSpeedData.iawscomponent_InitialSpeed->Show();
        asink->InitialSpeedData.iawscomponent_InitialSpeed->Raise();
      }
      else
        asink->InitialSpeedData.iawscomponent_InitialSpeed->Hide();
      break;
    case SECTION_INIT_ACCELERATION:
      if (asink->SectionState[i])
      {
        asink->InitialAccelerationData.iawscomponent_InitialAcceleration->Show();
        asink->InitialAccelerationData.iawscomponent_InitialAcceleration->Raise();
      }
      else
        asink->InitialAccelerationData.iawscomponent_InitialAcceleration->Hide();
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
    case SECTION_FIELD_SPEED:
      if (asink->SectionState[i])
      {
        asink->FieldSpeedData.iawscomponent_FieldSpeed->Show();
        asink->FieldSpeedData.iawscomponent_FieldSpeed->Raise();
      }
      else
        asink->FieldSpeedData.iawscomponent_FieldSpeed->Hide();
      break;
    case SECTION_FIELD_ACCELERATION:
      if (asink->SectionState[i])
      {
        asink->FieldAccelerationData.iawscomponent_FieldAccel->Show();
        asink->FieldAccelerationData.iawscomponent_FieldAccel->Raise();
      }
      else
        asink->FieldAccelerationData.iawscomponent_FieldAccel->Hide();
      break;
    case SECTION_LOADSAVE:
      if (asink->SectionState[i])
      {
        asink->LoadSaveData.iawscomponent_LoadSave->Show();
        asink->LoadSaveData.iawscomponent_LoadSave->Raise();
        asink->LoadSaveData.settings_changed=true;
      }
      else
        asink->LoadSaveData.iawscomponent_LoadSave->Hide();
      break;
    case SECTION_AGING_MOMENTS:
      if (asink->SectionState[i])
      {
        asink->AgingMomentsData.iawscomponent_AgingMoments->Show();
        asink->AgingMomentsData.iawscomponent_AgingMoments->Raise();
      }
      else
        asink->AgingMomentsData.iawscomponent_AgingMoments->Hide();
      break;
    default:
      csPrintf("Unhandled section state change %d.\n",i);
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


const char *PartEditSink::GetGraphicCWD()
{
  return GraphicSelectionData.currentdirectory->GetData();
}

void PartEditSink::SetGraphicCWD(const char *cwd)
{
  GraphicSelectionData.currentdirectory->Clear();
  GraphicSelectionData.currentdirectory->Append(cwd);
  FillGraphicFileList();
}

const char *PartEditSink::GetGraphicFile()
{
  return GraphicSelectionData.currentfilepath->GetData();
}

void PartEditSink::SetGraphicFile(const char *filestr)
{
  GraphicSelectionData.currentfilepath->Clear();
  GraphicSelectionData.currentfilepath->Append(GraphicSelectionData.currentdirectory);
  GraphicSelectionData.currentfilepath->Append(filestr);
}


const char *PartEditSink::GetGraphicFilter()
{
  return GraphicSelectionData.filter->GetData();
}

void PartEditSink::SetGraphicFilter(const char *filterstr)
{
  GraphicSelectionData.filter->Clear();
  GraphicSelectionData.filter->Append(filterstr);
  FillGraphicFileList();
}

/// Static callback to handle graphic file working directory change.
void PartEditSink::AwsSetGraphicFilter(intptr_t sk, iAwsSource *source)
{
  iString *cwd;
  csRef<iString> path=new scfString();
  csRef<iString> filter=new scfString();

  if (asink->GraphicSelectionData.iawscomponent_GraphicFilter->GetProperty("Text",(intptr_t*)&cwd) && cwd->Length())
  {
    size_t position=cwd->FindLast('/');
    if (position != (size_t) -1)
    {
      // Filter contains a path
      cwd->SubString(path,0,position+1);
      cwd->SubString(filter,position+1,cwd->Length()-position-1);
      asink->GraphicSelectionData.iawscomponent_GraphicFilter->SetProperty("Text",(intptr_t)(iString*)filter);
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
void PartEditSink::AwsGraphicFileSelected(intptr_t sk, iAwsSource *source)
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


void PartEditSink::FillGraphicFileList()
{
  if (GraphicSelectionData.iawscomponent_GraphicFileList != 0 && vfs != 0)
  {
    csRef<iStringArray> files;
    size_t length,i;
    const char *filename;
    iAwsParmList *pl=0;
    pl=wmgr->CreateParmList();

    // Update the path display
    GraphicSelectionData.iawscomponent_GraphicFileList->SetProperty 
      ("Column0Caption", (intptr_t)
      CS_CONST_CAST(char*, GraphicSelectionData.currentdirectory->GetData()));

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
/*
 * We need a special handler here since we want to set the value based off of a specific
 *  component regardless of which component generated the trigger.
 *
 * Although this is a radio button, it only has 2 options, so we can treat it like a checkbox if we 
 * only examine the state of one of the buttons.
 */
void PartEditSink::AwsSetParticleType(intptr_t sk, iAwsSource *source)
{
  bool *p_bvalue;
  if (asink->EmitterStateData.iawscomponent_RectParticlesRadio->GetProperty("State",(intptr_t*)&p_bvalue))
  {
    if (*p_bvalue)
      asink->EmitterStateData.state.rectangular_particles=true;
    else
      asink->EmitterStateData.state.rectangular_particles=false;
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetRectangularWidth,
									 EmitterStateData.state.rect_w,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetRectangularHeight,
									 EmitterStateData.state.rect_h,EmitterStateData.settings_changed,
									 UpdateEmitterStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_INT(AwsSetRegularNumber,
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


bool PartEditSink::EmitterStateChanged()
{
  return EmitterStateData.settings_changed;
}

void PartEditSink::ClearEmitterStateChanged()
{
  EmitterStateData.settings_changed=false;
}

EmitterState *PartEditSink::GetEmitterState()
{
  return &(EmitterStateData.state);
}

void PartEditSink::SetEmitterState(EmitterState *source)
{
  memcpy(&(EmitterStateData.state),source,sizeof(EmitterState));
  UpdateEmitterStateDisplay();
  ClearEmitterStateChanged();
}


void PartEditSink::UpdateEmitterStateDisplay()
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


bool PartEditSink::InitialPositionStateChanged()
{
  return InitialPositionData.settings_changed;
}

void PartEditSink::ClearInitialPositionStateChanged()
{
  InitialPositionData.settings_changed=false;
}

Emitter3DState *PartEditSink::GetInitialPositionState()
{
  return &(InitialPositionData.state);
}

void PartEditSink::SetInitialPositionState(Emitter3DState *source)
{
  memcpy(&(InitialPositionData.state),source,sizeof(Emitter3DState));
  ClearInitialPositionStateChanged();
  UpdateInitialPositionStateDisplay();
}

void PartEditSink::UpdateInitialPositionStateDisplay()
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
//  Initial Speed Display
////
IMPLEMENT_REGISTER_FUNCTION(RegisterInitialSpeed,InitialSpeedData.iawscomponent_InitialSpeed)
IMPLEMENT_REGISTER_FUNCTION(RegisterISFPX,InitialSpeedData.iawscomponent_ISFPX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISFPY,InitialSpeedData.iawscomponent_ISFPY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISFPZ,InitialSpeedData.iawscomponent_ISFPZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISFWeight,InitialSpeedData.iawscomponent_ISFWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterISLSX,InitialSpeedData.iawscomponent_ISLSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISLSY,InitialSpeedData.iawscomponent_ISLSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISLSZ,InitialSpeedData.iawscomponent_ISLSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISLEX,InitialSpeedData.iawscomponent_ISLEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISLEY,InitialSpeedData.iawscomponent_ISLEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISLEZ,InitialSpeedData.iawscomponent_ISLEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISLWeight,InitialSpeedData.iawscomponent_ISLWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterISBMX,InitialSpeedData.iawscomponent_ISBMX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISBMY,InitialSpeedData.iawscomponent_ISBMY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISBMZ,InitialSpeedData.iawscomponent_ISBMZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISBXX,InitialSpeedData.iawscomponent_ISBXX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISBXY,InitialSpeedData.iawscomponent_ISBXY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISBXZ,InitialSpeedData.iawscomponent_ISBXZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISBWeight,InitialSpeedData.iawscomponent_ISBWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSCX,InitialSpeedData.iawscomponent_ISSCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSCY,InitialSpeedData.iawscomponent_ISSCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSCZ,InitialSpeedData.iawscomponent_ISSCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSMin,InitialSpeedData.iawscomponent_ISSMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSMax,InitialSpeedData.iawscomponent_ISSMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSWeight,InitialSpeedData.iawscomponent_ISSWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNOX,InitialSpeedData.iawscomponent_ISCNOX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNOY,InitialSpeedData.iawscomponent_ISCNOY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNOZ,InitialSpeedData.iawscomponent_ISCNOZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNElev,InitialSpeedData.iawscomponent_ISCNElevation)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNAzim,InitialSpeedData.iawscomponent_ISCNAzimuth)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNAper,InitialSpeedData.iawscomponent_ISCNAperture)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNMin,InitialSpeedData.iawscomponent_ISCNMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNMax,InitialSpeedData.iawscomponent_ISCNMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCNWeight,InitialSpeedData.iawscomponent_ISCNWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYSX,InitialSpeedData.iawscomponent_ISCYSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYSY,InitialSpeedData.iawscomponent_ISCYSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYSZ,InitialSpeedData.iawscomponent_ISCYSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYEX,InitialSpeedData.iawscomponent_ISCYEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYEY,InitialSpeedData.iawscomponent_ISCYEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYEZ,InitialSpeedData.iawscomponent_ISCYEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYMin,InitialSpeedData.iawscomponent_ISCYMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYMax,InitialSpeedData.iawscomponent_ISCYMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYWeight,InitialSpeedData.iawscomponent_ISCYWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSTCX,InitialSpeedData.iawscomponent_ISSTCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSTCY,InitialSpeedData.iawscomponent_ISSTCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSTCZ,InitialSpeedData.iawscomponent_ISSTCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSTMin,InitialSpeedData.iawscomponent_ISSTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSTMax,InitialSpeedData.iawscomponent_ISSTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterISSTWeight,InitialSpeedData.iawscomponent_ISSTWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTSX,InitialSpeedData.iawscomponent_ISCYTSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTSY,InitialSpeedData.iawscomponent_ISCYTSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTSZ,InitialSpeedData.iawscomponent_ISCYTSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTEX,InitialSpeedData.iawscomponent_ISCYTEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTEY,InitialSpeedData.iawscomponent_ISCYTEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTEZ,InitialSpeedData.iawscomponent_ISCYTEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTMin,InitialSpeedData.iawscomponent_ISCYTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTMax,InitialSpeedData.iawscomponent_ISCYTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterISCYTWeight,InitialSpeedData.iawscomponent_ISCYTWeight)


// Fixed Position
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISFPositionX,
									 InitialSpeedData.state.fixed_position.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISFPositionY,
									 InitialSpeedData.state.fixed_position.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISFPositionZ,
									 InitialSpeedData.state.fixed_position.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISFWeight,
									 InitialSpeedData.state.fixed_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
// Line
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISLStartX,
									 InitialSpeedData.state.line_start.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISLStartY,
									 InitialSpeedData.state.line_start.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISLStartZ,
									 InitialSpeedData.state.line_start.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISLEndX,
									 InitialSpeedData.state.line_end.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISLEndY,
									 InitialSpeedData.state.line_end.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISLEndZ,
									 InitialSpeedData.state.line_end.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISLWeight,
									 InitialSpeedData.state.line_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)

// Box
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISBMinX,
									 InitialSpeedData.state.box_min.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISBMinY,
									 InitialSpeedData.state.box_min.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISBMinZ,
									 InitialSpeedData.state.box_min.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISBMaxX,
									 InitialSpeedData.state.box_max.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISBMaxY,
									 InitialSpeedData.state.box_max.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISBMaxZ,
									 InitialSpeedData.state.box_max.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISBWeight,
									 InitialSpeedData.state.box_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)

// Sphere
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSCenterX,
									 InitialSpeedData.state.sphere_center.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSCenterY,
									 InitialSpeedData.state.sphere_center.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSCenterZ,
									 InitialSpeedData.state.sphere_center.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSMin,
									 InitialSpeedData.state.sphere_min,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSMax,
									 InitialSpeedData.state.sphere_max,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSWeight,
									 InitialSpeedData.state.sphere_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)

// Cone
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNOriginX,
									 InitialSpeedData.state.cone_origin.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNOriginY,
									 InitialSpeedData.state.cone_origin.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNOriginZ,
									 InitialSpeedData.state.cone_origin.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNElev,
									 InitialSpeedData.state.cone_elevation,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNAzim,
									 InitialSpeedData.state.cone_azimuth,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNAper,
									 InitialSpeedData.state.cone_aperture,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNMin,
									 InitialSpeedData.state.cone_min,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNMax,
									 InitialSpeedData.state.cone_max,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCNWeight,
									 InitialSpeedData.state.cone_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)

// Cylinder
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYStartX,
									 InitialSpeedData.state.cylinder_start.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYStartY,
									 InitialSpeedData.state.cylinder_start.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYStartZ,
									 InitialSpeedData.state.cylinder_start.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYEndX,
									 InitialSpeedData.state.cylinder_end.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYEndY,
									 InitialSpeedData.state.cylinder_end.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYEndZ,
									 InitialSpeedData.state.cylinder_end.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYMin,
									 InitialSpeedData.state.cylinder_min,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYMax,
									 InitialSpeedData.state.cylinder_max,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYWeight,
									 InitialSpeedData.state.cylinder_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)

// Sphere Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSTCenterX,
									 InitialSpeedData.state.spheretangent_center.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSTCenterY,
									 InitialSpeedData.state.spheretangent_center.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSTCenterZ,
									 InitialSpeedData.state.spheretangent_center.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSTMin,
									 InitialSpeedData.state.spheretangent_min,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSTMax,
									 InitialSpeedData.state.spheretangent_max,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISSTWeight,
									 InitialSpeedData.state.spheretangent_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)

// Cylinder Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTStartX,
									 InitialSpeedData.state.cylindertangent_start.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTStartY,
									 InitialSpeedData.state.cylindertangent_start.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTStartZ,
									 InitialSpeedData.state.cylindertangent_start.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTEndX,
									 InitialSpeedData.state.cylindertangent_end.x,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTEndY,
									 InitialSpeedData.state.cylindertangent_end.y,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTEndZ,
									 InitialSpeedData.state.cylindertangent_end.z,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTMin,
									 InitialSpeedData.state.cylindertangent_min,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTMax,
									 InitialSpeedData.state.cylindertangent_max,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetISCYTWeight,
									 InitialSpeedData.state.cylindertangent_weight,InitialSpeedData.settings_changed,
									 UpdateInitialSpeedStateDisplay)


bool PartEditSink::InitialSpeedStateChanged()
{
  return InitialSpeedData.settings_changed;
}

void PartEditSink::ClearInitialSpeedStateChanged()
{
  InitialSpeedData.settings_changed=false;
}

Emitter3DState *PartEditSink::GetInitialSpeedState()
{
  return &(InitialSpeedData.state);
}

void PartEditSink::SetInitialSpeedState(Emitter3DState *source)
{
  memcpy(&(InitialSpeedData.state),source,sizeof(Emitter3DState));
  ClearInitialSpeedStateChanged();
  UpdateInitialSpeedStateDisplay();
}

void PartEditSink::UpdateInitialSpeedStateDisplay()
{
  csRef<iString> value;

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISFPX,InitialSpeedData.state.fixed_position.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISFPY,InitialSpeedData.state.fixed_position.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISFPZ,InitialSpeedData.state.fixed_position.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISFWeight,InitialSpeedData.state.fixed_weight);

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISLSX,InitialSpeedData.state.line_start.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISLSY,InitialSpeedData.state.line_start.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISLSZ,InitialSpeedData.state.line_start.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISLEX,InitialSpeedData.state.line_end.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISLEY,InitialSpeedData.state.line_end.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISLEZ,InitialSpeedData.state.line_end.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISLWeight,InitialSpeedData.state.line_weight);

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISBMX,InitialSpeedData.state.box_min.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISBMY,InitialSpeedData.state.box_min.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISBMZ,InitialSpeedData.state.box_min.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISBXX,InitialSpeedData.state.box_max.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISBXY,InitialSpeedData.state.box_max.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISBXZ,InitialSpeedData.state.box_max.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISBWeight,InitialSpeedData.state.box_weight);

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSCX,InitialSpeedData.state.sphere_center.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSCY,InitialSpeedData.state.sphere_center.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSCZ,InitialSpeedData.state.sphere_center.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSMin,InitialSpeedData.state.sphere_min);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSMax,InitialSpeedData.state.sphere_max);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSWeight,InitialSpeedData.state.sphere_weight);

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNOX,InitialSpeedData.state.cone_origin.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNOY,InitialSpeedData.state.cone_origin.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNOZ,InitialSpeedData.state.cone_origin.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNElevation,InitialSpeedData.state.cone_elevation);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNAzimuth,InitialSpeedData.state.cone_azimuth);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNAperture,InitialSpeedData.state.cone_aperture);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNMin,InitialSpeedData.state.cone_min);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNMax,InitialSpeedData.state.cone_max);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCNWeight,InitialSpeedData.state.cone_weight);

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYSX,InitialSpeedData.state.cylinder_start.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYSY,InitialSpeedData.state.cylinder_start.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYSZ,InitialSpeedData.state.cylinder_start.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYEX,InitialSpeedData.state.cylinder_end.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYEY,InitialSpeedData.state.cylinder_end.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYEZ,InitialSpeedData.state.cylinder_end.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYMin,InitialSpeedData.state.cylinder_min);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYMax,InitialSpeedData.state.cylinder_max);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYWeight,InitialSpeedData.state.cylinder_weight);

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSTCX,InitialSpeedData.state.spheretangent_center.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSTCY,InitialSpeedData.state.spheretangent_center.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSTCZ,InitialSpeedData.state.spheretangent_center.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSTMin,InitialSpeedData.state.spheretangent_min);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSTMax,InitialSpeedData.state.spheretangent_max);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISSTWeight,InitialSpeedData.state.spheretangent_weight);

  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTSX,InitialSpeedData.state.cylindertangent_start.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTSY,InitialSpeedData.state.cylindertangent_start.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTSZ,InitialSpeedData.state.cylindertangent_start.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTEX,InitialSpeedData.state.cylindertangent_end.x);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTEY,InitialSpeedData.state.cylindertangent_end.y);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTEZ,InitialSpeedData.state.cylindertangent_end.z);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTMin,InitialSpeedData.state.cylindertangent_min);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTMax,InitialSpeedData.state.cylindertangent_max);
  SET_TEXTBOX_FLOAT(InitialSpeedData.iawscomponent_ISCYTWeight,InitialSpeedData.state.cylindertangent_weight);
}


////
//  Initial Acceleration Display
////
IMPLEMENT_REGISTER_FUNCTION(RegisterInitialAcceleration,InitialAccelerationData.iawscomponent_InitialAcceleration)
IMPLEMENT_REGISTER_FUNCTION(RegisterIAFPX,InitialAccelerationData.iawscomponent_IAFPX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIAFPY,InitialAccelerationData.iawscomponent_IAFPY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIAFPZ,InitialAccelerationData.iawscomponent_IAFPZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIAFWeight,InitialAccelerationData.iawscomponent_IAFWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIALSX,InitialAccelerationData.iawscomponent_IALSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIALSY,InitialAccelerationData.iawscomponent_IALSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIALSZ,InitialAccelerationData.iawscomponent_IALSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIALEX,InitialAccelerationData.iawscomponent_IALEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIALEY,InitialAccelerationData.iawscomponent_IALEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIALEZ,InitialAccelerationData.iawscomponent_IALEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIALWeight,InitialAccelerationData.iawscomponent_IALWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIABMX,InitialAccelerationData.iawscomponent_IABMX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIABMY,InitialAccelerationData.iawscomponent_IABMY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIABMZ,InitialAccelerationData.iawscomponent_IABMZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIABXX,InitialAccelerationData.iawscomponent_IABXX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIABXY,InitialAccelerationData.iawscomponent_IABXY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIABXZ,InitialAccelerationData.iawscomponent_IABXZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIABWeight,InitialAccelerationData.iawscomponent_IABWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASCX,InitialAccelerationData.iawscomponent_IASCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASCY,InitialAccelerationData.iawscomponent_IASCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASCZ,InitialAccelerationData.iawscomponent_IASCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASMin,InitialAccelerationData.iawscomponent_IASMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASMax,InitialAccelerationData.iawscomponent_IASMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASWeight,InitialAccelerationData.iawscomponent_IASWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNOX,InitialAccelerationData.iawscomponent_IACNOX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNOY,InitialAccelerationData.iawscomponent_IACNOY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNOZ,InitialAccelerationData.iawscomponent_IACNOZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNElev,InitialAccelerationData.iawscomponent_IACNElevation)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNAzim,InitialAccelerationData.iawscomponent_IACNAzimuth)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNAper,InitialAccelerationData.iawscomponent_IACNAperture)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNMin,InitialAccelerationData.iawscomponent_IACNMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNMax,InitialAccelerationData.iawscomponent_IACNMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACNWeight,InitialAccelerationData.iawscomponent_IACNWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYSX,InitialAccelerationData.iawscomponent_IACYSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYSY,InitialAccelerationData.iawscomponent_IACYSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYSZ,InitialAccelerationData.iawscomponent_IACYSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYEX,InitialAccelerationData.iawscomponent_IACYEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYEY,InitialAccelerationData.iawscomponent_IACYEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYEZ,InitialAccelerationData.iawscomponent_IACYEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYMin,InitialAccelerationData.iawscomponent_IACYMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYMax,InitialAccelerationData.iawscomponent_IACYMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYWeight,InitialAccelerationData.iawscomponent_IACYWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASTCX,InitialAccelerationData.iawscomponent_IASTCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASTCY,InitialAccelerationData.iawscomponent_IASTCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASTCZ,InitialAccelerationData.iawscomponent_IASTCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASTMin,InitialAccelerationData.iawscomponent_IASTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASTMax,InitialAccelerationData.iawscomponent_IASTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIASTWeight,InitialAccelerationData.iawscomponent_IASTWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTSX,InitialAccelerationData.iawscomponent_IACYTSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTSY,InitialAccelerationData.iawscomponent_IACYTSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTSZ,InitialAccelerationData.iawscomponent_IACYTSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTEX,InitialAccelerationData.iawscomponent_IACYTEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTEY,InitialAccelerationData.iawscomponent_IACYTEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTEZ,InitialAccelerationData.iawscomponent_IACYTEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTMin,InitialAccelerationData.iawscomponent_IACYTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTMax,InitialAccelerationData.iawscomponent_IACYTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterIACYTWeight,InitialAccelerationData.iawscomponent_IACYTWeight)



// Fixed Position
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIAFPositionX,
									 InitialAccelerationData.state.fixed_position.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIAFPositionY,
									 InitialAccelerationData.state.fixed_position.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIAFPositionZ,
									 InitialAccelerationData.state.fixed_position.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIAFWeight,
									 InitialAccelerationData.state.fixed_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
// Line
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIALStartX,
									 InitialAccelerationData.state.line_start.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIALStartY,
									 InitialAccelerationData.state.line_start.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIALStartZ,
									 InitialAccelerationData.state.line_start.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIALEndX,
									 InitialAccelerationData.state.line_end.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIALEndY,
									 InitialAccelerationData.state.line_end.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIALEndZ,
									 InitialAccelerationData.state.line_end.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIALWeight,
									 InitialAccelerationData.state.line_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)

// Box
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIABMinX,
									 InitialAccelerationData.state.box_min.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIABMinY,
									 InitialAccelerationData.state.box_min.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIABMinZ,
									 InitialAccelerationData.state.box_min.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIABMaxX,
									 InitialAccelerationData.state.box_max.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIABMaxY,
									 InitialAccelerationData.state.box_max.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIABMaxZ,
									 InitialAccelerationData.state.box_max.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIABWeight,
									 InitialAccelerationData.state.box_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)

// Sphere
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASCenterX,
									 InitialAccelerationData.state.sphere_center.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASCenterY,
									 InitialAccelerationData.state.sphere_center.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASCenterZ,
									 InitialAccelerationData.state.sphere_center.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASMin,
									 InitialAccelerationData.state.sphere_min,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASMax,
									 InitialAccelerationData.state.sphere_max,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASWeight,
									 InitialAccelerationData.state.sphere_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)

// Cone
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNOriginX,
									 InitialAccelerationData.state.cone_origin.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNOriginY,
									 InitialAccelerationData.state.cone_origin.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNOriginZ,
									 InitialAccelerationData.state.cone_origin.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNElev,
									 InitialAccelerationData.state.cone_elevation,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNAzim,
									 InitialAccelerationData.state.cone_azimuth,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNAper,
									 InitialAccelerationData.state.cone_aperture,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNMin,
									 InitialAccelerationData.state.cone_min,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNMax,
									 InitialAccelerationData.state.cone_max,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACNWeight,
									 InitialAccelerationData.state.cone_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)

// Cylinder
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYStartX,
									 InitialAccelerationData.state.cylinder_start.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYStartY,
									 InitialAccelerationData.state.cylinder_start.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYStartZ,
									 InitialAccelerationData.state.cylinder_start.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYEndX,
									 InitialAccelerationData.state.cylinder_end.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYEndY,
									 InitialAccelerationData.state.cylinder_end.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYEndZ,
									 InitialAccelerationData.state.cylinder_end.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYMin,
									 InitialAccelerationData.state.cylinder_min,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYMax,
									 InitialAccelerationData.state.cylinder_max,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYWeight,
									 InitialAccelerationData.state.cylinder_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)

// Sphere Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASTCenterX,
									 InitialAccelerationData.state.spheretangent_center.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASTCenterY,
									 InitialAccelerationData.state.spheretangent_center.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASTCenterZ,
									 InitialAccelerationData.state.spheretangent_center.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASTMin,
									 InitialAccelerationData.state.spheretangent_min,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASTMax,
									 InitialAccelerationData.state.spheretangent_max,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIASTWeight,
									 InitialAccelerationData.state.spheretangent_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)

// Cylinder Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTStartX,
									 InitialAccelerationData.state.cylindertangent_start.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTStartY,
									 InitialAccelerationData.state.cylindertangent_start.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTStartZ,
									 InitialAccelerationData.state.cylindertangent_start.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTEndX,
									 InitialAccelerationData.state.cylindertangent_end.x,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTEndY,
									 InitialAccelerationData.state.cylindertangent_end.y,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTEndZ,
									 InitialAccelerationData.state.cylindertangent_end.z,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTMin,
									 InitialAccelerationData.state.cylindertangent_min,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTMax,
									 InitialAccelerationData.state.cylindertangent_max,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetIACYTWeight,
									 InitialAccelerationData.state.cylindertangent_weight,InitialAccelerationData.settings_changed,
									 UpdateInitialAccelerationStateDisplay)


bool PartEditSink::InitialAccelerationStateChanged()
{
  return InitialAccelerationData.settings_changed;
}

void PartEditSink::ClearInitialAccelerationStateChanged()
{
  InitialAccelerationData.settings_changed=false;
}

Emitter3DState *PartEditSink::GetInitialAccelerationState()
{
  return &(InitialAccelerationData.state);
}

void PartEditSink::SetInitialAccelerationState(Emitter3DState *source)
{
  memcpy(&(InitialAccelerationData.state),source,sizeof(Emitter3DState));
  ClearInitialAccelerationStateChanged();
  UpdateInitialAccelerationStateDisplay();
}

void PartEditSink::UpdateInitialAccelerationStateDisplay()
{
  csRef<iString> value;

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IAFPX,InitialAccelerationData.state.fixed_position.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IAFPY,InitialAccelerationData.state.fixed_position.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IAFPZ,InitialAccelerationData.state.fixed_position.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IAFWeight,InitialAccelerationData.state.fixed_weight);

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IALSX,InitialAccelerationData.state.line_start.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IALSY,InitialAccelerationData.state.line_start.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IALSZ,InitialAccelerationData.state.line_start.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IALEX,InitialAccelerationData.state.line_end.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IALEY,InitialAccelerationData.state.line_end.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IALEZ,InitialAccelerationData.state.line_end.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IALWeight,InitialAccelerationData.state.line_weight);

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IABMX,InitialAccelerationData.state.box_min.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IABMY,InitialAccelerationData.state.box_min.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IABMZ,InitialAccelerationData.state.box_min.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IABXX,InitialAccelerationData.state.box_max.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IABXY,InitialAccelerationData.state.box_max.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IABXZ,InitialAccelerationData.state.box_max.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IABWeight,InitialAccelerationData.state.box_weight);

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASCX,InitialAccelerationData.state.sphere_center.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASCY,InitialAccelerationData.state.sphere_center.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASCZ,InitialAccelerationData.state.sphere_center.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASMin,InitialAccelerationData.state.sphere_min);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASMax,InitialAccelerationData.state.sphere_max);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASWeight,InitialAccelerationData.state.sphere_weight);

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNOX,InitialAccelerationData.state.cone_origin.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNOY,InitialAccelerationData.state.cone_origin.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNOZ,InitialAccelerationData.state.cone_origin.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNElevation,InitialAccelerationData.state.cone_elevation);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNAzimuth,InitialAccelerationData.state.cone_azimuth);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNAperture,InitialAccelerationData.state.cone_aperture);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNMin,InitialAccelerationData.state.cone_min);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNMax,InitialAccelerationData.state.cone_max);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACNWeight,InitialAccelerationData.state.cone_weight);

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYSX,InitialAccelerationData.state.cylinder_start.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYSY,InitialAccelerationData.state.cylinder_start.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYSZ,InitialAccelerationData.state.cylinder_start.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYEX,InitialAccelerationData.state.cylinder_end.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYEY,InitialAccelerationData.state.cylinder_end.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYEZ,InitialAccelerationData.state.cylinder_end.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYMin,InitialAccelerationData.state.cylinder_min);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYMax,InitialAccelerationData.state.cylinder_max);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYWeight,InitialAccelerationData.state.cylinder_weight);

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASTCX,InitialAccelerationData.state.spheretangent_center.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASTCY,InitialAccelerationData.state.spheretangent_center.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASTCZ,InitialAccelerationData.state.spheretangent_center.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASTMin,InitialAccelerationData.state.spheretangent_min);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASTMax,InitialAccelerationData.state.spheretangent_max);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IASTWeight,InitialAccelerationData.state.spheretangent_weight);

  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTSX,InitialAccelerationData.state.cylindertangent_start.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTSY,InitialAccelerationData.state.cylindertangent_start.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTSZ,InitialAccelerationData.state.cylindertangent_start.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTEX,InitialAccelerationData.state.cylindertangent_end.x);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTEY,InitialAccelerationData.state.cylindertangent_end.y);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTEZ,InitialAccelerationData.state.cylindertangent_end.z);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTMin,InitialAccelerationData.state.cylindertangent_min);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTMax,InitialAccelerationData.state.cylindertangent_max);
  SET_TEXTBOX_FLOAT(InitialAccelerationData.iawscomponent_IACYTWeight,InitialAccelerationData.state.cylindertangent_weight);
}


////
//  Field Speed Display
////
IMPLEMENT_REGISTER_FUNCTION(RegisterFieldSpeed,FieldSpeedData.iawscomponent_FieldSpeed)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSActive,FieldSpeedData.iawscomponent_FSActive)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSFPX,FieldSpeedData.iawscomponent_FSFPX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSFPY,FieldSpeedData.iawscomponent_FSFPY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSFPZ,FieldSpeedData.iawscomponent_FSFPZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSFWeight,FieldSpeedData.iawscomponent_FSFWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLSX,FieldSpeedData.iawscomponent_FSLSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLSY,FieldSpeedData.iawscomponent_FSLSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLSZ,FieldSpeedData.iawscomponent_FSLSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLEX,FieldSpeedData.iawscomponent_FSLEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLEY,FieldSpeedData.iawscomponent_FSLEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLEZ,FieldSpeedData.iawscomponent_FSLEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLWeight,FieldSpeedData.iawscomponent_FSLWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSBMX,FieldSpeedData.iawscomponent_FSBMX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSBMY,FieldSpeedData.iawscomponent_FSBMY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSBMZ,FieldSpeedData.iawscomponent_FSBMZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSBXX,FieldSpeedData.iawscomponent_FSBXX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSBXY,FieldSpeedData.iawscomponent_FSBXY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSBXZ,FieldSpeedData.iawscomponent_FSBXZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSBWeight,FieldSpeedData.iawscomponent_FSBWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSCX,FieldSpeedData.iawscomponent_FSSCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSCY,FieldSpeedData.iawscomponent_FSSCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSCZ,FieldSpeedData.iawscomponent_FSSCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSMin,FieldSpeedData.iawscomponent_FSSMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSMax,FieldSpeedData.iawscomponent_FSSMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSWeight,FieldSpeedData.iawscomponent_FSSWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNOX,FieldSpeedData.iawscomponent_FSCNOX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNOY,FieldSpeedData.iawscomponent_FSCNOY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNOZ,FieldSpeedData.iawscomponent_FSCNOZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNElev,FieldSpeedData.iawscomponent_FSCNElevation)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNAzim,FieldSpeedData.iawscomponent_FSCNAzimuth)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNAper,FieldSpeedData.iawscomponent_FSCNAperture)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNMin,FieldSpeedData.iawscomponent_FSCNMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNMax,FieldSpeedData.iawscomponent_FSCNMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCNWeight,FieldSpeedData.iawscomponent_FSCNWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYSX,FieldSpeedData.iawscomponent_FSCYSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYSY,FieldSpeedData.iawscomponent_FSCYSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYSZ,FieldSpeedData.iawscomponent_FSCYSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYEX,FieldSpeedData.iawscomponent_FSCYEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYEY,FieldSpeedData.iawscomponent_FSCYEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYEZ,FieldSpeedData.iawscomponent_FSCYEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYMin,FieldSpeedData.iawscomponent_FSCYMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYMax,FieldSpeedData.iawscomponent_FSCYMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYWeight,FieldSpeedData.iawscomponent_FSCYWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSTCX,FieldSpeedData.iawscomponent_FSSTCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSTCY,FieldSpeedData.iawscomponent_FSSTCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSTCZ,FieldSpeedData.iawscomponent_FSSTCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSTMin,FieldSpeedData.iawscomponent_FSSTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSTMax,FieldSpeedData.iawscomponent_FSSTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSSTWeight,FieldSpeedData.iawscomponent_FSSTWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTSX,FieldSpeedData.iawscomponent_FSCYTSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTSY,FieldSpeedData.iawscomponent_FSCYTSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTSZ,FieldSpeedData.iawscomponent_FSCYTSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTEX,FieldSpeedData.iawscomponent_FSCYTEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTEY,FieldSpeedData.iawscomponent_FSCYTEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTEZ,FieldSpeedData.iawscomponent_FSCYTEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTMin,FieldSpeedData.iawscomponent_FSCYTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTMax,FieldSpeedData.iawscomponent_FSCYTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSCYTWeight,FieldSpeedData.iawscomponent_FSCYTWeight)



IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(AwsSetFSActive,
									 FieldSpeedData.state.active,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)

// Fixed Position
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSFPositionX,
									 FieldSpeedData.state.e3d_state.fixed_position.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSFPositionY,
									 FieldSpeedData.state.e3d_state.fixed_position.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSFPositionZ,
									 FieldSpeedData.state.e3d_state.fixed_position.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSFWeight,
									 FieldSpeedData.state.e3d_state.fixed_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
// Line
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSLStartX,
									 FieldSpeedData.state.e3d_state.line_start.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSLStartY,
									 FieldSpeedData.state.e3d_state.line_start.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSLStartZ,
									 FieldSpeedData.state.e3d_state.line_start.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSLEndX,
									 FieldSpeedData.state.e3d_state.line_end.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSLEndY,
									 FieldSpeedData.state.e3d_state.line_end.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSLEndZ,
									 FieldSpeedData.state.e3d_state.line_end.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSLWeight,
									 FieldSpeedData.state.e3d_state.line_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)

// Box
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSBMinX,
									 FieldSpeedData.state.e3d_state.box_min.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSBMinY,
									 FieldSpeedData.state.e3d_state.box_min.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSBMinZ,
									 FieldSpeedData.state.e3d_state.box_min.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSBMaxX,
									 FieldSpeedData.state.e3d_state.box_max.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSBMaxY,
									 FieldSpeedData.state.e3d_state.box_max.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSBMaxZ,
									 FieldSpeedData.state.e3d_state.box_max.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSBWeight,
									 FieldSpeedData.state.e3d_state.box_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)

// Sphere
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSCenterX,
									 FieldSpeedData.state.e3d_state.sphere_center.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSCenterY,
									 FieldSpeedData.state.e3d_state.sphere_center.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSCenterZ,
									 FieldSpeedData.state.e3d_state.sphere_center.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSMin,
									 FieldSpeedData.state.e3d_state.sphere_min,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSMax,
									 FieldSpeedData.state.e3d_state.sphere_max,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSWeight,
									 FieldSpeedData.state.e3d_state.sphere_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)

// Cone
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNOriginX,
									 FieldSpeedData.state.e3d_state.cone_origin.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNOriginY,
									 FieldSpeedData.state.e3d_state.cone_origin.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNOriginZ,
									 FieldSpeedData.state.e3d_state.cone_origin.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNElev,
									 FieldSpeedData.state.e3d_state.cone_elevation,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNAzim,
									 FieldSpeedData.state.e3d_state.cone_azimuth,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNAper,
									 FieldSpeedData.state.e3d_state.cone_aperture,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNMin,
									 FieldSpeedData.state.e3d_state.cone_min,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNMax,
									 FieldSpeedData.state.e3d_state.cone_max,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCNWeight,
									 FieldSpeedData.state.e3d_state.cone_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)

// Cylinder
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYStartX,
									 FieldSpeedData.state.e3d_state.cylinder_start.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYStartY,
									 FieldSpeedData.state.e3d_state.cylinder_start.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYStartZ,
									 FieldSpeedData.state.e3d_state.cylinder_start.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYEndX,
									 FieldSpeedData.state.e3d_state.cylinder_end.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYEndY,
									 FieldSpeedData.state.e3d_state.cylinder_end.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYEndZ,
									 FieldSpeedData.state.e3d_state.cylinder_end.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYMin,
									 FieldSpeedData.state.e3d_state.cylinder_min,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYMax,
									 FieldSpeedData.state.e3d_state.cylinder_max,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYWeight,
									 FieldSpeedData.state.e3d_state.cylinder_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)

// Sphere Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSTCenterX,
									 FieldSpeedData.state.e3d_state.spheretangent_center.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSTCenterY,
									 FieldSpeedData.state.e3d_state.spheretangent_center.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSTCenterZ,
									 FieldSpeedData.state.e3d_state.spheretangent_center.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSTMin,
									 FieldSpeedData.state.e3d_state.spheretangent_min,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSTMax,
									 FieldSpeedData.state.e3d_state.spheretangent_max,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSSTWeight,
									 FieldSpeedData.state.e3d_state.spheretangent_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)

// Cylinder Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTStartX,
									 FieldSpeedData.state.e3d_state.cylindertangent_start.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTStartY,
									 FieldSpeedData.state.e3d_state.cylindertangent_start.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTStartZ,
									 FieldSpeedData.state.e3d_state.cylindertangent_start.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTEndX,
									 FieldSpeedData.state.e3d_state.cylindertangent_end.x,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTEndY,
									 FieldSpeedData.state.e3d_state.cylindertangent_end.y,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTEndZ,
									 FieldSpeedData.state.e3d_state.cylindertangent_end.z,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTMin,
									 FieldSpeedData.state.e3d_state.cylindertangent_min,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTMax,
									 FieldSpeedData.state.e3d_state.cylindertangent_max,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFSCYTWeight,
									 FieldSpeedData.state.e3d_state.cylindertangent_weight,FieldSpeedData.settings_changed,
									 UpdateFieldSpeedStateDisplay)


bool PartEditSink::FieldSpeedStateChanged()
{
  return FieldSpeedData.settings_changed;
}

void PartEditSink::ClearFieldSpeedStateChanged()
{
  FieldSpeedData.settings_changed=false;
}

FieldState *PartEditSink::GetFieldSpeedState()
{
  return &(FieldSpeedData.state);
}

void PartEditSink::SetFieldSpeedState(FieldState *source)
{
  memcpy(&(FieldSpeedData.state),source,sizeof(FieldState));
  ClearFieldSpeedStateChanged();
  UpdateFieldSpeedStateDisplay();
}

void PartEditSink::UpdateFieldSpeedStateDisplay()
{
  csRef<iString> value;
  bool bvalue;

  SET_CHECKBOX_BOOL(FieldSpeedData.iawscomponent_FSActive,FieldSpeedData.state.active);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSFPX,FieldSpeedData.state.e3d_state.fixed_position.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSFPY,FieldSpeedData.state.e3d_state.fixed_position.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSFPZ,FieldSpeedData.state.e3d_state.fixed_position.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSFWeight,FieldSpeedData.state.e3d_state.fixed_weight);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSLSX,FieldSpeedData.state.e3d_state.line_start.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSLSY,FieldSpeedData.state.e3d_state.line_start.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSLSZ,FieldSpeedData.state.e3d_state.line_start.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSLEX,FieldSpeedData.state.e3d_state.line_end.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSLEY,FieldSpeedData.state.e3d_state.line_end.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSLEZ,FieldSpeedData.state.e3d_state.line_end.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSLWeight,FieldSpeedData.state.e3d_state.line_weight);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSBMX,FieldSpeedData.state.e3d_state.box_min.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSBMY,FieldSpeedData.state.e3d_state.box_min.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSBMZ,FieldSpeedData.state.e3d_state.box_min.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSBXX,FieldSpeedData.state.e3d_state.box_max.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSBXY,FieldSpeedData.state.e3d_state.box_max.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSBXZ,FieldSpeedData.state.e3d_state.box_max.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSBWeight,FieldSpeedData.state.e3d_state.box_weight);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSCX,FieldSpeedData.state.e3d_state.sphere_center.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSCY,FieldSpeedData.state.e3d_state.sphere_center.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSCZ,FieldSpeedData.state.e3d_state.sphere_center.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSMin,FieldSpeedData.state.e3d_state.sphere_min);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSMax,FieldSpeedData.state.e3d_state.sphere_max);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSWeight,FieldSpeedData.state.e3d_state.sphere_weight);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNOX,FieldSpeedData.state.e3d_state.cone_origin.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNOY,FieldSpeedData.state.e3d_state.cone_origin.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNOZ,FieldSpeedData.state.e3d_state.cone_origin.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNElevation,FieldSpeedData.state.e3d_state.cone_elevation);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNAzimuth,FieldSpeedData.state.e3d_state.cone_azimuth);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNAperture,FieldSpeedData.state.e3d_state.cone_aperture);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNMin,FieldSpeedData.state.e3d_state.cone_min);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNMax,FieldSpeedData.state.e3d_state.cone_max);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCNWeight,FieldSpeedData.state.e3d_state.cone_weight);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYSX,FieldSpeedData.state.e3d_state.cylinder_start.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYSY,FieldSpeedData.state.e3d_state.cylinder_start.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYSZ,FieldSpeedData.state.e3d_state.cylinder_start.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYEX,FieldSpeedData.state.e3d_state.cylinder_end.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYEY,FieldSpeedData.state.e3d_state.cylinder_end.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYEZ,FieldSpeedData.state.e3d_state.cylinder_end.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYMin,FieldSpeedData.state.e3d_state.cylinder_min);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYMax,FieldSpeedData.state.e3d_state.cylinder_max);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYWeight,FieldSpeedData.state.e3d_state.cylinder_weight);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSTCX,FieldSpeedData.state.e3d_state.spheretangent_center.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSTCY,FieldSpeedData.state.e3d_state.spheretangent_center.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSTCZ,FieldSpeedData.state.e3d_state.spheretangent_center.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSTMin,FieldSpeedData.state.e3d_state.spheretangent_min);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSTMax,FieldSpeedData.state.e3d_state.spheretangent_max);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSSTWeight,FieldSpeedData.state.e3d_state.spheretangent_weight);

  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTSX,FieldSpeedData.state.e3d_state.cylindertangent_start.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTSY,FieldSpeedData.state.e3d_state.cylindertangent_start.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTSZ,FieldSpeedData.state.e3d_state.cylindertangent_start.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTEX,FieldSpeedData.state.e3d_state.cylindertangent_end.x);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTEY,FieldSpeedData.state.e3d_state.cylindertangent_end.y);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTEZ,FieldSpeedData.state.e3d_state.cylindertangent_end.z);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTMin,FieldSpeedData.state.e3d_state.cylindertangent_min);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTMax,FieldSpeedData.state.e3d_state.cylindertangent_max);
  SET_TEXTBOX_FLOAT(FieldSpeedData.iawscomponent_FSCYTWeight,FieldSpeedData.state.e3d_state.cylindertangent_weight);
}


////
//  Field Acceleration Display
////
IMPLEMENT_REGISTER_FUNCTION(RegisterFieldAccel,FieldAccelerationData.iawscomponent_FieldAccel)
IMPLEMENT_REGISTER_FUNCTION(RegisterFAActive,FieldAccelerationData.iawscomponent_FAActive)
IMPLEMENT_REGISTER_FUNCTION(RegisterFAFPX,FieldAccelerationData.iawscomponent_FAFPX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFAFPY,FieldAccelerationData.iawscomponent_FAFPY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFAFPZ,FieldAccelerationData.iawscomponent_FAFPZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFAFWeight,FieldAccelerationData.iawscomponent_FAFWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFALSX,FieldAccelerationData.iawscomponent_FALSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFALSY,FieldAccelerationData.iawscomponent_FALSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFALSZ,FieldAccelerationData.iawscomponent_FALSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFALEX,FieldAccelerationData.iawscomponent_FALEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFALEY,FieldAccelerationData.iawscomponent_FALEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFALEZ,FieldAccelerationData.iawscomponent_FALEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFALWeight,FieldAccelerationData.iawscomponent_FALWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFABMX,FieldAccelerationData.iawscomponent_FABMX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFABMY,FieldAccelerationData.iawscomponent_FABMY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFABMZ,FieldAccelerationData.iawscomponent_FABMZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFABXX,FieldAccelerationData.iawscomponent_FABXX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFABXY,FieldAccelerationData.iawscomponent_FABXY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFABXZ,FieldAccelerationData.iawscomponent_FABXZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFABWeight,FieldAccelerationData.iawscomponent_FABWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASCX,FieldAccelerationData.iawscomponent_FASCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASCY,FieldAccelerationData.iawscomponent_FASCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASCZ,FieldAccelerationData.iawscomponent_FASCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASMin,FieldAccelerationData.iawscomponent_FASMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASMax,FieldAccelerationData.iawscomponent_FASMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASWeight,FieldAccelerationData.iawscomponent_FASWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNOX,FieldAccelerationData.iawscomponent_FACNOX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNOY,FieldAccelerationData.iawscomponent_FACNOY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNOZ,FieldAccelerationData.iawscomponent_FACNOZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNElev,FieldAccelerationData.iawscomponent_FACNElevation)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNAzim,FieldAccelerationData.iawscomponent_FACNAzimuth)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNAper,FieldAccelerationData.iawscomponent_FACNAperture)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNMin,FieldAccelerationData.iawscomponent_FACNMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNMax,FieldAccelerationData.iawscomponent_FACNMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACNWeight,FieldAccelerationData.iawscomponent_FACNWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYSX,FieldAccelerationData.iawscomponent_FACYSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYSY,FieldAccelerationData.iawscomponent_FACYSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYSZ,FieldAccelerationData.iawscomponent_FACYSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYEX,FieldAccelerationData.iawscomponent_FACYEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYEY,FieldAccelerationData.iawscomponent_FACYEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYEZ,FieldAccelerationData.iawscomponent_FACYEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYMin,FieldAccelerationData.iawscomponent_FACYMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYMax,FieldAccelerationData.iawscomponent_FACYMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYWeight,FieldAccelerationData.iawscomponent_FACYWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASTCX,FieldAccelerationData.iawscomponent_FASTCX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASTCY,FieldAccelerationData.iawscomponent_FASTCY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASTCZ,FieldAccelerationData.iawscomponent_FASTCZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASTMin,FieldAccelerationData.iawscomponent_FASTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASTMax,FieldAccelerationData.iawscomponent_FASTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFASTWeight,FieldAccelerationData.iawscomponent_FASTWeight)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTSX,FieldAccelerationData.iawscomponent_FACYTSX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTSY,FieldAccelerationData.iawscomponent_FACYTSY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTSZ,FieldAccelerationData.iawscomponent_FACYTSZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTEX,FieldAccelerationData.iawscomponent_FACYTEX)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTEY,FieldAccelerationData.iawscomponent_FACYTEY)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTEZ,FieldAccelerationData.iawscomponent_FACYTEZ)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTMin,FieldAccelerationData.iawscomponent_FACYTMin)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTMax,FieldAccelerationData.iawscomponent_FACYTMax)
IMPLEMENT_REGISTER_FUNCTION(RegisterFACYTWeight,FieldAccelerationData.iawscomponent_FACYTWeight)



IMPLEMENT_COMPONENT_CHECKBOX_TO_BOOL(AwsSetFAActive,
									 FieldAccelerationData.state.active,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
// Fixed Position
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFAFPositionX,
									 FieldAccelerationData.state.e3d_state.fixed_position.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFAFPositionY,
									 FieldAccelerationData.state.e3d_state.fixed_position.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFAFPositionZ,
									 FieldAccelerationData.state.e3d_state.fixed_position.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFAFWeight,
									 FieldAccelerationData.state.e3d_state.fixed_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
// Line
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFALStartX,
									 FieldAccelerationData.state.e3d_state.line_start.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFALStartY,
									 FieldAccelerationData.state.e3d_state.line_start.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFALStartZ,
									 FieldAccelerationData.state.e3d_state.line_start.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFALEndX,
									 FieldAccelerationData.state.e3d_state.line_end.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFALEndY,
									 FieldAccelerationData.state.e3d_state.line_end.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFALEndZ,
									 FieldAccelerationData.state.e3d_state.line_end.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFALWeight,
									 FieldAccelerationData.state.e3d_state.line_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)

// Box
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFABMinX,
									 FieldAccelerationData.state.e3d_state.box_min.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFABMinY,
									 FieldAccelerationData.state.e3d_state.box_min.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFABMinZ,
									 FieldAccelerationData.state.e3d_state.box_min.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFABMaxX,
									 FieldAccelerationData.state.e3d_state.box_max.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFABMaxY,
									 FieldAccelerationData.state.e3d_state.box_max.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFABMaxZ,
									 FieldAccelerationData.state.e3d_state.box_max.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFABWeight,
									 FieldAccelerationData.state.e3d_state.box_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)

// Sphere
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASCenterX,
									 FieldAccelerationData.state.e3d_state.sphere_center.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASCenterY,
									 FieldAccelerationData.state.e3d_state.sphere_center.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASCenterZ,
									 FieldAccelerationData.state.e3d_state.sphere_center.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASMin,
									 FieldAccelerationData.state.e3d_state.sphere_min,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASMax,
									 FieldAccelerationData.state.e3d_state.sphere_max,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASWeight,
									 FieldAccelerationData.state.e3d_state.sphere_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)

// Cone
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNOriginX,
									 FieldAccelerationData.state.e3d_state.cone_origin.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNOriginY,
									 FieldAccelerationData.state.e3d_state.cone_origin.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNOriginZ,
									 FieldAccelerationData.state.e3d_state.cone_origin.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNElev,
									 FieldAccelerationData.state.e3d_state.cone_elevation,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNAzim,
									 FieldAccelerationData.state.e3d_state.cone_azimuth,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNAper,
									 FieldAccelerationData.state.e3d_state.cone_aperture,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNMin,
									 FieldAccelerationData.state.e3d_state.cone_min,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNMax,
									 FieldAccelerationData.state.e3d_state.cone_max,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACNWeight,
									 FieldAccelerationData.state.e3d_state.cone_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)

// Cylinder
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYStartX,
									 FieldAccelerationData.state.e3d_state.cylinder_start.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYStartY,
									 FieldAccelerationData.state.e3d_state.cylinder_start.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYStartZ,
									 FieldAccelerationData.state.e3d_state.cylinder_start.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYEndX,
									 FieldAccelerationData.state.e3d_state.cylinder_end.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYEndY,
									 FieldAccelerationData.state.e3d_state.cylinder_end.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYEndZ,
									 FieldAccelerationData.state.e3d_state.cylinder_end.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYMin,
									 FieldAccelerationData.state.e3d_state.cylinder_min,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYMax,
									 FieldAccelerationData.state.e3d_state.cylinder_max,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYWeight,
									 FieldAccelerationData.state.e3d_state.cylinder_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)

// Sphere Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASTCenterX,
									 FieldAccelerationData.state.e3d_state.spheretangent_center.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASTCenterY,
									 FieldAccelerationData.state.e3d_state.spheretangent_center.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASTCenterZ,
									 FieldAccelerationData.state.e3d_state.spheretangent_center.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASTMin,
									 FieldAccelerationData.state.e3d_state.spheretangent_min,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASTMax,
									 FieldAccelerationData.state.e3d_state.spheretangent_max,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFASTWeight,
									 FieldAccelerationData.state.e3d_state.spheretangent_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)

// Cylinder Tangent
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTStartX,
									 FieldAccelerationData.state.e3d_state.cylindertangent_start.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTStartY,
									 FieldAccelerationData.state.e3d_state.cylindertangent_start.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTStartZ,
									 FieldAccelerationData.state.e3d_state.cylindertangent_start.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTEndX,
									 FieldAccelerationData.state.e3d_state.cylindertangent_end.x,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTEndY,
									 FieldAccelerationData.state.e3d_state.cylindertangent_end.y,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTEndZ,
									 FieldAccelerationData.state.e3d_state.cylindertangent_end.z,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTMin,
									 FieldAccelerationData.state.e3d_state.cylindertangent_min,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTMax,
									 FieldAccelerationData.state.e3d_state.cylindertangent_max,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)
IMPLEMENT_COMPONENT_TEXTBOX_TO_FLOAT(AwsSetFACYTWeight,
									 FieldAccelerationData.state.e3d_state.cylindertangent_weight,FieldAccelerationData.settings_changed,
									 UpdateFieldAccelStateDisplay)


bool PartEditSink::FieldAccelStateChanged()
{
  return FieldAccelerationData.settings_changed;
}

void PartEditSink::ClearFieldAccelStateChanged()
{
  FieldAccelerationData.settings_changed=false;
}

FieldState *PartEditSink::GetFieldAccelState()
{
  return &(FieldAccelerationData.state);
}

void PartEditSink::SetFieldAccelState(FieldState *source)
{
  memcpy(&(FieldAccelerationData.state),source,sizeof(FieldState));
  ClearFieldAccelStateChanged();
  UpdateFieldAccelStateDisplay();
}

void PartEditSink::UpdateFieldAccelStateDisplay()
{
  csRef<iString> value;
  bool bvalue;

  SET_CHECKBOX_BOOL(FieldAccelerationData.iawscomponent_FAActive,FieldAccelerationData.state.active);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FAFPX,FieldAccelerationData.state.e3d_state.fixed_position.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FAFPY,FieldAccelerationData.state.e3d_state.fixed_position.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FAFPZ,FieldAccelerationData.state.e3d_state.fixed_position.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FAFWeight,FieldAccelerationData.state.e3d_state.fixed_weight);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FALSX,FieldAccelerationData.state.e3d_state.line_start.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FALSY,FieldAccelerationData.state.e3d_state.line_start.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FALSZ,FieldAccelerationData.state.e3d_state.line_start.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FALEX,FieldAccelerationData.state.e3d_state.line_end.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FALEY,FieldAccelerationData.state.e3d_state.line_end.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FALEZ,FieldAccelerationData.state.e3d_state.line_end.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FALWeight,FieldAccelerationData.state.e3d_state.line_weight);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FABMX,FieldAccelerationData.state.e3d_state.box_min.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FABMY,FieldAccelerationData.state.e3d_state.box_min.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FABMZ,FieldAccelerationData.state.e3d_state.box_min.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FABXX,FieldAccelerationData.state.e3d_state.box_max.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FABXY,FieldAccelerationData.state.e3d_state.box_max.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FABXZ,FieldAccelerationData.state.e3d_state.box_max.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FABWeight,FieldAccelerationData.state.e3d_state.box_weight);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASCX,FieldAccelerationData.state.e3d_state.sphere_center.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASCY,FieldAccelerationData.state.e3d_state.sphere_center.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASCZ,FieldAccelerationData.state.e3d_state.sphere_center.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASMin,FieldAccelerationData.state.e3d_state.sphere_min);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASMax,FieldAccelerationData.state.e3d_state.sphere_max);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASWeight,FieldAccelerationData.state.e3d_state.sphere_weight);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNOX,FieldAccelerationData.state.e3d_state.cone_origin.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNOY,FieldAccelerationData.state.e3d_state.cone_origin.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNOZ,FieldAccelerationData.state.e3d_state.cone_origin.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNElevation,FieldAccelerationData.state.e3d_state.cone_elevation);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNAzimuth,FieldAccelerationData.state.e3d_state.cone_azimuth);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNAperture,FieldAccelerationData.state.e3d_state.cone_aperture);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNMin,FieldAccelerationData.state.e3d_state.cone_min);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNMax,FieldAccelerationData.state.e3d_state.cone_max);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACNWeight,FieldAccelerationData.state.e3d_state.cone_weight);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYSX,FieldAccelerationData.state.e3d_state.cylinder_start.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYSY,FieldAccelerationData.state.e3d_state.cylinder_start.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYSZ,FieldAccelerationData.state.e3d_state.cylinder_start.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYEX,FieldAccelerationData.state.e3d_state.cylinder_end.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYEY,FieldAccelerationData.state.e3d_state.cylinder_end.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYEZ,FieldAccelerationData.state.e3d_state.cylinder_end.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYMin,FieldAccelerationData.state.e3d_state.cylinder_min);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYMax,FieldAccelerationData.state.e3d_state.cylinder_max);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYWeight,FieldAccelerationData.state.e3d_state.cylinder_weight);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASTCX,FieldAccelerationData.state.e3d_state.spheretangent_center.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASTCY,FieldAccelerationData.state.e3d_state.spheretangent_center.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASTCZ,FieldAccelerationData.state.e3d_state.spheretangent_center.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASTMin,FieldAccelerationData.state.e3d_state.spheretangent_min);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASTMax,FieldAccelerationData.state.e3d_state.spheretangent_max);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FASTWeight,FieldAccelerationData.state.e3d_state.spheretangent_weight);

  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTSX,FieldAccelerationData.state.e3d_state.cylindertangent_start.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTSY,FieldAccelerationData.state.e3d_state.cylindertangent_start.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTSZ,FieldAccelerationData.state.e3d_state.cylindertangent_start.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTEX,FieldAccelerationData.state.e3d_state.cylindertangent_end.x);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTEY,FieldAccelerationData.state.e3d_state.cylindertangent_end.y);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTEZ,FieldAccelerationData.state.e3d_state.cylindertangent_end.z);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTMin,FieldAccelerationData.state.e3d_state.cylindertangent_min);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTMax,FieldAccelerationData.state.e3d_state.cylindertangent_max);
  SET_TEXTBOX_FLOAT(FieldAccelerationData.iawscomponent_FACYTWeight,FieldAccelerationData.state.e3d_state.cylindertangent_weight);


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


bool PartEditSink::AttractorStateChanged()
{
  return AttractorData.settings_changed;
}

void PartEditSink::ClearAttractorStateChanged()
{
  AttractorData.settings_changed=false;
}

AttractorState *PartEditSink::GetAttractorState()
{
  return &(AttractorData.state);
}

void PartEditSink::SetAttractorState(AttractorState *source)
{
  memcpy(&(AttractorData.state),source,sizeof(AttractorState));
  ClearAttractorStateChanged();
  UpdateAttractorStateDisplay();
}

void PartEditSink::UpdateAttractorStateDisplay()
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


IMPLEMENT_REGISTER_FUNCTION(RegisterFSWindow,FreeScrollData.iawscomponent_FSWindow)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSLabel,FreeScrollData.iawscomponent_FSLabel)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSTextBox,FreeScrollData.iawscomponent_FSTextBox)
IMPLEMENT_REGISTER_FUNCTION(RegisterFSScrollBar,FreeScrollData.iawscomponent_FSScrollBar)


////
//  "Free Scroll" test
////
void PartEditSink::FreeScrollSetComponent(bool floatval,intptr_t value_pointer,iAwsComponent *associated,bool *invalidate_pointer)
{
  csRef<iString> value;
  asink->FreeScrollData.iawscomponent_AssociatedTextBox=associated;
  asink->FreeScrollData.floatval=floatval;
  asink->FreeScrollData.value_pointer=value_pointer;
  asink->FreeScrollData.invalidate_pointer=invalidate_pointer;

  if (asink->FreeScrollData.floatval)
  {
    float min,max,change,bigchange;
    // Set textbox value
    SET_TEXTBOX_FLOAT(asink->FreeScrollData.iawscomponent_FSTextBox,*((float *)(value_pointer)));
    // Range guesswork.  We can take the current value and figure out a reasonable max
    float fval=*((float *)(value_pointer));
    if (fval<0)
      fval=-fval;

    min=0.0f;
    if (min==0.0f && fval==0.0f)
    {
      // 0 is a hard case to determine
      min=-100.0f;
      change=0.1f;
      bigchange=1;
    }
    if (min==0.0f && fval<=.0001)
    {
      min=-0.001f;
      change=0.000001f;
      bigchange=0.00001f;
    }
    if (min==0.0f && fval<=.001)
    {
      min=-0.01f;
      change=0.00001f;
      bigchange=0.0001f;
    }
    if (min==0.0f && fval<=.01)
    {
      min=-0.1f;
      change=0.0001f;
      bigchange=0.001f;
    }
    if (min==0.0f && fval<=.1)
    {
      min=-1.0f;
      change=0.001f;
      bigchange=0.01f;
    }
    if (min==0.0f && fval<=1)
    {
      min=-10.0f;
      change=0.01f;
      bigchange=0.1f;
    }
    if (min==0.0f && fval<=10)
    {
      min=-100.0f;
      change=0.1f;
      bigchange=1;
    }
    if (min==0.0f && fval<=500)
    {
      min=-1000.0f;
      change=1;
      bigchange=10;
    }
    if (min==0.0f && fval<=5000)
    {
      min=-10000.0f;
      change=10;
      bigchange=100;
    }
    if (min==0.0f)
    {
      min=-50000.0f;
      change=50;
      bigchange=500;
    }
    max=-min;


    // Set range
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Min",(intptr_t)&min);
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Max",(intptr_t)&max);
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Change",(intptr_t)&change);
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("BigChange",(intptr_t)&bigchange);
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Value",(intptr_t)&value_pointer);
  }
  else
  {
    float fvalue,min,max,change,bigchange;
    // Set textbox value
    SET_TEXTBOX_INT(asink->FreeScrollData.iawscomponent_FSTextBox,*((int *)(value_pointer)));
    // Range guesswork.  We can take the current value and figure out a reasonable max
    int ival=*((int *)(value_pointer));
    if (ival<0)
      ival=-ival;

    min=0.0f;
    if (min==0.0f && ival<500)
    {
      min=-1000.0f;
      change=1;
      bigchange=10;
    }
    if (min==0.0f && ival<5000)
    {
      min=-10000.0f;
      change=10;
      bigchange=100;
    }
    if (min==0.0f)
    {
      min=-50000.0f;
      change=50;
      bigchange=500;
    }
    max=-min;

    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Min",(intptr_t)&min);
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Max",(intptr_t)&max);
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Change",(intptr_t)&change);
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("BigChange",(intptr_t)&bigchange);
    fvalue=(float)(*((int *)(value_pointer)));
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Value",(intptr_t)&fvalue);
  }
}

void PartEditSink::AwsSetFSScrollBar(intptr_t sk, iAwsSource *source)
{
  csRef<iString> value;
  float *value_as_float;
  if (asink->FreeScrollData.iawscomponent_AssociatedTextBox==0)
    return;

  if (source->GetComponent()->GetProperty("Value",(intptr_t*)&value_as_float))
  {
    if (asink->FreeScrollData.floatval)
    {
      // This is a float
      *((float *)(asink->FreeScrollData.value_pointer))=*value_as_float;
      SET_TEXTBOX_FLOAT(asink->FreeScrollData.iawscomponent_FSTextBox,*value_as_float);
      SET_TEXTBOX_FLOAT(asink->FreeScrollData.iawscomponent_AssociatedTextBox,*value_as_float);
    }
    else
    {
      // This is an int
      *((int *)(asink->FreeScrollData.value_pointer))=(int)(*value_as_float);
      SET_TEXTBOX_INT(asink->FreeScrollData.iawscomponent_FSTextBox,(int)(*value_as_float));
      SET_TEXTBOX_INT(asink->FreeScrollData.iawscomponent_AssociatedTextBox,(int)(*value_as_float));
    }
    *(asink->FreeScrollData.invalidate_pointer)=true;

  }
}

void PartEditSink::AwsSetFSTextBox(intptr_t sk, iAwsSource *source)
{
  iString *textvalue;
  csRef<iString> value;
  float value_as_float;
  if (asink->FreeScrollData.iawscomponent_AssociatedTextBox==0)
    return;

  if (source->GetComponent()->GetProperty("Text",(intptr_t*)&textvalue) && textvalue->Length())
  {
    if (asink->FreeScrollData.floatval)
    {
      // This is a float
      *((float *)(asink->FreeScrollData.value_pointer))=atof(textvalue->GetData());
      value_as_float=*((float *)(asink->FreeScrollData.value_pointer));
      SET_TEXTBOX_FLOAT(asink->FreeScrollData.iawscomponent_AssociatedTextBox,value_as_float);
    }
    else
    {
      // This is an int
      *((int *)(asink->FreeScrollData.value_pointer))=strtol(textvalue->GetData(),0,10);
      value_as_float=(float)(*((int *)(asink->FreeScrollData.value_pointer)));
      SET_TEXTBOX_INT(asink->FreeScrollData.iawscomponent_AssociatedTextBox,(int)value_as_float);
    }
    *(asink->FreeScrollData.invalidate_pointer)=true;

    // Update the scroll bar
    asink->FreeScrollData.iawscomponent_FSScrollBar->SetProperty("Value",(intptr_t)&value_as_float);
  }
}


IMPLEMENT_REGISTER_FUNCTION(RegisterLoadSave,LoadSaveData.iawscomponent_LoadSave)
IMPLEMENT_REGISTER_FUNCTION(RegisterAgingMoments,AgingMomentsData.iawscomponent_AgingMoments)



void PartEditSink::SetVFS(csRef<iVFS> newvfs)
{
  vfs=newvfs;
}

csRef<iVFS> PartEditSink::GetVFS()
{
  return vfs;
}
