/*
  Copyright (C) 2008 by Marten Svanfeldt

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

#ifndef __CS_SKELETONUTILITIES_H__
#define __CS_SKELETONUTILITIES_H__

#include "csgeom/vector3.h"
#include "csgeom/quaternion.h"

CS_PLUGIN_NAMESPACE_BEGIN(Skeleton2)
{
  
  inline void TransformQVFrame (const csQuaternion& parentQ, const csVector3& parentV,
    const csQuaternion& relQ, const csVector3& relV,
    csQuaternion& resultQ, csVector3& resultV)
  {
    resultQ = parentQ * relQ;
    resultV = parentV + parentQ.Rotate (relV);
  }

  inline void TransformQVFrameInv (const csQuaternion& parentQ, const csVector3& parentV,
    const csQuaternion& myQ, const csVector3& myV,
    csQuaternion& resultQ, csVector3& resultV)
  {
    resultQ = parentQ.GetConjugate () * myQ;
    resultV = parentQ.GetConjugate ().Rotate (myV - parentV);
  }

  inline void TransformQVFrameInv2 (const csQuaternion& parentQ, const csVector3& parentV,
    const csQuaternion& myQ, const csVector3& myV,
    csQuaternion& resultQ, csVector3& resultV)
  {
    resultQ = myQ * parentQ.GetConjugate ();
    resultV = myV - resultQ.Rotate (parentV);
  }

}
CS_PLUGIN_NAMESPACE_END(Skeleton2)
  

#endif
