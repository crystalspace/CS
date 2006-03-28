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

#include "cssysdef.h"
#include "manager.h"
#include "frame.h"
#include "script_manager.h"
#include "script_console.h"
#include "script_object.h"
#include "widget.h"
#include "color.h"
#include "gradient.h"
#include "font.h"
#include "skin.h"


#include "csutil/csevent.h"
#include "iengine/engine.h"
#include "iutil/comp.h"
#include "iutil/event.h"
#include "iutil/eventh.h"
#include "iutil/eventq.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/virtclk.h"
#include "iutil/evdefs.h"
#include "csutil/event.h"
#include "csutil/timer.h"
#include "ivaria/reporter.h"
#include "ivideo/txtmgr.h"

/**** AWS Specific Events *******************************************/
// The primary system mouse has entered a component
#define awsMouseEnter(reg) (csEventNameRegistry::GetID((reg), "crystalspace.plugin.aws.mouse.enter"))
// The primary system mouse has exited a component
#define awsMouseExit(reg) (csEventNameRegistry::GetID((reg), "crystalspace.plugin.aws.mouse.exit"))
// The component has lost keyboard focus
#define awsLoseFocus(reg) (csEventNameRegistry::GetID((reg), "crystalspace.plugin.aws.focus.lost"))
// The component has gained keyboard focus
#define awsGainFocus(reg) (csEventNameRegistry::GetID((reg), "crystalspace.plugin.aws.focus.gained"))
// A component in a group has been selected, everyone else should go to their off state.
#define awsGroupOff(reg) (csEventNameRegistry::GetID((reg), "crystalspace.plugin.aws.group.off"))
// The frame is about to start rendering
#define awsFrameStart(reg) (csEventNameRegistry::GetID((reg), "crystalspace.plugin.aws.frame.start"))
/********************************************************************/

SCF_IMPLEMENT_FACTORY(awsManager2)

SCF_IMPLEMENT_IBASE(awsManager2)
  SCF_IMPLEMENTS_INTERFACE(iAws)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE(awsManager2::eiComponent)
  SCF_IMPLEMENTS_INTERFACE(iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

static awsManager2 *theMgr=0;

awsManager2 *AwsMgr() { return theMgr; }

awsManager2::awsManager2(iBase *the_base)
{
  SCF_CONSTRUCT_IBASE (the_base);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);
  scfiEventHandler = 0;
  
  theMgr = this;
    
  CreateScriptManager();    
}

awsManager2::~awsManager2()
{
  if (scfiEventHandler)
  {
    csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
    if (q)
      q->RemoveListener (scfiEventHandler);

    scfiEventHandler->DecRef ();
  }
  
  DestroyScriptManager();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiComponent);
  SCF_DESTRUCT_IBASE ();
}

bool 
awsManager2::Initialize (iObjectRegistry *_object_reg)
{
  object_reg = _object_reg;
    
  PreProcess = csevPreProcess (object_reg);
  MouseDown = csevMouseDown (object_reg, 0);
  MouseUp = csevMouseUp (object_reg, 0);
  MouseClick = csevMouseClick (object_reg, 0);
  MouseMove = csevMouseMove (object_reg, 0);
  KeyboardDown = csevKeyboardDown (object_reg);
  KeyboardUp = csevKeyboardUp (object_reg);

  MouseEnter = awsMouseEnter (object_reg);
  MouseExit = awsMouseExit (object_reg);
  LoseFocus = awsLoseFocus (object_reg);
  GainFocus = awsGainFocus (object_reg);
  GroupOff = awsGroupOff (object_reg);
  FrameStart = awsFrameStart (object_reg);
  
  timer = csEventTimer::GetStandardTimer(object_reg);
  
  mouse_focus=0;
  keyboard_focus=0;
  mouse_captured=false;
  
  ScriptCon()->Initialize(object_reg);
  ScriptMgr()->Initialize(object_reg);
  
  Color_SetupAutomation();
  Gradient_SetupAutomation();
  Skin_SetupAutomation();
  Pen_SetupAutomation();
  Font_SetupAutomation();
  Widget_SetupAutomation();	
      
  return true;
}

void 
awsManager2::SetDrawTarget(iGraphics2D *_g2d, iGraphics3D *_g3d)
{
  g2d = _g2d;
  g3d = _g3d;

  default_font = g2d->GetFontServer()->LoadFont (CSFONT_LARGE);
  ScriptCon()->SetFont(default_font);
}

/*********************************************************************
 ***************** Event Handling ************************************
 ********************************************************************/
 
void awsManager2::CaptureMouse(aws::widget *w)
{
	if (w)
	{
		 mouse_focus=w;
		 mouse_captured=true;
	 }
}
  
void awsManager2::ReleaseMouse()
{
	mouse_captured=false;	
}

bool awsManager2::HandleEvent (iEvent &Event)
{ 
  if (CS_IS_MOUSE_EVENT(object_reg, Event))
  {
	  aws::widget *new_mouse_focus=0;
	  
	  if (mouse_captured)
	  {	    		  
		if (mouse_focus) 
			mouse_focus->HandleEvent(Event);  
		  
		return true;	  
	  }  
	  
	  // Check all widgets for new focus.
	  for(size_t i=0; i<aws::widgets.Length(); ++i)
	  {
	 	new_mouse_focus = aws::widgets[i]->Contains(csMouseEventHelper::GetX(&Event), csMouseEventHelper::GetY(&Event));		
	 	if (new_mouse_focus!=0) break;
	  }
	  	  
	  // Did the move over a new widget?
	  if (new_mouse_focus!=mouse_focus)
	  {
	  	// Reusing this event, save the orignal type
	  	csEventID et = Event.Name;
		
	  	// Tell this widget that it's losing mouse focus.  
	    if (mouse_focus)
	    {
	      Event.Name = MouseExit;
	      mouse_focus->HandleEvent (Event);
	    }
		    		   
	    // Set the new focus widget
		mouse_focus = new_mouse_focus;

		// Tell the new widget that it's getting mouse focus.		    
	    if (mouse_focus)
	    {
	      Event.Name = MouseEnter;
	      mouse_focus->HandleEvent (Event);
	    }
	    
		// Restore the event's name.    
		Event.Name = et;		 		  
	  }  
	  
	  //  If any widget has mouse focus then handle the event.
	  // A child widget will pass the message up the line to it's parents
	  // if it does not handle the event.
	  if (mouse_focus)
	  {
		mouse_focus->HandleEvent(Event);	  
	  }
	  
	   
  }
  else if (Event.Name == KeyboardDown)
  {
	  csKeyEventData eventData;
      csKeyEventHelper::GetEventData (&Event, eventData);
	  
	  switch(eventData.codeCooked)
	  {
		case CSKEY_F1:
			if (csKeyEventHelper::GetModifiersBits(&Event) & (CSMASK_CTRL | CSMASK_ALT))
			{
				ScriptCon()->FlipActiveState();					
			}
			break;
									
		default:	
			if (ScriptCon()->Active())			
				ScriptCon()->OnKeypress(eventData);
				
			break;  
	  } // end switch code
	  return true;
  } // end if event is keyboard
	
  return false;
}

/*********************************************************************
 ***************** Redrawing *****************************************
 ********************************************************************/


void awsManager2::Redraw()
{  
  csPen pen(g2d, g3d);    
   
  g2d->Write(default_font, 90, 90, g2d->FindRGB(128,128,128,128), -1, "AWS Redrawing");
  ScriptCon()->Redraw(g2d);
  
//   float angle;
//   
//   pen.SetColor(1,1,1,1);
//   for(angle=0; angle<M_PI*2.0; angle+=M_PI/8.0)
//   {
//   	pen.DrawThickLine(100,100,(uint)(100+(cos(angle)*100)),(uint)(100+(sin(angle)*100)));
//   }

  // Draw all widgets (this is a hack for testing.)
  for(size_t i=0; i<aws::widgets.Length(); ++i)
  {
 	aws::widgets[i]->Draw(&pen);		
  }
}

/*********************************************************************
 ***************** Definition Files **********************************
 ********************************************************************/

bool awsManager2::Load(const scfString &_filename)
{
  return prefs.load(object_reg, _filename);
}


/*********************************************************************
 ***************** Scripting *****************************************
 ********************************************************************/

iAwsScriptObject *awsManager2::CreateScriptObject(const char *name)
{
	return new scriptObject(name);	
}

// newline
