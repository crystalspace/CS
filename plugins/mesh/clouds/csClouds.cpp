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
#include "csClouds.h"

SCF_IMPLEMENT_FACTORY(csClouds)

//-----------------------------------------------------------//

bool csClouds::HandleEvent(iEvent& rEvent)
{
  if(m_iFramesUntilNextStep == 0)
  {
    if(m_Dynamics->NewTimeStepStarted())
    {
      //Measure Time
      const UINT iEndTickCount = m_Clock->GetCurrentTicks();
      m_fTimeStep = static_cast<float>(iEndTickCount - m_iStartTickCount) * 0.001f * m_fTimeScaleFactor;
      m_iStartTickCount = iEndTickCount;
      printf("Time for one simulation step: %.4f\n", m_fTimeStep);
    }
    //Do a step
    //CS::MeasureTime Timer("DoComputationSteps");
    m_Dynamics->DoComputation(m_iIterationsPerInvocation, m_fTimeStep);

    m_iFramesUntilNextStep = m_iSkippingFrameCount;
  }
  else --m_iFramesUntilNextStep;

  return true;
}

//-----------------------------------------------------------//