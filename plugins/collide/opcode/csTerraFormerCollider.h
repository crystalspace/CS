/*
    Copyright (C) 2006 by Jorrit Tyberghein

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

#ifndef __CS_TERRAFORMER_COLLIDER_H__
#define __CS_TERRAFORMER_COLLIDER_H__

#include "csutil/scf.h"
#include "ivaria/collider.h"
#include "ivaria/terraform.h"
#include "csutil/scf_implementation.h"

class csTerraFormerCollider : public scfImplementation1<csTerraFormerCollider, iCollider>
{
  csRef<iTerraFormer> former;
  csStringID stringHeights;
  iObjectRegistry *object_reg;

public:

  csTerraFormerCollider (iTerraFormer* terraformer, iObjectRegistry* object_reg);

  float SampleFloat (float x, float z);

  csColliderType GetColliderType () {return CS_TERRAFORMER_COLLIDER;}
  virtual ~csTerraFormerCollider (){;}

};

#endif // __CS_TERRAFORMER_COLLIDER_H__