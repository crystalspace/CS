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

import com.crystalspace.*;

class EventHandler extends csJEventHandler
{
    protected iGraphics3D myG3D;
    protected csView view;
    protected iKeyboardDriver kbd;
    protected iVirtualClock vc;

    public EventHandler (csView v)
    {
        iObjectRegistry object_reg = getTheObjectRegistry();
        myG3D = (iGraphics3D) CS_QUERY_REGISTRY(object_reg, iGraphics3D.class);
	vc = (iVirtualClock) CS_QUERY_REGISTRY(object_reg, iVirtualClock.class);
        kbd = (iKeyboardDriver) CS_QUERY_REGISTRY(object_reg, iKeyboardDriver.class);
        view = v;
    }

    protected void SetupFrame ()
    {
        long elapsed_time = vc.GetElapsedTicks();
        long current_time = vc.GetCurrentTicks();
        // Now rotate the camera according to keyboard state
        float speed = (float) ((elapsed_time / 1000.) * (0.03 * 20));
        if (kbd.GetKeyState(CSKEY_RIGHT))
            view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_ROT_RIGHT, speed);
        if (kbd.GetKeyState(CSKEY_LEFT))
            view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_ROT_LEFT, speed);
        if (kbd.GetKeyState(CSKEY_PGUP))
            view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_TILT_UP, speed);
        if (kbd.GetKeyState(CSKEY_PGDN))
            view.GetCamera().GetTransform().RotateThis(CS.CS_VEC_TILT_DOWN, speed);
        if (kbd.GetKeyState(CSKEY_UP))
            view.GetCamera().Move(CS.CS_VEC_FORWARD.multiply(4 * speed), true);
        if (kbd.GetKeyState(CSKEY_DOWN))
            view.GetCamera().Move(CS.CS_VEC_BACKWARD.multiply(4 * speed), true);
        // Tell 3D driver we're going to display 3D things.
        myG3D.BeginDraw(CSDRAW_3DGRAPHICS);
        view.Draw();
    }

    protected void FinishFrame ()
    {
        myG3D.FinishDraw();
        myG3D.Print(null);
    }

    public boolean HandleEvent (iEvent ev)
    {
        if (ev.getType() == csevKeyboard &&
	    (csKeyEventHelper.GetEventType(ev) == csKeyEventTypeDown) &&
            (csKeyEventHelper.GetCookedCode(ev) == CSKEY_ESC))
        {
            // escape key to quit
            iEventQueue q = (iEventQueue) CS_QUERY_REGISTRY(getTheObjectRegistry(), iEventQueue.class);
            if (q != null)
            {
                q.GetEventOutlet().Broadcast(cscmdQuit);
                return true;
            }
        }
        if (ev.getType() == csevBroadcast && ev.getCommand().getCode() == cscmdProcess)
            SetupFrame();
        if (ev.getType() == csevBroadcast && ev.getCommand().getCode() == cscmdFinalProcess)
            FinishFrame();
        return true;
    }
};

class SimpleRoom extends CS
{
    protected static csView view;

    public static void CreateRoom ()
    {
        iObjectRegistry object_reg = getTheObjectRegistry();
        System.out.println("getting engine");
	iEngine engine = (iEngine) CS_QUERY_REGISTRY(object_reg, iEngine.class);
        System.out.println("getting clock");
	iVirtualClock vc = (iVirtualClock) CS_QUERY_REGISTRY(object_reg, iVirtualClock.class);
        System.out.println("getting loader");
	iLoader loader = (iLoader) CS_QUERY_REGISTRY(object_reg, iLoader.class);
        System.out.println("getting keyboard driver");
        iKeyboardDriver kbd = (iKeyboardDriver) CS_QUERY_REGISTRY(object_reg, iKeyboardDriver.class);
        System.out.println("getting texture");
	String matname = "mystone";
	loader.LoadTexture(matname, "/lib/stdtex/bricks.jpg", CS_TEXTURE_3D, null, false, true);
        engine.SetLightingCacheMode(0);
        engine.CreateSector("room");
        System.out.println("getting room sectors");
	iSector room = engine.GetSectors().FindByName("room");
        System.out.println("getting walls mesh");
	iMeshWrapper walls = engine.CreateSectorWallsMesh(room, "walls");
        System.out.println("getting thingstate");
	iThingState thingstate = (iThingState) SCF_QUERY_INTERFACE(walls.GetMeshObject(), iThingState.class);
        System.out.println("getting naterial");
	iMaterialWrapper material = engine.GetMaterialList().FindByName(matname);
	
        System.out.println("creating walls");
	iThingFactoryState fact = thingstate.GetFactory();
	fact.AddInsideBox (
	      new csVector3(-5,0,-5),
	      new csVector3(5,20,5));

	csPolygonRange CS_POLYRANGE_LAST = new csPolygonRange(-1, -1);
	fact.SetPolygonTextureMapping(CS_POLYRANGE_LAST, 3);
	fact.SetPolygonMaterial(CS_POLYRANGE_LAST, material);

        iLight light = engine.CreateLight("", new csVector3(0, 5, 0), 10f, new csColor(1, 0, 0),
		CS_LIGHT_DYNAMICTYPE_STATIC);
        room.GetLights().Add(light);

        engine.Prepare(null);

        iGraphics3D myG3D = (iGraphics3D) CS_QUERY_REGISTRY(getTheObjectRegistry(), iGraphics3D.class);
        view = new csView(engine, myG3D);
        view.GetCamera().SetSector(room);
        view.GetCamera().GetTransform().SetOrigin(new csVector3(0, 2, 0));
        iGraphics2D g2d = myG3D.GetDriver2D();
        view.SetRectangle(2, 2, g2d.GetWidth() - 4, g2d.GetHeight() - 4);
    }

    public static void main (String args[])
    {
	try
	{
	    System.out.println("Starting the java simple program.");

	    System.out.println("Creating Environment...");
	    iObjectRegistry object_reg = csInitializer.CreateEnvironment(args);
	    setTheObjectRegistry(object_reg);
	    boolean result;
	    System.out.println("Requesting Plugins...");
	    result = requestPlugin("crystalspace.kernel.vfs", "iVFS");
	    result = requestPlugin("crystalspace.graphics3d.opengl", "iGraphics3D");
	    result = requestPlugin("crystalspace.engine.3d", "iEngine");
	    result = requestPlugin("crystalspace.graphic.image.io.multiplex", "iImageIO");
	    result = requestPlugin("crystalspace.level.loader", "iLoader");
	    result = requestPlugin("crystalspace.font.server.default", "iFontServer");

	    System.out.println("Opening application...");
	    result = csInitializer.OpenApplication(object_reg);

	    System.out.println("Application opened");

	    System.out.println("Creating the room...");
	    int mask = (CSMASK_FrameProcess|CSMASK_Input|CSMASK_Broadcast);
	    CreateRoom();

	    System.out.println("Setting up event handlers...");
	    EventHandler eventHandler = new EventHandler(view);
	    result = csInitializer._SetupEventHandler(object_reg, eventHandler, mask);
	    System.out.println("Event handler added");

	    System.out.println("Starting the main runloop...");
	    csDefaultRunLoop(object_reg);
	}
	catch(Exception e)
	{
	    System.out.println("Errr something went wrong. We caught an exception: " + e);
	}
    }
};
