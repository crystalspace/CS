/*
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
*/

#ifndef __AWS_PEN_H__
#define __AWS_PEN_H__

#include "cstool/pen.h"
#include "csutil/memfile.h"

#include "javascript.h"

/** @file Defines the pen object
 * 
 *   The pen object is used as a base for all user-interface
 * objects.  
 */

/** Creates an initializes the automation objects for a pen. */
void Pen_SetupAutomation();
 
namespace aws
{
  /** @brief The pen class is the native base for all visible pens.
   * It also is the automation base for all visible pens, and creates
   * several automation properties.
   *
   * It makes good sense to keep all the properties in the automation engine,
   * and provide native methods to access them, considering that most of the
   * manipulation of those properties will be done from scripts. */
  class pen : public csMemoryPen
  {    
	/** The pen object. */
    JSObject  *p_object;   
    
    /** The buffer that contains the shtuff we want. */
    csMemFile *buf;
    
       
  public:         
  	 /// Sets the object for this pen
     void SetPenObject(JSObject *o) { p_object=o; }
    
     /// Gets the object for this pen.
     JSObject *PenObject() { return p_object; }     
         
  public:
  	 pen():buf(0) { Clear(); }
  	 virtual ~pen() {}
  	   	  	  	   
  	 /** Used by the callback to set the value of a property. */
  	 bool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
  	 
  	 /** Used by the callback to get the value of a property. */
  	 bool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);    	       
  };  
  
  
}

/** Returns true if the object is a pen object. */
  bool IsPenObject(JSObject *o);
  

#endif
