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

void csCloudsDynamics::SetGridSize(const UINT x, const UINT y, const UINT z)
{
	//Free already reserved fields
	FreeReservedMemory();

	//Reserve new one
	m_iGridSizeX = x;
	m_iGridSizeY = y;
	m_iGridSizeZ = z;
	m_arfPotTemperature[0].AttachNew(new csField3<float>(this));
	m_arfPotTemperature[0]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arfPotTemperature[1].AttachNew(new csField3<float>(this));
	m_arfPotTemperature[1]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arfCondWaterMixingRatios[0].AttachNew(new csField3<float>(this));
	m_arfCondWaterMixingRatios[0]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arfCondWaterMixingRatios[1].AttachNew(new csField3<float>(this));
	m_arfCondWaterMixingRatios[1]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arfWaterVaporMixingRatios[0].AttachNew(new csField3<float>(this));
	m_arfWaterVaporMixingRatios[0]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arfWaterVaporMixingRatios[1].AttachNew(new csField3<float>(this));
	m_arfWaterVaporMixingRatios[1]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	//Velocity field is bigger by one (because of definition!)
	m_arvVelocityField[0].AttachNew(new csField3<csVector3>(this));
	m_arvVelocityField[0]->SetSize(m_iGridSizeX + 1, m_iGridSizeY + 1, m_iGridSizeZ + 1);
	m_arvVelocityField[1].AttachNew(new csField3<csVector3>(this));
	m_arvVelocityField[1]->SetSize(m_iGridSizeX + 1, m_iGridSizeY + 1, m_iGridSizeZ + 1);
	//Normal size
	m_arvRotVelField.AttachNew(new csField3<csVector3>(this));
	m_arvRotVelField->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arvForceField.AttachNew(new csField3<csVector3>(this));
	m_arvForceField->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);

	m_iActualIndex	= 0;
	m_iLastIndex	= 1;

	//Fill fields with initial values
	for(UINT x = 0; x < m_iGridSizeX + 1; ++x)
	{
		for(UINT y = 0; y < m_iGridSizeY + 1; ++y)
		{
			for(UINT z = 0; z < m_iGridSizeZ + 1; ++z)
			{
				if(x < m_iGridSizeX && y < m_iGridSizeY && z < m_iGridSizeZ)
				{
					m_arfPotTemperature[0]->SetValue(m_fAmbientTemperature, x, y, z);
					m_arfPotTemperature[1]->SetValue(m_fAmbientTemperature, x, y, z);
					m_arfWaterVaporMixingRatios[0]->SetValue(m_fInitWaterVaporMixingRatio, x, y, z);
					m_arfWaterVaporMixingRatios[1]->SetValue(m_fInitWaterVaporMixingRatio, x, y, z);
					m_arfCondWaterMixingRatios[0]->SetValue(m_fInitCondWaterMixingRatio, x, y, z);
					m_arfCondWaterMixingRatios[1]->SetValue(m_fInitCondWaterMixingRatio, x, y, z);
				}
				m_arvVelocityField[0]->SetValue(csVector3(0.f, 0.f, 0.f), x, y, z);
				m_arvVelocityField[1]->SetValue(csVector3(0.f, 0.f, 0.f), x, y, z);
				m_arvRotVelField->SetValue(csVector3(0.f, 0.f, 0.f), x, y, z);
			}
		}
	}
}

//----------------------------------------------------------//

const csVector3 csCloudsDynamics::ComputeBuoyantForce(const UINT x, const UINT y, const UINT z)
{
	const float fVirtPotTemp = (1.f + 0.61f * m_arfWaterVaporMixingRatios[m_iActualIndex]->GetValue(x, y, z)) * 
								m_arfPotTemperature[m_iActualIndex]->GetValue(x, y, z);
	return m_vGravitationAcc * (fVirtPotTemp * m_fInvRefVirtPotTemp - m_fCondWaterScaleFactor * 
								m_arfCondWaterMixingRatios[m_iActualIndex]->GetValue(x, y, z));
}

//----------------------------------------------------------//

const csVector3 csCloudsDynamics::ComputeVorticityConfinement(const UINT x, const UINT y, const UINT z)
{
	const csVector3 vRotu = m_arvRotVelField->GetValue(x, y, z);
	csVector3 vEta = CalcGradient(m_arvRotVelField, x, y, z, m_fGridScale);
	vEta.Normalize();
	// % == cross product
	return m_fGridScale * m_fVCEpsilon * (vEta % vRotu);
}

//----------------------------------------------------------//

void csCloudsDynamics::AdvectAllQuantities()
{
	//First we advect all quatities which are defined at cell center!
	for(UINT x = 0; x < m_iGridSizeX; ++x)
	{
		for(UINT y = 0; y < m_iGridSizeY; ++y)
		{
			for(UINT z = 0; z < m_iGridSizeZ; ++z)
			{
				const csVector3 vVel = GetVelocityOfCellCenter(m_arvVelocityField[m_iLastIndex], x, y, z);
				const csVector3 vPos = Clamp(csVector3(x, y, z) - m_fTimeStep * m_fInvGridScale * vVel, m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
				//Even if vPos is on boundary, the interpolation works fine!
				m_arfPotTemperature[m_iActualIndex]->SetValue(GetInterpolatedValue(m_arfPotTemperature[m_iLastIndex], vPos),
					static_cast<UINT>(vPos.x), static_cast<UINT>(vPos.y), static_cast<UINT>(vPos.z));
			}
		}
	}
}

//----------------------------------------------------------//

void csCloudsDynamics::AddAcceleratingForces()
{
	//This loops handle only the interior of the grid.
	//Boundaries are treated seperatly
	for(UINT x = 1; x < m_iGridSizeX; ++x)
	{
		for(UINT y = 1; y < m_iGridSizeY; ++y)
		{
			for(UINT z = 1; z < m_iGridSizeZ; ++z)
			{
				const float f1 = m_arvForceField->GetValue(x, y, z).x + m_arvForceField->GetValue(x - 1, y, z).x;
				const float f2 = m_arvForceField->GetValue(x, y, z).y + m_arvForceField->GetValue(x, y - 1, z).y;
				const float f3 = m_arvForceField->GetValue(x, y, z).z + m_arvForceField->GetValue(x, y, z - 1).z;
				const csVector3 vCurrent = m_arvVelocityField[m_iActualIndex]->GetValue(x, y, z);
				m_arvVelocityField[m_iActualIndex]->SetValue(vCurrent + m_fTimeStep * 0.5f * csVector3(f1, f2, f3), x, y, z);
			}
		}
	}
}

//----------------------------------------------------------//

void csCloudsDynamics::SatisfyVelocityBoundaryCond()
{
	//sides: user-defined windspeeds --> all eight corners are going to be overwritten by
	//userdefined windspeeds
	//xy-plane, z = 0 && z = MAX
	for(UINT x = 0; x <= m_iGridSizeX; ++x)
	{
		for(UINT y = 0; y <= m_iGridSizeY; ++y)
		{
			m_arvVelocityField[m_iActualIndex]->SetValue(m_vWindSpeed, x, y, 0);
			m_arvVelocityField[m_iActualIndex]->SetValue(m_vWindSpeed, x, y, m_iGridSizeZ);
		}
	}
	//yz-Plane, x = 0 && x = MAX
	for(UINT y = 0; y <= m_iGridSizeY; ++y)
	{
		for(UINT z = 0; z <= m_iGridSizeZ; ++z)
		{
			m_arvVelocityField[m_iActualIndex]->SetValue(m_vWindSpeed, 0, y, z);
			m_arvVelocityField[m_iActualIndex]->SetValue(m_vWindSpeed, m_iGridSizeX, y, z);
		}
	}

	//Bottom: no-slip (xz plane, y = 0)
	for(UINT x = 1; x < m_iGridSizeX; ++x)
	{
		for(UINT z = 1; z < m_iGridSizeZ; ++z)
		{
			const csVector3 vCurr	= m_arvVelocityField[m_iActualIndex]->GetValue(x, 0, z);
			const csVector3 vAbove  = m_arvVelocityField[m_iActualIndex]->GetValue(x, 1, z);
			//Which one is correct?
			//m_arvVelocityField[m_iActualIndex]->SetValue(csVector3(vCurr.x, -vAbove.y, vCurr.z), x, 0, z);
			m_arvVelocityField[m_iActualIndex]->SetValue(csVector3(0.f, -vAbove.y, 0.f), x, 0, z);
		}
	}

	//Top: free-slip (xz plane, y = MAX)
	for(UINT x = 1; x < m_iGridSizeX; ++x)
	{
		for(UINT z = 1; z < m_iGridSizeZ; ++z)
		{
			const csVector3 vBorder = m_arvVelocityField[m_iActualIndex]->GetValue(x, m_iGridSizeY, z);
			const csVector3 vInner  = m_arvVelocityField[m_iActualIndex]->GetValue(x, m_iGridSizeY - 1, z);
			m_arvVelocityField[m_iActualIndex]->SetValue(csVector3(vBorder.x, vInner.y, vBorder.z), x, m_iGridSizeY, z);
		}
	}
}

//----------------------------------------------------------//
//----------------------------------------------------------//

const bool csCloudsDynamics::DoComputationSteps(const UINT iStepCount, const float fTime)
{
	m_fTimeStep = fTime;
	//DEBUG
	AdvectAllQuantities();
	ComputeRotationField();
	ComputeForceField();
	AddAcceleratingForces();
	SatisfyVelocityBoundaryCond();



	SwapFieldIndizes();
	return true;
}

//----------------------------------------------------------//

//----------------------------------------------------------//