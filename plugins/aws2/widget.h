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

#ifndef __AWS_WIDGET_H__
#define __AWS_WIDGET_H__

#include "frame.h"
#include "preferences.h"
#include "csutil/scfstr.h"
#include "csutil/array.h"
#include "pen.h"

extern "C" {
#include "js/jsapi.h"
}

/** @file Defines the widget object
 * 
 *   The widget object is used as a base for all user-interface
 * objects.  
 */

/** Creates an initializes the automation objects for a widget. */
void Widget_SetupAutomation();
 
namespace aws
{
  /** @brief The widget class is the native base for all visible widgets.
   * It also is the automation base for all visible widgets, and creates
   * several automation properties.
   *
   * It makes good sense to keep all the properties in the automation engine,
   * and provide native methods to access them, considering that most of the
   * manipulation of those properties will be done from scripts. */
  class widget 
  {    
	/** The widget object. */
    JSObject  *w_object;
    
    /** The context. */
    JSContext *ctx; 
    
    /** The pen for this widget.  A widget should not share this pen with
     * any siblings, children, or parents. */    
    pen *wpen;
    
  private:
  
    /** The frame provided for this widget. */
    frame fr;
    	
  	JSBool   dirty; /**< True if the widget has dirty state. */	
  	        
  public:
    /// Updates the skin preferences.
    // virtual void UpdateSkin(preferences &prefs)=0;
    
  public:
  	/// Sets the object for this widget.
     void SetWidgetObject(JSObject *o) { w_object=o; }
                
     /// Gets the object for this widget.
     JSObject *WidgetObject() { return w_object; }
     
     /// Gets the Javascript context for this widget.
     JSContext *AutomContext() { return ctx; }
     
  public:
  	 widget():wpen(0), dirty(JS_TRUE) {}
  	 virtual ~widget() {}
  	 
  	 /** Sets the pen used for drawing this widget. */
  	 void SetPen(pen *_pen) { wpen = _pen; }
  	 
  	 /** Gets the pen used for drawing this widget. */
  	 pen *GetPen() { return wpen; }
  	   
  	 /** Used by the callback to set the value of a property. */
  	 bool SetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
  	 
  	 /** Used by the callback to get the value of a property. */
  	 bool GetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);  
  	 
  	 /** Gets a reference to the bounds. */
  	 csRect &Bounds() { return fr.Bounds(); }
  	 
  	 /** Gets a reference to the frame. */
  	 frame &Frame() { return fr; }
       
     /** @brief Invalidates the widget so that it will be redrawn. */
     void Invalidate() { dirty = JS_TRUE; }
      
     /** @brief Returns true if the widget has been invalidated, false otherwise. */
     bool IsDirty()    { return dirty == JS_TRUE; }
          
     /** Smart draw function, only calls the Automation draw if the
      * widget is dirty.  Otherwise it just replays the cached
      * content. */
     virtual void Draw(iPen *output_pen);
      
  };  
  
  /** @brief The type for the widget list. */
  typedef csArray<widget *> widget_list_t;
  
  /** The global list of all widgets. */
  extern widget_list_t widgets;
}

#endif
