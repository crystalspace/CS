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
#include "csCloudsDynamics.h"

CS_IMPLEMENT_PLUGIN
SCF_IMPLEMENT_FACTORY(csCloudsDynamics)

//----------------------------------------------------------//

const csVector3 csCloudsDynamics::ComputeBuoyantForce(const UINT x, const UINT y, const UINT z)
{
	const float fVirtPotTemp = (1.f + 0.61f * m_arfWaterVaporMixingRatios[m_iLastIndex]->GetValue(x, y, z)) * 
								m_arfPotTemperature[m_iLastIndex]->GetValue(x, y, z);
	return m_vGravitationAcc * (fVirtPotTemp * m_fInvRefVirtPotTemp - m_fCondWaterScaleFactor * 
								m_arfCondWaterMixingRatios[m_iLastIndex]->GetValue(x, y, z));
}

//----------------------------------------------------------//

const csVector3 csCloudsDynamics::ComputeVorticityConfinement(const UINT x, const UINT y, const UINT z)
{
	return csVector3(0.f);
}

//----------------------------------------------------------//

const bool csCloudsDynamics::DoComputationSteps(const UINT iStepCount, const float fTime)
{

	return true;
}

//----------------------------------------------------------//

//----------------------------------------------------------//