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

bool csMotionManager::Initialize (iSystem *iSys) {
	return true;
}

iMotion* csMotionManager::FindByName (const char* name) {
	return motions.FindSortedKey(name);
}

iMotion* csMotionManager::AddMotion (const char* name) {
	csMotion* mot=new csMotion(this);
	mot->SetName(name);
	motions.InsertSorted(mot);
	return mot;
}

IMPLEMENT_IBASE (csMotion)
  IMPLEMENTS_INTERFACE (iMotion)
IMPLEMENT_IBASE_END

csMotion::csMotion(iBase *iParent) {
  CONSTRUCT_IBASE (iParent);
	name=NULL;
}

csMotion::~csMotion() {

}

void csMotion::SetName(const char* newname) {
	if(name) {
		free(name);
	}
	name=(char*)malloc(strlen(newname));
	strcpy(name, newname);
}

const char* csMotion::GetName() {
	return name;
}

virtual iMotionAnim* csMotion::FindAnimByName (const char* name) {
	return anims.FindSortedKey(name);
}

virtual iMotionAnim* csMotion::AddAnim (const char* name) {
	csMotionAnim *motanim=new csMotionAnim(NULL);
	motanim->SetName(name);
	anims.InsertSorted(motanim);
	return motanim;
}

virtual iMotionFrame* csMotion::AddFrame (int framenumber) {
	csMotionFrame *motframe=new csMotionFrame(NULL);
	motframe->SetNumber(0);
	frames.InsertSorted(motframe);
	return motframe;
}

