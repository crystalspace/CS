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
enum { W_STICK_NORTH=1,  W_STICK_SOUTH=2,  W_STICK_EAST=4,  W_STICK_WEST=8, 
       W_TRACK_NORTH=16, W_TRACK_SOUTH=32, W_TRACK_EAST=64, W_TRACK_WEST=128};
 
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
     * sibling gets a move or resize request, it updates 
     * all of the widgets in it's docked list.
     * That way they end up moving/resizing together. */
    widget *docked[4];
    
    /** The list of <i>sibling</i> widgets that
     * this widget is docked to.  If this widget
     * receives a move or resize request, the resize or move will
     * be reflected to the appropriate widget. */
    widget *docked_to[4];
    
    /** A set of bits that tell us what sides of our parent
     * we want to stick to. */
    uint8 sticky_frame_flags;
    
    /** A set of spacer margins for the sticky and dock adjustments.
     * This much space will be inserted between the edges for each adjustment. */
    int32 margins[4];     
      
    /** The frame provided for this widget. */
    frame fr;
    	
    /** True if the widget has dirty state. */	
  	JSBool   dirty; 
  	
  	/** The widget above this one. */
  	widget *above;
  	
  	/** The widget below this one. */
  	widget *below;
  	        
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
  	 widget():wpen(0), parent(0), sticky_frame_flags(0), dirty(JS_TRUE), above(0), below(0)
  	 {
	 	memset(docked, 0, sizeof(docked)); 
	 	memset(docked_to, 0, sizeof(docked_to)); 
	 	memset(margins, 0, sizeof(margins));	 
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
  	 
  	 /** Gets the widget above this one. */
  	 widget *Above() { return above; }
  	 
  	 /** Gets the widget below this one. */
  	 widget *Below() { return below; }
  	 
  	 /** Gets the top parent. */
  	 widget *TopParent() { if (parent) return parent->TopParent(); else return this; }
  	 
  	 /** Gets the Northernmost widget. */
  	 widget *NorthMost() { if (docked_to[W_DOCK_NORTH]) return docked_to[W_DOCK_NORTH]->NorthMost(); else return this; }
  	 
  	 /** Slides this window to the top. */
  	 void Raise(widget *top);
  	   	   	 
  	 /** Removes a widget from the input and drawing chain. */
  	 void Unlink();
  	 
  	 /** Removes all docked widgets from the view/input chain. */
  	 void UnlinkDocked();
  	 
  	 /** Links above the given widget.   */
  	 void Link(widget *w);
  	 
  	 /** Adds all docked widgets into the view/input chain. */
  	 void LinkDocked(widget *w);  	 
  	 
  	 /** Returns true if the widget is a child of this widget. */
  	 bool HasChild(widget *w)
  	 {
	  	 for(size_t i=0; i<children.Length(); ++i)
	  	 {
		   if (children[i] == w) return true;	   		   
		   if (children[i]->HasChild(w)) return true;
	  	 }	  	 
	  	 
	  	 return false;
  	 }
  	 
  	 /** Adjusts the coordinates from screen space to widget space. */
  	 void ScreenToWidget(int &x, int &y)
  	 {	  	 
	  	 x-=Bounds().xmin;
	  	 y-=Bounds().ymin;	  	 
	  	 
	  	 if (parent) parent->ScreenToWidget(x,y);
  	 }
  	 
  	 /// Events ////
  	 
  	 /// Dispatches events fed to this widget.
  	 virtual bool HandleEvent (iEvent &);
  	 
  	 /** Broadcasts an event (by triggering a given function) to all children 
  	  * of this widget. */
  	 void Broadcast(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);  
  	 
  	 /** Sends a "signal", which is really just calling a given function for this object.
  	  * A simple generic signal has no arguments. */
  	 void Signal(const char *func_name);
  	 
  	 
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
	  	 switch(where)
	  	 {
		  	 case W_DOCK_NORTH:
		  	 	docked[W_DOCK_NORTH] = w;
		  	 	w->docked_to[W_DOCK_SOUTH] = this;
		  	 	AdjustDocked(W_DOCK_NORTH);
		  	 	break;
		  	 	
		  	 case W_DOCK_SOUTH:
		  	 	docked[W_DOCK_SOUTH] = w;
		  	 	w->docked_to[W_DOCK_NORTH] = this;
		  	 	AdjustDocked(W_DOCK_SOUTH);
		  	 	break;
		  	 	
		  	 case W_DOCK_EAST:
		  	 	docked[W_DOCK_EAST] = w;
		  	 	w->docked_to[W_DOCK_WEST] = this;
		  	 	AdjustDocked(W_DOCK_EAST);
		  	 	break;
		  	 	
		  	 case W_DOCK_WEST:
		  	 	docked[W_DOCK_WEST] = w;
		  	 	w->docked_to[W_DOCK_EAST] = this;
		  	 	AdjustDocked(W_DOCK_WEST);
		  	 	break;
		  	 	
		  	 default:
		  	 	return;
	  	 };		  	 
	  		  	 		  	
  	 }
  	 
  	 /** @brief Adjusts widgets docked to this widget. */
  	 void AdjustDocked(int which=-1)
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
			  		w->Bounds().SetPos(Bounds().xmin, Bounds().ymin-w->Bounds().Height()-margins[W_DOCK_SOUTH]);
			  		w->Bounds().SetSize(Bounds().Width(), w->Bounds().Height());
			  		w->Invalidate();
			  		break;
			  		
			  	case W_DOCK_SOUTH:
			  		w->Bounds().SetPos(Bounds().xmin, Bounds().ymax+1+margins[W_DOCK_NORTH]);
			  		w->Bounds().SetSize(Bounds().Width(), w->Bounds().Height());
			  		w->Invalidate();
			  		break;
			  		
			  	case W_DOCK_EAST:
			  		w->Bounds().SetPos(Bounds().xmax+1+margins[W_DOCK_WEST], Bounds().ymin);
			  		w->Bounds().SetSize(w->Bounds().Width(), Bounds().Height());
			  		w->Invalidate();
			  		break;
			  		
			  	case W_DOCK_WEST:
			  		w->Bounds().SetPos(Bounds().xmin-w->Bounds().Width()-margins[W_DOCK_EAST], Bounds().ymin);
			  		w->Bounds().SetSize(w->Bounds().Width(), Bounds().Height());
			  		w->Invalidate();		  		
			  		break;			  	
		  	}
		  	
		  	w->AdjustDocked();
		  	w->AdjustChildrenForStickiness();
		  			  	 
	  	}
  	 }  
  	   	
  	 /** @brief Reflects the movement to the appropriate widget. */
  	 void ReflectMove(int32 dx, int32 dy)
  	 {
	  	 for(int where=0; where<4; ++where)
	  	 {
		  	 if (docked_to[where]) 
		  	 {
			  	 docked_to[where]->ReflectMove(dx, dy);		  	 
			  	 return;
		  	 }
	  	 }	 		 
	  	 
	  	 Bounds().Move(dx, dy);
		 AdjustDocked();	  	 
  	 }
  	 
  	 /** Reflects the resize to the appropriate widget. */
  	 void ReflectResize(int32 dw, int32 dh)
  	 {
	  	if (dw!=0 && docked_to[W_DOCK_NORTH])
	  	{
		 	docked_to[W_DOCK_NORTH]->ReflectResize(dw, 0);
		 	dw=0; 	
	  	}
	  	else if (dw!=0 && docked_to[W_DOCK_SOUTH])
	  	{
		 	docked_to[W_DOCK_SOUTH]->ReflectResize(dw, 0);
		 	dw=0; 	
	  	}
	  	
	  	if (dw!=0 && docked_to[W_DOCK_EAST])
	  	{
		 	docked_to[W_DOCK_EAST]->ReflectResize(0, dh);
		 	dh=0; 	
	  	}
	  	else if (dw!=0 && docked_to[W_DOCK_WEST])
	  	{
		 	docked_to[W_DOCK_WEST]->ReflectResize(0, dh);
		 	dh=0; 	
	  	}	  
	  	
	  	if (dw!=0 || dh!=0)
	    {
		 Bounds().xmax+=dw;
		 Bounds().ymax+=dh;
	
		 AdjustDocked();
		 AdjustChildrenForStickiness();
		 Invalidate();   
	    }	  	  		
  	 }
  	 
  	 /** @brief Adjusts all children when we've been resized. */
  	 void AdjustChildrenForStickiness()
  	 {
	  	 for(size_t i=0; i<children.Length(); ++i)
	  	 {
		 	children[i]->AdjustForStickiness();	 
		 	children[i]->AdjustDocked();
		 	children[i]->Invalidate();
	  	 }	  	 
  	 }
  	 
  	 /** @brief Returns a pointer to the widget that contains
  	  * this point.  Either this widget or a child will be returned
  	  * on success.  On failure, return null. 
  	  * @param x The X coord
  	  * @param y The Y coord */
  	 widget *Contains(int x, int y)
  	 {
	  	 if (Bounds().Contains(x,y)==false) return 0;
	  	 
	  	 for(size_t i=0; i<children.Length(); ++i)
	  	 {
		 	widget *w = children[i]->Contains(x-Bounds().xmin,y-Bounds().ymin);
		 	if (w) return w;	 		 	
	  	 }	  	 
	  	 
	  	 return this;
  	 }
  	 
  	 /** @brief Adjusts our frame to our parent's frame, accounting for the sticky bits. 
  	  * There are two sets of sticky bits.  The first set indicates whether the sticky
  	  * side is on or off.  If it's off, nothing happens.  If it's on, the default behavior
  	  * is to stick that side of the frame the the same side of the parent frame.  This will
  	  * generally cause that side to move in and out while the other side remains static.
  	  * The net result is that the frame will stretch and shrink.  In many cases this is what
  	  * you want.  In other cases you might want the frame to stick to one side of the parent
  	  * frame, but retain it's shape.  In that case, the second set of sticky bits comes into
  	  * play.  If the second set of bits (the no-stretch set) has the bit set, this function
  	  * makes sure that the widget retains it's shape. */
  	 void AdjustForStickiness()
  	 {
	  	 // No parent, can't adjust for stickiness
	  	 if (parent==0) return;
	  	 
// 	  	 csString msg;
// 	  	 
// 	  	 msg.Format("Sticky flags: %x", (uint32)sticky_frame_flags);
// 	  	 ScriptCon()->Message(msg);
	  	 
	  	 if (sticky_frame_flags & W_STICK_NORTH)
	  	 {
		  	  // If the no-resize flag is set, adjust the bottom too. This keeps
		  	  // the frame the same size, while making sure it stays anchored to
		  	  // that edge.
		  	  if (sticky_frame_flags & W_TRACK_NORTH)
		  	  {			  	  
			  	  Bounds().ymax = Bounds().Height() + margins[W_DOCK_NORTH];
		  	  }
			  	  
		  	  Bounds().ymin = margins[W_DOCK_NORTH]; 		  	  
	  	 }
	  	 
	  	 if (sticky_frame_flags & W_STICK_SOUTH)
	  	 {		  	 
		  	  if (sticky_frame_flags & W_TRACK_SOUTH)
		  	  {			  	  
			  	  Bounds().ymin = parent->Bounds().Height() - Bounds().Height() - margins[W_DOCK_SOUTH];
		  	  }
		  	 
		  	  Bounds().ymax = parent->Bounds().Height() - margins[W_DOCK_SOUTH];
	  	 }
	  	  
	  	 if (sticky_frame_flags & W_STICK_EAST)
	  	 {
		  	 if (sticky_frame_flags & W_TRACK_EAST)
		  	 {			  	  
			  	  Bounds().xmin = parent->Bounds().Width() - Bounds().Width() - margins[W_DOCK_EAST];
		  	 }
		  	 
		  	 Bounds().xmax = parent->Bounds().Width()- margins[W_DOCK_EAST];
	  	 }
	  	 
	  	 if (sticky_frame_flags & W_STICK_WEST)  
	  	 {
		  	 if (sticky_frame_flags & W_TRACK_WEST)
		  	 {			  	  
			  	  Bounds().xmax = Bounds().Width() + margins[W_DOCK_WEST];
		  	 }
		  	 
		  	 Bounds().xmin = margins[W_DOCK_WEST];	  	 
	  	 }
  	 }	 
  	 
  	 /** @brief Sets a stick frame bit. */
  	 void SetFrameAnchor(int32 flags)
  	 {
	  	sticky_frame_flags |= flags;	 
  	 }
  	 
  	 /** @brief Clears a sticky frame bit. */
  	 void ClearFrameAnchor(int32 flags)
  	 {
	  	sticky_frame_flags &= ~flags;	 
  	 }
  	 
  	 /** @brief Sets the margin for the given side. */
  	 void SetMargin(int32 size, int32 margin)
  	 {
	 	margins[margin] = size;	 
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
