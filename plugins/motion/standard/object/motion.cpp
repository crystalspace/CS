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

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csMotionManager)
  SCF_IMPLEMENTS_INTERFACE (iPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iMotionManager)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY (csMotionManager)

SCF_EXPORT_CLASS_TABLE (motion)
  SCF_EXPORT_CLASS_DEP (csMotionManager, "crystalspace.motion.manager.default",
    "Skeletal Motion Manager for Crystal Space", NULL)
SCF_EXPORT_CLASS_TABLE_END

csMotionManager::csMotionManager(iBase *iParent)
{
  SCF_CONSTRUCT_IBASE (iParent);
  oldtime=0;
  iSys=NULL;
}

csMotionManager::~csMotionManager()
{
  int i;
  motions.DeleteAll();
  for ( i = 0; i < skels.Length(); i++ )
	DeleteAppliedMotion( i, false );
  for ( i = 0; i < cache.Length(); i++ )
	DeleteAppliedMotion( i, true );
  skels.DeleteAll();
  cache.DeleteAll();
}

bool csMotionManager::Initialize (iSystem* TiSys)
{
  iSys=TiSys;
  slerp = true;
  return true;
}

csMotion* csMotionManager::FindClassByName (const char* name)
{
  int index = motions.FindSortedKey(name);
  if (index == -1)
    return NULL;
  return motions.Get(index);
}

iMotion* csMotionManager::AddMotion (const char* name)
{
#ifdef MOTION_DEBUG
  printf("AddMotion(%s)\n", name);
#endif
  csMotion* mot = new csMotion();
  mot->SetName(name);
  motions.InsertSorted(mot);
  return mot;
}

void csMotionManager::DeleteMotion( const char *name )
{
#ifdef MOTION_DEBUG
  printf("DeleteMotion(%s)\n", name);
#endif

  csMotion *mot = FindClassByName (name);
  if (mot)
  {
	int len = skels.Length();
	int i;
	for (i = 0; i < len; i++ )
	  if (skels[i]->curmotion == mot )
		DeleteAppliedMotion( i, false );

	len = cache.Length();
	for (i = 0; i < len; i++ )
	  if (cache[i]->curmotion == mot )
		DeleteAppliedMotion( i, true );
	delete mot;
  }
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

int FindFrameSet ( csMotion *mot, unsigned int hash )
{
  for (int i = 0; i < mot->framesets.Length(); i++ )
	if ( mot->framesets[i]->name == hash ) return i;
  
  return -1;
}

void csMotionManager::CompileMotion ( csAppliedMotion *mot )
{
  /// Compile motion takes the frames given in csMotion and fills in the pointers in
  /// csAppliedFrame. The processing hit is taken now to avoid duplicating effort while
  /// the animation is running. Mik.
  
  csFrameSet *frs = mot->curmotion->framesets[mot->curframeset];
  mot->numframes = frs->numframes;
  int numlink;
  int i, j;

  for ( i = 0; i < mot->numframes ; i++ )
  {
	csAppliedFrame *fr = new csAppliedFrame();
	fr->numqlinks = 0 ;
	fr->nummlinks = 0 ;
	fr->numvlinks = 0 ;
	fr->keyframe = frs->frames[i].keyframe;

#ifdef MOTION_DEBUG
	printf("Compile motion qlinks %d mlinks %d vlinks %d\n", 
	  frs->frames[i].numqlinks,
	  frs->frames[i].nummlinks,
	  frs->frames[i].numvlinks
	  );
#endif

	if ((numlink =  frs->frames[i].numqlinks))
	{
	  for ( j = 0; j < numlink; j++ )
	  {
		iSkeletonBone *bone = FindBone( mot->skel, frs->frames[i].qaffector[j] );
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
		  fr->qlinks[fr->numqlinks] = &mot->curmotion->transquat[frs->frames[i].qlinks[j]];
		  fr->qaffector[fr->numqlinks] = bone;
		  fr->numqlinks++;
		}
	  }
	}
	
	if ((numlink =  frs->frames[i].nummlinks))
	{
	  for ( j = 0; j < numlink; j++ )
	  {
		iSkeletonBone *bone = FindBone( mot->skel, frs->frames[i].maffector[j] );
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
		  fr->mlinks[fr->nummlinks] = &mot->curmotion->transmat[frs->frames[i].mlinks[j]];
		  fr->maffector[fr->nummlinks] = bone;
		  fr->nummlinks++;
		}
	  }
	}
	
	if ((numlink =  frs->frames[i].numvlinks))
	{
	  for ( j = 0; j < numlink; j++ )
	  {
		iSkeletonBone *bone = FindBone( mot->skel, frs->frames[i].vaffector[j] );
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
		  fr->vlinks[fr->numvlinks] = &mot->curmotion->translate[frs->frames[i].vlinks[j]];
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
int csMotionManager::ApplyMotion(iSkeletonBone *skel, const char* motion, const char *frameset, 
	  bool loop, bool sweep, float rate, int time, bool iscached)
{
  int fs_num;
  
#ifdef MOTION_DEBUG
  printf("Apply Motion : Skel %p motion %s frameset %s dir %d loop %d rate %f time %d\n", 
			  skel, motion, frameset, dir, loop, rate, time);	
#endif

  csMotion* newmotion=FindClassByName((const char*)csHashCompute(motion));
  if(!newmotion) 
  {
	printf("Motion Manager: Cannot find motion : `%s' ( frameset : `%s' skelbone `%s' )\n", 
		motion, frameset, skel->GetName());
	return -1;
  }

  if ((fs_num = FindFrameSet(newmotion, csHashCompute(frameset))) < 0) 
  {
	printf("csMotionManager: Cannot find frameset :`%s' ( motion: `%s' skelbone: `%s')\n",
	  frameset, motion, skel->GetName());
	return -1;
  }

// I have added this so that I do not have to check for single frame action sets on every
// UpdateAll(). If you want this then tell me about it, oterwise thats the way it is for now. Mik.

  if ( newmotion->framesets[fs_num]->numframes < 2 )
  {
	printf("Single frame actionset found : `%s' ( motion : `%s' skelbone `%s')\n", 
	  frameset, motion, skel->GetName());	
	printf("These are no longer supported. Hassle Michael if you want this feature\n");
	return -1;
  }

// behavior changed to always create a new applied motion. If you want to delete / recompile
// a motion then use the relevant calls. Apply always applies a new motion ( even if it is on top
// of another ). This is a feature, not a bug. ;) Mik.

  csAppliedMotion *am = new csAppliedMotion();
  ( iscached ) ? cache.Push(am) : skels.Push(am);
  
  am->skel = skel;
  am->curmotion = newmotion;
  am->curframeset = fs_num;
  am->curtime = time;
  am->totaltime = 0;
  am->numframes = 0;
  am->curframe = 0;
  am->Rate = rate;
  am->Loop = loop;
  am->Sweep = sweep;
  
#ifdef MOTION_DEBUG
  printf("Apply motion frames length %d\n", am->frames.Length());
#endif
  CompileMotion(am);
  return (iscached) ? cache.Length() - 1 :  skels.Length() - 1;
}

void csMotionManager::RecompileMotion( int idx, bool cached )
{
  csAppliedMotion *am = (cached) ? cache[idx] : skels[idx];

  int len;
  if ((len = am->frames.Length()))
	for ( int i = 0; i < len ; i++ )
	{
	  csAppliedFrame *f = am->frames[i];
	  if ( f->numqlinks ) { free ( f->qlinks ); free( f->qaffector ); }
	  if ( f->nummlinks ) { free ( f->mlinks ); free( f->maffector ); }
	  if ( f->numvlinks ) { free ( f->vlinks ); free( f->vaffector ); }
	  free (f);
	}
  CompileMotion(am);	
}

int csMotionManager::ReserveMotion( int idx )
{
  if ((idx < 0) || (idx > skels.Length() - 1)) return -1;
  csAppliedMotion *am = skels[idx];
  cache.Push(am);
  skels.Delete(idx);
#ifdef MOTION_DEBUG
  printf("Motion: Reserve motion idx: %d  new %d\n", idx, cache.Length() - 1);
#endif
  return cache.Length() - 1;
}

int csMotionManager::RestoreMotion( int idx )
{
  if ((idx < 0) || (idx >= cache.Length())) return -1;
  csAppliedMotion *am = cache[idx];
  skels.Push(am);
  cache.Delete(idx);
#ifdef MOTION_DEBUG
  printf("Motion: Restore motion idx: %d  new %d\n", idx, skels.Length() - 1);
#endif
  return skels.Length() - 1;
}

void csMotionManager::DeleteAppliedMotion( int idx, bool cached )
{
  csAppliedMotion *mot = (cached) ? cache[idx] : skels[idx];
  int len;
  if ((len = mot->frames.Length()))
  {
	for ( int i = 0; i < len; i++ )
	{
	  csAppliedFrame *f = mot->frames[i];
	  if ( f->numqlinks ) { free ( f->qlinks ); free( f->qaffector ); }
	  if ( f->nummlinks ) { free ( f->mlinks ); free( f->maffector ); }
	  if ( f->numvlinks ) { free ( f->vlinks ); free( f->vaffector ); }
	  free (f);
	}
	mot->frames.DeleteAll();
  }
  free(mot);
  (cached) ? cache.Delete(idx) : skels.Delete(idx);
}


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

//TODO Azverkan support frame interpolation & make looping optional
bool csMotionManager::UpdateAppliedMotion(csAppliedMotion *am, cs_time elapsedtime)
{
#ifdef MOTION_DEBUG
  printf("Updating motion on Loop: %d, Sweep: %d, Rate: %f\n",
		  am->Loop, am->Sweep, am->Rate); 
#endif
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
}

void csMotionManager::UpdateAll()
{
  cs_time newtime = iSys->GetTime();
  UpdateAll( newtime );
}

void csMotionManager::UpdateAll( int time )
{
#ifdef MOTION_DEBUG
  printf("Update all: time %d numskels %d\n",time,skels.Length());
#endif
  if (oldtime == 0) oldtime = time;
  cs_time elapsed_time = time - oldtime;
  oldtime = time;
  int size = skels.Length();
  int i;
  for (i = 0; i < size; i++)
  {
#ifdef MOTION_DEBUG
	printf("skel: curfs %d curt %d tot %d num %d curf %d next %d Loop %d Sweep %d Rate %f\n",
	  skels[i]->curframeset, skels[i]->curtime, skels[i]->totaltime, skels[i]->numframes,
		skels[i]->curframe, skels[i]->nextframe, skels[i]->Loop, skels[i]->Sweep, skels[i]->Rate);
#endif
	UpdateAppliedMotion( skels[i], elapsed_time);
  }
}

SCF_IMPLEMENT_IBASE (csMotion)
  SCF_IMPLEMENTS_INTERFACE (iMotion)
SCF_IMPLEMENT_IBASE_END

csMotion::csMotion()
{
  SCF_CONSTRUCT_IBASE (NULL);
  name = NULL;
  hash = 0;
  transquat = NULL;
  transmat = NULL;
  translate = NULL;
  numtransquat = 0;
  numtransmat = 0;
  numtranslate = 0;
}

csMotion::~csMotion()
{
  if (name) free (name);
  if (transquat) free (transquat);
  if (transmat) free (transmat);
  if (translate) free (translate);
  if (framesets.Length() > 0)
  {
	int i,j;
	for (j = 0; j < framesets.Length(); j++)
    for (i = 0; i < framesets[j]->numframes; i++)
    {
      if (framesets[j]->frames[i].numqlinks)
      {
        free (framesets[j]->frames[i].qlinks);
        free (framesets[j]->frames[i].qaffector);
      }
	  if (framesets[j]->frames[i].nummlinks)
	  {
		free (framesets[j]->frames[i].mlinks);
		free (framesets[j]->frames[i].maffector);
	  }
	  if (framesets[j]->frames[i].numvlinks)
	  {
		free (framesets[j]->frames[i].vlinks);
		free (framesets[j]->frames[i].vaffector);
	  }
    }
  }
  framesets.DeleteAll();
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
#ifdef MOTION_DEBUG
	printf("AddAnim(%g, %g, %g) (%g, %g, %g) (%g, %g, %g)\n", 
		  mat.m11, mat.m12, mat.m13, 
		  mat.m21, mat.m22, mat.m23,
		  mat.m31, mat.m32, mat.m33);
#endif
  transmat = (transmat) ?
	(csMatrix3 *) realloc(transmat, sizeof(csMatrix3)*(numtransmat+1))
      : (csMatrix3 *) malloc(sizeof(csMatrix3));
    
  transmat[numtransmat] = mat;
  numtransmat++;
  return true;
}

bool csMotion::AddAnim ( const csVector3 &vec )
{
#ifdef MOTION_DEBUG
	printf("AddAnim(%g, %g, %g)\n", vec.x, vec.y, vec.z);
#endif
  translate = (translate) ?
	(csVector3 *) realloc(translate, sizeof(csVector3) * (numtranslate + 1))
	  : (csVector3 *) malloc( sizeof(csVector3) );

  translate[numtranslate] = vec;
  numtranslate++;
  return true;
}

void csMotion::AddFrameSet( const char* name )
{
#ifdef MOTION_DEBUG
	printf("AddFrameSet(%s)\n", name);
#endif
  csFrameSet *fr = new csFrameSet;
  fr->name = csHashCompute( name );
  fr->numframes = 0;
  fr->frames = NULL;
  framesets.Push(fr);
#ifdef MOTION_DEBUG
	printf("AddFrameSet:New length %d\n", framesets.Length());
#endif

}

int csMotion::AddFrame (int frametime)
{

  int cur = framesets.Length() - 1;
#ifdef MOTION_DEBUG
	printf("AddFrame(%d) %d to %d\n", frametime,framesets[cur]->numframes, cur);
#endif
  if (cur <  0)
  {
	printf("Please add a frameset first before adding frames\n");
	return -1;
  }
  
  if (!(framesets[cur]->frames))
    framesets[cur]->frames = (csMotionFrame*)malloc(sizeof(csMotionFrame));
  else
    framesets[cur]->frames = (csMotionFrame*)realloc(framesets[cur]->frames,
		  sizeof(csMotionFrame)*(framesets[cur]->numframes+1));
  int numf = framesets[cur]->numframes;
  framesets[cur]->frames[numf].keyframe = frametime;
  framesets[cur]->frames[numf].numqlinks = 0;
  framesets[cur]->frames[numf].nummlinks = 0;
  framesets[cur]->frames[numf].numvlinks = 0;
  framesets[cur]->numframes++;
  return framesets[cur]->numframes-1;
}

#include <signal.h>

void csMotion::AddFrameQLink (int frameindex, const char* affector, int link)
{
#ifdef MOTION_DEBUG
	printf("AddFrameQLink(%d, '%s', %d)\n", frameindex, affector, link);
#endif

  int cur = framesets.Length() - 1;
  if ( cur >=  0 )
  {

  CS_ASSERT(frameindex >= 0);
  CS_ASSERT(frameindex < framesets[cur]->numframes);

  csMotionFrame *mf = &framesets[cur]->frames[frameindex];
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
  mf->qaffector[mf->numqlinks] = csHashCompute(affector);
  mf->numqlinks++;
  }
  else
	printf("QLINK with no valid frameset ignored: bone name `%s' link `%d'\n",affector,link);
}

void csMotion::AddFrameMLink ( int frameindex, const char *affector, int link)
{
#ifdef MOTION_DEBUG
	printf("AddFrameMLink(%d, '%s', %d)\n", frameindex, affector, link);
#endif

  int cur = framesets.Length() - 1;
  if ( cur >= 0 )
  {

  CS_ASSERT( frameindex >= 0 );
  CS_ASSERT( frameindex < framesets[cur]->numframes );
  
  csMotionFrame *mf = &framesets[cur]->frames[frameindex];
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
  else
	printf("MLINK with no valid frameset ignored: bone name `%s' link `%d'\n", affector, link);
}

void csMotion::AddFrameVLink ( int frameindex, const char *affector, int link)
{
#ifdef MOTION_DEBUG
	printf("AddFrameVLink(%d, '%s', %d)\n", frameindex, affector, link);
#endif

  int cur = framesets.Length() - 1;
  if ( cur >= 0 )
  {

  CS_ASSERT( frameindex >= 0 );
  CS_ASSERT( frameindex < framesets[cur]->numframes );
  
  csMotionFrame *mf = &framesets[cur]->frames[frameindex];
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
  else
	printf("VLINK with no valid frameset ignored: bone name `%s' link `%d'\n", affector, link);
}
