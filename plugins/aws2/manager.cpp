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
#include "border.h"
#include "script_manager.h"
#include "script_console.h"
#include "script_object.h"
#include "widget.h"
#include "color.h"
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
#include "ivaria/reporter.h"
#include "ivideo/txtmgr.h"

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
  
  KeyboardDown = csevKeyboardDown (object_reg);
  
  ScriptCon()->Initialize(object_reg);
  ScriptMgr()->Initialize(object_reg);
  
  Color_SetupAutomation();
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

bool awsManager2::HandleEvent (iEvent &Event)
{  
  if (Event.Name == KeyboardDown)
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
  static float angle=0.0;
  if (angle>6.28318531) angle=0;

  csPen pen(g2d, g3d);
  csVector3 tv(-250,-250,0);  
  
  angle+=0.001F;
  
  /*pen.Translate(tv);  
  pen.SetOrigin(tv);
  pen.Rotate(angle);  

  tv.Set(350,350,0);
  pen.Translate(tv);  */
  
  g2d->Write(default_font, 90, 90, g2d->FindRGB(128,128,128,128), -1, "AWS Redrawing");
  ScriptCon()->Redraw(g2d);

  /*pen.SetColor(0.25,0.25,0.25,1);
  pen.SwapColors();
  pen.SetColor(0.75,0.75,0.75,1);

  pen.DrawRoundedRect(0,0,500,500,0.5,false,true); */
  
  /*pen.SetColor(0.5,0.5,0.5,1);
  pen.DrawRect(50,50,450,450,false,true);  
  pen.SetColor(0.75,0.75,0.75,1);
  pen.DrawMiteredRect(100,100,400,400,0.5,false,true);  
  pen.SetColor(0.85f,0.85f,0.85f,1);
  pen.DrawArc(150,150,350,350,0.14F,2.23F,false,true); */
  
//   pen.SetColor(1,1,1,1);

//   pen.WriteBoxed(default_font, 0,0,500,500, CS_PEN_TA_CENTER, CS_PEN_TA_CENTER, "Test Boxed Text - Centered");
//   pen.WriteBoxed(default_font, 0,0,500,500, CS_PEN_TA_RIGHT, CS_PEN_TA_TOP, "Test Boxed Text - Right, Top");
//   pen.WriteBoxed(default_font, 0,0,500,500, CS_PEN_TA_LEFT, CS_PEN_TA_BOT, "Test Boxed Text - Left, Bot");
  
  /*pen.DrawPoint(0,0);
  pen.DrawRoundedRect(0,0,500,500,0.5,true); 
  pen.DrawRect(50,50,450,450,true);  
  pen.DrawMiteredRect(100,100,400,400,0.5,true);  
  pen.DrawArc(150,150,350,350,0.14F,2.23F,true); */

//   aws::border b;
//   
//   
//   b.Bounds().SetSize(200,200);
//   b.SetBorderStyle(aws::AWS_BORDER_BEVELED);
//   b.SetBorderShape(aws::AWS_BORDER_RECT);    
//   b.Transform(&pen, angle, 300, 300);

//   b.UpdateSkin(prefs);

//   b.Prepare(&pen);  

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
