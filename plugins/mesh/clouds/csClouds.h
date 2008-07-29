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

#ifndef __CSCLOUD_PLUGIN_H__
#define __CSCLOUD_PLUGIN_H__

#include <csgeom/vector3.h>
#include "imesh/clouds.h"
#include "csCloudsDynamics.h"
#include "csCloudsRenderer.h"

//Supervisor-class implementation
class csClouds : public scfImplementation1<csClouds, iClouds>
{
private:
	csRef<iCloudsDynamics>			m_Dynamics;
	csRef<iCloudsRenderer>			m_Renderer;

public:
	csClouds(iBase* pParent) : scfImplementationType(this, pParent)
	{
		m_Dynamics.AttachNew(new csCloudsDynamics(this));
		m_Renderer.AttachNew(new csCloudsRenderer(this));
    
	}
	~csClouds()
	{
		m_Dynamics.Invalidate();
		m_Renderer.Invalidate();
	}

  //All of following Setters refer to the csCloudsDynamics instance, and are delegated!
  virtual inline const bool SetGridSize(const UINT x, const UINT y, const UINT z) {return m_Dynamics->SetGridSize(x, y, z);}
  virtual inline void SetGridScale(const float dx) {return m_Dynamics->SetGridScale(dx);}
	virtual inline void SetCondensedWaterScaleFactor(const float fqc) {return m_Dynamics->SetCondensedWaterScaleFactor(fqc);}
	virtual inline void SetGravityAcceleration(const csVector3& vG) {return m_Dynamics->SetGravityAcceleration(vG);}
	virtual inline void SetVorticityConfinementForceEpsilon(const float e) {return m_Dynamics->SetVorticityConfinementForceEpsilon(e);}
	virtual inline void SetReferenceVirtPotTemperature(const float T) {return m_Dynamics->SetReferenceVirtPotTemperature(T);}
	virtual inline void SetTempLapseRate(const float G) {return m_Dynamics->SetTempLapseRate(G);}
	virtual inline void SetReferenceTemperature(const float T) {return m_Dynamics->SetReferenceTemperature(T);}
	virtual inline void SetReferencePressure(const float p) {return m_Dynamics->SetReferencePressure(p);}
	virtual inline void SetIdealGasConstant(const float R) {return m_Dynamics->SetIdealGasConstant(R);}
	virtual inline void SetLatentHeat(const float L) {return m_Dynamics->SetLatentHeat(L);}
	virtual inline void SetSpecificHeatCapacity(const float cp) {return m_Dynamics->SetSpecificHeatCapacity(cp);}
	virtual inline void SetAmbientTemperature(const float T) {return m_Dynamics->SetAmbientTemperature(T);}
	virtual inline void SetInitialCondWaterMixingRatio(const float qc) {return m_Dynamics->SetInitialCondWaterMixingRatio(qc);}
	virtual inline void SetInitialWaterVaporMixingRatio(const float qv) {return m_Dynamics->SetInitialWaterVaporMixingRatio(qv);}
	virtual inline void SetGlobalWindSpeed(const csVector3& vWind) {return m_Dynamics->SetGlobalWindSpeed(vWind);}
	virtual inline void SetBaseAltitude(const float H) {return m_Dynamics->SetBaseAltitude(H);}
	virtual inline void SetTemperaturBottomInputField(csRef<iField2<float>> Field) {return m_Dynamics->SetTemperaturBottomInputField(Field);}
	virtual inline void SetWaterVaporBottomInputField(csRef<iField2<float>> Field) {return m_Dynamics->SetWaterVaporBottomInputField(Field);}
	virtual const bool DoTimeStep(const float fTime = 0.f);
	virtual const bool DoAmortTimeStep(const float fTime = 0.f);
	virtual const bool RenderClouds();
};

#endif // __CSCLOUD_PLUGIN_H__