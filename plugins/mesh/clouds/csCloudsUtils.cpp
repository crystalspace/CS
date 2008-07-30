/*
  Copyright (C) 2008 by Julian Mautner

  This application is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This application is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this application; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <cssysdef.h>
#include "csCloudsUtils.h"

//-----------------------------------------------------//

const float GetInterpolatedValue(const csRef<csField3<float>>& rSrc, const csVector3& vPos)
{
	const UINT x = static_cast<UINT>(vPos.x);
	const UINT y = static_cast<UINT>(vPos.y);
	const UINT z = static_cast<UINT>(vPos.z);
	const float i	= static_cast<float>(x);
	const float j	= static_cast<float>(y);
	const float k	= static_cast<float>(z);
	//With this lines I guarantee that always when the interpolation
	//would like to make an access outside of the field, the weight for it
	//is zero!
	const float ip1 = x + 1 >= rSrc->GetSizeX() ? vPos.x : static_cast<float>(i + 1);
	const float jp1 = y + 1 >= rSrc->GetSizeY() ? vPos.y : static_cast<float>(j + 1);
	const float kp1 = z + 1 >= rSrc->GetSizeZ() ? vPos.z : static_cast<float>(k + 1);

	return	(ip1 - vPos.x) * (jp1 - vPos.y) * (kp1 - vPos.z) * rSrc->GetValue(x, y, z) +
			(vPos.x - i) * (jp1 - vPos.y) * (kp1 - vPos.z) * rSrc->GetValueClamp(x + 1, y, z) +
			(ip1 - vPos.x) * (vPos.y - j) * (kp1 - vPos.z) * rSrc->GetValueClamp(x, y + 1, z) +
			(vPos.x - i) * (vPos.y - j) * (kp1 - vPos.z) * rSrc->GetValueClamp(x + 1, y + 1, z) +
			(ip1 - vPos.x) * (jp1 - vPos.y) * (vPos.z - k) * rSrc->GetValueClamp(x, y, z + 1) +
			(vPos.x - i) * (jp1 - vPos.y) * (vPos.z - k) * rSrc->GetValueClamp(x + 1, y, z + 1) +
			(ip1 - vPos.x) * (vPos.y - j) * (vPos.z - k) * rSrc->GetValueClamp(x, y + 1, z + 1) +
			(vPos.x - i) * (vPos.y - j) * (vPos.z - k) * rSrc->GetValueClamp(x + 1, y + 1, z + 1);
}

//-----------------------------------------------------//

const float GetInterpolatedValue(const csRef<csField3<csVector3>>& rSrc, const csVector3& vPos, const UINT iIndex)
{
	const UINT x = static_cast<UINT>(vPos.x);
	const UINT y = static_cast<UINT>(vPos.y);
	const UINT z = static_cast<UINT>(vPos.z);
	const float i	= static_cast<float>(x);
	const float j	= static_cast<float>(y);
	const float k	= static_cast<float>(z);
	//With this lines I guarantee that always when the interpolation
	//would like to make an access outside of the field, the weight for it
	//is zero!
	const float ip1 = x + 1 >= rSrc->GetSizeX() ? vPos.x : static_cast<float>(i + 1);
	const float jp1 = y + 1 >= rSrc->GetSizeY() ? vPos.y : static_cast<float>(j + 1);
	const float kp1 = z + 1 >= rSrc->GetSizeZ() ? vPos.z : static_cast<float>(k + 1);

	return	(ip1 - vPos.x) * (jp1 - vPos.y) * (kp1 - vPos.z) * rSrc->GetValue(x, y, z).m[iIndex] +
			(vPos.x - i) * (jp1 - vPos.y) * (kp1 - vPos.z) * rSrc->GetValueClamp(x + 1, y, z).m[iIndex] +
			(ip1 - vPos.x) * (vPos.y - j) * (kp1 - vPos.z) * rSrc->GetValueClamp(x, y + 1, z).m[iIndex] +
			(vPos.x - i) * (vPos.y - j) * (kp1 - vPos.z) * rSrc->GetValueClamp(x + 1, y + 1, z).m[iIndex] +
			(ip1 - vPos.x) * (jp1 - vPos.y) * (vPos.z - k) * rSrc->GetValueClamp(x, y, z + 1).m[iIndex] +
			(vPos.x - i) * (jp1 - vPos.y) * (vPos.z - k) * rSrc->GetValueClamp(x + 1, y, z + 1).m[iIndex] +
			(ip1 - vPos.x) * (vPos.y - j) * (vPos.z - k) * rSrc->GetValueClamp(x, y + 1, z + 1).m[iIndex] +
			(vPos.x - i) * (vPos.y - j) * (vPos.z - k) * rSrc->GetValueClamp(x + 1, y + 1, z + 1).m[iIndex];
}

//-----------------------------------------------------//

void JacobiSolver(csRef<csField3<float>> rNew, const csRef<csField3<float>>& rOld, 
				  const csRef<csField3<float>>& rBField, const float fAlpha, const float fInvBeta)
{
	for(UINT x = 0; x < rNew->GetSizeX(); ++x)
	{
		for(UINT y = 0; y < rNew->GetSizeY(); ++y)
		{
			for(UINT z = 0; z < rNew->GetSizeZ(); ++z)
			{
				const float fB		= fAlpha * rBField->GetValue(x, y, z);
				const float fTemp	= rOld->GetValueClamp(x + 1, y, z) + rOld->GetValueClamp(x - 1, y, z) +
									  rOld->GetValueClamp(x, y + 1, z) + rOld->GetValueClamp(x, y - 1, z) +
									  rOld->GetValueClamp(x, y, z + 1) + rOld->GetValueClamp(x, y, z - 1);
				rNew->SetValue((fTemp + fB) * fInvBeta, x, y, z);
			}
		}
	}
}

//-----------------------------------------------------//