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

// #define MOTION_DEBUG

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

iSkeletonBone *FindBone ( iSkeletonBone *bone, unsigned int hash )
{
  const char *name = bone->GetName();
  if (name)
  {
	unsigned int newhash = csHashCompute( name );
	if ( newhash == hash ) return bone;
  }
  iSkeletonBone* child = bone->GetChildren();
  while ( child )
  {
	iSkeletonBone *newbone = FindBone ( child, hash );
	if (newbone) return newbone;
	  else child = child->GetNext();
  }
  return NULL;
}

void csMotionManager::CompileMotion ( csAppliedMotion *mot )
{
  /// Compile motion takes the frames given in csMotion and fills in the pointers in
  /// csAppliedFrame. The processing hit is taken now to avoid duplicating effort while
  /// the animation is running. Mik.
  
  mot->numframes = mot->curmotion->numframes;
  int numlink;

  for (int i = 0; i < mot->numframes ; i++ )
  {
	csAppliedFrame *fr = new csAppliedFrame();
	fr->numqlinks = 0 ;
	fr->nummlinks = 0 ;
	fr->numvlinks = 0 ;
	fr->keyframe = mot->curmotion->frames[i].keyframe;

#ifdef MOTION_DEBUG
	printf("Compile motion qlinks %d mlinks %d vlinks %d\n", 
	  mot->curmotion->frames[i].numqlinks,
	  mot->curmotion->frames[i].nummlinks,
	  mot->curmotion->frames[i].numvlinks
	  );
#endif

	if ((numlink =  mot->curmotion->frames[i].numqlinks))
	{
	  for ( int j = 0; j < numlink; j++ )
	  {
		iSkeletonBone *bone = FindBone( mot->skel, mot->curmotion->frames[i].qaffector[j] );
		if (bone)
		{
		  if (fr->numqlinks)
		  {
			fr->qlinks = (csQuaternion **)realloc( fr->qlinks, sizeof( csQuaternion *) * (fr->numqlinks + 1));
			fr->qaffector = 
			  (iSkeletonBone **)realloc ( fr->qaffector, sizeof( iSkeletonBone * ) * ( fr->numqlinks + 1));
		  }
		  else
		  {
			fr->qlinks = (csQuaternion **) malloc ( sizeof ( csQuaternion *));
			fr->qaffector = (iSkeletonBone **) malloc ( sizeof ( iSkeletonBone * ));
		  }
		  fr->qlinks[fr->numqlinks] = &mot->curmotion->transquat[mot->curmotion->frames[i].qlinks[j]];
		  fr->qaffector[fr->numqlinks] = bone;
		  fr->numqlinks++;
		}
	  }
	}
	
	if ((numlink =  mot->curmotion->frames[i].nummlinks))
	{
	  for ( int j = 0; j < numlink; j++ )
	  {
		iSkeletonBone *bone = FindBone( mot->skel, mot->curmotion->frames[i].maffector[j] );
		if (bone)
		{
		  if (fr->nummlinks)
		  {
			fr->mlinks = (csMatrix3 **) realloc( fr->mlinks, sizeof( csMatrix3* ) * (fr->nummlinks + 1));
			fr->maffector = 
			  (iSkeletonBone **) realloc ( fr->maffector, sizeof( iSkeletonBone * ) * ( fr->nummlinks + 1));
		  }
		  else
		  {
			fr->mlinks = (csMatrix3 **) malloc ( sizeof ( csMatrix3 * ));
			fr->maffector = (iSkeletonBone **) malloc ( sizeof ( iSkeletonBone * ));
		  }
		  fr->mlinks[fr->nummlinks] = &mot->curmotion->transmat[mot->curmotion->frames[i].mlinks[j]];
		  fr->maffector[fr->nummlinks] = bone;
		  fr->nummlinks++;
		}
	  }
	}
	
	if ((numlink =  mot->curmotion->frames[i].numvlinks))
	{
	  for ( int j = 0; j < numlink; j++ )
	  {
		iSkeletonBone *bone = FindBone( mot->skel, mot->curmotion->frames[i].vaffector[j] );
		if (bone)
		{
		  if (fr->numvlinks)
		  {
			fr->vlinks = (csVector3 **) realloc( fr->vlinks, sizeof( csVector3 *) * (fr->numvlinks + 1));
			fr->vaffector = 
			  (iSkeletonBone **) realloc ( fr->vaffector, sizeof( iSkeletonBone * ) * ( fr->numvlinks + 1));
		  }
		  else
		  {
			fr->vlinks = (csVector3 **) malloc ( sizeof ( csVector3 * ));
			fr->vaffector = (iSkeletonBone **) malloc ( sizeof ( iSkeletonBone * ));
		  }
		  fr->vlinks[fr->numvlinks] = &mot->curmotion->translate[mot->curmotion->frames[i].vlinks[j]];
		  fr->vaffector[fr->numvlinks] = bone;
		  fr->numvlinks++;
		}
	  }
	}
	
#ifdef MOTION_DEBUG
  printf("Compiled frame : qlink %d mlink %d vlink %d\n", fr->numqlinks, fr->nummlinks, fr->numvlinks );	
#endif

  mot->frames.Push(fr);
  }
}

//TODO Azverkan use a sorted array
bool csMotionManager::ApplyMotion(iSkeletonBone *skel, const char* motion, int time)
{
  csMotion* newmotion=FindClassByName((const char*)csHashCompute(motion));
  if(!newmotion) return false;

  int size = skels.Length();
  int i=0;
  for(; i<size; i++)
  {
    if( ((iSkeletonBone*)(skels[i]->skel)) == skel)
      break;
  }
  csAppliedMotion *am=NULL;
  if(i==size)
  {
    am = new csAppliedMotion();
    skels.Push(am);
  }
  else
    am=skels[i];

  am->name = NULL;
  am->skel = skel;
  am->curmotion = newmotion;
  am->curtime = time;
  am->numframes = 0;
  am->curframe = 0;
  if (am->frames.Length()) 
  {
	int len = am->frames.Length();
	for ( i = 0; i < len; i++ )
	{
	  csAppliedFrame* fr = am->frames[i];
	  if (fr->numqlinks) { free (fr->qlinks); free(fr->qaffector); }
	  if (fr->nummlinks) { free (fr->mlinks); free(fr->maffector); }
	  if (fr->numvlinks) { free (fr->vlinks); free(fr->vaffector); }
	}
	am->frames.DeleteAll();
  }
#ifdef MOTION_DEBUG
  printf("Apply motion 3 frames length %d\n",am->frames.Length());
#endif
  CompileMotion( am );
  return true;
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

void csMotionManager::UpdateAppliedFrame(csAppliedFrame *fr)
{
  int i;
  for (i = 0; i < fr->numqlinks; i++ )
	UpdateTransform( fr->qaffector[i], fr->qlinks[i] );
  for (i = 0; i < fr->nummlinks; i++ )
	UpdateTransform( fr->maffector[i], fr->mlinks[i] );
  for (i = 0; i < fr->numvlinks;  i++ )
	UpdateTransform( fr->vaffector[i], fr->vlinks[i] );
}

//TODO Azverkan support frame interpolation & make looping optional
bool csMotionManager::UpdateAppliedMotion(csAppliedMotion *am, cs_time elapsedtime)
{
  am->curtime += elapsedtime;

  int size = am->numframes;

  //Check to see if motion has looped
  CS_ASSERT(am->frames[size-1]->keyframe > 0);

  while(am->frames[size-1]->keyframe < am->curtime)
  {
    am->curtime -= am->frames[size-1]->keyframe;
  }

  for(int i=0; i<size; i++)
  {
    if(am->frames[i]->keyframe==am->curtime)
    {
      am->curframe = i;
      break;
    }
    else if (am->frames[i]->keyframe > am->curtime)
    {
      am->curframe = ( i != 0 ) ? i - 1 : i;
      break;
    }
  }

  if (!am->curframe) return false;

#ifdef MOTION_DEBUG
  printf("UpdateAppliedMotion %d %d\n", am->curtime, am->frames[am->curframe]->keyframe); 
#endif

  UpdateAppliedFrame(am->frames[am->curframe]);
  return true;
}

void csMotionManager::UpdateAll()
{
  cs_time newtime = iSys->GetTime();
  UpdateAll( newtime );
}

void csMotionManager::UpdateAll( int time )
{
  if (oldtime == 0) oldtime = time;
  cs_time elapsed_time = time - oldtime;
  oldtime = time;
  int size = skels.Length();
  for (int i = 0; i < size; i++)
  {
	UpdateAppliedMotion( skels[i], elapsed_time);
  }
}

IMPLEMENT_IBASE (csMotion)
  IMPLEMENTS_INTERFACE (iMotion)
IMPLEMENT_IBASE_END

csMotion::csMotion()
{
  CONSTRUCT_IBASE (NULL);
  name = NULL;
  hash = 0;
  transquat = NULL;
  transmat = NULL;
  translate = NULL;
  numtransquat = 0;
  numtransmat = 0;
  numtranslate = 0;
  frames=NULL;
  numframes=0;
}

csMotion::~csMotion()
{
  if (name) free (name);
  if (transquat) free (transquat);
  if (transmat) free (transmat);
  if (translate) free (translate);
  if (frames)
  {
    for (int i = 0; i < numframes; i++)
    {
      if (frames[i].numqlinks)
      {
        free (frames[i].qlinks);
        free (frames[i].qaffector);
      }
	  if (frames[i].nummlinks)
	  {
		free (frames[i].mlinks);
		free (frames[i].maffector);
	  }
	  if (frames[i].numvlinks)
	  {
		free (frames[i].vlinks);
		free (frames[i].vaffector);
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

  if (!transquat)
    transquat = (csQuaternion *) malloc(sizeof(csQuaternion));
  else
    transquat = (csQuaternion *) realloc(transquat, sizeof(csQuaternion)*(numtransquat+1));

  transquat[numtransquat] = quat;
  numtransquat++;
  return true;
}

bool csMotion::AddAnim (const csMatrix3 &mat)
{
  transmat = (transmat) ?
	(csMatrix3 *) realloc(transmat, sizeof(csMatrix3)*(numtransmat+1))
      : (csMatrix3 *) malloc(sizeof(csMatrix3));
    
  transmat[numtransmat] = mat;
  numtransmat++;
  return true;
}

bool csMotion::AddAnim ( const csVector3 &vec )
{
  translate = (translate) ?
	(csVector3 *) realloc(translate, sizeof(csVector3) * (numtranslate + 1))
	  : (csVector3 *) malloc( sizeof(csVector3) );

  translate[numtranslate] = vec;
  numtranslate++;
  return true;
}

int csMotion::AddFrame (int frametime)
{
#ifdef MOTION_DEBUG
	printf("AddFrame(%d) %d\n", frametime, numframes);
#endif

  if (!frames)
    frames=(csMotionFrame*)malloc(sizeof(csMotionFrame));
  else
    frames=(csMotionFrame*)realloc(frames,sizeof(csMotionFrame)*(numframes+1));
  frames[numframes].keyframe = frametime;
  frames[numframes].numqlinks = 0;
  frames[numframes].nummlinks = 0;
  frames[numframes].numvlinks = 0;
  numframes++;
  return numframes-1;
}

#include <signal.h>

void csMotion::AddFrameQLink (int frameindex, const char* affector, int link)
{
#ifdef MOTION_DEBUG
	printf("AddFrameQLink(%d, '%s', %d)\n", frameindex, affector, link);
#endif

  CS_ASSERT(frameindex>=0);
  CS_ASSERT(frameindex<numframes);

  csMotionFrame *mf=&frames[frameindex];
  if (!mf->numqlinks)
  {
    mf->qlinks = (int *) malloc(sizeof(int));
    mf->qaffector = (unsigned int *) malloc(sizeof(unsigned int));
  }
  else
  {
    mf->qlinks = (int *) realloc(mf->qlinks, sizeof(int)*(mf->numqlinks+1));
    mf->qaffector=
      (unsigned int *) realloc(mf->qaffector, sizeof(unsigned int)*(mf->numqlinks+1));
  }
  mf->qlinks[mf->numqlinks] = link;
  mf->qaffector[mf->numqlinks]=csHashCompute(affector);
  mf->numqlinks++;
}

void csMotion::AddFrameMLink ( int frameindex, const char *affector, int link)
{
#ifdef MOTION_DEBUG
	printf("AddFrameMLink(%d, '%s', %d)\n", frameindex, affector, link);
#endif

  CS_ASSERT( frameindex >= 0 );
  CS_ASSERT( frameindex < numframes );
  
  csMotionFrame *mf = &frames[frameindex];
  if (!mf->nummlinks)
  {
	mf->mlinks = (int *) malloc ( sizeof(int) );
	mf->maffector = (unsigned int *) malloc( sizeof(unsigned int) );
  }
  else
  {
	mf->mlinks = (int *) realloc( mf->mlinks, sizeof(int) * (mf->nummlinks+1));
	mf->maffector = 
	  (unsigned int *) realloc ( mf->maffector, sizeof (unsigned int) * (mf->nummlinks+1));
  }
  mf->mlinks[mf->nummlinks] = link;
  mf->maffector[mf->nummlinks] = csHashCompute(affector);
  mf->nummlinks++;
}

void csMotion::AddFrameVLink ( int frameindex, const char *affector, int link)
{
#ifdef MOTION_DEBUG
	printf("AddFrameVLink(%d, '%s', %d)\n", frameindex, affector, link);
#endif

  CS_ASSERT( frameindex >= 0 );
  CS_ASSERT( frameindex < numframes );
  csMotionFrame *mf = &frames[frameindex];
  if (!mf->numvlinks)
  {
	mf->vlinks = (int *) malloc ( sizeof(int) );
	mf->vaffector = (unsigned int *) malloc( sizeof(unsigned int) );
  }
  else
  {
	mf->vlinks = (int *) realloc( mf->vlinks, sizeof(int) * (mf->numvlinks+1));
	mf->vaffector = 
	  (unsigned int *) realloc ( mf->vaffector, sizeof (unsigned int) * (mf->numvlinks+1));
  }
  mf->vlinks[mf->numvlinks] = link;
  mf->vaffector[mf->numvlinks] = csHashCompute(affector);
  mf->numvlinks++;
}
