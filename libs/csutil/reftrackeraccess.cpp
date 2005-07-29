/*
    Copyright (C) 2004 by Jorrit Tyberghein
	      (C) 2004 by Frank Richter

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
#include "csutil/reftrackeraccess.h"
#include "csutil/scf.h"
#include "iutil/reftrack.h"

#ifdef CS_REF_TRACKER

#define TRACKER_CALL_(method, params)		    	\
  {						    	\
    iRefTracker* refTracker = (iSCF::SCF) ?		\
      ((iRefTracker*)(iSCF::SCF)->QueryInterface (  	\
      scfInterfaceTraits<iRefTracker>::GetID (),	    	\
      scfInterfaceTraits<iRefTracker>::GetVersion())) : 0;    \
    if (refTracker)				    	\
    {						    	\
      refTracker-> method params;  			\
      refTracker->DecRef ();			    	\
    }						    	\
  }

#define TRACKER_CALL1(method, p1)	  TRACKER_CALL_(method, (p1))
#define TRACKER_CALL2(method, p1, p2)	  TRACKER_CALL_(method, (p1, p2))
#define TRACKER_CALL3(method, p1, p2, p3) TRACKER_CALL_(method, (p1, p2, p3))
  
void csRefTrackerAccess::TrackIncRef (void* object, int refCount)
{
  TRACKER_CALL2 (TrackIncRef, object, refCount);
}

void csRefTrackerAccess::TrackDecRef (void* object, int refCount)
{
  TRACKER_CALL2 (TrackDecRef, object, refCount);
}

void csRefTrackerAccess::TrackConstruction (void* object)
{
  TRACKER_CALL1 (TrackConstruction, object);
}

void csRefTrackerAccess::TrackDestruction (void* object, int refCount)
{
  TRACKER_CALL2 (TrackDestruction, object, refCount);
}

void csRefTrackerAccess::MatchIncRef (void* object, int refCount, void* tag)
{
  TRACKER_CALL3 (MatchIncRef, object, refCount, tag);
}

void csRefTrackerAccess::MatchDecRef (void* object, int refCount, void* tag)
{
  TRACKER_CALL3 (MatchDecRef, object, refCount, tag);
}

void csRefTrackerAccess::AddAlias (void* obj, void* mapTo)
{
  TRACKER_CALL2 (AddAlias, obj, mapTo);
}

void csRefTrackerAccess::RemoveAlias (void* obj, void* mapTo)
{
  TRACKER_CALL2 (RemoveAlias, obj, mapTo);
}

void csRefTrackerAccess::SetDescription (void* obj, const char* description)
{
  TRACKER_CALL2 (SetDescription, obj, description);
}

#endif
