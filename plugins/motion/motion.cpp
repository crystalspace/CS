/*
    Simple Console
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
#include "isystem.h"
#include "csutil/csmd5.h"

IMPLEMENT_IBASE (csMotionManager)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iMotionManager)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csMotionManager)

EXPORT_CLASS_TABLE (motion)
  EXPORT_CLASS_DEP (csMotionManager, "crystalspace.motion.manager.default",
    "Skeletal Motion Manager for Crystal Space", NULL)
EXPORT_CLASS_TABLE_END

csMotionManager::csMotionManager(iBase *iParent) {
  CONSTRUCT_IBASE (iParent);
}

csMotionManager::~csMotionManager() {

}

bool csMotionManager::Initialize (iSystem * /*iSys*/) {
	return true;
}

iMotion* csMotionManager::FindByName (const char* name) {
	int index=motions.FindSortedKey(name);
	if(index==-1)
		return NULL;
	return motions.Get(index);
}

iMotion* csMotionManager::AddMotion (const char* name) {
	csMotion* mot=new csMotion();
	mot->SetName(name);
	motions.InsertSorted(mot);
	return mot;
}

IMPLEMENT_IBASE (csMotion)
  IMPLEMENTS_INTERFACE (iMotion)
IMPLEMENT_IBASE_END

csMotion::csMotion() {
  CONSTRUCT_IBASE (NULL);
	name=NULL;
	matrixmode=-1;
	hash=0;
	transforms=NULL;
	numtransforms=0;
	frames=NULL;
	numframes=0;
}

csMotion::~csMotion() {
	if(name) {
		free(name);
	}
	if(transforms) {
		free(transforms);
	}
	if(frames) {
		for(int i=0; i<numframes; i++) {
			if(frames[i].size) {
				free(frames[i].links);
				free(frames[i].affectors);
			}
		}
		free(frames);
	}
}

void csMotion::SetName(const char* newname) {
	if(name) {
		free(name);
	}
	name=(char*)malloc(strlen(newname));
	strcpy(name, newname);
	hash=csMD5::GetReducedHash(csMD5::Encode(name));
}

const char* csMotion::GetName() {
	return name;
}

bool csMotion::AddAnim (const csQuaternion &quat) {
//	printf("AddAnim(%g, %g, %g, %g)\n", quat.x, quat.y, quat.z, quat.r);

	if(matrixmode==-1) {
		matrixmode=0;
	} else if(matrixmode==1) {
		return false;
	}

	if(!transforms) {
		transforms=malloc(sizeof(csQuaternion));
	} else {
		transforms=realloc(transforms, sizeof(csQuaternion)*(numtransforms+1));
	}
	((csQuaternion*)transforms)[numtransforms]=quat;
	numtransforms++;
	return true;
}

bool csMotion::AddAnim (const csMatrix3 &mat) {
	if(matrixmode==-1) {
		matrixmode=1;
	} else if(matrixmode==0) {
		return false;
	}

	if(!transforms) {
		transforms=malloc(sizeof(csMatrix3));
	} else {
		transforms=realloc(transforms, sizeof(csMatrix3)*(numtransforms+1));
	}
	((csMatrix3*)transforms)[numtransforms]=mat;
	numtransforms++;
	return true;
}

int csMotion::AddFrame (int framenumber) {
//	printf("AddFrame(%d) %d\n", framenumber, numframes);

	if(!frames) {
		frames=(csMotionFrame*)malloc(sizeof(csMotionFrame));
	} else {
		frames=(csMotionFrame*)realloc(frames, sizeof(csMotionFrame)*(numframes+1));
	}
	frames[numframes].keyframe=framenumber;
	frames[numframes].size=0;
	numframes++;
	return numframes-1;
}

void csMotion::AddFrameLink (int frameindex, const char* affector, int link) {
//	printf("AddFrameLink(%d, '%s', %d)\n", frameindex, affector, link);

	CS_ASSERT(frameindex>=0);
	CS_ASSERT(frameindex<numframes);

	csMotionFrame *mf=&frames[frameindex];
	if(!mf->size) {
		mf->links=(int*)malloc(sizeof(int));
		mf->affectors=(unsigned int*)malloc(sizeof(unsigned int));
	} else {
		mf->links=(int*)realloc(mf->links, sizeof(int)+(mf->size+1));
		mf->affectors=(unsigned int*)realloc(mf->links, sizeof(unsigned int)+(mf->size+1));
	}
	mf->links[mf->size]=link;
	mf->affectors[mf->size]=csMD5::GetReducedHash(csMD5::Encode(affector));
	mf->size++;
}

