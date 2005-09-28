/**************************************************************************
    Copyright (C) 2005 by Christopher Nelson

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
*****************************************************************************/

#ifndef __AWS_INTERFACE_20_H__
#define __AWS_INTERFACE_20_H__

#ifdef __AWS_INTERFACE_10_H__
# error "aws.h included before aws2.h.  You cannot mix and match the two versions!"
#endif

/**\file 
 * Advanced Windowing System
 */

#include "csutil/scf.h"
#include "csutil/refarr.h"
#include "csutil/stringarray.h"
#include "csutil/scfstr.h"
#include "csgeom/csrect.h"
#include "csgeom/vector2.h"
#include "iutil/event.h"
#include "iutil/string.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"

struct iObjectRegistry;

namespace aws
{

namespace autom
{
  class string;
  class integer;
  class floating;
  
  /** Base class for all Keila objects. */
  struct iObject 
  {    
    SCF_INTERFACE(autom::iObject,0,0,1);
    enum TYPE { T_STRING, T_INT, T_FLOAT, T_LIST, T_MAP, T_FUNCTION,
      	T_REFERENCE, T_BLOB, T_NIL };

    /**
     * Returns the type of the object, a member of the object::TYPE
     * enumeration.
     */
    virtual TYPE ObjectType() = 0;
      
    /** Sets the name of the object. */
    virtual void SetName(iString* _name) = 0;
      
    /** Gets the name of the object. */
    virtual csRef<iString> GetName() = 0;
      
    /** Converts the object into a string object if possible. */
    virtual string ToString() = 0;
      
    /** Converts the object into an integer object, if possible. */
    virtual integer ToInt() = 0;
      
    /** Converts the object into a float object, if possible. */
    virtual floating ToFloat() = 0;	
      
    /**
     * Converts the object into the text representation of it. This is the
     * inverse of parsing.
     */
    virtual csRef<iString> ReprObject() = 0;      
  };

} // namespace autom

} // namespace aws

/*SCF_VERSION(aws::autom::iObject, 1, 0, 1);*/


SCF_VERSION(iAwsWindow, 1, 0, 1);
struct iAwsWindow : public iBase
{
  int empty;

};


SCF_VERSION(iAws, 1, 0, 1);
struct iAws  : public iBase
{
  /// Must be called before anything else.
  virtual bool Initialize (iObjectRegistry *_object_reg)=0;

  /// Setup the drawing targets.
  virtual void SetDrawTarget(iGraphics2D *_g2d, iGraphics3D *_g3d)=0;

  /// Load a definition file.
  virtual bool Load(const scfString &_filename)=0;

  /// Dispatches events to the proper components.
  virtual bool HandleEvent (iEvent &)=0;  

  /// Redraws all the windows into the current graphics contexts.
  virtual void Redraw()=0;
};


#endif
