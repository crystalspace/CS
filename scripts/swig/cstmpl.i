// This is the template which cs.i is constructed from.
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



//-----------------------------------------------------------------------------
// A Swig interface definition which provides access to many classes within
// the Crystal Space engine.
//-----------------------------------------------------------------------------


%module cspace
%{
  #include "css.h"
//***** SCF Wrappers
  int MakeVersion(int version0, int version1, int version2)
  {
    return SCF_CONSTRUCT_VERSION(version0, version1, version2);
  }

#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/objreg.h"


#include "iengine/camera.h"
#include "iengine/campos.h"
#include "imesh/object.h"
#include "imesh/thing/thing.h"
#include "imesh/thing/lightmap.h"
#include "imap/parser.h"
#include "ivideo/graph2d.h"
#include "ivideo/fontserv.h"
#include "ivideo/halo.h"

%%include_generated_headers%%

%}

%include pointer.i

// Include the hand written custom parts.
%include "scripts/swig/custom.i"



//%include "include/iutil/evdefs.h"
//%include "scripts/swig/iutil/csinput.h"

//%include "scripts/swig/cs_incs/iutil/evdefs.h"
//%include "scripts/swig/cs_incs/iutil/csinput.h"

//%include "scripts/swig/generatd.i"


// ----------------------------------------------------------
// The contents of the processed header files go here.

%%processed_header_contents%%

// contents of the processed header files ends here.
// ----------------------------------------------------------


//****** System Interface
struct iObjectRegistry:public iBase
{
public:
  %addmethods
  {
    iEngine* Query_iEngine()
    {
      iEngine* en = CS_QUERY_REGISTRY (self, iEngine);
      en->DecRef ();
      return en;
    }
    iGraphics3D* Query_iGraphics3D()
    {
      iGraphics3D* g3d = CS_QUERY_REGISTRY (self, iGraphics3D);
      g3d->DecRef ();
      return g3d;
    }
    iKeyboardDriver* Query_iKeyboardDriver()
    {
      iKeyboardDriver* kbd = CS_QUERY_REGISTRY (self, iKeyboardDriver);

//      iKeyboardDriver* kbd = (  iKeyboardDriver*)(( self )->Get ("iKeyboardDriver", iSCF::SCF->GetInterfaceID ("iKeyboardDriver"), VERSION_iKeyboardDriver)) ;
//      kbd->DecRef ();
      return kbd;
    }


    void Print(int mode, const char* format) {
      printf (format);
    }
  }
};












