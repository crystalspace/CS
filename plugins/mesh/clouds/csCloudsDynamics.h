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
#include "csCloudsStandards.h"

/**
Supervisor-class implementation:
This class represents a three dimensional voxel-grid of user specific size.
Each parcel is a cube (simplifies calculations!) of size dx * dx * dx (m_fGridSize).
The whole simulation updates each timestep the entire grid. Output of the simulation
is the condensed water mixing ratio of each parcel. (MAC-Grid is used for the voxel volume)
The local coordinate system is equal to the one used as global with:
+y   +/-z (left or right handed! In practice it doesn't matter)
|  /
|/
---- +x
*/
class csCloudsDynamics : public scfImplementation1<csCloudsDynamics, iCloudsDynamics>
{
  static const UINT s_iTotalStepCount = 10;
private:
  /**
  From each field there are two instances, because each time-step all of
  them have to be advected, while the last vaild state has to be mantained.
  Therefore iAcutalIndex gives the index of the last updated instance 
  and iLastIndex gives the other one. Both are either	0 or 1. 
  At the end of each timestep those two are going to be swapped.
  */
  UINT   m_iLastIndex;
  UINT   m_iActualIndex;
  /**
  Temperature, pressure and both mixing ratios are definded at the center
  of each voxel. Indexing is therefore as always f(x, y, z)
  */
  csRef<csField3<float>>         m_arfPotTemperature[2];               // T, potential temperature
  csRef<csField3<float>>         m_arfCondWaterMixingRatios[2];        // qc
  csRef<csField3<float>>         m_arfWaterVaporMixingRatios[2];       // qv
  csRef<csField3<csVector3>>     m_arvForceField;
  csRef<csField3<float>>         m_arfVelDivergence;
  csRef<csField3<float>>         m_arfPressureField[2];                // p
  /**
  Velocity is defined at the boundaries of each cell. Half-way index
  notation is used in consequence. These fields are of size N + 1
  */
  csRef<csField3<csVector3>>   m_arvVelocityField[2];                  // u

  //This rotation-field is defined at Cell-Centers!
  csRef<csField3<csVector3>>   m_arvRotVelField;                       // rot(u)

  float                       m_fTimeStep;
  float                       m_fTimePassed;
  UINT                        m_iGridSizeX;
  UINT                        m_iGridSizeY;
  UINT                        m_iGridSizeZ;
  float                       m_fGridScale;                           // dx
  UINT                        m_iNewPressureField;
  UINT                        m_iOldPressureField;
  bool                        m_bNewTimeStep;

  //State variables for amortized computation
  UINT                        m_iCurrentStep;
  /**
  The user is able to specify the total number of iterations for each invokation
  of the simulation each N frames. When iIterationsLeft reaches zero, the computation
  is interreupted and the current state is saved. Afterwards the simulation is continued
  from this certain point.
  */
  int                         m_iIterationsLeft;
  UINT                        m_iIterationsPerInvokation;
  UINT                        m_iPoissonSolverIterationsCount;
  UINT                        m_iTempX;
  UINT                        m_iTempY;
  UINT                        m_iTempZ;
  bool                        m_bRestore;
  inline const bool LimitReached() {return --m_iIterationsLeft <= 0;}
  inline void SaveState(const UINT x, const UINT y, const UINT z) {m_iTempX = x; m_iTempY = y; m_iTempZ = z; m_bRestore = true;}
  inline void RestoreState(UINT* px, UINT* py, UINT* pz) {*px = m_iTempX; *py = m_iTempY; *pz = m_iTempZ; m_bRestore = false;}

  //Precomputed constants
  float                       m_fInvGridScale;                        // 1 / dx
  float                       m_fKappa;                               // _R / _cp
  float                       m_fAltitudeExponent;                    // |_g| / (_R * _G)

  //====================================================//
  //            USER SPECIFIC VARIABLES				  //
  //====================================================//
  //Epslion for the vorticityConfinement-Force calculaion
  float                       m_fVCEpsilon;                       // _e
  //Inverse of reference virtual potential temperature
  float                       m_fInvRefVirtPotTemp;               // _Tp
  //Scaling factor for condensed water in buoyant-force-calculaion
  float                       m_fCondWaterScaleFactor;            // _fqc
  //Acceleration due to gravitation
  csVector3                   m_vGravitationAcc;                  // _g
  //Condensation-Rate
  float                       m_fCondensationRate;                // _C
  //Preasure at sealevel
  float                       m_fRefPressure;                     // _p0
  //Temperature Lapse rate
  float                       m_fTempLapseRate;                   // _G
  //Temperature at sea-level
  float                       m_fRefTemperature;                  // _T0
  //Latent heat of vaporization of water
  float                       m_fLatentHeat;                      // _L
  //Ideal gas konstant for dry air
  float                       m_fIdealGasConstant;                // _R
  //Specific heat capacity (dry air, constant pressure)
  float                       m_fSpecificHeatCapacity;            // _cp
  //Ambient temperature
  float                       m_fAmbientTemperature;              // _TA
  //Initial-value for condensed water mixing ratio
  float                       m_fInitCondWaterMixingRatio;
  //Initial-value for water vapor mixing ratio
  float                       m_fInitWaterVaporMixingRatio;
  //Global windspeed
  csVector3                   m_vWindSpeed;
  //Absolute Height of the bottom grid face
  float                       m_fBaseAltitude;
  //Bottom input field for Temperature
  csRef<iField2>              m_arfInputTemperature;
  //Bottom input field for water vapor
  csRef<iField2>              m_arfInputWaterVapor;
  //====================================================//

  //Calculates the rotation of the velocity field u, and stores it in arvRotVelField
  //O(n^3)
  inline void ComputeRotationField()
  {
    for(UINT x = 0; x < m_iGridSizeX; ++x)
    {
      for(UINT y = 0; y < m_iGridSizeY; ++y)
      {
        for(UINT z = 0; z < m_iGridSizeZ; ++z)
        {
          if(m_bRestore) RestoreState(&x, &y, &z);
          if(LimitReached()) {SaveState(x, y, z); return;}
          m_arvRotVelField->SetValue(CalcRotation(m_arvVelocityField[m_iActualIndex], x, y, z, m_fGridScale), x, y, z);
        }
      }
    }
  }

  //Calculates the divergence of the velocity field u, and stores it in arfVelDivergence
  //O(n^3)
  inline void ComputeDivergenceField()
  {
    for(UINT x = 0; x < m_iGridSizeX; ++x)
    {
      for(UINT y = 0; y < m_iGridSizeY; ++y)
      {
        for(UINT z = 0; z < m_iGridSizeZ; ++z)
        {
          if(m_bRestore) RestoreState(&x, &y, &z);
          if(LimitReached()) {SaveState(x, y, z); return;}
          m_arfVelDivergence->SetValue(CalcDivergence(m_arvVelocityField[m_iActualIndex], x, y, z, m_fGridScale), x, y, z);
        }
      }
    }
  }

  //Computes for each cell the buoyant and the vorticity confinement force
  //O(n^3)
  inline void ComputeForceField()
  {
    for(UINT x = 0; x < m_iGridSizeX; ++x)
    {
      for(UINT y = 0; y < m_iGridSizeY; ++y)
      {
        for(UINT z = 0; z < m_iGridSizeZ; ++z)
        {
          if(m_bRestore) RestoreState(&x, &y, &z);
          if(LimitReached()) {SaveState(x, y, z); return;}
          const csVector3 vForce = ComputeBuoyantForce(x, y, z) + ComputeVorticityConfinement(x, y, z);
          m_arvForceField->SetValue(vForce, x, y, z);
        }
      }
    }
  }

  //Calculates the saturation vapor mixing ratio, depending on T (temperature) and p (pressure)
  inline const float ComputeSatVaporMixingRatioOnly(const float T, const float p) const
  {
    return (380.16f / p) * ::expf((17.67 * T) / (T + 243.5f));
  }
  //Calculates the Temperature depending on p and T (potential temperature)
  inline const float ComputeTemperature(const float p, const float fPotTemp) const
  {
    return fPotTemp * ::powf(p / m_fRefPressure, m_fKappa);
  }
  //Calcuates the pressure depending on altitude (absolute height!)
  inline const float ComputePressure(const float h) const
  {
    return m_fRefPressure * ::powf(1 - (h * m_fTempLapseRate / m_fRefTemperature), m_fAltitudeExponent);
  }

  //All-In-One computation of qs (saturation vapor mixing ratio), depending on height
  //and potential temperature
  inline const float ComputeSatVaporMixingRatio(const float fPotTemp, const float h) const
  {
    const float p = ComputePressure(h);
    const float T = ComputeTemperature(p, fPotTemp);
    return ComputeSatVaporMixingRatioOnly(T, p);
  }

  //Returns the vorticity confinement force of a certain parcel depending on rot(u), dx, _e
  const csVector3 ComputeVorticityConfinement(const UINT x, const UINT y, const UINT z);
  //Returns the buoyant force of a certain parcel depending on _g, qc, T, _Tp, _fqc
  const csVector3 ComputeBuoyantForce(const UINT x, const UINT y, const UINT z);

  //Implements the straightforward jacobi solver
  //O(n^3)
  void JacobiSolver(csRef<csField3<float>> rNew, const csRef<csField3<float>>& rOld, 
                    const csRef<csField3<float>>& rBField, const float fAlpha, const float fInvBeta);

  //Updates qc and qv and T
  //O(n^3)
  void UpdateMixingRatiosAndPotentialTemp();

  //Advects temperature, mixing ratios and velocity itself
  //O(n^3)
  void AdvectAllQuantities();

  //Add accelerating forces (Buoyant and VoricityConfinement) to u
  //O(n^3)
  void AddAcceleratingForces();

  //Solves the poisson-pressure equation. Uses k iteration of a solver to do so
  //O(k * n^3)    --> BottleNeck! k = ca. 40-80
  void SolvePoissonPressureEquation(const UINT k);

  //Subtracts from velocity field the gradient of the calculated pressure-field
  //O(n^3)
  void MakeVelocityFieldDivergenceFree();

  //After this method was invoked all velocity boundarycondition on u ar satisfied
  void SatisfyVelocityBoundaryCond();
  //Updates boundaries of qc, qv and PotT
  void SatisfyScalarBoundaryCond();

  //Frees all reserved memory
  inline void FreeReservedMemory()
  {
    m_iGridSizeX = m_iGridSizeY = m_iGridSizeZ = 0;
    m_arfCondWaterMixingRatios[0].Invalidate(); m_arfCondWaterMixingRatios[1].Invalidate();
    m_arfWaterVaporMixingRatios[0].Invalidate(); m_arfWaterVaporMixingRatios[1].Invalidate();
    m_arfPotTemperature[0].Invalidate(); m_arfPotTemperature[1].Invalidate();
    m_arvVelocityField[0].Invalidate(); m_arvVelocityField[1].Invalidate();
    m_arvRotVelField.Invalidate();
    m_arvForceField.Invalidate();
    m_arfVelDivergence.Invalidate();
    m_arfPressureField[0].Invalidate();
    m_arfPressureField[1].Invalidate();
  }

  //For all userspecific values there are standard once too, which are set here!
  inline void SetStandardValues()
  {
    SetGridScale(1.f);
    SetCondensedWaterScaleFactor(1.f);
    SetGravityAcceleration(csVector3(0.f, -9.81f, 0.f));
    SetVorticityConfinementForceEpsilon(0.01f);
    SetReferenceVirtPotTemperature(290.f);
    SetTempLapseRate(0.01f);                               //10 K/km = 0.01 K/m
    SetReferenceTemperature(290.f);
    SetReferencePressure(101300.f);                        //1.013 bar = 101300 N/m²
    SetIdealGasConstant(287.f);                            //287 J/(kg K)
    SetLatentHeat(2.501f);                                 //2.501 J/kg
    SetSpecificHeatCapacity(1005.f);                       //1005 J/(kg K)
    SetAmbientTemperature(290.f);
    SetInitialCondWaterMixingRatio(0.0f);
    SetInitialWaterVaporMixingRatio(0.8f);
    SetBaseAltitude(0.f);
    SetIterationLimitPerInvokation(3000);

    //Input-fields
    m_arfInputTemperature.Invalidate();
    m_arfInputTemperature.AttachNew(new csStdTemperatureInputField(this));
    m_arfInputWaterVapor.Invalidate();
    m_arfInputWaterVapor.AttachNew(new csStdWaterVaporInputField(this));
  }

  //swaps actualIndex and LastIndex
  inline void SwapFieldIndizes()
  {
    m_iActualIndex ^= m_iLastIndex ^= m_iActualIndex ^= m_iLastIndex;
  }

public:
  csCloudsDynamics(iBase* pParent) : scfImplementationType(this, pParent), m_iLastIndex(1), m_iActualIndex(0),
    m_iGridSizeX(0), m_iGridSizeY(0), m_iGridSizeZ(0), m_fSpecificHeatCapacity(1.f), m_fTimePassed(0.f),
    m_iCurrentStep(0), m_iNewPressureField(0), m_iOldPressureField(1), m_bRestore(false),
    m_iPoissonSolverIterationsCount(0), m_bNewTimeStep(true)
  {
    SetStandardValues();
    UpdateAllDependParameters();
    SetGridSize(16, 16, 16);
  }
  ~csCloudsDynamics() {}

  //Std grid size is 16x16x16
  virtual inline const bool SetGridSize(const UINT x, const UINT y, const UINT z);

  //Configuration-Setter
  virtual inline void SetGridScale(const float dx) {m_fGridScale = dx; m_fInvGridScale = 1.f / dx;}
  virtual inline void SetCondensedWaterScaleFactor(const float fqc) {m_fCondWaterScaleFactor = fqc;}
  virtual inline void SetGravityAcceleration(const csVector3& vG) {m_vGravitationAcc = vG;}
  virtual inline void SetVorticityConfinementForceEpsilon(const float e) {m_fVCEpsilon = e;}
  virtual inline void SetReferenceVirtPotTemperature(const float T) {m_fInvRefVirtPotTemp = 1.f / T;}
  virtual inline void SetTempLapseRate(const float G) {m_fTempLapseRate = G;}
  virtual inline void SetReferenceTemperature(const float T) {m_fRefTemperature = T;}
  virtual inline void SetReferencePressure(const float p) {m_fRefPressure = p;}
  virtual inline void SetIdealGasConstant(const float R) {m_fIdealGasConstant = R;}
  virtual inline void SetLatentHeat(const float L) {m_fLatentHeat = L;}
  virtual inline void SetSpecificHeatCapacity(const float cp) {m_fSpecificHeatCapacity = cp;}
  virtual inline void SetAmbientTemperature(const float T) {m_fAmbientTemperature = T;}
  virtual inline void SetInitialCondWaterMixingRatio(const float qc) {m_fInitCondWaterMixingRatio = qc;}
  virtual inline void SetInitialWaterVaporMixingRatio(const float qv) {m_fInitWaterVaporMixingRatio = qv;}
  virtual inline void SetGlobalWindSpeed(const csVector3& vWind) {m_vWindSpeed = vWind;}
  virtual inline void SetBaseAltitude(const float H) {m_fBaseAltitude = H;}
  virtual inline void SetIterationLimitPerInvokation(const UINT i) {m_iIterationsPerInvokation = i;}
  virtual inline void SetTemperaturBottomInputField(csRef<iField2> Field)
  {
    //Größen überprüfen
    if(Field->GetSizeX() != m_iGridSizeX || Field->GetSizeY() != m_iGridSizeZ) return;
    m_arfInputTemperature.Invalidate();
    m_arfInputTemperature = Field;
  }
  virtual inline void SetWaterVaporBottomInputField(csRef<iField2> Field)
  {
    //Größen überprüfen
    if(Field->GetSizeX() != m_iGridSizeX || Field->GetSizeY() != m_iGridSizeZ) return;
    m_arfInputWaterVapor.Invalidate();
    m_arfInputWaterVapor = Field;
  }

  //Updates all constant and precomputeted parameters according to the user specific values set!
  virtual inline void UpdateAllDependParameters()
  {
    m_fKappa            = m_fIdealGasConstant / m_fSpecificHeatCapacity;
    m_fAltitudeExponent = m_vGravitationAcc.Norm() / (m_fTempLapseRate * m_fIdealGasConstant);
  }

  //Getter
  inline const UINT GetCurrentStep() const {return m_iCurrentStep;}
  inline const bool NewTimeStepStarted() const {return m_bNewTimeStep;}

  /**
  Computes N steps of the entire simulation. If iStepCount == 0, then an entire timestep
  is calculated
  */
  virtual const bool DoComputationSteps(const UINT iStepCount, const float fTime = 0.f);

  //Returns the simulation output!
  virtual inline const csRef<csField3<float>>& GetCondWaterMixingRatios() const
  {
    /**
    Always when an entire timestep was done, the acutal-index becomes the last-index
    So the lastindex fields are those of the LAST COMPLETLY DONE TIMESTEP!
    */
    return m_arfCondWaterMixingRatios[m_iLastIndex];
  }
};

#endif // __CSCLOUDDYNAMICS_PLUGIN_H__