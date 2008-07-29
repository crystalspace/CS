/*
    Copyright (C) 2008 by Julian Mautner

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

#ifndef __CS_IMESH_CLOUDS_H
#define __CS_IMESH_CLOUDS_H

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

#include "csutil/array.h"
#include "csutil/ref.h"
#include "csgeom/vector3.h"

//--------------------------------------------------------------------------------------------//

/**
This class represents a 2 dimensional field (may scalar or vector!)
(used for boundary conditions e.g.)
*/
template <typename T>
struct iField2 : public virtual iBase
{
	SCF_INTERFACE(iField2, 1, 0, 0);

	/**
	*/
	virtual void SetSize(const UINT iSizeX, const UINT iSizeY) = 0;
	virtual const UINT GetSizeX() const = 0;
	virtual const UINT GetSizeY() const = 0;

	/**
	Accses operator and method. Returns the value of the scalarfield
	at position P = (x, y)
	*/
	virtual const T operator () (const UINT x, const UINT y) const = 0;
	virtual const T GetValue(const UINT x, const UINT y) const = 0;
};

/**
This class represents a 3 dimensional field (may scalar or vector!)
(used as input for the rendering algorithm e.g.)
*/
template <typename T>
struct iField3 : public virtual iBase
{
	SCF_INTERFACE(iField3, 1, 0, 0);

	/**
	*/
	virtual void SetSize(const UINT iSizeX, const UINT iSizeY, const UINT iSizeZ) = 0;
	virtual const UINT GetSizeX() const = 0;
	virtual const UINT GetSizeY() const = 0;
	virtual const UINT GetSizeZ() const = 0;

	/**
	Sets a value at position x, y, z
	*/
	virtual void SetValue(const T& Value, const UINT x, const UINT y, const UINT z) = 0;

	/**
	Accses operator and method. Returns the value of the scalarfield
	at position P = (x, y, z)
	*/
	virtual const T operator () (const UINT x, const UINT y, const UINT z) const = 0;
	virtual const T GetValue(const UINT x, const UINT y, const UINT z) const = 0;

	/**
	Returns the value of the field at Position x, y, z. If x, y or z are not in
	range, they are going to be clamped first!
	*/
	virtual const T GetValueClamp(const int x, const int y, const int z) const = 0;
};

//--------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------//

/**
This APIs implementation is a supervisor-class, which takes 
care and handles the overall cloud-simulation process on a higher
level of abstraction.
*/
struct iClouds : public virtual iBase
{
	SCF_INTERFACE(iClouds, 0, 5, 1);

	/**
	All of following Setters refer to the iCloudsDynamics instance.
	*/
  virtual inline const bool SetGridSize(const UINT x, const UINT y, const UINT z) = 0;
	virtual inline void SetGridScale(const float dx) = 0;
	virtual inline void SetCondensedWaterScaleFactor(const float fqc) = 0;
	virtual inline void SetGravityAcceleration(const csVector3& vG) = 0;
	virtual inline void SetVorticityConfinementForceEpsilon(const float e) = 0;
	virtual inline void SetReferenceVirtPotTemperature(const float T) = 0;
	virtual inline void SetTempLapseRate(const float G) = 0;
	virtual inline void SetReferenceTemperature(const float T) = 0;
	virtual inline void SetReferencePressure(const float p) = 0;
	virtual inline void SetIdealGasConstant(const float R) = 0;
	virtual inline void SetLatentHeat(const float L) = 0;
	virtual inline void SetSpecificHeatCapacity(const float cp) = 0;
	virtual inline void SetAmbientTemperature(const float T) = 0;
	virtual inline void SetInitialCondWaterMixingRatio(const float qc) = 0;
	virtual inline void SetInitialWaterVaporMixingRatio(const float qv) = 0;
	virtual inline void SetGlobalWindSpeed(const csVector3& vWind) = 0;
	virtual inline void SetBaseAltitude(const float H) = 0;
	virtual inline void SetTemperaturBottomInputField(csRef<iField2<float>> Field) = 0;
	virtual inline void SetWaterVaporBottomInputField(csRef<iField2<float>> Field) = 0;

  /**
	All of following Setters refer to the iCloudsRenderer instance.
	*/

	/**
	Does a single timestep of size dTime for the cloud-dynamics-simulation
	If dTime is not set, the time is measured automatically, in order to achive realtime-simulation
	*/
	virtual const bool DoTimeStep(const float fTime = 0.f) = 0;

	/**
	This method does an amortized time step: Means it doesn't do all the computations
	which would be necessary to complete an entire time step. No, it only does some minor
	calculations. For example 10 calls of this method would then be equal to one call
	of DoTimeStep. This method is designed for real-time-use
	dTime is used for the entire time-step. So if it varies between the single calls
	of this method, only the first value is considered! 
	If not set, the time is measured automatically, in order to achive realtime-simulation.
	*/
	virtual const bool DoAmortTimeStep(const float fTime = 0.f) = 0;

	/**
	This method starts the rendering process. All clouds are rendered at
	the calculated positions and formations.
	*/
	virtual const bool RenderClouds(/* Transformationmatrix? */) = 0;
};

//--------------------------------------------------------------------------------------------//

/**
This class does all the rendering and illumination stuff.
The only input it needs is the condensed-water-mixing-ration
scalar field!
*/
struct iCloudsRenderer : public virtual iBase
{
	SCF_INTERFACE(iCloudsRenderer, 0, 0, 1);

	/**
	Rendermethod. Renders the whole cloud scene.
	*/
	virtual const bool Render(const csRef<iField3<float>>& rCondWaterMixingRatios /*, const csMatrix& mTransformation */) = 0;
};

//--------------------------------------------------------------------------------------------//

/**
This class simulates all the physics behind the cloud dynamics. This methods will
contain the bottleneck of the overall system. The output of the simulition
is a 3d- scalar field containing all the condensed water mixing ratios.
*/
struct iCloudsDynamics : public virtual iBase
{
	SCF_INTERFACE(iCloudsDynamics, 0, 5, 2);

	/**
	This is the most importand initialisation method. It defines the dimensions
	of the entire grid. The standard after constructor was called is 10x10x10
	*/
	virtual inline const bool SetGridSize(const UINT x, const UINT y, const UINT z) = 0;

	/**
	Following methods are used to configure the entire dynamics simulation
	*/
	virtual inline void SetGridScale(const float dx) = 0;
	virtual inline void SetCondensedWaterScaleFactor(const float fqc) = 0;
	virtual inline void SetGravityAcceleration(const csVector3& vG) = 0;
	virtual inline void SetVorticityConfinementForceEpsilon(const float e) = 0;
	virtual inline void SetReferenceVirtPotTemperature(const float T) = 0;
	virtual inline void SetTempLapseRate(const float G) = 0;
	virtual inline void SetReferenceTemperature(const float T) = 0;
	virtual inline void SetReferencePressure(const float p) = 0;
	virtual inline void SetIdealGasConstant(const float R) = 0;
	virtual inline void SetLatentHeat(const float L) = 0;
	virtual inline void SetSpecificHeatCapacity(const float cp) = 0;
	virtual inline void SetAmbientTemperature(const float T) = 0;
	virtual inline void SetInitialCondWaterMixingRatio(const float qc) = 0;
	virtual inline void SetInitialWaterVaporMixingRatio(const float qv) = 0;
	virtual inline void SetGlobalWindSpeed(const csVector3& vWind) = 0;
	virtual inline void SetBaseAltitude(const float H) = 0;
	virtual inline void SetTemperaturBottomInputField(csRef<iField2<float>> Field) = 0;
	virtual inline void SetWaterVaporBottomInputField(csRef<iField2<float>> Field) = 0;

	/**
	Updates all constant and precomputeted parameters according to the user specific values set!
	Has to be called whenever one value has been changed!
	*/
	virtual inline void UpdateAllDependParameters() = 0;

	/**
	Does n computation steps. The overall calculations are split into several
	subtasks. This method computes n of those. If it gets a zero as input, it
	performs all for an entire time-step
	*/
	virtual const bool DoComputationSteps(const UINT iStepCount, const float fTime = 0.) = 0;

	/**
	Returns the simulation-output. A scalarfield which contains all the condensed
	water mixing ratios for the entire cloud-volume
	*/
	virtual inline const csRef<iField3<float>>& GetCondWaterMixingRatios() const = 0;
};

//--------------------------------------------------------------------------------------------//


#endif // __CS_IMESH_CLOUDS_H