/*
    Copyright (C) 2000 by Jorrit Tyberghein

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

#ifndef _ENGINEP_H_
#define _ENGINEP_H_

#include "iengine.h"
class csWorld;

/**
 * Engine plugin.
 */
struct csEngine : public iEngine
{
private:
  csWorld* world;

public:
  DECLARE_IBASE;

  /// Constructor.
  csEngine (iBase* parent);

  /// Destructor.
  virtual ~csEngine ();

  /// Initialize.
  virtual bool Initialize (iSystem* sys);
  /// Get pointer to the world.
  virtual iWorld *GetWorld () { return (iWorld*)world; }
};

#endif // _ENGINEP_H_

