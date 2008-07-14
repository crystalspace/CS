/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#ifndef __CS_CSCLOUDSAPI_H__
#define __CS_CSCLOUDSAPI_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

/**
This APIs implementation is a supervisor-class, which takes 
care and handles the overall cloud-simulation process on a higher
level of abstraction.
*/
struct iClouds : public virtual iBase
{
	SCF_INTERFACE(iClouds, 0, 0, 1);

	/**
	Initialize is used to startup the cloud-system according to some configuration
	variables. Such as grid-scale, grid size, ecc
	*/

	/**
	Does a single timestep of size dTime for the cloud-dynamics-simulation
	If dTime is not set, the time is measured automatically, in order to achive realtime-simulation
	*/
	virtual const bool DoTimeStep(const double dTime = 0.f) = 0;

	/**
	This method does an amortized time step: Means it doesn't do all the computations
	which would be necessary to complete an entire time step. No, it only does some minor
	calculations. For example 10 calls of this method would then be equal to one call
	of DoTimeStep. This method is designed for real-time-use
	dTime is used for the entire time-step. So if it varies between the single calls
	of this method, only the first value is considered! 
	If not set, the time is measured automatically, in order to achive realtime-simulation.
	*/
	virtual const bool DoAmortTimeStep(const double dTime = 0.f) = 0;

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
	virtual const bool Render(const iField3<double>& aaaMixingRatios /*, const csMatrix& mTransformation */) = 0;
};

//--------------------------------------------------------------------------------------------//

/**
This class simulates all the physics behind the cloud dynamics. This methods will
contain the bottleneck of the overall system. The output of the simulition
is a 3d- scalar field containing all the condensed water mixing ratios.
*/
struct iCloudsDynamics : public virtual iBase
{
	SCF_INTERFACE(iCloudsDynamics, 0, 0, 1);

	/**
	Does n computation steps. The overall calculations are split into several
	subtasks. This method computes n of those. If it gets a zero as input, it
	performs all for an entire time-step
	*/
	virtual const bool DoComputationSteps(const UINT iStepCount, const double dTime = 0.) = 0;

	/**
	Returns the simulation-output. A scalarfield which contains all the condensed
	water mixing ratios for the entire cloud-volume
	*/
	virtual inline const iField3<double> GetCondWaterMixingRatios() const = 0;
};

//--------------------------------------------------------------------------------------------//

/**
This class represents a 2 dimensional field (may scalar or vector!)
(used for boundary conditions e.g.)
*/
template <typename T>
struct iField2 : public virtual iBase
{
	SCF_INTERFACE(iField2, 0, 0, 1);

	/**
	*/
	virtual void SetSize(const UINT iSizeX, const UINT iSizeY) = 0;

	/**
	Sets a value at position x, y
	*/
	virtual void SetValue(const T& Value, const UINT x, const UINT y) = 0;

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
	SCF_INTERFACE(iField3, 0, 0, 1);

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
	at position P = (x, y)
	*/
	virtual const T operator () (const UINT x, const UINT y, const UINT z) const = 0;
	virtual const T GetValue(const UINT x, const UINT y, const UINT z) const = 0;
};


#endif // __CS_CSCLOUDSAPI_H__