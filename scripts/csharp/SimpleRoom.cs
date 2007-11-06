/*
    Copyright (C) 2003 by Rene Jager
    (renej@frog.nl, renej.frog@yucom.be, renej_frog@sourceforge.net)

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

using System;
using System.Collections;
using CrystalSpace;

namespace SimpleRoom
{
  public class GlobRefs
  {
    public iGraphics3D g3d;
    public iKeyboardDriver kbd;
    public iVirtualClock vc;
    public iLoader loader;
    public iEngine engine;
    private static GlobRefs instance = null;

    public static GlobRefs Get()
    {
      if (instance == null)
      {
	instance = new GlobRefs();
      }
      return instance;
    }

    private GlobRefs()
    {
      iObjectRegistry objreg = CS.theObjectRegistry;
      g3d = (iGraphics3D) objreg.Get(typeof(iGraphics3D));
      vc = (iVirtualClock) objreg.Get(typeof(iVirtualClock));
      kbd = (iKeyboardDriver) objreg.Get(typeof(iKeyboardDriver));
      loader = (iLoader) objreg.Get(typeof(iLoader));
      engine = (iEngine) objreg.Get(typeof(iEngine));
    }
  };

  public class MovingObject
  {
    private iMeshWrapper sprite;
		
    public MovingObject(iSector room, csVector3 pos)
    {
      GlobRefs r = GlobRefs.Get();
      iTextureManager txtmgr = r.g3d.GetTextureManager();
      iTextureWrapper txt = r.loader.LoadTexture("spark", 
      "/lib/std/spark.png", CS.CS_TEXTURE_3D, txtmgr, true, true);
      iMeshFactoryWrapper imeshfact = r.loader.LoadMeshObjectFactory("/lib/std/sprite1", null);
      sprite = r.engine.CreateMeshWrapper(imeshfact, "MySprite", room, pos);
      sprite.GetMovable().UpdateMove();
      sprite.SetZBufMode(csZBufMode.CS_ZBUF_USE);
      sprite.SetRenderPriority(r.engine.GetObjectRenderPriority());
    }
			                         
    public iMovable GetMovable()
    {
      return sprite.GetMovable();
    }
  }

  public class EventHandler: CrystalSpace.csSharpEventHandler
  {
    protected iGraphics3D myG3D;
    protected csView view;
    protected iKeyboardDriver kbd;
    protected iVirtualClock vc;
    protected uint evPreProcess;
    protected uint evFinalProcess;
				
    public EventHandler(csView v)
    {
      GlobRefs r = GlobRefs.Get();
      myG3D = r.g3d;
      vc = r.vc;
      kbd = r.kbd;
      view = v;
      evPreProcess = CS.csevPreProcess (CS.theObjectRegistry);
      evFinalProcess = CS.csevFinalProcess (CS.theObjectRegistry);
    }
		
    protected void SetupFrame()
    {
      uint elapsed_time = vc.GetElapsedTicks();
      uint current_time = vc.GetCurrentTicks();
		
      //Now rotate the camera according to the keyboard state
      float speed = (float)((elapsed_time / 1000.0f) * (0.03 * 20));
      if(kbd.GetKeyState(CS.CSKEY_RIGHT))
	view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_ROT_RIGHT, speed);
      if(kbd.GetKeyState(CS.CSKEY_LEFT))
	view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_ROT_LEFT, speed);
      if(kbd.GetKeyState(CS.CSKEY_PGUP))
	view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_TILT_UP, speed);
      if(kbd.GetKeyState(CS.CSKEY_PGDN))
	view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_TILT_DOWN, speed);
      if(kbd.GetKeyState(CS.CSKEY_UP))
	view.GetCamera().Move(CS.CS_VEC_FORWARD * 4 * speed);
      if(kbd.GetKeyState(CS.CSKEY_DOWN))
	view.GetCamera().Move(CS.CS_VEC_BACKWARD * 4 * speed);
			
      //Move our object
      iMovable movable = SimpleRoom.moving_object.GetMovable();
      csVector3 pos = movable.GetTransform().GetOrigin();
      pos.y = (float) (1 + (float) (current_time % 5000) / 2000.0);
      movable.GetTransform().SetOrigin(pos);
      movable.UpdateMove();
			
      myG3D.BeginDraw(CS.CSDRAW_3DGRAPHICS);
      view.Draw();
    }
		
    protected void FinishFrame()
    {
      myG3D.FinishDraw();
      myG3D.Print(null);
    }
		
    public override bool HandleEvent(iEvent ev)
    {
      iObjectRegistry object_reg = CS.theObjectRegistry;
			
      if(CS.CS_IS_KEYBOARD_EVENT(object_reg, ev))
      {
	csKeyEventType eventType = csKeyEventHelper.GetEventType(ev);
	if( eventType == csKeyEventType.csKeyEventTypeDown)
	{
	  uint eventCode = csKeyEventHelper.GetCookedCode(ev);
	  if (eventCode == CS.CSKEY_ESC)
	  {
	    //escape key to quit
	    iEventQueue eventQueue = (iEventQueue)object_reg.Get(typeof(iEventQueue));
	    if(eventQueue != null)
	    {
	      eventQueue.GetEventOutlet().Broadcast(CS.csevQuit(object_reg));
	      return true;
	    }
	  }
	}
      }
      else if (ev.GetName() == evPreProcess)
      {
	SetupFrame();
      }
      else if(ev.GetName() == evFinalProcess)
      {
	FinishFrame();
      }
      return true;
    }
  };

  public class SimpleRoom: CrystalSpace.CS
  {		
    protected static csView view = null;
    public static MovingObject moving_object = null;
		
    public SimpleRoom()
    {
    }
		
    public static void CreateRoom()
    {
      //Retrieve somes objects
      iObjectRegistry object_reg = theObjectRegistry;
			
      //Retrieve the engine
      Console.WriteLine("Getting engine");
      iEngine engine = (iEngine) object_reg.Get(typeof(iEngine));
      //Retrieve the loader
      Console.WriteLine("Getting the loader");
      iLoader loader = (iLoader) object_reg.Get(typeof(iLoader));

      //Retrieve the room texture
      Console.WriteLine("Getting the texture");
      String matname = "mystone";
      loader.LoadTexture(matname, "/lib/stdtex/bricks.jpg",
			                   CS_TEXTURE_3D, null, false, true);
      //Create the room
      engine.SetLightingCacheMode(0);
      engine.CreateSector("room");
			
      //Retrieve the sector
      iSector room = engine.GetSectors().FindByName("room");
			
      //Create sector walls
      Console.WriteLine("Creating sector wall mesh");
      iMeshWrapper walls = engine.CreateSectorWallsMesh(room, "walls");
			
      //Retrieve the material
      Console.WriteLine("Getting the material");
      iMaterialWrapper material = engine.GetMaterialList().FindByName(matname);
			
      //Create the sector walls
      Console.WriteLine("Creating walls");
      iThingFactoryState fact =
	(iThingFactoryState) walls.GetMeshObject().GetFactory().QueryInterface(typeof(iThingFactoryState));
			
      fact.AddInsideBox(new csVector3(-5.0f, 0.0f, -5.0f),
			new csVector3(5.0f, 20.0f, 5.0f));
			
      csPolygonRange CS_POLYRANGE_LAST = new csPolygonRange(-1, -1);
      fact.SetPolygonTextureMapping(CS_POLYRANGE_LAST, 3);
      fact.SetPolygonMaterial(CS_POLYRANGE_LAST, material);
			
      iLight light = engine.CreateLight("", new csVector3(0, 5, 0), 10f,
         new csColor(1, 0, 0), csLightDynamicType.CS_LIGHT_DYNAMICTYPE_STATIC);
      room.GetLights().Add(light);
			
      engine.Prepare(null);
			
      iGraphics3D myG3D = (iGraphics3D) object_reg.Get(typeof(iGraphics3D));
      view = new csView(engine, myG3D);
      view.GetCamera().SetSector(room);
      view.GetCamera().GetTransform().SetOrigin(new csVector3(0, 2, 0));
      iGraphics2D g2d = myG3D.GetDriver2D();
      view.SetRectangle(2, 2, g2d.GetWidth() - 4, g2d.GetHeight() - 4);
    }
		
    public static void CreateObjects()
    {
      moving_object = new MovingObject(view.GetCamera().GetSector(),
	                                 new csVector3(1,2,3));
    }
		
    public static void FatalError(string err)
    {
      Console.WriteLine("Fatal error: {0}", err);
      Environment.Exit(1);
    }

    public static void Main(string[] args)
    {
      try
      {
	Console.WriteLine("Starting Crystal Space Sharp");
	iObjectRegistry object_reg = csInitializer.CreateEnvironment(args);
	if(object_reg == null)
	  FatalError("Couldn't create environment!");
	theObjectRegistry = object_reg;
				
	csPluginRequest[] plugins =
	{
	  CS.CS_REQUEST_VFS(),
	  CS.CS_REQUEST_OPENGL3D(),
	  CS.CS_REQUEST_IMAGELOADER(),
	  CS.CS_REQUEST_FONTSERVER(),
	  CS.CS_REQUEST_ENGINE(),
	  CS.CS_REQUEST_LEVELLOADER(),
	};
				
	bool result = RequestPlugins(object_reg, plugins);
	if(!result)
	  FatalError("cannot request plugins!");

	Console.WriteLine("Opening Application...");
	result = csInitializer.OpenApplication(object_reg);
	if(!result)
	  FatalError("cannot open application");

	Console.WriteLine("Application opened...");
	csPrintf("Printing from csPrintf" + Environment.NewLine);
							
	iVFS VFS = (iVFS)object_reg.Get(typeof(iVFS));
	if(VFS == null)
	  FatalError("couldn't request vitual filesystem!");
	iGraphics3D G3D = (iGraphics3D)object_reg.Get(typeof(iGraphics3D));
	if(G3D == null)
	  FatalError("couldn't request 3d graphics driver!");
	iNativeWindow nativeWindow = G3D.GetDriver2D().GetNativeWindow();
	nativeWindow.SetTitle("Test Crystal Space#");

	Console.WriteLine("Creating room");
	CreateRoom();
			
	Console.WriteLine("Creating objects");
	CreateObjects();
				
	Console.WriteLine("Setting up Event Handlers");
	EventHandler eventHandler = new EventHandler(view);
	iEventQueue q = (iEventQueue)object_reg.Get(typeof(iEventQueue));
	if(q != null)
	{
	  q.RegisterListener(eventHandler, csevPreProcess(object_reg));
	  q.Subscribe(eventHandler, csevFinalProcess(object_reg));
	  q.Subscribe(eventHandler, csevKeyboardEvent(object_reg));
	  Console.WriteLine("Entering to the default run loop");
	  csDefaultRunLoop(object_reg);
	}
			
	Console.WriteLine("Exiting");
      }
      catch(ApplicationException e)
      {
	Console.WriteLine("Error, we caught a exception: {0}", e.ToString());
      }
    }
  };
};

