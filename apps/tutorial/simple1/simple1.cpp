/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#include "simple1.h"

CS_IMPLEMENT_APPLICATION

#pragma warning(disable : 4250)

template<typename Class>
class scfInterface2
{
public:
  static int GetVersion()
  {
    return Class::GetVersion();
  }

  static scfInterfaceID GetID()
  {
    scfInterfaceID& ID = GetMyID();
    if (ID == (scfInterfaceID)(-1))
    {
      ID = iSCF::SCF->GetInterfaceID(GetName());
      /* Mild hack to reset SCF ID on CS shutdown */
      csStaticVarCleanup (CleanupID);
    }
    return ID;
  }

  /**
  * Retrieve the interface's name as a string.
  */
  static char const* GetName()
  {
    return Class::GetName();
  }
private:
  static scfInterfaceID& GetMyID()
  {
    static scfInterfaceID ID = (scfInterfaceID)-1;
    return ID;
  }
  static void CleanupID()
  {
    GetMyID() = (scfInterfaceID)-1;
  }
};


struct iFoo : public virtual iBase
{
  static int GetVersion()
  {
    return SCF_CONSTRUCT_VERSION(0, 0, 1);
  }
  static char const* GetName()
  {
    return "iFoo";
  }
  virtual void DoFoo()=0;
};

struct iBar : public virtual iBase
{
  virtual void DoBar()=0;
};

template<>
class scfInterface2<iBar> 
{
public:
  static int GetVersion()
  {
    return SCF_CONSTRUCT_VERSION(0, 0, 1);
  }
  static char const* GetName()
  {
    return "iBar";
  }

  static scfInterfaceID GetID()
  {
    scfInterfaceID& ID = GetMyID();
    if (ID == (scfInterfaceID)(-1))
    {
      ID = iSCF::SCF->GetInterfaceID(GetName());
      /* Mild hack to reset SCF ID on CS shutdown */
      csStaticVarCleanup (CleanupID);
    }
    return ID;
  }
private:
  static scfInterfaceID& GetMyID()
  {
    static scfInterfaceID ID = (scfInterfaceID)-1;
    return ID;
  }
  static void CleanupID()
  {
    GetMyID() = (scfInterfaceID)-1;
  }
};

namespace crystal
{

  template<typename Class>
  class scfImplementation : public virtual iBase
  {
  public:
    scfImplementation (Class *object, iBase *parent=0)
      : scfParent (parent), scfRefCount (1), scfObject(object)
    {}

    void DecRef()
    {
      scfRefCount--;
      if (scfRefCount == 0)
      {
        if (scfParent) scfParent->DecRef ();
        delete scfObject;
      }
    }

    iBase *scfParent;

    void IncRef ()
    {
      scfRefCount++;
    }

    int GetRefCount ()
    {
      return scfRefCount;
    }

    void AddRefOwner (iBase** ref_owner)
    {
    }
    void RemoveRefOwner (iBase** ref_owner)
    {
    }

  protected:
    int scfRefCount;		/* Reference counter */

    void scfRemoveRefOwners ()
    {
    }
    
  protected:
    Class *scfObject;
    
    void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion)
    {
      if (iInterfaceID == scfInterface<iBase>::GetID () &&
        scfCompatibleVersion (iVersion, scfInterface<iBase>::GetVersion ()))
      {
        scfObject->IncRef();
        return static_cast<iBase*>(scfObject);
      }
      if (scfParent) 
        return scfParent->QueryInterface (iInterfaceID, iVersion);
      return 0;
    }
  };

  template<typename Class, typename I1>
  class scfImplementation1 : public scfImplementation<Class>, 
                             public I1
  {
  public:
    scfImplementation1 (Class *object, iBase *parent=0)
      : scfImplementation<Class> (object, parent)
    {}

    void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion)
    {
      if (iInterfaceID == scfInterface2<I1>::GetID () &&
          scfCompatibleVersion (iVersion, scfInterface2<I1>::GetVersion ()))
      {
        scfObject->IncRef();
        return static_cast<I1*>(scfObject);
      }
      return scfImplementation<Class>::QueryInterface (iInterfaceID, iVersion);
    }

  protected:
    typedef scfImplementation1<Class, I1> scfImplementationType;
  };

  template<typename Class, typename I1, typename I2>
  class scfImplementation2 : public scfImplementation<Class>,
                             public I1,
                             public I2
  {
  public:
    scfImplementation2 (Class *object, iBase *parent=0)
      : scfImplementation<Class> (object, parent)
    {}

    void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion)
    {
      if (iInterfaceID == scfInterface2<I1>::GetID () &&
          scfCompatibleVersion (iVersion, scfInterface2<I1>::GetVersion ()))
      {
        scfObject->IncRef();
        return static_cast<I1*>(scfObject);
      }
      if (iInterfaceID == scfInterface2<I2>::GetID () &&
        scfCompatibleVersion (iVersion, scfInterface2<I2>::GetVersion ()))
      {
        scfObject->IncRef();
        return static_cast<I2*>(scfObject);
      }
      return scfImplementation<Class>::QueryInterface (iInterfaceID, iVersion);
    }
  protected:
    typedef scfImplementation2<Class, I1, I2> scfImplementationType;
  };

  template<typename Class, typename Super, typename In1>
  class scfImplementationExt1 : public Super, 
                                public In1
  {
  public:
    scfImplementationExt1 (Class *object)
      : Super (), scfObject (object)
    {
    }

    void *QueryInterface (scfInterfaceID iInterfaceID, int iVersion)
    {
      if (iInterfaceID == scfInterface2<In1>::GetID () &&
        scfCompatibleVersion (iVersion, scfInterface2<In1>::GetVersion ()))
      {
        scfObject->IncRef();
        return static_cast<In1*>(scfObject);
      }
      return Super::QueryInterface (iInterfaceID, iVersion);
    }

  protected:
    Class* scfObject;
    typedef scfImplementationExt1<Class, Super, In1> scfImplementationType;
  };
}

// An implementation
class csFoo : public crystal::scfImplementation1<csFoo, iFoo>
{
public:
  csFoo ()
    : scfImplementationType (this)
  {
  }

  void DoFoo()
  {
    printf ("FOO!\n");
  }
};

class csFooBar : public crystal::scfImplementation2<csFooBar,iFoo,iBar>
{
public:
  csFooBar ()
    : scfImplementationType (this)
  {
  }

  void DoFoo()
  {
    printf ("FOObar!\n");
  }

  void DoBar ()
  {
    printf ("fooBAR\n");
  }
};

//---------------------------------------------------------------------------

Simple::Simple ()
{
  SetApplicationName ("CrystalSpace.Simple1");
}

Simple::~Simple ()
{
}

void Simple::ProcessFrame ()
{
  // First get elapsed time from the virtual clock.
  csTicks elapsed_time = vc->GetElapsedTicks ();
  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.06 * 20);

  iCamera* c = view->GetCamera();

  if (kbd->GetKeyState (CSKEY_SHIFT))
  {
    // If the user is holding down shift, the arrow keys will cause
    // the camera to strafe up, down, left or right from it's
    // current position.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      c->Move (CS_VEC_RIGHT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_LEFT))
      c->Move (CS_VEC_LEFT * 4 * speed);
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_UP * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_DOWN * 4 * speed);
  }
  else
  {
    // left and right cause the camera to rotate on the global Y
    // axis; page up and page down cause the camera to rotate on the
    // _camera's_ X axis (more on this in a second) and up and down
    // arrows cause the camera to go forwards and backwards.
    if (kbd->GetKeyState (CSKEY_RIGHT))
      rotY += speed;
    if (kbd->GetKeyState (CSKEY_LEFT))
      rotY -= speed;
    if (kbd->GetKeyState (CSKEY_PGUP))
      rotX += speed;
    if (kbd->GetKeyState (CSKEY_PGDN))
      rotX -= speed;
    if (kbd->GetKeyState (CSKEY_UP))
      c->Move (CS_VEC_FORWARD * 4 * speed);
    if (kbd->GetKeyState (CSKEY_DOWN))
      c->Move (CS_VEC_BACKWARD * 4 * speed);
  }

  // We now assign a new rotation transformation to the camera.  You
  // can think of the rotation this way: starting from the zero
  // position, you first rotate "rotY" radians on your Y axis to get
  // the first rotation.  From there you rotate "rotX" radians on the
  // your X axis to get the final rotation.  We multiply the
  // individual rotations on each axis together to get a single
  // rotation matrix.  The rotations are applied in right to left
  // order .
  csMatrix3 rot = csXRotMatrix3 (rotX) * csYRotMatrix3 (rotY);
  csOrthoTransform ot (rot, c->GetTransform().GetOrigin ());
  c->SetTransform (ot);

  // Tell 3D driver we're going to display 3D things.
  if (!g3d->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
    return;

  // Tell the camera to render into the frame buffer.
  view->Draw ();
}

void Simple::FinishFrame ()
{
  // Just tell the 3D renderer that everything has been rendered.
  g3d->FinishDraw ();
  g3d->Print (0);
}

bool Simple::OnKeyboard(iEvent& ev)
{
  // We got a keyboard event.
  csKeyEventType eventtype = csKeyEventHelper::GetEventType(&ev);
  if (eventtype == csKeyEventTypeDown)
  {
    // The user pressed a key (as opposed to releasing it).
    utf32_char code = csKeyEventHelper::GetCookedCode(&ev);
    if (code == CSKEY_ESC)
    {
      // The user pressed escape to exit the application.
      // The proper way to quit a Crystal Space application
      // is by broadcasting a cscmdQuit event. That will cause the
      // main runloop to stop. To do that we get the event queue from
      // the object registry and then post the event.
      csRef<iEventQueue> q = 
        CS_QUERY_REGISTRY(GetObjectRegistry(), iEventQueue);
      if (q.IsValid()) q->GetEventOutlet()->Broadcast(cscmdQuit);
    }
  }
  return false;
}

bool Simple::OnInitialize(int argc, char* argv[])
{
  // RequestPlugins() will load all plugins we specify. In addition
  // it will also check if there are plugins that need to be loaded
  // from the config system (both the application config and CS or
  // global configs). In addition it also supports specifying plugins
  // on the commandline.
  if (!csInitializer::RequestPlugins(GetObjectRegistry(),
    CS_REQUEST_VFS,
    CS_REQUEST_OPENGL3D,
    CS_REQUEST_ENGINE,
    CS_REQUEST_FONTSERVER,
    CS_REQUEST_IMAGELOADER,
    CS_REQUEST_LEVELLOADER,
    CS_REQUEST_REPORTER,
    CS_REQUEST_REPORTERLISTENER,
    CS_REQUEST_END))
    return ReportError("Failed to initialize plugins!");

  // Now we need to setup an event handler for our application.
  // Crystal Space is fully event-driven. Everything (except for this
  // initialization) happens in an event.
  if (!RegisterQueue(GetObjectRegistry()))
    return ReportError("Failed to set up event handler!");

  csRef<csFooBar> csfb;
  csfb.AttachNew (new csFooBar);
  csRef<iFoo> foo = csPtr<iFoo> ((iFoo*)(csfb->QueryInterface (
    scfInterface2<iFoo>::GetID (),
    scfInterface2<iFoo>::GetVersion ())));
  
  printf("%d\n", sizeof(iFoo));
  printf("%d\n", sizeof(iBar));
  printf("%d\n", sizeof(csFoo));
  printf("%d\n", sizeof(csFooBar));

  if (foo)
  {
    foo->DoFoo ();

    csRef<iBar> bar = csPtr<iBar> ((iBar*)(foo->QueryInterface (
      scfInterface2<iBar>::GetID (),
      scfInterface2<iBar>::GetVersion ())));
    bar->DoBar ();
  }


  return true;
}

void Simple::OnExit()
{
}

bool Simple::Application()
{
  // Open the main system. This will open all the previously loaded plug-ins.
  // i.e. all windows will be opened.
  if (!OpenApplication(GetObjectRegistry()))
    return ReportError("Error opening system!");

  // Now get the pointer to various modules we need. We fetch them
  // from the object registry. The RequestPlugins() call we did earlier
  // registered all loaded plugins with the object registry.
  g3d = CS_QUERY_REGISTRY(GetObjectRegistry(), iGraphics3D);
  if (!g3d) return ReportError("Failed to locate 3D renderer!");

  engine = CS_QUERY_REGISTRY(GetObjectRegistry(), iEngine);
  if (!engine) return ReportError("Failed to locate 3D engine!");

  vc = CS_QUERY_REGISTRY(GetObjectRegistry(), iVirtualClock);
  if (!vc) return ReportError("Failed to locate Virtual Clock!");

  kbd = CS_QUERY_REGISTRY(GetObjectRegistry(), iKeyboardDriver);
  if (!kbd) return ReportError("Failed to locate Keyboard Driver!");

  loader = CS_QUERY_REGISTRY(GetObjectRegistry(), iLoader);
  if (!loader) return ReportError("Failed to locate Loader!");

  // We need a View to the virtual world.
  view.AttachNew(new csView (engine, g3d));
  iGraphics2D* g2d = g3d->GetDriver2D ();
  // We use the full window to draw the world.
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Here we create our world.
  CreateRoom();

  // Let the engine prepare all lightmaps for use and also free all images 
  // that were loaded for the texture manager.
  engine->Prepare ();

  // these are used store the current orientation of the camera
  rotY = rotX = 0;

  // Now we need to position the camera in our world.
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 5, -3));

  // This calls the default runloop. This will basically just keep
  // broadcasting process events to keep the game going.
  Run();

  return true;
}

void Simple::CreateRoom ()
{
  // Load the texture from the standard library.  This is located in
  // CS/data/standard.zip and mounted as /lib/std using the Virtual
  // File System (VFS) plugin.
  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
    ReportError("Error loading 'stone4' texture!");

  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  // We create a new sector called "room".
  room = engine->CreateSector ("room");

  // Creating the walls for our room.
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
    SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-5, 0, -5), csVector3 (5, 20, 5));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  // Now we need light to see something.
  csRef<iLight> light;
  iLightList* ll = room->GetLights ();

  light = engine->CreateLight(0, csVector3(-3, 5, 0), 10, csColor(1, 0, 0));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(3, 5,  0), 10, csColor(0, 0, 1));
  ll->Add (light);

  light = engine->CreateLight(0, csVector3(0, 5, -3), 10, csColor(0, 1, 0));
  ll->Add (light);
}

/*-------------------------------------------------------------------------*
 * Main function
 *-------------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  /* Runs the application. 
   *
   * csApplicationRunner<> is a small wrapper to support "restartable" 
   * applications (ie where CS needs to be completely shut down and loaded 
   * again). Simple1 does not use that functionality itself, however, it
   * allows you to later use "Simple.Restart();" and it'll just work.
   */
  return csApplicationRunner<Simple>::Run (argc, argv);
}
