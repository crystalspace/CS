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
#include "cssys/sysfunc.h"
#include "motion.h"
#include "csgeom/transfrm.h"
#include "csutil/hashmap.h"

//#define MOTION_DEBUG

#ifdef MOTION_DEBUG
#define MOT_DPRINTF(arg) printf arg
#else
#define MOT_DPRINTF(arg)
#endif

CS_IMPLEMENT_PLATFORM_PLUGIN

SCF_IMPLEMENT_IBASE (csMotionManager)
  SCF_IMPLEMENTS_INTERFACE (iMotionManager)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csMotionManager::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csMotionManager)

SCF_EXPORT_CLASS_TABLE (motion)
  SCF_EXPORT_CLASS_DEP (csMotionManager, "crystalspace.motion.manager.default",
    "Skeletal Motion Manager for Crystal Space", NULL)
SCF_EXPORT_CLASS_TABLE_END

SCF_IMPLEMENT_IBASE (csMotionTemplate)
  SCF_IMPLEMENTS_INTERFACE (iMotionTemplate)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (csMotionController)
  SCF_IMPLEMENTS_INTERFACE (iMotionController)
SCF_IMPLEMENT_IBASE_END

//************************************************************ Motion

csMotionTemplate::csMotionTemplate()
{
  SCF_CONSTRUCT_IBASE (NULL);
  name = NULL;
  hash = 0;
  duration = 0.0f;
  loopcount = 0;
  loopflip = 0;
}

csMotionTemplate::~csMotionTemplate()
{
  if (name) free (name);
}

void csMotionTemplate::SetName (const char* newname)
{
  if (name) free (name);
  name = strdup (newname);
  hash = csHashCompute (name);
}

const char* csMotionTemplate::GetName()
{
  return name;
}

void csMotionTemplate::SetDuration (float newduration)
{
  MOT_DPRINTF(("SetDuration %f\n", newduration));
  duration=newduration;
}

void csMotionTemplate::SetLoopCount (int newloopcount)
{
  MOT_DPRINTF(("SetLoopCount %d\n", newloopcount));
  loopcount=newloopcount;
}

void csMotionTemplate::SetLoopFlip (bool enable) {
  MOT_DPRINTF(("SetLoopFlip %d\n", enable));
  loopflip=enable;
}

int csMotionTemplate::AddBone (const char* name)
{
  MOT_DPRINTF(("AddBone '%s'\n", name));

  csMotionBone *bone=new csMotionBone;
  bone->name = strdup (name);
  bone->hash = csHashCompute (name);
  return bones.InsertSorted(bone);
}

int csMotionTemplate::FindBoneByName (const char* name) {
  return bones.FindSortedKey((void*)csHashCompute(name));
}

//TODO Make sure bones are sorted by time
void csMotionTemplate::AddFrameBone (int boneid, float frametime, const csVector3 &v, const csQuaternion &q)
{
  MOT_DPRINTF(("AddFrameBone %d %f\n", boneid, frametime));

  csMotionBone *bone=bones[boneid];
  if(!bone->frames) {
    bone->frames=(csMotionFrame*)malloc(sizeof(csMotionFrame));
    bone->framecount=1;
  } else {
    bone->framecount++;
    bone->frames=(csMotionFrame*)realloc(bone->frames, sizeof(csMotionFrame)*bone->framecount);
  }

  csMotionFrame *frame=&bone->frames[bone->framecount-1];
  frame->frametime=frametime;
  frame->pos=v;
  frame->rot=q;
}

//************************************************************ Motion Bone

void csMotionBone::SelectFrameForTime(float time, float *weight, int *curframe, int *nextframe)
{
  assert(weight);
  assert(curframe);
  assert(nextframe);

  int i=0;
  for(; i<framecount-1; i++) {
    if(frames[i+1].frametime>time)
      break;
  }

  //No Lerp needed
  if(frames[i].frametime==time) {
    *weight=1.0f;
    *curframe=i;
    *nextframe=-1;
    return;
  }

  //Lerp needed
  int ni=(i+1<framecount)?i+1:0; //TODO need to support LOOPFLIP()
  float cur=time-frames[i].frametime;
  float end=frames[ni].frametime-frames[i].frametime;
  *weight=cur/end;
  *curframe=i;
  *nextframe=ni;
}

void csMotionBone::Animate(float time, csVector3 &v, csQuaternion &q, bool interpolate)
{
  float frameweight;
  int curframe, nextframe;
  SelectFrameForTime(time, &frameweight, &curframe, &nextframe);
  if(nextframe>=0 && interpolate) {
    q=frames[curframe].rot.Slerp(frames[nextframe].rot, frameweight);
    v=(1.0f-frameweight)*frames[curframe].pos + frameweight*frames[nextframe].pos;
  } else {
    q=frames[curframe].rot;
    v=frames[curframe].pos;
  }
}

//************************************************************ Motion Controller

csMotionController::csMotionController(iSkeletonBone *Tskel)
{
  SCF_CONSTRUCT_IBASE (NULL);
  paused=0;
  stackchanged=0;
  skel=Tskel;
  bonecache=NULL;
  bonecachesize=0;
  bonecachelimit=0;
}

csMotionController::~csMotionController()
{
  stack.DeleteAll();
  if(bonecache) free(bonecache);
}

void csMotionController::SetMotion(iMotionTemplate *motion)
{
  stack.DeleteAll();
  stack.Push(new csMotionStackItem((csMotionTemplate*)motion));
  stackchanged=1;
}

void csMotionController::BlendMotion(iMotionTemplate *motion)
{
  stack.Push(new csMotionStackItem((csMotionTemplate*)motion));
  stackchanged=1;
}

void csMotionController::Pause(bool enable)
{
  paused=enable;
}

void csMotionController::Update(float timedelta)
{
  if(paused)
    return;

  //Update the time of the anims and delete expired motions from stack
  for(int i=0; i<stack.Length(); i++) {
    if(!stack[i]->Update(timedelta)) {
      stack.Delete(i--);
      stackchanged=1;
    }
  }

  if(stackchanged) {
    RecalculateBoneCache();
  }

  //TODO At this point we could hardware accelerate the motion
  Animate();
}

iSkeletonBone *csFindBone ( iSkeletonBone *bone, unsigned int hash )
{
  const char *name = bone->GetName();
  if (name)
  {
	unsigned int newhash = csHashCompute( name );
//        MOT_DPRINTF(("csFindBone '%s' %d %d\n", name, newhash, hash));
	if ( newhash == hash ) return bone;
  }
  iSkeletonBone* child = bone->GetChildren();
  while ( child )
  {
	iSkeletonBone *newbone = csFindBone ( child, hash );
	if (newbone) return newbone;
	else child = child->GetNext();
  }
  return NULL;
}

#define BONE_CACHE_ADD_SIZE 64
bool csMotionController::AddToBoneCache(unsigned int hash, int stack, int boneid)
{
  iSkeletonBone* bone=csFindBone(skel, hash);
  if(!bone) {
    return false;
  }

  for(int i=0; i<bonecachesize; i++) {
    if(bonecache[i].hash==hash) {
      int idx=bonecache[i].nummotions;
      if(idx>=MAX_MOTION_PER_BONE)
        return false;
      bonecache[i].bone=bone;
      bonecache[i].stacks[idx]=stack;
      bonecache[i].boneids[idx]=boneid;
      bonecache[i].nummotions++;
      return true;
    }
  }

  if(bonecachesize>=bonecachelimit || !bonecache) {
    bonecachelimit+=BONE_CACHE_ADD_SIZE;
    bonecache=(csMotionBoneCacheItem*)realloc(bonecache, bonecachelimit*sizeof(csMotionBoneCacheItem));
  }

  bonecache[bonecachesize].hash=hash;
  bonecache[bonecachesize].bone=bone;
  bonecache[bonecachesize].stacks[0]=stack;
  bonecache[bonecachesize].boneids[0]=boneid;
  bonecache[bonecachesize++].nummotions=1;
  return true;
}

void csMotionController::RecalculateBoneCache()
{
  bonecachesize=0;
  for(int i=0; i<stack.Length(); i++) {
    csMotionBoneVector& bones=stack[i]->motion->bones;
    for(int j=0; j<bones.Length(); j++) {
      if(!AddToBoneCache(bones[j]->GetHash(), i, j)) {
        MOT_DPRINTF(("RecalculateBoneCache missing '%s'\n", bones[j]->GetName()));
      }
    }
  }
  stackchanged=0;
}

void csMotionController::Animate()
{
  csVector3 sv;
  csQuaternion sq;

  for(int i=0; i<bonecachesize; i++) {
    csMotionBoneCacheItem* bone=&bonecache[i];
    if(!bone->nummotions)
      continue;

    csVector3 v;
    csQuaternion q;

    csMotionStackItem *si=stack[bone->stacks[0]];
    csMotionBone* motbone=si->motion->bones[bone->boneids[0]];
    motbone->Animate(si->frametime, v, q, 1); //TODO pass interpolate flag

//TODO Enable stack interpolation
    (void) sv; // fix warning
    (void) sq;
/*    for(int j=1; j<bone->nummotions; j++) {
      si=stack[bone->stacks[j]];
      motbone=si->motion->bones[bone->boneids[j]];
      motbone->Animate(si->frametime, sv, sq, 0); //TODO pass interpolate flag
      q=
      v=
    }*/
    csMatrix3 m(q);
    bone->bone->SetTransformation(csTransform(m, -m.GetInverse()*v));
  }
}

//************************************************************ Motion Stack

csMotionStackItem::csMotionStackItem(csMotionTemplate *Tmotion)
{
  motion=Tmotion;
  frametime=0.0f;
  rate=1.0f;
  loopcount=motion->loopcount;
  loopflip=motion->loopflip;
}

csMotionStackItem::~csMotionStackItem()
{

}

void csMotionStackItem::DoLoop() {
  if(loopcount>0) {
    loopcount--;
  }

  if(loopflip) {
    rate=-rate;
    if(rate<=0) {
      frametime=motion->duration-(frametime-motion->duration);
    } else {
      frametime=-frametime;
    }
  } else {
    if(rate>=0) {
      frametime-=motion->duration;
    } else {
      frametime+=motion->duration;
    }
  }
}

bool csMotionStackItem::Update(float timedelta)
{
  frametime+=timedelta*rate;

  //Reset the frametime if looping animation, else expire the motion
  while(frametime<0 || frametime>=motion->duration) {
    if(loopcount != 0) {
      DoLoop();
    } else {
      return false;
    }
  }
  return true;
}

//************************************************************ Motion Manager

csMotionManager::csMotionManager(iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiComponent);
  oldtime=0;
  object_reg=NULL;
}

csMotionManager::~csMotionManager()
{
  motions.DeleteAll();
}

bool csMotionManager::Initialize (iObjectRegistry* object_reg)
{
  csMotionManager::object_reg = object_reg;
  return true;
}

csMotionTemplate* csMotionManager::FindMotionTemplateByName (const char* name)
{
  int index = motions.FindSortedKey((void*)csHashCompute(name));
  if (index == -1)
    return NULL;
  return motions.Get(index);
}

csMotionTemplate* csMotionManager::AddMotionTemplate (const char* name)
{
  MOT_DPRINTF(("AddMotion(%s)\n", name));

  csMotionTemplate* mot = new csMotionTemplate();
  mot->SetName(name);
  motions.InsertSorted(mot);
  return mot;
}

void csMotionManager::DeleteMotion( iMotionTemplate* motiontemp )
{
  MOT_DPRINTF(("DeleteMotion(%s)\n", motiontemp->GetName()));

  if(motiontemp)
  {
    //TODO Tell children I am going to disappear
  }
  motions.Delete((csMotionTemplate*)motiontemp);
}

csMotionController* csMotionManager::FindMotionControllerBySkeleton (iSkeletonBone *skel)
{
  int index = controllers.FindSortedKey(skel);
  if (index == -1)
    return NULL;
  return controllers.Get(index);
}

csMotionController* csMotionManager::AddMotionController (iSkeletonBone *skel)
{
  MOT_DPRINTF(("AddController(%p)\n", skel));

  assert(controllers.FindSortedKey(skel)==-1);

  csMotionController *mc=new csMotionController(skel);
  controllers.InsertSorted(mc);
  return mc;
}

void csMotionManager::DeleteController( iMotionController* inst )
{
  MOT_DPRINTF(("DeleteController(%p) %p\n", inst, inst->GetSkeleton()));

  controllers.Delete((csMotionController*)inst);
}

void csMotionManager::UpdateController(csMotionController *controller, float timedelta)
{
  controller->Update(timedelta);
}

void csMotionManager::UpdateAll( float timedelta )
{
  MOT_DPRINTF(("Update all: time %f numskels %d\n",timedelta,controllers.Length()));

  int size = controllers.Length();
  for (int i = 0; i < size; i++)
  {
    UpdateController( controllers[i], timedelta );
  }
}

void csMotionManager::UpdateAll( unsigned int curtime )
{

  if (oldtime == 0) oldtime = curtime;
  csTicks elapsed_time = curtime - oldtime;
  oldtime = curtime;
  float timedelta=elapsed_time*.001;

  UpdateAll(timedelta);
}

void csMotionManager::UpdateAll()
{
  csTicks newtime = csGetTicks ();
  UpdateAll( newtime );
}

/*
void csMotionManager::UpdateTransform( iSkeletonBone *bone, csQuaternion *quat1, csQuaternion *quat2, float ratio )
{
  csVector3 vec = bone->GetTransformation().GetO2TTranslation();
  csQuaternion quat = quat1->Slerp(*quat2, ratio);
#ifdef MOTION_DEBUG
    printf("UpdateTransform Q(%g,%g,%g,%g)\n", quat.x, quat.y, quat.z, quat.r);
#endif
  bone->SetTransformation(csTransform(csMatrix3(quat), vec));
}

void csMotionManager::UpdateTransform( iSkeletonBone *bone, csVector3 vec1, csVector3 vec2, float ratio )
{
  csMatrix3 mat = bone->GetTransformation().GetO2T();
  csVector3 vec = ( vec2 - vec1 ) * ratio;
  vec += vec1;
  bone->SetTransformation( csTransform( mat, vec ));
}

void csMotionManager::UpdateTransform( iSkeletonBone *bone, csQuaternion *quat )
{
  csVector3 vec = bone->GetTransformation().GetO2TTranslation();
  bone->SetTransformation(csTransform(csMatrix3(*quat), vec));
}

void csMotionManager::UpdateTransform( iSkeletonBone *bone, csMatrix3 *mat )
{
  csVector3 vec = bone->GetTransformation().GetO2TTranslation();
  bone->SetTransformation(csTransform(*mat, vec));
}

void csMotionManager::UpdateTransform( iSkeletonBone *bone, csVector3 *vec )
{
  csMatrix3 mat = bone->GetTransformation().GetO2T();
  bone->SetTransformation( csTransform( mat, *vec ));
}

void csMotionManager::UpdateAppliedFrame(csAppliedFrame *fr, csAppliedFrame *next, float ratio )
{
  int i;
  for ( i = 0; i < fr->numqlinks; i++ )
	UpdateTransform( fr->qaffector[i], fr->qlinks[i], next->qlinks[i], ratio );
  for ( i = 0; i < fr->nummlinks; i++ )
	UpdateTransform( fr->maffector[i], fr->mlinks[i] );
  for ( i = 0; i < fr->numvlinks;  i++ )
	UpdateTransform( fr->vaffector[i], *fr->vlinks[i] , *next->vlinks[i], ratio );
}

void csMotionManager::UpdateAppliedFrame(csAppliedFrame *fr)
{
  int i;
  for ( i = 0; i < fr->numqlinks; i++ )
	UpdateTransform( fr->qaffector[i], fr->qlinks[i] );
  for ( i = 0; i < fr->nummlinks; i++ )
	UpdateTransform( fr->maffector[i], fr->mlinks[i] );
  for ( i = 0; i < fr->numvlinks;  i++ )
	UpdateTransform( fr->vaffector[i], fr->vlinks[i] );
}
*/
/*
//TODO Azverkan support frame interpolation & make looping optional
bool csMotionManager::UpdateAppliedMotion(csAppliedMotion *am, csTicks elapsedtime)
{
  MOT_DPRINTF(("Updating motion on Loop: %d, Sweep: %d, Rate: %f\n", am->Loop, am->Sweep, am->Rate));
  if (!am->Rate) return true;

  int size = am->numframes, keyf = am->frames[size-1]->keyframe, t_elapse;
  bool set=false;
  t_elapse = int( am->Rate * elapsedtime);
  if ( t_elapse >= keyf ) return true; // Going too fast.

  if (!(am->Loop))
  {
	  if ( am->curtime + t_elapse <= 0 )
	  {
		if (!am->Sweep) return true;
		am->Rate = -am->Rate ;
		am->curtime += t_elapse;
		am->curtime = -am->curtime;
		set = true;
	  }
	  else
	  if ( am->curtime + t_elapse >= keyf )
	  {
		if (!am->Sweep) return true;
	    am->Rate = -am->Rate ;
		am->curtime += t_elapse;
		am->curtime = ( keyf << 1 ) - am->curtime;
		set = true;
	  }
  }

  if (!set)
	am->curtime += t_elapse;

  if ( am->curtime < 0 ) am->curtime += keyf;
	else am->curtime %= keyf;

  int i;
  for ( i = 0; i < size; i++ )
  {
	keyf = am->frames[i]->keyframe;
    if (keyf >= am->curtime)
    {
	  am->curframe = i;
	  am->nextframe = ( am->Rate > 0 ) ?  i + 1 : i - 1;
      break;
    }
  }

  if (!i && (am->Rate < 0)) am->nextframe = (am->Loop) ? size - 1 : 0;
  else
	if ((i == size - 1) && (am->Rate > 0))
	  am->nextframe = (am->Loop) ? 0 : size - 1;

  if ( slerp && (am->nextframe != am->curframe))
  {
	float dist1= (float)( am->curtime ) - (float)(am->frames[am->curframe]->keyframe);
	float dist2= (float)(am->frames[am->nextframe]->keyframe) - (float)(am->frames[am->curframe]->keyframe);
	float ratio = dist1 / dist2;
	UpdateAppliedFrame(am->frames[am->curframe], am->frames[am->nextframe], ratio);
	return true;
  }
#ifdef MOTION_DEBUG
  printf("UpdateAppliedMotion %d %d\n", am->curtime, am->frames[am->curframe]->keyframe);
#endif

  UpdateAppliedFrame(am->frames[am->curframe]);
  return true;
}*/

