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

SCF_IMPLEMENT_FACTORY(csCloudsDynamics)

//----------------------------------------------------------//

const bool csCloudsDynamics::SetGridSize(const UINT x, const UINT y, const UINT z)
{
	if(x <= 1 || y <= 1 || z <= 1) return false;
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
	m_arfVelDivergence.AttachNew(new csField3<float>(this));
	m_arfVelDivergence->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arfPressureField[0].AttachNew(new csField3<float>(this));
	m_arfPressureField[0]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	m_arfPressureField[1].AttachNew(new csField3<float>(this));
	m_arfPressureField[1]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);

	m_iActualIndex	= 0;
	m_iLastIndex	= 1;
	m_iCurrentStep	= 0;

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
					m_arfPressureField[0]->SetValue(0.f, x, y, z);
					m_arfPressureField[1]->SetValue(0.f, x, y, z);
					m_arvRotVelField->SetValue(csVector3(0.f, 0.f, 0.f), x, y, z);
				}
				m_arvVelocityField[0]->SetValue(csVector3(0.f, 0.f, 0.f), x, y, z);
				m_arvVelocityField[1]->SetValue(csVector3(0.f, 0.f, 0.f), x, y, z);
			}
		}
	}
	//Make boundary conditions hold
	SatisfyScalarBoundaryCond();

	return true;
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
	//Then the velocity
	for(UINT x = 0; x <= m_iGridSizeX; ++x)
	{
		for(UINT y = 0; y <= m_iGridSizeY; ++y)
		{
			for(UINT z = 0; z <= m_iGridSizeZ; ++z)
			{
				const csVector3 vVel = GetVelocityOfCellCenter(m_arvVelocityField[m_iLastIndex], x, y, z);
				if(x < m_iGridSizeX && y < m_iGridSizeY && z < m_iGridSizeZ)
				{
					const csVector3 vPos = Clamp(csVector3(x, y, z) - m_fTimeStep * m_fInvGridScale * vVel, m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
					//Even if vPos is on boundary, the interpolation works fine!
					m_arfPotTemperature[m_iActualIndex]->SetValue(GetInterpolatedValue(m_arfPotTemperature[m_iLastIndex], vPos), x, y, z);
					m_arfCondWaterMixingRatios[m_iActualIndex]->SetValue(GetInterpolatedValue(m_arfCondWaterMixingRatios[m_iLastIndex], vPos), x, y, z);
					m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(GetInterpolatedValue(m_arfWaterVaporMixingRatios[m_iLastIndex], vPos), x, y, z);
				}
				
				//Velocity-Field advection
				const csVector3 vPos2 = Clamp(csVector3(x + 0.5f, y + 0.5f, z + 0.5f) - m_fTimeStep * m_fInvGridScale * vVel, m_iGridSizeX + 1, m_iGridSizeY + 1, m_iGridSizeZ + 1);
				m_arvVelocityField[m_iActualIndex]->SetValue(GetInterpolatedVelocity(m_arvVelocityField[m_iLastIndex], vPos2), x, y, z);
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
	//Top: free-slip (xz plane, y = MAX)
	for(UINT x = 1; x < m_iGridSizeX; ++x)
	{
		for(UINT z = 1; z < m_iGridSizeZ; ++z)
		{
			//Bottom
			const csVector3 vCurr	= m_arvVelocityField[m_iActualIndex]->GetValue(x, 0, z);
			const csVector3 vAbove  = m_arvVelocityField[m_iActualIndex]->GetValue(x, 1, z);
			//Which one is correct?
			//m_arvVelocityField[m_iActualIndex]->SetValue(csVector3(vCurr.x, -vAbove.y, vCurr.z), x, 0, z);
			m_arvVelocityField[m_iActualIndex]->SetValue(csVector3(0.f, -vAbove.y, 0.f), x, 0, z);

			//Top
			const csVector3 vBorder = m_arvVelocityField[m_iActualIndex]->GetValue(x, m_iGridSizeY, z);
			const csVector3 vInner  = m_arvVelocityField[m_iActualIndex]->GetValue(x, m_iGridSizeY - 1, z);
			m_arvVelocityField[m_iActualIndex]->SetValue(csVector3(vBorder.x, vInner.y, vBorder.z), x, m_iGridSizeY, z);
		}
	}
}

//----------------------------------------------------------//

void csCloudsDynamics::UpdateMixingRatiosAndPotentialTemp()
{
	//Only the innercells are updated! All boudaries are set later
	for(UINT x = 1; x < m_iGridSizeX - 1; ++x)
	{
		for(UINT y = 1; y < m_iGridSizeY - 1; ++y)
		{
			const float fHeight = y * m_fGridScale + m_fBaseAltitude;
			for(UINT z = 1; z < m_iGridSizeZ - 1; ++z)
			{
				const float p		= ComputePressure(fHeight);
				const float fPi		= ::powf(p / m_fRefPressure, m_fKappa);
				const float T		= m_arfPotTemperature[m_iLastIndex]->GetValue(x, y, z) * fPi;

				//First update mixing ratios
				const float qvs		= ComputeSatVaporMixingRatioOnly(T, p);
				const float qv		= m_arfWaterVaporMixingRatios[m_iActualIndex]->GetValue(x, y, z);
				const float qc		= m_arfCondWaterMixingRatios[m_iActualIndex]->GetValue(x, y, z);
				//Negative condensation-Rate
				const float fmC		= qvs - qv < qc ? qvs - qv : qc;
				m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(qv + fmC, x, y, z);
				m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(qc - fmC, x, y, z);

				//Now potential temperature!
				const float fPotT	= m_arfPotTemperature[m_iActualIndex]->GetValue(x, y, z) + 
									  m_fLatentHeat * (-fmC) / (m_fSpecificHeatCapacity * fPi);
				m_arfPotTemperature[m_iActualIndex]->SetValue(fPotT, x, y, z);
			}
		}
	}
}

//----------------------------------------------------------//

/**
Boundaries are set for qc, qv and potT. 
qc.Top/Bottom/Sides = zero,
qv.Top = zero, qv.Sides = periodic, qv.Bottom = UserInputfield
potT.Top/Sides = Userspecific ambient temperature, potT.Bottom = userInputfield
*/
void csCloudsDynamics::SatisfyScalarBoundaryCond()
{
	//Front-Back: xy planes, z = 0 && z = MAX
	for(UINT x = 0; x < m_iGridSizeX; ++x)
	{
		for(UINT y = 0; y < m_iGridSizeY; ++y)
		{
			m_arfCondWaterMixingRatios[m_iActualIndex]->SetValue(0.f, x, y, m_iGridSizeZ - 1);
			m_arfCondWaterMixingRatios[m_iActualIndex]->SetValue(0.f, x, y, 0);
			m_arfPotTemperature[m_iActualIndex]->SetValue(m_fAmbientTemperature, x, y, 0);
			m_arfPotTemperature[m_iActualIndex]->SetValue(m_fAmbientTemperature, x, y, m_iGridSizeZ - 1);
			//Periodic sideboundaries
			const float qv1 = ::fabsf(::sinf((x * 0.2f + y * 2.4f) * m_fTimePassed));
			const float qv2 = ::fabsf(::sinf((x * 0.6f + y * 0.4f) * m_fTimePassed));
			m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(qv1, x, y, m_iGridSizeZ - 1);
			m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(qv2, x, y, 0);
		}
	}

	//Left-Right: yz-Plane, x = 0 && x = MAX
	for(UINT y = 0; y < m_iGridSizeY; ++y)
	{
		for(UINT z = 0; z < m_iGridSizeZ; ++z)
		{
			m_arfCondWaterMixingRatios[m_iActualIndex]->SetValue(0.f, m_iGridSizeX - 1, y, z);
			m_arfCondWaterMixingRatios[m_iActualIndex]->SetValue(0.f, 0, y, z);
			m_arfPotTemperature[m_iActualIndex]->SetValue(m_fAmbientTemperature, 0, y, z);
			m_arfPotTemperature[m_iActualIndex]->SetValue(m_fAmbientTemperature, m_iGridSizeX - 1, y, z);
			//Periodic sideboundaries
			const float qv1 = ::fabsf(::sinf((z * 1.9f + y * 4.1f) * m_fTimePassed));
			const float qv2 = ::fabsf(::sinf((z * 0.7f + y * 0.2f) * m_fTimePassed));
			m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(qv1, 0, y, z);
			m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(qv2, m_iGridSizeX - 1, y, z);
		}
	}

	//Bottom-Top: xz plane, y = 0 && y = MAX
	for(UINT x = 1; x < m_iGridSizeX - 1; ++x)
	{
		for(UINT z = 1; z < m_iGridSizeZ - 1; ++z)
		{
			m_arfCondWaterMixingRatios[m_iActualIndex]->SetValue(0.f, x, m_iGridSizeY - 1, z);
			m_arfCondWaterMixingRatios[m_iActualIndex]->SetValue(0.f, x, 0, z);
			m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(0.f, x, m_iGridSizeY - 1, z);
			m_arfPotTemperature[m_iActualIndex]->SetValue(m_fAmbientTemperature, x, m_iGridSizeY - 1, z);
			//Userspecific bottom-fields
			m_arfPotTemperature[m_iActualIndex]->SetValue(m_arfInputTemperature->GetValue(x, z), x, 0, z);
			m_arfWaterVaporMixingRatios[m_iActualIndex]->SetValue(m_arfInputWaterVapor->GetValue(x, z), x, 0, z);
		}
	}
}

//----------------------------------------------------------//

//After this method was invoked, the best approximation to the solution
//is found in m_arfPressureField[m_iNewPressureField]
void csCloudsDynamics::SolvePoissonPressureEquation(const UINT k)
{
	static const float s_fInvBeta = 1.f / 6.f;
	const float fGridScale2 = m_fGridScale * m_fGridScale;
	for(UINT i = 0; i < k; ++i)
	{
		//Switch roles
		m_iNewPressureField ^= m_iOldPressureField ^= m_iNewPressureField ^= m_iOldPressureField;
		JacobiSolver(m_arfPressureField[m_iNewPressureField], m_arfPressureField[m_iOldPressureField], 
					 m_arfVelDivergence, fGridScale2, s_fInvBeta);
	}
}

//----------------------------------------------------------//

void csCloudsDynamics::MakeVelocityFieldDivergenceFree()
{
	//Only the innercells are updated! All boudaries are set seperatly!
	for(UINT x = 1; x < m_iGridSizeX; ++x)
	{
		for(UINT y = 1; y < m_iGridSizeY; ++y)
		{
			for(UINT z = 1; z < m_iGridSizeZ; ++z)
			{
				const csVector3 vVel	= m_arvVelocityField[m_iActualIndex]->GetValue(x, y, z);
				const csVector3 vGrad	= CalcGradient(m_arfPressureField[m_iNewPressureField], x, y, z, m_fGridScale);
				//Ok this way?
				m_arvVelocityField[m_iActualIndex]->SetValue(vVel - vGrad, x, y, z);
			}
		}
	}
}

//----------------------------------------------------------//
//----------------------------------------------------------//

/**
fTime is taken only if the current simulation step is zero. Means
only at the beginning of an entire timestep. The value passed there
is used for the whole timestep!
*/
const bool csCloudsDynamics::DoComputationSteps(const UINT iStepCount, const float fTime)
{
	//preconditions hold? --> initialized?
	if(m_iGridSizeX <= 0 || m_iGridSizeY <= 0 || m_iGridSizeZ <= 0) return false;

	int iStepsLeft = iStepCount == 0 ? s_iTotalStepCount - m_iCurrentStep : iStepCount;
	while(iStepsLeft-- > 0)
	{
		switch(m_iCurrentStep)
		{
			case 0: 
			{
				m_fTimeStep = fTime;
				AdvectAllQuantities();
				break;
			}
			case 1: ComputeRotationField(); break;
			case 2: ComputeForceField(); break;
			case 3: AddAcceleratingForces(); break;

			case 4: UpdateMixingRatiosAndPotentialTemp(); break;
			case 5: SatisfyScalarBoundaryCond(); break;

			//Solving poisson-pressure equation
			case 6: ComputeDivergenceField(); break;
			//60 iterations
			case 7: SolvePoissonPressureEquation(30); break;
			case 8: SolvePoissonPressureEquation(30); break;
			//Subtruction of the pressure gradient from velocity
			case 9: MakeVelocityFieldDivergenceFree(); break;
			case 10: SatisfyVelocityBoundaryCond(); break;
		}
	
		if(++m_iCurrentStep >= s_iTotalStepCount)
		{
			m_iCurrentStep = 0;
			m_fTimePassed += m_fTimeStep;
			SwapFieldIndizes();
		}
	}

	return true;
}

//----------------------------------------------------------//

//----------------------------------------------------------//