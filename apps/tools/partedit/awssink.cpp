#include "cssysdef.h"
#include "iutil/vfs.h"
#include "csutil/scfstr.h"
#include "csutil/csstring.h"
#include "iengine/engine.h"
#include "awssink.h"

#include "iaws/awsparm.h"

#include <stdio.h>


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
    case SECTION_INIT_SPEED:
    case SECTION_INIT_ACCELERATION:
    case SECTION_FIELD_SPEED:
    case SECTION_FIELD_ACCELERATION:
    case SECTION_ATTRACTOR:
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
void awsSink::RegisterGraphicSelection(void *sk, iAwsSource *source)
{
  asink->GraphicSelectionData.iawscomponent_GraphicSelection=source->GetComponent();
}

void awsSink::RegisterGraphicFilter(void *sk, iAwsSource *source)
{
	asink->GraphicSelectionData.iawscomponent_GraphicFilter = source->GetComponent();
}

void awsSink::RegisterGraphicFileList(void *sk, iAwsSource *source)
{
	asink->GraphicSelectionData.iawscomponent_GraphicFileList = source->GetComponent();
}





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
void awsSink::RegisterEmitterState(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_EmitterState=source->GetComponent();
}
void awsSink::RegisterParticleCount(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_ParticleCount=source->GetComponent();
}
void awsSink::RegisterParticleMaxAge(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_ParticleMaxAge=source->GetComponent();
}
void awsSink::RegisterLighting(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_Lighting=source->GetComponent();
}
void awsSink::RegisterAlphaBlend(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_AlphaBlend=source->GetComponent();
}
void awsSink::RegisterRectParticlesRadio(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_RectParticlesRadio=source->GetComponent();
}
void awsSink::RegisterRegParticlesRadio(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_RegParticlesRadio=source->GetComponent();
}
void awsSink::RegisterRectParticlesWidth(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_RectParticlesWidth=source->GetComponent();
}
void awsSink::RegisterRectParticlesHeight(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_RectParticlesHeight=source->GetComponent();
}
void awsSink::RegisterRegParticlesNumber(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_RegParticlesNumber=source->GetComponent();
}
void awsSink::RegisterRegParticlesRadius(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_RegParticlesRadius=source->GetComponent();
}
void awsSink::RegisterUseBoundingBox(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_UseBoundingBox=source->GetComponent();
}
void awsSink::RegisterBBoxMinX(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_BBoxMinX=source->GetComponent();
}
void awsSink::RegisterBBoxMinY(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_BBoxMinY=source->GetComponent();
}
void awsSink::RegisterBBoxMinZ(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_BBoxMinZ=source->GetComponent();
}
void awsSink::RegisterBBoxMaxX(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_BBoxMaxX=source->GetComponent();
}
void awsSink::RegisterBBoxMaxY(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_BBoxMaxY=source->GetComponent();
}
void awsSink::RegisterBBoxMaxZ(void *sk, iAwsSource *source)
{
  asink->EmitterStateData.iawscomponent_BBoxMaxZ=source->GetComponent();
}

void awsSink::AwsSetParticleCount(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_ParticleCount->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.particle_count=strtol(textvalue->GetData(),NULL,10);
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}

void awsSink::AwsSetParticleMaxAge(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_ParticleMaxAge->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.particle_max_age=strtol(textvalue->GetData(),NULL,10);
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

void awsSink::AwsSetLighting(void *sk, iAwsSource *source)
{
  bool *p_bvalue;
  if (asink->EmitterStateData.iawscomponent_Lighting->GetProperty("State",(void **)&p_bvalue))
  {
    if (*p_bvalue)
      asink->EmitterStateData.state.lighting=true;
    else
      asink->EmitterStateData.state.lighting=false;
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

void awsSink::AwsSetAlphaBlend(void *sk, iAwsSource *source)
{
  bool *p_bvalue;
  if (asink->EmitterStateData.iawscomponent_AlphaBlend->GetProperty("State",(void **)&p_bvalue))
  {
    if (*p_bvalue)
      asink->EmitterStateData.state.alpha_blend=true;
    else
      asink->EmitterStateData.state.alpha_blend=false;
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

void awsSink::AwsSetParticleType(void *sk, iAwsSource *source)
{
  bool *p_bvalue;
  if (asink->EmitterStateData.iawscomponent_RectParticlesRadio->GetProperty("State",(void **)&p_bvalue))
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

void awsSink::AwsSetRectangularWidth(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_RectParticlesWidth->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.rect_w=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

void awsSink::AwsSetRectangularHeight(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_RectParticlesHeight->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.rect_h=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

void awsSink::AwsSetRegularNumber(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_RegParticlesNumber->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.reg_number=strtol(textvalue->GetData(),NULL,10);
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}

void awsSink::AwsSetRegularRadius(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_RegParticlesRadius->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.reg_radius=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}

void awsSink::AwsSetUseBoundingBox(void *sk, iAwsSource *source)
{
  bool *p_bvalue;
  if (asink->EmitterStateData.iawscomponent_UseBoundingBox->GetProperty("State",(void **)&p_bvalue))
  {
    if (*p_bvalue)
      asink->EmitterStateData.state.using_bounding_box=true;
    else
      asink->EmitterStateData.state.using_bounding_box=false;
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

void awsSink::AwsSetBBoxMinX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_BBoxMinX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.bbox_minx=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}

void awsSink::AwsSetBBoxMinY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_BBoxMinY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.bbox_miny=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();
}

void awsSink::AwsSetBBoxMinZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_BBoxMinZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.bbox_minz=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}

void awsSink::AwsSetBBoxMaxX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_BBoxMaxX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.bbox_maxx=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}

void awsSink::AwsSetBBoxMaxY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_BBoxMaxY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.bbox_maxy=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}

void awsSink::AwsSetBBoxMaxZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->EmitterStateData.iawscomponent_BBoxMaxZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->EmitterStateData.state.bbox_maxz=atof(textvalue->GetData());
    asink->EmitterStateData.settings_changed=true;
  }
  else
    asink->UpdateEmitterStateDisplay();

}



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
  value=new scfString();
  value->Format("%d",EmitterStateData.state.particle_count);
  EmitterStateData.iawscomponent_ParticleCount->SetProperty("Text",value);

  value=new scfString();
  value->Format("%d",EmitterStateData.state.particle_max_age);
  EmitterStateData.iawscomponent_ParticleMaxAge->SetProperty("Text",value);

  if (EmitterStateData.state.lighting)
  {
    bvalue=true;
    EmitterStateData.iawscomponent_Lighting->SetProperty("State",&bvalue);
  }
  else
  {
    bvalue=false;
    EmitterStateData.iawscomponent_Lighting->SetProperty("State",&bvalue);
  }
  if (EmitterStateData.state.alpha_blend)
  {
    bvalue=true;
    EmitterStateData.iawscomponent_AlphaBlend->SetProperty("State",&bvalue);
  }
  else
  {
    bvalue=false;
    EmitterStateData.iawscomponent_AlphaBlend->SetProperty("State",&bvalue);
  }
  if (EmitterStateData.state.rectangular_particles)
  {
    bvalue=true;
    EmitterStateData.iawscomponent_RectParticlesRadio->SetProperty("State",&bvalue);
    bvalue=false;
    EmitterStateData.iawscomponent_RegParticlesRadio->SetProperty("State",&bvalue);
  }
  else
  {
    bvalue=true;
    EmitterStateData.iawscomponent_RegParticlesRadio->SetProperty("State",&bvalue);
    bvalue=false;
    EmitterStateData.iawscomponent_RectParticlesRadio->SetProperty("State",&bvalue);
  }
  if (EmitterStateData.state.using_bounding_box)
  {
    bvalue=true;
    EmitterStateData.iawscomponent_UseBoundingBox->SetProperty("State",&bvalue);
  }
  else
  {
    bvalue=false;
    EmitterStateData.iawscomponent_UseBoundingBox->SetProperty("State",&bvalue);
  }

  value=new scfString();
  value->Format("%f",EmitterStateData.state.rect_w);
  EmitterStateData.iawscomponent_RectParticlesWidth->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.rect_h);
  EmitterStateData.iawscomponent_RectParticlesHeight->SetProperty("Text",value);

  value=new scfString();
  value->Format("%d",EmitterStateData.state.reg_number);
  EmitterStateData.iawscomponent_RegParticlesNumber->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.reg_radius);
  EmitterStateData.iawscomponent_RegParticlesRadius->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.bbox_minx);
  EmitterStateData.iawscomponent_BBoxMinX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.bbox_miny);
  EmitterStateData.iawscomponent_BBoxMinY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.bbox_minz);
  EmitterStateData.iawscomponent_BBoxMinZ->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.bbox_maxx);
  EmitterStateData.iawscomponent_BBoxMaxX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.bbox_maxy);
  EmitterStateData.iawscomponent_BBoxMaxY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",EmitterStateData.state.bbox_maxz);
  EmitterStateData.iawscomponent_BBoxMaxZ->SetProperty("Text",value);

}




////
//  Initial Position Display
////
void awsSink::RegisterInitialPosition(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_InitialPosition = source->GetComponent();
}
void awsSink::RegisterIPFPX(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPFPX = source->GetComponent();
}
void awsSink::RegisterIPFPY(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPFPY = source->GetComponent();
}
void awsSink::RegisterIPFPZ(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPFPZ = source->GetComponent();
}
void awsSink::RegisterIPFWeight(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPFWeight = source->GetComponent();
}
void awsSink::RegisterIPLSX(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPLSX = source->GetComponent();
}
void awsSink::RegisterIPLSY(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPLSY = source->GetComponent();
}
void awsSink::RegisterIPLSZ(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPLSZ = source->GetComponent();
}
void awsSink::RegisterIPLEX(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPLEX = source->GetComponent();
}
void awsSink::RegisterIPLEY(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPLEY = source->GetComponent();
}
void awsSink::RegisterIPLEZ(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPLEZ = source->GetComponent();
}
void awsSink::RegisterIPLWeight(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPLWeight = source->GetComponent();
}
void awsSink::RegisterIPBMX(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPBMX = source->GetComponent();
}
void awsSink::RegisterIPBMY(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPBMY = source->GetComponent();
}
void awsSink::RegisterIPBMZ(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPBMZ = source->GetComponent();
}
void awsSink::RegisterIPBXX(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPBXX = source->GetComponent();
}
void awsSink::RegisterIPBXY(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPBXY = source->GetComponent();
}
void awsSink::RegisterIPBXZ(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPBXZ = source->GetComponent();
}
void awsSink::RegisterIPBWeight(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPBWeight = source->GetComponent();
}
void awsSink::RegisterIPSCX(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPSCX = source->GetComponent();
}
void awsSink::RegisterIPSCY(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPSCY = source->GetComponent();
}
void awsSink::RegisterIPSCZ(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPSCZ = source->GetComponent();
}
void awsSink::RegisterIPSMin(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPSMin = source->GetComponent();
}
void awsSink::RegisterIPSMax(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPSMax = source->GetComponent();
}
void awsSink::RegisterIPSWeight(void *sk, iAwsSource *source)
{
  asink->InitialPositionData.iawscomponent_IPSWeight = source->GetComponent();
}





void awsSink::AwsSetIPFPositionX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPFPX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.fixed_position.x=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPFPositionY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPFPY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.fixed_position.y=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPFPositionZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPFPZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.fixed_position.z=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPFWeight(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPFWeight->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.fixed_weight=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPLStartX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPLSX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.line_start.x=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();

}
void awsSink::AwsSetIPLStartY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPLSY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.line_start.y=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPLStartZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPLSZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.line_start.z=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPLEndX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPLEX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.line_end.x=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPLEndY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPLEY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.line_end.y=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPLEndZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPLEZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.line_end.z=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPLWeight(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPLWeight->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.line_weight=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPBMinX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPBMX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.box_min.x=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPBMinY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPBMY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.box_min.y=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPBMinZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPBMZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.box_min.z=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPBMaxX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPBXX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.box_max.x=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPBMaxY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPBXY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.box_max.y=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPBMaxZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPBXZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.box_max.z=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPBWeight(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPBWeight->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.box_weight=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPSCenterX(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPSCX->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.sphere_center.x=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPSCenterY(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPSCY->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.sphere_center.y=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPSCenterZ(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPSCZ->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.sphere_center.z=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPSMin(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPSMin->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.sphere_min=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPSMax(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPSMax->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.sphere_max=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}
void awsSink::AwsSetIPSWeight(void *sk, iAwsSource *source)
{
  iString *textvalue;
  if (asink->InitialPositionData.iawscomponent_IPSWeight->GetProperty("Text",(void **)&textvalue) && textvalue->Length())
  {
    asink->InitialPositionData.state.sphere_weight=atof(textvalue->GetData());
    asink->InitialPositionData.settings_changed=true;
  }
  else
    asink->UpdateInitialPositionStateDisplay();
}


















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

  value=new scfString();
  value->Format("%f",InitialPositionData.state.fixed_position.x);
  InitialPositionData.iawscomponent_IPFPX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.fixed_position.y);
  InitialPositionData.iawscomponent_IPFPY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.fixed_position.z);
  InitialPositionData.iawscomponent_IPFPZ->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.fixed_weight);
  InitialPositionData.iawscomponent_IPFWeight->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.line_start.x);
  InitialPositionData.iawscomponent_IPLSX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.line_start.y);
  InitialPositionData.iawscomponent_IPLSY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.line_start.z);
  InitialPositionData.iawscomponent_IPLSZ->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.line_end.x);
  InitialPositionData.iawscomponent_IPLEX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.line_end.y);
  InitialPositionData.iawscomponent_IPLEY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.line_end.z);
  InitialPositionData.iawscomponent_IPLEZ->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.line_weight);
  InitialPositionData.iawscomponent_IPLWeight->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.box_min.x);
  InitialPositionData.iawscomponent_IPBMX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.box_min.y);
  InitialPositionData.iawscomponent_IPBMY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.box_min.z);
  InitialPositionData.iawscomponent_IPBMZ->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.box_max.x);
  InitialPositionData.iawscomponent_IPBXX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.box_max.y);
  InitialPositionData.iawscomponent_IPBXY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.box_max.z);
  InitialPositionData.iawscomponent_IPBXZ->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.box_weight);
  InitialPositionData.iawscomponent_IPBWeight->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.sphere_center.x);
  InitialPositionData.iawscomponent_IPSCX->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.sphere_center.y);
  InitialPositionData.iawscomponent_IPSCY->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.sphere_center.z);
  InitialPositionData.iawscomponent_IPSCZ->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.sphere_min);
  InitialPositionData.iawscomponent_IPSMin->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.sphere_max);
  InitialPositionData.iawscomponent_IPSMax->SetProperty("Text",value);

  value=new scfString();
  value->Format("%f",InitialPositionData.state.sphere_weight);
  InitialPositionData.iawscomponent_IPSWeight->SetProperty("Text",value);


}


















void awsSink::SetVFS(csRef<iVFS> newvfs)
{
  vfs=newvfs;
}

csRef<iVFS> awsSink::GetVFS()
{
  return vfs;
}








