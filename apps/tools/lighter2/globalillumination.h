/*
  Copyright (C) 2008 by Greg Hoffman

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

#ifndef __GLOBALILLUMINATION_H__
#define __GLOBALILLUMINATION_H__

#include "common.h"
#include <csutil/threading/thread.h>

namespace lighter
{
  class GIRunnable : public CS::Threading::Runnable
  {
  public:
    /**
    * Default Constructor
    * Computes the lightmaps for the given sector.
    * /param sect - the sector to compute
    */
    GIRunnable(Sector *sect);
	
    /**
    * Run
    * Implemented function from Runnable for CS threading. This function
    * does the actual work.
    */
    void Run();

  private:
    Sector *sector;
  };
	
  class GlobalIllumination : private CS::NonCopyable
  {
  public:
    /**
    * Default Constructor
    * Initializes the class to do the global illumination calculations
    */
    GlobalIllumination();

    /**
    * Constructor
    * Initializes the class to do the global illumination calculations
    * /param config - the config structure for indirect lighting
    */
    GlobalIllumination(Configuration::INDIProperties config);

    /**
    * Shade Indirect Lighting
    * Shades the sector for indirect lighting using photon maps
    * /param sect - the sector to shade for 
    */
    void ShadeIndirectLighting(Sector *sect);

    /**
    * EmitPhotons
    * Emits photons in the given sector from the lights
    */
    void EmitPhotons(Sector *sect);

  private:
    csRandomVectorGen randVect;
    csRandomFloatGen randFloat;
    bool finalGather;
    int numFinalGatherRays;
    float searchRadius;
    int numPhotonsPerLight;
  };

}

#endif
