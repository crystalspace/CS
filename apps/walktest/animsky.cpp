/*
    Copyright (C) 2008 by Jorrit Tyberghein

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
#include "csutil/scanstr.h"
#include "iengine/movable.h"
#include "iengine/sector.h"

#include "walktest.h"
#include "animsky.h"

WalkTestAnimateSky::WalkTestAnimateSky (WalkTest* walktest) : walktest (walktest)
{
  anim_sky = 0;
  anim_dirlight = 0;
}

void WalkTestAnimateSky::AnimateDirLight (iObject* src)
{
  csRef<iMeshWrapper> wrap = scfQueryInterface<iMeshWrapper> (src);
  if (wrap)
    anim_dirlight = wrap;	// @@@ anim_dirlight should be csRef
}

void WalkTestAnimateSky::AnimateSky (const char* value, iObject* src)
{
  csRef<iSector> Sector (scfQueryInterface<iSector> (src));
  if (Sector)
  {
    char name[100], rot[100];
    csScanStr (value, "%s,%s,%f", name, rot, &anim_sky_speed);
    if (rot[0] == 'x') anim_sky_rot = 0;
    else if (rot[0] == 'y') anim_sky_rot = 1;
    else anim_sky_rot = 2;
    anim_sky = Sector->GetMeshes ()->FindByName (name);
  }
}

void WalkTestAnimateSky::MoveSky (csTicks elapsed_time, csTicks current_time)
{
  // First move the sky.
  if (anim_sky)
  {
    iMovable* move = anim_sky->GetMovable ();
    switch (anim_sky_rot)
    {
      case 0:
	{
          csXRotMatrix3 mat (anim_sky_speed * TWO_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
      case 1:
	{
          csYRotMatrix3 mat (anim_sky_speed * TWO_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
      case 2:
	{
          csZRotMatrix3 mat (anim_sky_speed * TWO_PI
	  	* (float)elapsed_time/1000.);
          move->Transform (mat);
	  break;
	}
    }
    move->UpdateMove ();
  }
}

