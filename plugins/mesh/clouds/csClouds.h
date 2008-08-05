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

#include <iutil/objreg.h>
#include <iutil/comp.h>
#include <iutil/eventq.h>
#include <iutil/virtclk.h>
#include <csutil/measuretime.h>
#include <csgeom/vector3.h>
#include <csutil/csbaseeventh.h>
#include "imesh/clouds.h"
#include "csCloudsDynamics.h"
#include "csCloudsRenderer.h"

//Supervisor-class implementation
class csClouds : public scfImplementation3<csClouds, iClouds, iComponent, iEventHandler>
{
private:
  csRef<csCloudsDynamics>      m_Dynamics;
  csRef<csCloudsRenderer>      m_Renderer;
  iObjectRegistry*             m_pObjectRegistry;
  csRef<iEventQueue>           m_pEventQueue;
  csRef<iVirtualClock>         m_Clock;

  UINT                         m_iStartTickCount;
  UINT                         m_iFramesUntilNextStep;
  float                        m_fTimeStep;

  //Config-Vars
  UINT                         m_iIterationsPerInvocation;
  float                        m_fTimeScaleFactor;
  UINT                         m_iSkippingFrameCount;

public:
  csClouds(iBase* pParent) : scfImplementationType(this, pParent), m_iFramesUntilNextStep(0), m_fTimeStep(1.f)
  {
    m_fTimeScaleFactor       = 1.f;
    m_iSkippingFrameCount    = 10;
  }
  ~csClouds()
  {
    m_Dynamics.Invalidate();
    m_Renderer.Invalidate();
    m_pEventQueue->Unsubscribe(this, csevFrame(m_pObjectRegistry));
    m_pEventQueue.Invalidate();
    m_Clock.Invalidate();
  }

  //Initialisation method from iComponent
  virtual bool Initialize(iObjectRegistry* pObjectReg)
  {
    m_Dynamics.AttachNew(new csCloudsDynamics(this));
    m_Renderer.AttachNew(new csCloudsRenderer(this));
    m_pObjectRegistry = pObjectReg;
    //Getting Virtual Clock
    m_Clock = csQueryRegistry<iVirtualClock>(m_pObjectRegistry);
    m_iStartTickCount = m_Clock->GetCurrentTicks();
    //Getting EventQueue
    m_pEventQueue = csQueryRegistry<iEventQueue>(m_pObjectRegistry);
    m_pEventQueue->RegisterListener(this, csevFrame(m_pObjectRegistry));
    return true;
  }

  //Own setters
  virtual inline void SetIterationLimitPerInvocation(const UINT i) {m_iIterationsPerInvocation = i;}
  virtual inline void SetTimeScaleFactor(const float f) {m_fTimeScaleFactor = fabsf(f);}
  virtual inline void SetSkippingFrameCount(const UINT i) {m_iSkippingFrameCount = i;}

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
  virtual inline void SetTemperaturBottomInputField(csRef<iField2> Field) {return m_Dynamics->SetTemperaturBottomInputField(Field);}
  virtual inline void SetWaterVaporBottomInputField(csRef<iField2> Field) {return m_Dynamics->SetWaterVaporBottomInputField(Field);}

  //All of following Setters refer to the csCloudsRenderer instance, and are delegated!

  //EventHandler-Part
  virtual bool HandleEvent(iEvent& rEvent);
  CS_EVENTHANDLER_NAMES("crystalspace.mesh.clouds");
  CS_EVENTHANDLER_NIL_CONSTRAINTS;
};

#endif // __CSCLOUD_PLUGIN_H__