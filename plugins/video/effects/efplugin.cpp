/*
    Copyright (C) 2002 by Anders Stenberg
    Written by Anders Stenberg

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
#include "csutil/scf.h"

#include "ivideo/effects/efserver.h"
#include "efserver.h"
#include "ivideo/effects/efdef.h"
#include "efdef.h"
#include "ivideo/effects/eftech.h"
#include "eftech.h"
#include "ivideo/effects/efpass.h"
#include "efpass.h"
#include "ivideo/effects/eflayer.h"
#include "eflayer.h"


CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE( csEffectServer )
  SCF_IMPLEMENTS_INTERFACE( iEffectServer )
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE( iComponent )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE( csEffectServer::Component )
  SCF_IMPLEMENTS_INTERFACE( iComponent )
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE( csEffectDefinition )
  SCF_IMPLEMENTS_INTERFACE( iEffectDefinition )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE( csEffectTechnique )
  SCF_IMPLEMENTS_INTERFACE( iEffectTechnique )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE( csEffectPass )
  SCF_IMPLEMENTS_INTERFACE( iBase )
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE( iEffectPass )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE( csEffectPass::EffectPass )
  SCF_IMPLEMENTS_INTERFACE( iEffectPass )
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE( csEffectLayer )
  SCF_IMPLEMENTS_INTERFACE( iBase )
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE( iEffectLayer )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE( csEffectLayer::EffectLayer )
  SCF_IMPLEMENTS_INTERFACE( iEffectLayer )
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_IBASE( csStateHandler )
  SCF_IMPLEMENTS_INTERFACE( iBase )
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_FACTORY( csEffectServer )

SCF_EXPORT_CLASS_TABLE( effects )
  SCF_EXPORT_CLASS( csEffectServer, "crystalspace.video.effects.stdserver", "Effects system" )
SCF_EXPORT_CLASS_TABLE_END

