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
#include "csgeom/matrix4.h"
#include "ivideo/texture.h"

//--------------------------------------------------------------------------------------------//

/**
This class represents a 2 dimensional float field
(used for boundary conditions e.g.)
*/
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
  virtual const float operator () (const UINT x, const UINT y) const = 0;
  virtual const float GetValue(const UINT x, const UINT y) const = 0;
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
  SCF_INTERFACE(iClouds, 0, 6, 5);

  /**
  These setters configure the entire system:
  TimeScaleFactor: The system measures the time which is taken to compute an
  entire timestep and passes the value as next timestep-size scaled by this factor. 
  (1.f means realtime)!
  SkippingFrameCount: Count of frames which are skipped doing NO dynamic computation
  pass. Means, if SkippingFrameCount is equal to N, then every N-th frame is done ONE
  computation step.
  IterationLimitPerInvocation: For the dynamicsimulation. Each invocation of DoComputation
  there is passed a limit for the iterations to be done. This limit can be set here.
  */
  virtual inline void SetTimeScaleFactor(const float f) = 0;
  virtual inline void SetSkippingFrameCount(const UINT i) = 0;                //Tuning parameter
  virtual inline void SetIterationLimitPerInvocation(const UINT i) = 0;       //Tuning parameter

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
  virtual inline void SetTemperaturBottomInputField(csRef<iField2> Field) = 0;
  virtual inline void SetWaterVaporBottomInputField(csRef<iField2> Field) = 0;

  /**
  All of following Setters refer to the iCloudsRenderer instance.
  */
  virtual inline void SetRenderGridScale(const float dx) = 0;
  virtual inline void SetCloudPosition(const csVector3& vPosition) = 0;
  virtual inline void SetLightDirection(const csVector3& vLightDir) = 0;
  virtual inline void SetImpostorValidityAngle(const float fAngle) = 0;
  //Getter of iCloudsRenderer
  virtual inline const UINT GetOLVSliceCount() const = 0;
  virtual inline const UINT GetOLVWidth() const = 0;
  virtual inline const UINT GetOLVHeight() const = 0;
  virtual inline const CS::Math::Matrix4 GetOLVProjectionMatrix() const = 0;
  virtual inline const CS::Math::Matrix4 GetOLVCameraMatrix() const = 0;
  virtual inline iTextureHandle* GetOLVTexture() const = 0;
};

//--------------------------------------------------------------------------------------------//

/**
This class does all the rendering and illumination stuff.
The only input it needs is the condensed-water-mixing-ration
scalar field!
*/
struct iCloudsRenderer : public virtual iBase
{
  SCF_INTERFACE(iCloudsRenderer, 0, 3, 1);

  /**
  Some usful getter
  */
  virtual inline const UINT GetOLVSliceCount() const = 0;
  virtual inline const UINT GetOLVWidth() const = 0;
  virtual inline const UINT GetOLVHeight() const = 0;
  virtual inline const CS::Math::Matrix4 GetOLVProjectionMatrix() const = 0;
  virtual inline const CS::Math::Matrix4 GetOLVCameraMatrix() const = 0;
  virtual inline iTextureHandle* GetOLVTexture() const = 0;

  /**
  All following methods are used to configure the entire rendering process
  */
  virtual inline void SetGridScale(const float dx) = 0;
  virtual inline void SetCloudPosition(const csVector3& vPosition) = 0;
  virtual inline void SetLightDirection(const csVector3& vLightDir) = 0;
  virtual inline void SetImpostorValidityAngle(const float fAngle) = 0;
};

//--------------------------------------------------------------------------------------------//

/**
This class simulates all the physics behind the cloud dynamics. This methods will
contain the bottleneck of the overall system. The output of the simulition
is a 3d- scalar field containing all the condensed water mixing ratios.
*/
struct iCloudsDynamics : public virtual iBase
{
  SCF_INTERFACE(iCloudsDynamics, 0, 6, 1);

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
  virtual inline void SetTemperaturBottomInputField(csRef<iField2> Field) = 0;
  virtual inline void SetWaterVaporBottomInputField(csRef<iField2> Field) = 0;

  /**
  Updates all constant and precomputeted parameters according to the user specific values set!
  Has to be called whenever one value has been changed!
  */
  virtual inline void UpdateAllDependParameters() = 0;
};

//--------------------------------------------------------------------------------------------//

/**
This class takes care of an entire cloud field. It's instance is registred in object
registry and all clouds should be added and removed through this one.
*/
struct iCloudSystem : public virtual iBase
{
  SCF_INTERFACE(iCloudSystem, 0, 2, 3);

  /**
  Some getter which provide various information about the field
  */
  virtual inline const UINT GetCloudCount() const = 0;
  virtual inline const iClouds* GetCloud(const UINT i) const = 0;

  /**
  Add and remove clouds from the field
  */
  virtual iClouds* AddCloud() = 0;
  virtual const bool RemoveCloud(iClouds* pCloud) = 0;
  virtual const bool RemoveCloud(const UINT iIndex) = 0;
  virtual inline const bool RemoveAllClouds() = 0;
};

//--------------------------------------------------------------------------------------------//


#endif // __CS_IMESH_CLOUDS_H