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

/**\file 
 * Advanced Windowing System 2
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
struct iAws2ScriptObject;

typedef void Aws2ScriptObjectFunc(iAws2ScriptObject *);
	
SCF_VERSION(iAws2ScriptObject, 2, 0, 0);
struct iAws2ScriptObject
{			
	/** Sets the notification function. */
	virtual void SetNotification(Aws2ScriptObjectFunc *_func)=0;
	
	/** Returns the number of arguments in the notification. */
	virtual uint32 NumArgs()=0;
	
	/** Returns the argument in position arg as an integer. */
	virtual int32 GetIntArg(uint arg)=0;		
	
	/** Returns the argument in position arg as a double. */
	virtual double GetDoubleArg(uint arg)=0;		
	
	/** Returns the argument in position arg as a string. */
	virtual scfString GetStringArg(uint arg)=0;		
	
	/** Returns the named property as an integer, or zero if it doesn't exist. */
    virtual int32 GetIntProp(const scfString &name)=0;		
	
  	/** Returns the named property as a double, or zero if it doesn't exist. */
    virtual double GetDoubleProp(const scfString &name)=0;		
	
  	/** Returns the named property as a string, or an empty string if it doesn't exist. */
  	virtual scfString GetStringProp(const scfString &name)=0;		
  	
  	/** Sets the named property to the integer val. */
    virtual void SetIntProp(const scfString &name, int32 val)=0;		
	
    /** Sets the named property to the double val. */
    virtual void SetDoubleProp(const scfString &name, double val)=0;		
		
    /** Sets the named property to the string val. */
    virtual void SetStringProp(const scfString &name, const scfString val)=0;			
};


SCF_VERSION(iAws2, 2, 0, 0);
struct iAws2  : public iBase
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
  
  /// Creates a new script object with the given name. notification_func may be null, otherwise it should be the function to call.
  virtual iAws2ScriptObject *CreateScriptObject(const char *name, Aws2ScriptObjectFunc *notification_func=0)=0;
  
  /// Cached event names.  These should be set up in the ::Initialize method.
  csEventID PreProcess;
  csEventID MouseDown;
  csEventID MouseUp;
  csEventID MouseClick;
  csEventID MouseMove;
  csEventID KeyboardDown;
  csEventID KeyboardUp;

  csEventID MouseEnter;
  csEventID MouseExit;
  csEventID LoseFocus;
  csEventID GainFocus;
  csEventID GroupOff;
  csEventID FrameStart;
};


#endif
