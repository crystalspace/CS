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

enum { W_DOCK_NORTH, W_DOCK_SOUTH, W_DOCK_EAST,  W_DOCK_WEST };
 
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
    
    /** The parent widget, if any. */
    widget *parent;
    
    /** The list of children that this widget has.
     * Children draw *after* the parent. */
    csArray<widget *> children;
    
    /** The list of <i>sibling</i> widgets that
     * are docked to this widget.  Whenever a 
     * sibling gets a move request, it updates 
     * all of the widgets in it's docked list.
     * That way they end up moving together. */
    widget *docked[4];
    
    /** A set of bits that tell us what sides of our parent
     * we want to stick to. */
    uint8 sticky_frame_flags;
      
    /** The frame provided for this widget. */
    frame fr;
    	
    /** True if the widget has dirty state. */	
  	JSBool   dirty; 
  	
  	        
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
  	 widget():wpen(0), parent(0), sticky_frame_flags(0), dirty(JS_TRUE) 
  	 {
	 	memset(docked, 0, sizeof(docked)); 	 
	 }  	 
  	 
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
  	 
  	 /// Children ////
  	 
  	 /** @brief Adds a child widget into this widget. */
  	 void AddChild(widget *w)
  	 {
	  	w->parent = this;
	  	children.Push(w);	 
  	 }
  	 
  	 /** @brief Removes a child widget from this widget. */
  	 void RemoveChild(widget *w)
  	 {
	  	w->parent = 0;
	  	children.Delete(w);	 
  	 }
  	 
  	 /** @brief Adds a docking widget to this widget. */
  	 void Dock(widget *w, int where)
  	 {
	  	if (where>=0 && where<4)
	  	{ 
		  	docked[where]=w;	  	 
	  	 	MoveDocked(where);
  	 	}	  	 		  	
  	 }
  	 
  	 /** @brief Moves widgets docked to this widget. */
  	 void MoveDocked(int which=-1)
  	 {
	  	 int start = (which==-1 ? 0 : which);
	  	 int end   = (which==-1 ? 4 : which+1);
	  	 
	  	 for(int where=start; where<end; ++where)
	  	 {	  	 	
		  	widget *w = docked[where];
		  	
		  	if (w==0) continue; 
		  	   	 
		  	switch(where)
		  	{
			  	case W_DOCK_NORTH:		  		
			  		w->Bounds().SetPos(Bounds().xmin, Bounds().ymin-w->Bounds().Height());
			  		w->Bounds().SetSize(Bounds().Width(), w->Bounds().Height());
			  		break;
			  		
			  	case W_DOCK_SOUTH:
			  		w->Bounds().SetPos(Bounds().xmin, Bounds().ymax+1);
			  		w->Bounds().SetSize(Bounds().Width(), w->Bounds().Height());
			  		break;
			  		
			  	case W_DOCK_EAST:
			  		w->Bounds().SetPos(Bounds().xmax+1, Bounds().ymin);
			  		w->Bounds().SetSize(w->Bounds().Width(), Bounds().Height());
			  		break;
			  		
			  	case W_DOCK_WEST:
			  		w->Bounds().SetPos(Bounds().xmin-w->Bounds().Width(), Bounds().ymin);
			  		w->Bounds().SetSize(w->Bounds().Width(), Bounds().Height());
			  		break;
		  	}		  	 
	  	}
  	 }  
  	 
  	 /** @brief Adjusts all children when we've been resized. */
  	 void AdjustChildrenForStickiness()
  	 {
	  	 for(size_t i=0; i<children.Length(); ++i)
	  	 {
		 	children[i]->AdjustForStickiness();	 
	  	 }	  	 
  	 }
  	 
  	 /** @brief Adjusts our frame to our parent's frame, accounting for the sticky bits. */
  	 void AdjustForStickiness()
  	 {
	  	 // No parent, can't adjust for stickiness
	  	 if (parent==0) return;
	  	 
	  	 if (sticky_frame_flags & (1<<W_DOCK_NORTH)) Bounds().ymin = 0; 
	  	 if (sticky_frame_flags & (1<<W_DOCK_SOUTH)) Bounds().ymax = parent->Bounds().Height();
	  	 if (sticky_frame_flags & (1<<W_DOCK_EAST))  Bounds().xmax = parent->Bounds().Width();
	  	 if (sticky_frame_flags & (1<<W_DOCK_WEST))  Bounds().xmin = 0;	  	 
  	 }	 
  	 
  	 /** @brief Sets a stick frame bit. */
  	 void SetFrameAnchor(int32 bit)
  	 {
	  	sticky_frame_flags |= (bit<<1);	 
  	 }
  	 
  	 /** @brief Clears a sticky frame bit. */
  	 void ClearFrameAnchor(int32 bit)
  	 {
	  	sticky_frame_flags &= ~(bit<<1);	 
  	 }
  	 
  	 /// Drawing ////
       
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
