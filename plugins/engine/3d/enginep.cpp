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

#include "cssysdef.h"
#include "csengine/engine.h"

CS_IMPLEMENT_PLATFORM_PLUGIN

/*
 * Package the engine library into a plug-in module.
 * Since the engine is already entirely plug-in compatible (that is, it already
 * implements all necessary SCF goop), all we need to do here is to create a
 * dummy reference to it in order to force the linker to include the engine
 * library in the generated plug-in file.
 */
void EnginePluginDummyReference()
{
  csEngine* p = new csEngine(0);
  delete p;
}
