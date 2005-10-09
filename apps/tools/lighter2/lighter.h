/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __LIGHTER_H__
#define __LIGHTER_H__

namespace lighter
{
  class Scene;
  struct Sector;
  struct Light;
  class RadPrimitive;
  class Raytracer;

  class Lighter
  {
  public:
    Lighter (iObjectRegistry *objectRegistry);
    ~Lighter ();

    // Initialize and load plugins we want
    bool Initialize ();

    // Do the fancy lighting
    bool LightEmUp ();

    // Report an error/warning, always returns false
    bool Report (const char* msg, ...);

    // Public members
    csRef<iDocumentSystem> docSystem;
    csRef<iEngine> engine;
    csRef<iImageIO> imageIO;
    csRef<iLoader> loader;
    csRef<iPluginManager> pluginManager;
    csRef<iReporter> reporter;
    csRef<iVFS> vfs;
    iObjectRegistry *objectRegistry;

  protected:
    // Parse the commandline and load any files specified
    bool LoadFiles ();

    // Shoot direct lighting for given sector
    void ShootDirectLighting (Sector* sector);

    // Shade a rad primitive from a lightsource
    void ShadeRadPrimitive (Raytracer &tracer, RadPrimitive &prim, Light* light);

    
    Scene *scene;
  };

  // Global lighter
  extern Lighter* globalLighter;
}

#endif

