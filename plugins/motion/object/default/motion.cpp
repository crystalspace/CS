/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
#include "motion.h"
#include "isys/system.h"
#include "csgeom/transfrm.h"
#include "csutil/hashmap.h"

//#define MOTION_DEBUG

IMPLEMENT_IBASE (csMotionManager)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iMotionManager)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csMotionManager)

EXPORT_CLASS_TABLE (motion)
  EXPORT_CLASS_DEP (csMotionManager, "crystalspace.motion.manager.default",
    "Skeletal Motion Manager for Crystal Space", NULL)
EXPORT_CLASS_TABLE_END

csMotionManager::csMotionManager(iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
  oldtime=0;
  iSys=NULL;
}

csMotionManager::~csMotionManager()
{
}

bool csMotionManager::Initialize (iSystem* TiSys)
{
  iSys=TiSys;
  return true;
}

csMotion* csMotionManager::FindClassByName (const char* name)
{
  int index=motions.FindSortedKey(name);
  if(index==-1)
    return NULL;
  return motions.Get(index);
}

iMotion* csMotionManager::AddMotion (const char* name)
{
#ifdef MOTION_DEBUG
  printf("AddMotion(%s)\n", name);
#endif
  csMotion* mot=new csMotion();
  mot->SetName(name);
  motions.InsertSorted(mot);
  return mot;
}

//TODO Azverkan use a sorted array
bool csMotionManager::ApplyMotion(iSkeletonBone *skel, const char* motion)
{
  csMotion* newmotion=FindClassByName((const char*)csHashCompute(motion));
  if(!newmotion) return false;

  int size=skels.Length();
  int i=0;
  for(; i<size; i++)
  {
    if( ((iSkeletonBone*)(skels[i]->skel)) == skel)
      break;
  }
  csAppliedMotion *am=NULL;
  if(i==size)
  {
    am=new csAppliedMotion();
    skels.Push(am);
  }
  else
    am=skels[i];
  am->curframe=NULL;
  am->skel=skel;
  am->curmotion=newmotion;
  am->curtime=0;
  return true;
}

void csMotionManager::UpdateTransform (csAppliedMotion *am, iSkeletonBone *bone, int link1, int link2)
{
  
  csVector3 vec = bone->GetTransformation().GetO2TTranslation();
  if (am->curmotion->matrixmode==0)
  {
    csQuaternion quat;
    csQuaternion &quat1=((csQuaternion*)am->curmotion->transforms)[link1];
    if(am->nextframe)
    {
      csQuaternion &quat2=((csQuaternion*)am->curmotion->transforms)[link2];
      float dist1=(float)(am->curtime)-(float)(am->curframe->keyframe);
      float dist2=(float)(am->nextframe->keyframe)-(float)(am->curframe->keyframe);
      float ratio = dist1 / dist2;

#ifdef MOTION_DEBUG
      printf("Slerp Q(%g,%g,%g,%g) Q(%g,%g,%g,%g) %g\n", quat1.x, quat1.y, quat1.z, quat1.r, quat2.x, quat2.y, quat2.z, quat2.r, ratio);
#endif

      quat=quat1.Slerp(quat2, ratio);
    }
    else
      quat=quat1;

#ifdef MOTION_DEBUG
    printf("UpdateTransform Q(%g,%g,%g,%g)\n", quat.x, quat.y, quat.z, quat.r);
#endif
    bone->SetTransformation(csTransform(csMatrix3(quat), vec));
  }
  else if (am->curmotion->matrixmode==1)
  {
    csMatrix3 &mat=((csMatrix3*)am->curmotion->transforms)[link1];
    bone->SetTransformation(csTransform(mat, vec));
  }
}

bool csMotionManager::UpdateBone(csAppliedMotion *am, iSkeletonBone *bone, unsigned int hash)
{
  int link1=-1;
  int link2=-1;

  int size=am->curframe->size;
  for(int i=0; i<size; i++)
  {
    if(am->curframe->affectors[i]==hash)
    {
      link1=am->curframe->links[i];
      break;
    }
  }
  if(link1<0)
  {
#ifdef MOTION_DEBUG
    printf("UpdateBone() link1 fail\n");
#endif			
    return false;
  }

  if(am->nextframe)
  {
    int size=am->nextframe->size;
    for(int i=0; i<size; i++)
    {
      if(am->nextframe->affectors[i]==hash)
      {
        link2=am->nextframe->links[i];
        break;
      }
    }
    if(link2<0)
    {
#ifdef MOTION_DEBUG
      printf("UpdateBone() link2 fail\n");
#endif			
      return false;
    }
  }

  UpdateTransform(am, bone, link1, link2);
  return true;
}

void csMotionManager::UpdateAppliedBones(csAppliedMotion *am, iSkeletonBone *bone)
{
  const char* name=bone->GetName();
  if(name)
  {
    unsigned int hash=csHashCompute(name);
    UpdateBone(am, bone, hash);
  }
  iSkeletonBone *child=bone->GetChildren();
  while(child)
  {
    UpdateAppliedBones(am, child);
    child=child->GetNext();
  }
}

//TODO Azverkan support frame interpolation & make looping optional
bool csMotionManager::UpdateAppliedMotion(csAppliedMotion *am, cs_time elapsedtime)
{
  am->curtime+=elapsedtime;

  int size=am->curmotion->numframes;

  //Check to see if motion has looped
  CS_ASSERT(am->curmotion->frames[size-1].keyframe>0);

  while(am->curmotion->frames[size-1].keyframe < am->curtime)
  {
    am->curtime -= am->curmotion->frames[size-1].keyframe;
  }

  am->nextframe=NULL;
  for(int i=0; i<size; i++)
  {
    if(am->curmotion->frames[i].keyframe==am->curtime)
    {
      am->curframe=&am->curmotion->frames[i];
      break;
    }
    else if (am->curmotion->frames[i].keyframe>am->curtime)
    {
      if(i!=0)
      {
	am->curframe=&am->curmotion->frames[i-1];
	am->nextframe=&am->curmotion->frames[i];
      }
      else
      {
	am->curframe=&am->curmotion->frames[i];
      }
      break;
    }
  }

  if (!am->curframe) return false;

#ifdef MOTION_DEBUG
  printf("UpdateAppliedMotion %d %d\n", am->curtime, am->curframe->keyframe); 
#endif

  UpdateAppliedBones(am, am->skel);
  return true;
}

void csMotionManager::UpdateAll()
{
  cs_time newtime=iSys->GetTime();
  if(oldtime==0)
  {
    //Handle first run
    oldtime=newtime;
  }
  cs_time elapsedtime=newtime-oldtime;
  oldtime=newtime;

  int size=skels.Length();
  for (int i=0; i<size; i++)
  {
    UpdateAppliedMotion(skels[i], elapsedtime);
  }
}

IMPLEMENT_IBASE (csMotion)
  IMPLEMENTS_INTERFACE (iMotion)
IMPLEMENT_IBASE_END

csMotion::csMotion()
{
  CONSTRUCT_IBASE (NULL);
  name=NULL;
  matrixmode=-1;
  hash=0;
  transforms=NULL;
  numtransforms=0;
  frames=NULL;
  numframes=0;
}

csMotion::~csMotion()
{
  if (name) free (name);
  if (transforms) free (transforms);
  if (frames)
  {
    for (int i = 0; i < numframes; i++)
    {
      if (frames[i].size)
      {
        free (frames[i].links);
        free (frames[i].affectors);
      }
    }
    free (frames);
  }
}

void csMotion::SetName (const char* newname)
{
  if (name) free (name);
  name = strdup (newname);
  hash = csHashCompute (name);
}

const char* csMotion::GetName()
{
  return name;
}

bool csMotion::AddAnim (const csQuaternion &quat)
{
#ifdef MOTION_DEBUG
	printf("AddAnim(%g, %g, %g, %g)\n", quat.x, quat.y, quat.z, quat.r);
#endif

  if (matrixmode==-1)
    matrixmode=0;
  else if (matrixmode==1)
    return false;

  if (!transforms)
    transforms=malloc(sizeof(csQuaternion));
  else
    transforms=realloc(transforms, sizeof(csQuaternion)*(numtransforms+1));

  ((csQuaternion*)transforms)[numtransforms]=quat;
  numtransforms++;
  return true;
}

bool csMotion::AddAnim (const csMatrix3 &mat)
{
  if (matrixmode==-1)
    matrixmode=1;
  else if (matrixmode==0)
    return false;

  if (!transforms)
    transforms=malloc(sizeof(csMatrix3));
  else
    transforms=realloc(transforms, sizeof(csMatrix3)*(numtransforms+1));

  ((csMatrix3*)transforms)[numtransforms]=mat;
  numtransforms++;
  return true;
}

int csMotion::AddFrame (int framenumber)
{
#ifdef MOTION_DEBUG
	printf("AddFrame(%d) %d\n", framenumber, numframes);
#endif

  if (!frames)
    frames=(csMotionFrame*)malloc(sizeof(csMotionFrame));
  else
    frames=(csMotionFrame*)realloc(frames,sizeof(csMotionFrame)*(numframes+1));
  frames[numframes].keyframe=framenumber;
  frames[numframes].size=0;
  numframes++;
  return numframes-1;
}

#include <signal.h>

void csMotion::AddFrameLink (int frameindex, const char* affector, int link)
{
#ifdef MOTION_DEBUG
	printf("AddFrameLink(%d, '%s', %d)\n", frameindex, affector, link);
#endif

  CS_ASSERT(frameindex>=0);
  CS_ASSERT(frameindex<numframes);

  csMotionFrame *mf=&frames[frameindex];
  if (!mf->size)
  {
    mf->links=(int*)malloc(sizeof(int));
    mf->affectors=(unsigned int*)malloc(sizeof(unsigned int));
  }
  else
  {
    mf->links=(int*)realloc(mf->links, sizeof(int)*(mf->size+1));
    mf->affectors=
      (unsigned int*)realloc(mf->affectors, sizeof(unsigned int)*(mf->size+1));
  }
  mf->links[mf->size]=link;
  mf->affectors[mf->size]=csHashCompute(affector);
  mf->size++;
}

