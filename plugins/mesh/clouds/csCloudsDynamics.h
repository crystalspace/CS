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

#ifndef __CSCLOUDDYNAMICS_PLUGIN_H__
#define __CSCLOUDDYNAMICS_PLUGIN_H__

#include <csgeom/vector3.h>
#include "imesh/clouds.h"
#include "csCloudsUtils.h"

//Supervisor-class implementation
class csCloudsDynamics : public scfImplementation1<csCloudsDynamics, iCloudsDynamics>
{
private:
	/**
	From each field there are two instances. Because each time-step all of
	them have to be advected, and so the older field is updated, while the other
	is mantained in the other instance. iAcutalIndex gives the index of the
	last updated instance and iLastIndex gives the other one. Both are either
	0 or 1.
	*/
	UINT						m_iLastIndex;
	UINT						m_iActualIndex;
	csRef<iField3<float>>		m_arfPotTemperature[2];				// T, potential temperature
	csRef<iField3<float>>		m_arfCondWaterMixingRatios[2];		// qc
	csRef<iField3<float>>		m_arfWaterVaporMixingRatios[2];		// qv
	csRef<iField3<csVector3>>	m_arvVelocityField[2];				// u
	UINT						m_iGridSizeX;
	UINT						m_iGridSizeY;
	UINT						m_iGridSizeZ;
	float						m_fGridScale;						// dx
	float						m_fInvGridScale;					// 1 / dx

	//====================================================//
	//            USER SPECIFIC VARIABLES				  //
	//====================================================//
	//Epslion for the vorticityConfinement-Force calculaion
	float						m_fVCEpsilon;						// _e
	//Inverse of reference virtual potential temperature. std: 1 / 300
	float						m_fInvRefVirtPotTemp;					// _T
	//Scaling factor for condensed water in buoyant-force-calculaion
	float						m_fCondWaterScaleFactor;			// _fqc
	//Acceleration due to gravitation
	csVector3					m_vGravitationAcc;					// _g
	

	//Returns the vorticity confinement force of a certain parcel depending on u, dx, _e
	const csVector3 ComputeVorticityConfinement(const UINT x, const UINT y, const UINT z);
	//Returns the buoyant force of a certain parcel depending on _g, qc, T, _T, _fqc
	const csVector3 ComputeBuoyantForce(const UINT x, const UINT y, const UINT z);

public:
	csCloudsDynamics(iBase* pParent) : scfImplementationType(this, pParent), m_iLastIndex(1), m_iActualIndex(0),
		m_iGridSizeX(0), m_iGridSizeY(0), m_iGridSizeZ(0)
	{
	}
	~csCloudsDynamics() {}

	//Configuration-Setter
	virtual inline void SetGridScale(const float dx) {m_fGridScale = dx; m_fInvGridScale = 1.f / dx;}
	virtual inline void SetGridSize(const UINT x, const UINT y, const UINT z)
	{
		m_iGridSizeX = x;
		m_iGridSizeY = y;
		m_iGridSizeZ = z;
		m_arfPotTemperature[0].AttachNew(new csField3<float>(this));
		m_arfPotTemperature[0]->SetSize(m_iGridSizeX, m_iGridSizeY, m_iGridSizeZ);
	}
	virtual inline void SetCondensedWaterScaleFactor(const float fqc) {m_fCondWaterScaleFactor = fqc;}
	virtual inline void SetGravityAcceleration(const csVector3& vG) {m_vGravitationAcc = vG;}
	virtual inline void SetVorticityConfinementForceEpsilon(const float e) {m_fVCEpsilon = e;}
	virtual inline void SetReferenceVirtPotTemperature(const float T) {m_fInvRefVirtPotTemp = 1.f / T;}

	virtual const bool DoComputationSteps(const UINT iStepCount, const float fTime = 0.f);
	virtual inline const iField3<float>* GetCondWaterMixingRatios() const
	{
		//Always when an entire timestep was done, the acutal-index becomes the last-index
		//So the lastindex fields are those of the LAST COMPLETLY DONE TIMESTEP!
		m_arfCondWaterMixingRatios[m_iLastIndex]->IncRef();
		return m_arfCondWaterMixingRatios[m_iLastIndex];
	}
};

#endif // __CSCLOUDDYNAMICS_PLUGIN_H__