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

/* ViewMesh: tool for displaying mesh objects (3d sprites) */
#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "csutil/cscolor.h"
#include "cstool/csview.h"
#include "cstool/initapp.h"
#include "csutil/cmdhelp.h"
#include "csutil/util.h"
#include "viewmesh.h"
#include "iutil/eventq.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "iutil/cmdline.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "iutil/csinput.h"
#include "iutil/virtclk.h"
#include "iengine/sector.h"
#include "iengine/engine.h"
#include "iengine/camera.h"
#include "iengine/light.h"
#include "iengine/texture.h"
#include "iengine/mesh.h"
#include "iengine/movable.h"
#include "iengine/material.h"
#include "imesh/thing.h"
#include "imesh/object.h"
#include "imesh/sprite3d.h"
#include "imesh/spritecal3d.h"
#include "imesh/fountain.h"
#include "imesh/partsys.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "ivideo/natwin.h"
#include "ivideo/txtmgr.h"
#include "ivideo/texture.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "igraphic/imageio.h"
#include "imap/parser.h"
#include "ivaria/reporter.h"
#include "ivaria/stdrep.h"
#include "iutil/vfs.h"
#include "iutil/cache.h"
#include "csutil/nulcache.h"
#include "csutil/event.h"

#include "iutil/plugin.h"
#include "imap/writer.h"

CS_IMPLEMENT_APPLICATION

#define VIEWMESH_COMMAND_LOADMESH       77701
#define VIEWMESH_COMMAND_SAVEMESH       77702
#define VIEWMESH_COMMAND_LOADLIB        77703
#define VIEWMESH_STATES_SELECT_START    77800
#define VIEWMESH_OVERRIDE_SELECT_START  77900
#define VIEWMESH_STATES_ADD_START       78000
#define VIEWMESH_STATES_CLEAR_START     78100
#define VIEWMESH_MESH_SELECT_START      78200
#define VIEWMESH_COMMAND_CAMMODE1       77711
#define VIEWMESH_COMMAND_CAMMODE2       77712
#define VIEWMESH_COMMAND_CAMMODE3       77713
#define VIEWMESH_COMMAND_MOVEANIMFASTER 77714
#define VIEWMESH_COMMAND_MOVEANIMSLOWER 77715
#define VIEWMESH_COMMAND_REVERSEACTION  77716
#define VIEWMESH_COMMAND_FORWARDACTION  77717
#define VIEWMESH_COMMAND_BLEND          78300
#define VIEWMESH_COMMAND_CLEAR          78400
#define VIEWMESH_COMMAND_SOCKET         78500
#define VIEWMESH_COMMAND_LOADSOCKET     79000
#define VIEWMESH_COMMAND_ATTACH_SOCKET  80000

#define DEFAULT_SOCKET_X_ROTATION -PI/2.0f
#define DEFAULT_SOCKET_Y_ROTATION 0.0f
#define DEFAULT_SOCKET_Z_ROTATION 0.0f

//-----------------------------------------------------------------------------


ViewMesh::ViewMesh (iObjectRegistry *object_reg, csSkin &Skin)
    : csApp (object_reg, Skin)
{
  SetBackgroundStyle(csabsNothing);
  menu = 0;
  dialog = 0;
  cammode = movenormal;
  spritepos = csVector3(0,10,0);
  move_sprite_speed = 0;
  scale = 1;
  is_cal3d = false;
}

ViewMesh::~ViewMesh ()
{
  delete menu;
  delete dialog;
}

void ViewMesh::Help ()
{
  printf ("Options for ViewMesh:\n");
  printf ("  -L=<file>          Load a library file (for textures/materials)\n");
  printf ("  -Scale=<ratio>     Scale the Object\n");
  printf ("  -RoomSize=<units>  Make the room the specified radius, and 4*size high.  (default 5).\n");
  printf ("  <meshfile>         Load the specified mesh object\n");
}

/* This is the data we keep for modal processing */
struct ModalData : public iBase
{
  uint code;
  csString meshName;
  uint socket;
  SCF_DECLARE_IBASE;
  ModalData() { SCF_CONSTRUCT_IBASE(0); }
  virtual ~ModalData() { SCF_DESTRUCT_IBASE(); }
};

SCF_IMPLEMENT_IBASE (ModalData)
SCF_IMPLEMENT_IBASE_END

bool ViewMesh::HandleEvent (iEvent& ev)
{
  if (ev.Type == csevBroadcast && ev.Command.Code == cscmdProcess)
  {
    // display next frame
    Draw();
  }

  if (csApp::HandleEvent(ev))
    return true;

  switch(ev.Type)
  {
    case csevKeyboard:
      if ((csKeyEventHelper::GetEventType (&ev) == csKeyEventTypeDown) &&
	(csKeyEventHelper::GetCookedCode (&ev) == CSKEY_ESC))
      {
	csRef<iEventQueue> q (CS_QUERY_REGISTRY (object_reg, iEventQueue));
	if (q)
	  q->GetEventOutlet()->Broadcast
	    (cscmdQuit);
	return true;
      }
      break;
    case csevMouseDown:
    case csevMouseDoubleClick:
      // show the menu
      if (menu->GetState(CSS_VISIBLE))
	menu->Hide();
      else
      {
	menu->Show();
	menu->SetPos(ev.Mouse.x,ev.Mouse.y);
      }
      return true;
    case csevCommand:
      switch(ev.Command.Code)
      {
	case VIEWMESH_COMMAND_LOADMESH:
	{
	  menu->Hide();
	  delete dialog;
	  dialog = csFileDialog (this, "Select Mesh Object", "/this/", "Open",
	      true);

	  ModalData *data=new ModalData;
	  data->code = VIEWMESH_COMMAND_LOADMESH;
	  StartModal (dialog, data);

	  return true;
	}
	case VIEWMESH_COMMAND_SAVEMESH:
	{
	  menu->Hide();
	  delete dialog;
	  dialog= csFileDialog (this, "Save As...", "/this/", "Save",
	      true);

	  ModalData *data=new ModalData;
	  data->code = VIEWMESH_COMMAND_SAVEMESH;
	  StartModal (dialog, data);

	  return true;
	}
	case VIEWMESH_COMMAND_LOADLIB:
	{
	  menu->Hide();
	  delete dialog;
	  dialog = csFileDialog (this, "Select Texture Lib", "/this/", "Open",
	      true);

	  ModalData *data=new ModalData;
	  data->code = VIEWMESH_COMMAND_LOADLIB;
	  StartModal (dialog, data);

	  return true;
	}
	case VIEWMESH_COMMAND_MOVEANIMFASTER:
	  move_sprite_speed += .5;
	  menu->Hide();
	  return true;

	case VIEWMESH_COMMAND_MOVEANIMSLOWER:
	  move_sprite_speed -= .5;
	  menu->Hide();
	  return true;

	case VIEWMESH_COMMAND_REVERSEACTION:
	{
	  csRef<iSprite3DState> spstate (
	      SCF_QUERY_INTERFACE(sprite->GetMeshObject(),iSprite3DState));
	  if (spstate)
	      spstate->SetReverseAction(true);
	  menu->Hide();
	  return true;
	}
	case VIEWMESH_COMMAND_FORWARDACTION:
	{
	  csRef<iSprite3DState> spstate (
	      SCF_QUERY_INTERFACE(sprite->GetMeshObject(),iSprite3DState));
	  if (spstate)
	      spstate->SetReverseAction(false);
	  menu->Hide();
	  return true;
	}
	case VIEWMESH_COMMAND_CAMMODE1:
	  cammode = movenormal;
	  menu->Hide();
	  return true;
	case VIEWMESH_COMMAND_CAMMODE2:
	  cammode = moveorigin;
	  menu->Hide();
	  return true;
	case VIEWMESH_COMMAND_CAMMODE3:
	  cammode = rotateorigin;
	  menu->Hide();
	  return true;
	case cscmdStopModal:
	{        
      char filename[1024];
      csQueryFileDialog (dialog, filename, sizeof(filename));
      ModalData *data = (ModalData *) GetTopModalUserdata();

      /* If the dialog was the rotation one then pick out the 
         rotation information and attach the mesh.
      */
      if ( data->code == VIEWMESH_COMMAND_ATTACH_SOCKET )
      {
        csComponent *rotX = 0;
        csComponent *rotY = 0;
        csComponent *rotZ = 0;
        
        csComponent * client = dialog->GetChild (1000);
        if ( client )
        {
            rotX = client->GetChild(1001);
            rotY = client->GetChild(1002);            
            rotZ = client->GetChild(1003);            
        }             
            
        if ( rotX && rotY && rotZ )
        {
        
          AttachMeshToSocket( data->socket, data->meshName.GetData(), 
                              atof( rotX->GetText() ),
                              atof( rotY->GetText() ),
                              atof( rotZ->GetText() ) );                                                                
        }
      }        
      
      delete dialog;
      dialog = 0;
      
      /* Check to see if this was from the dialog box to load a mesh for a
         socket.
       */            
      if (data->code >= VIEWMESH_COMMAND_LOADSOCKET &&
          data->code < VIEWMESH_COMMAND_LOADSOCKET + 100)
      {
        CreateRotationWindow(data->code - VIEWMESH_COMMAND_LOADSOCKET, filename);
      }
                  
	  switch (data->code)
	  {     
        case VIEWMESH_COMMAND_LOADMESH:
	      if (!LoadSprite(filename, scale))
	      {
		Printf (CS_REPORTER_SEVERITY_ERROR, "couldn't load mesh %s",
		    filename);
	      }
	      break;
	    case VIEWMESH_COMMAND_SAVEMESH:
	      if (!SaveSprite(filename))
	      {
		Printf (CS_REPORTER_SEVERITY_ERROR, "couldn't save mesh %s",
		    filename);
	      }
	      break;
	    case VIEWMESH_COMMAND_LOADLIB:
	      if (!loader->LoadLibraryFile(filename))
	      {
		Printf (CS_REPORTER_SEVERITY_ERROR, "couldn't load lib %s",
		    filename);
	      }
	      else
	      {
		// register the textures
		engine->Prepare();
	      }
	      break;
	  }
	  ConstructMenu();
	  return true;
	}
	default:
	  break;
      }
      if (ev.Command.Code >= VIEWMESH_STATES_SELECT_START &&
	  ev.Command.Code < VIEWMESH_STATES_SELECT_START + 100)
      {
      	csRef<iSprite3DState> spstate (
		SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
		iSprite3DState));
	if (spstate)
	  spstate->SetAction(
	      stateslist.Get(ev.Command.Code - VIEWMESH_STATES_SELECT_START) );
	else
	{
          csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
               iSpriteCal3DState));
          if (cal3dstate)
          {
	    cal3dstate->SetAnimCycle(stateslist.Get(ev.Command.Code - VIEWMESH_STATES_SELECT_START),1);
	  }
	}
	menu->Hide();
	return true;
      }
      if (ev.Command.Code >= VIEWMESH_OVERRIDE_SELECT_START &&
	  ev.Command.Code < VIEWMESH_OVERRIDE_SELECT_START + 100)
      {
      	csRef<iSprite3DState> spstate (
		SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
		iSprite3DState));
	if (spstate)
	  spstate->SetOverrideAction(
	      stateslist.Get(ev.Command.Code - VIEWMESH_OVERRIDE_SELECT_START) );
	else
	{
          csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
               iSpriteCal3DState));
          if (cal3dstate)
          {
	    cal3dstate->SetAnimAction(actionlist.Get(ev.Command.Code - VIEWMESH_OVERRIDE_SELECT_START),1,1);
	  }
	}
	menu->Hide();
	return true;
      }
      if (ev.Command.Code >= VIEWMESH_STATES_ADD_START &&
	  ev.Command.Code < VIEWMESH_STATES_ADD_START + 100)
      {
        csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
                iSpriteCal3DState));
        if (cal3dstate)
        {
	  cal3dstate->AddAnimCycle(stateslist.Get(ev.Command.Code - VIEWMESH_STATES_ADD_START),1,3);
	  activelist.Push(stateslist.Get(ev.Command.Code - VIEWMESH_STATES_ADD_START));
	  menu->Hide();
	  (void) new csMenuItem(activemenu,stateslist.Get(ev.Command.Code - VIEWMESH_STATES_ADD_START),VIEWMESH_STATES_CLEAR_START+activelist.Length());
	  return true;
	}
	menu->Hide();
	return true;
      }
      if (ev.Command.Code >= VIEWMESH_STATES_CLEAR_START &&
	  ev.Command.Code < VIEWMESH_STATES_CLEAR_START + 100)
      {
        csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
                iSpriteCal3DState));
        if (cal3dstate)
        {
	  if (ev.Command.Code > VIEWMESH_STATES_CLEAR_START)
	  {
	    csComponent *item = activemenu->GetItem (ev.Command.Code);
	    cal3dstate->ClearAnimCycle (item->GetText (),3);
	    activelist.Delete ((char*)item->GetText() );
	    menu->Hide ();
	    activemenu->Delete (item);
	    return true;
	  }
	}
	menu->Hide();
	return true;
      }
      if (ev.Command.Code >= VIEWMESH_MESH_SELECT_START &&
	  ev.Command.Code < VIEWMESH_MESH_SELECT_START + 100)
      {
        csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
                iSpriteCal3DState));
        if (cal3dstate)
        {
	  if (menu->GetCheck(ev.Command.Code))
	  {
	    csComponent *item = menu->GetItem (ev.Command.Code);
	    cal3dstate->DetachCoreMesh( item->GetText () );
            menu->SetCheck(ev.Command.Code, false);
	  }
	  else
	  {
	    csComponent *item = menu->GetItem (ev.Command.Code);
	    cal3dstate->AttachCoreMesh( item->GetText () );
            menu->SetCheck(ev.Command.Code, true);
	  }
	}
	menu->Hide();
	return true;
      }
      if (ev.Command.Code >= VIEWMESH_COMMAND_BLEND &&
          ev.Command.Code < VIEWMESH_COMMAND_BLEND + 100)
      {
        csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
                iSpriteCal3DState));
        if (cal3dstate)
        {
           int i = ev.Command.Code - VIEWMESH_COMMAND_BLEND;
           cal3dstate->BlendMorphTarget(i,1.0f,10.0f);
        }
      }
      if (ev.Command.Code >= VIEWMESH_COMMAND_CLEAR &&
          ev.Command.Code < VIEWMESH_COMMAND_CLEAR + 100)
      {
        csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
                iSpriteCal3DState));
        if (cal3dstate)
        {
           int i = ev.Command.Code - VIEWMESH_COMMAND_CLEAR;
           cal3dstate->ClearMorphTarget(i,10.0f);
        }
      }
                                
      if (ev.Command.Code >= VIEWMESH_COMMAND_SOCKET &&
          ev.Command.Code < VIEWMESH_COMMAND_SOCKET + 100)
      {
        csRef<iSpriteCal3DState> cal3dstate(
            SCF_QUERY_INTERFACE(sprite->GetMeshObject(), iSpriteCal3DState));
            
        if (cal3dstate)
        {
          int i = ev.Command.Code - VIEWMESH_COMMAND_SOCKET;
          menu->Hide();
          delete dialog;
          dialog = csFileDialog (this, "Select Mesh Object", "/this/", "Open",
                                 true);

          ModalData *data=new ModalData;
          
          data->code = VIEWMESH_COMMAND_LOADSOCKET+i;
          StartModal (dialog, data);     
        }    
      }
      break;
  }

  return false;
}


bool ViewMesh::LoadSprite(const char *filename, float scale)
{
  // grab the directory.
  char *path = new char[strlen(filename)+1];
  strcpy (path, filename);
  char* fn = path;
  char* slash = strrchr (path, '/');
  char* dir;
  if (slash)
  {
    fn = slash + 1;
    *slash = 0;
    dir = path;
  }
  else
    dir = "/";
  VFS->ChDir (dir);

  csRef<iMeshFactoryWrapper> imeshfactwrap (
  	loader->LoadMeshObjectFactory (fn));
  delete[] path;

  if (!imeshfactwrap)
    return false;

  // eventually remove the old sprite
  // FIXME: This badly fails if you load the same object again!
  if (sprite)
  {
    sprite->GetMovable()->ClearSectors();
    engine->GetMeshes()->Remove(sprite);
  }

  csMatrix3 scaling; scaling.Identity(); scaling *= scale;
  csReversibleTransform rT;
  rT.SetT2O (scaling);
  imeshfactwrap->HardTransform (rT);

  sprite = engine->CreateMeshWrapper(
      imeshfactwrap, "MySprite", room,
      csVector3 (0, 10, 0));
  csRef<iSprite3DState> spstate (SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
      iSprite3DState));
  if (spstate)
  {
    is_cal3d = false;
    spstate->SetAction("default");
  }
  // Update Sprite States menu
  stateslist.Push (csStrNew("default"));
  imeshfact = imeshfactwrap->GetMeshObjectFactory();
  csRef<iSprite3DFactoryState> factstate(SCF_QUERY_INTERFACE(imeshfact,
      iSprite3DFactoryState));
  if (factstate)
  {
    for (int i=0;i<factstate->GetActionCount ();i++)
    {
      iSpriteAction *spaction = factstate->GetAction(i);
      stateslist.Push (csStrNew (spaction->GetName ()));
    }
  }
  else
  {
    csRef<iSpriteCal3DState> cal3dstate(SCF_QUERY_INTERFACE(sprite->GetMeshObject(),
          iSpriteCal3DState));
    if (cal3dstate)
    {
      is_cal3d = true;
      for (int i=0;i<cal3dstate->GetAnimCount();i++)
      {
	if (cal3dstate->GetAnimType (i) == iSpriteCal3DState::C3D_ANIM_TYPE_ACTION)
          actionlist.Push (csStrNew (cal3dstate->GetAnimName (i)));
	else
          stateslist.Push (csStrNew (cal3dstate->GetAnimName (i)));
      }
      csRef<iSpriteCal3DFactoryState> factstate(SCF_QUERY_INTERFACE(imeshfact,
								    iSpriteCal3DFactoryState));
      if (factstate)
      {
        factstate->RescaleFactory(scale);
	  for (int i=0; i<factstate->GetMeshCount(); i++)
	  {
	      csString push;
	      if (factstate->IsMeshDefault(i))
		  push.Append("x"); // This is used as a flag to determine whether the item should be initially checked or not.
	      else
		  push.Append(" ");
	      push.Append(factstate->GetMeshName(i));
	      meshlist.Push(push);
	  }
	  int j;
          for (j=0;j<factstate->GetMorphAnimationCount();j++)
          {
            morphanimationlist.Push(csStrNew (factstate->GetMorphAnimationName(j)));
	  }
	  for(j=0;j< factstate->GetSocketCount();j++)
	  {
	    socketlist.Push(factstate->GetSocket(j)->GetName());
	  }
      }
    }
  }

  // try to get center of the sprite
  csBox3 box;
  sprite->GetWorldBoundingBox(box);
  spritepos = box.GetCenter();

  // light the sprite (needed for things).
  room->ShineLights(sprite);

  return true;
}

bool ViewMesh::SaveSprite(const char *filename)
{
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));

  csRef<iSaverPlugin> saver (CS_LOAD_PLUGIN (plugin_mgr,
    "crystalspace.mesh.saver.factory.sprite.3d.binary", iSaverPlugin));
	
	
  csRef<iVFS> VFS (CS_QUERY_REGISTRY (object_reg, iVFS));

  csRef<iFile> cf (VFS->Open (filename, VFS_FILE_WRITE));
  
  //TBD: change to new API
  //saver->WriteDown(imeshfact, cf);

  return true;
}

void ViewMesh::ConstructMenu()
{
  size_t i;

  if (menu)
    delete menu;

  menu = new csMenu(this, csmfs3D, 0);
  (void)new csMenuItem(menu,"Load Mesh", VIEWMESH_COMMAND_LOADMESH);
  (void)new csMenuItem(menu,"Save Mesh (Binary)", VIEWMESH_COMMAND_SAVEMESH);
  (void)new csMenuItem(menu,"Load TextureLib", VIEWMESH_COMMAND_LOADLIB);

  // AnimMenu
  csMenu *animmenu = new csMenu(0);
  (void)new csMenuItem(animmenu,"Move Sprite Faster", VIEWMESH_COMMAND_MOVEANIMFASTER);
  (void)new csMenuItem(animmenu,"Move Sprite Slower", VIEWMESH_COMMAND_MOVEANIMSLOWER);
  (void)new csMenuItem(animmenu,"Reverse Action",     VIEWMESH_COMMAND_REVERSEACTION);
  (void)new csMenuItem(animmenu,"Forward Action",     VIEWMESH_COMMAND_FORWARDACTION);
  (void)new csMenuItem(menu, "Action Effects",animmenu);

  // StateMenu
  csMenu *statesmenu = new csMenu(0);
  for (i=0;i<stateslist.Length();i++)
  {
    (void)new csMenuItem(statesmenu, stateslist.Get(i),
			 VIEWMESH_STATES_SELECT_START+i);
  }
  if (is_cal3d)
    (void)new csMenuItem(menu, "Set Action Loop", statesmenu);
  else
    (void)new csMenuItem(menu, "States", statesmenu);

  if (is_cal3d)
  {
    // AddActionMenu
    csMenu *addmenu = new csMenu(0);
    for (i=0;i<stateslist.Length();i++)
    {
      (void)new csMenuItem(addmenu, stateslist.Get(i),
			   VIEWMESH_STATES_ADD_START+i);
    }
    (void)new csMenuItem(menu, "Add Action Loop", addmenu);

    // ClearActionMenu
    activemenu = new csMenu(0);
    (void)new csMenuItem(activemenu, "(All)",
			   VIEWMESH_STATES_CLEAR_START);
    for (i=0;i<activelist.Length();i++)
    {
      (void)new csMenuItem(activemenu, activelist.Get(i),
			   VIEWMESH_STATES_CLEAR_START+i+1);
    }
    (void)new csMenuItem(menu, "Clear Action Loop", activemenu);

    csMenu *overridemenu = new csMenu(0);
    for (i=0;i<actionlist.Length();i++)
    {
      (void)new csMenuItem(overridemenu, actionlist.Get(i),
 	  		   VIEWMESH_OVERRIDE_SELECT_START+i);
    }
    (void)new csMenuItem(menu, "Overrides", overridemenu);

    csMenu *meshmenu = new csMenu(0);
    for (i=0;i<meshlist.Length();i++)
    {
      (void)new csMenuItem(meshmenu, meshlist.Get(i)+1,
 	  		   VIEWMESH_MESH_SELECT_START+i);
      if (*meshlist.Get(i) == 'x')
	  meshmenu->SetCheck(VIEWMESH_MESH_SELECT_START+i, true);
    }
    (void)new csMenuItem(menu, "Attach Meshes", meshmenu);
    //Blend morph animations
    csMenu *blendmenu = new csMenu(0);
    for(i=0;i<morphanimationlist.Length();i++)
    {
       (void)new csMenuItem(blendmenu, morphanimationlist.Get(i),
                           VIEWMESH_COMMAND_BLEND+i);
    }
    (void)new csMenuItem(menu, "Blend Morph Animation", blendmenu);
    //Clear morph animations
    csMenu *clearmenu = new csMenu(0);
    for(i=0;i<morphanimationlist.Length();i++)
    {
       (void)new csMenuItem(clearmenu, morphanimationlist.Get(i),
                           VIEWMESH_COMMAND_CLEAR+i);
    }
    (void)new csMenuItem(menu, "Clear Morph Animation", clearmenu);
    //sockets
    csMenu *socketmenu = new csMenu(0);
    for(i=0;i<socketlist.Length();i++)
    {
      (void)new csMenuItem(socketmenu, socketlist.Get(i),
			   VIEWMESH_COMMAND_SOCKET+i);
    }
    (void)new csMenuItem(menu, "Use Socket", socketmenu);

  }
  else
  {
    // OverrideActionMenu
    csMenu *overridemenu = new csMenu(0);
    for (i=0;i<stateslist.Length();i++)
    {
      (void)new csMenuItem(overridemenu, stateslist.Get(i),
  			   VIEWMESH_OVERRIDE_SELECT_START+i);
    }
    (void)new csMenuItem(menu, "Overrides", overridemenu);
  }
  // Camera Mode
  csMenu *cammode = new csMenu(0);
  (void)new csMenuItem(cammode, "Normal Movement", VIEWMESH_COMMAND_CAMMODE1);
  (void)new csMenuItem(cammode, "Look to Origin", VIEWMESH_COMMAND_CAMMODE2);
  (void)new csMenuItem(cammode, "Rotate", VIEWMESH_COMMAND_CAMMODE3);
  (void)new csMenuItem(menu, "Camera Mode", cammode);

  (void)new csMenuItem(menu,"~Quit", cscmdQuit);
  menu->Hide();
}

void ViewMesh::UpdateSpritePosition(csTicks elapsed)
{
    if (!sprite)
	return;
    if (!move_sprite_speed)
	return;

    csRef<iMovable> mov = sprite->GetMovable();
    
    csVector3 v(0,0,-move_sprite_speed*elapsed/1000);
    mov->MovePosition(v);

    v = mov->GetFullPosition();
    float const absz = (v.z < 0 ? -v.z : v.z);
    if (absz > 4.5)  // this should make the sprite loop.
    {
	v.z = -v.z;
	mov->SetPosition(v);
    }
    mov->UpdateMove();
}

void ViewMesh::Draw()
{
  // First get elapsed time from the system driver.
  csTicks elapsed_time, current_time;
  elapsed_time = vc->GetElapsedTicks ();
  current_time = vc->GetCurrentTicks ();

  // Now rotate the camera according to keyboard state
  float speed = (elapsed_time / 1000.0) * (0.03 * 20);

  if (sprite)
    UpdateSpritePosition(elapsed_time);

  if (!dialog && !menu->GetState(CSS_VISIBLE))
  {
    iCamera* c = view->GetCamera();
    switch (cammode)
    {
      case movenormal:
    	if (GetKeyState (CSKEY_RIGHT))
	  c->GetTransform ().RotateThis (CS_VEC_ROT_RIGHT, speed);
	if (GetKeyState (CSKEY_LEFT))
	  c->GetTransform ().RotateThis (CS_VEC_ROT_LEFT, speed);
	if (GetKeyState (CSKEY_PGUP))
	  c->GetTransform ().RotateThis (CS_VEC_TILT_UP, speed);
	if (GetKeyState (CSKEY_PGDN))
	  c->GetTransform ().RotateThis (CS_VEC_TILT_DOWN, speed);
	if (GetKeyState (CSKEY_UP))
	  c->Move (CS_VEC_FORWARD * 4 * speed);
	if (GetKeyState (CSKEY_DOWN))
	  c->Move (CS_VEC_BACKWARD * 4 * speed);
	break;
      case moveorigin:
	{
	  csVector3 orig = c->GetTransform().GetOrigin();

	  if (GetKeyState (CSKEY_DOWN))
	    c->GetTransform().SetOrigin (orig + CS_VEC_BACKWARD * 4 * speed);
	  if (GetKeyState (CSKEY_UP))
	    c->GetTransform().SetOrigin (orig + CS_VEC_FORWARD * 4 * speed);
	  if (GetKeyState (CSKEY_LEFT))
	    c->GetTransform().SetOrigin (orig + CS_VEC_LEFT * 4 * speed);
	  if (GetKeyState (CSKEY_RIGHT))
	    c->GetTransform().SetOrigin (orig + CS_VEC_RIGHT * 4 * speed);
	  if (GetKeyState (CSKEY_PGUP))
	    c->GetTransform().SetOrigin (orig + CS_VEC_UP * 4 * speed);
	  if (GetKeyState (CSKEY_PGDN))
	    c->GetTransform().SetOrigin (orig + CS_VEC_DOWN * 4 * speed);
	  c->GetTransform().LookAt (spritepos-orig, csVector3(0,1,0) );
  	  break;
	}
      case rotateorigin:
	{
	  csVector3 orig = c->GetTransform().GetOrigin();
	  if (GetKeyState (CSKEY_LEFT))
	    orig = csYRotMatrix3(-speed) * (orig-spritepos) + spritepos;
	  if (GetKeyState (CSKEY_RIGHT))
	    orig = csYRotMatrix3(speed) * (orig-spritepos) + spritepos;
	  if (GetKeyState (CSKEY_UP))
	    orig = csXRotMatrix3(speed) * (orig-spritepos) + spritepos;
	  if (GetKeyState (CSKEY_DOWN))
	    orig = csXRotMatrix3(-speed) * (orig-spritepos) + spritepos;
	  c->GetTransform().SetOrigin(orig);
	  if (GetKeyState (CSKEY_PGUP))
	    c->Move(CS_VEC_FORWARD * 4 * speed);
	  if (GetKeyState (CSKEY_PGDN))
	    c->Move(CS_VEC_BACKWARD * 4 * speed);
	  c->GetTransform().LookAt (spritepos-orig, csVector3(0,1,0) );
	  break;
	}
      default:
	break;
    }
  }

  //pplBeginDraw(CSDRAW_2DGRAPHICS);
  csApp::Draw();
  pplBeginDraw(CSDRAW_3DGRAPHICS);
  view->Draw();
  pplInvalidate(bound);
  if (menu->GetState(CSS_VISIBLE)) {
    menu->Invalidate(true);
  }
  if (dialog) {
    dialog->Invalidate(true);
  }
}

#define VM_QUERYPLUGIN(var, intf, str)				\
  var = CS_QUERY_REGISTRY (object_reg, intf);			\
  if (!var)							\
  {								\
    Printf (CS_REPORTER_SEVERITY_ERROR, "No " str " plugin!");	\
    return false;						\
  }

bool ViewMesh::Initialize ()
{
  if (!csApp::Initialize())
    return false;

  // Query for plugins
  // Find the pointer to engine plugin
  VM_QUERYPLUGIN (engine, iEngine, "iEngine");

  VM_QUERYPLUGIN (loader, iLoader, "iLoader");
  VM_QUERYPLUGIN (g3d, iGraphics3D, "iGraphics3D");

  csRef<iCommandLineParser> cmdline;
  VM_QUERYPLUGIN (cmdline, iCommandLineParser, "iCommandLineParser");

  const char* meshfilename = cmdline->GetName (0);
  const char* texturefilename = cmdline->GetName (1);
  const char* texturename = cmdline->GetName (2);
  const char* scaleTxt = cmdline->GetOption("Scale");
  const char* roomSize = cmdline->GetOption("RoomSize");

  float size = (roomSize)?atof(roomSize):5;

  // Set up a null cache.
  iCacheManager *cachemgr = new csNullCacheManager ();
  engine->SetCacheManager (cachemgr);
  cachemgr->DecRef ();

  // Open the main system. This will open all the previously loaded plug-ins.
  iGraphics2D* g2d = g3d->GetDriver2D ();
  iNativeWindow* nw = g2d->GetNativeWindow ();
  if (nw)
    nw->SetTitle ("View Mesh");

  // Setup the texture manager
  iTextureManager* txtmgr = g3d->GetTextureManager ();

  Printf (CS_REPORTER_SEVERITY_NOTIFY,
    "View Mesh version 0.1.");

  // First disable the lighting cache. Our app is simple enough
  // not to need this.
  engine->SetLightingCacheMode (0);

  // Create our world.
  Printf (CS_REPORTER_SEVERITY_NOTIFY, "Creating world!...");

  if (!loader->LoadTexture ("stone", "/lib/std/stone4.gif"))
  {
    Printf (CS_REPORTER_SEVERITY_ERROR, "Error loading 'stone4' texture!");
    return false;
  }
  if (!loader->LoadTexture ("spark", "/lib/std/spark.png"))
  {
    Printf (CS_REPORTER_SEVERITY_ERROR, "Error loading 'stone4' texture!");
    return false;
  }
  iMaterialWrapper* tm = engine->GetMaterialList ()->FindByName ("stone");

  room = engine->CreateSector ("room");
  csRef<iMeshWrapper> walls (engine->CreateSectorWallsMesh (room, "walls"));
  csRef<iThingState> ws =
  	SCF_QUERY_INTERFACE (walls->GetMeshObject (), iThingState);
  csRef<iThingFactoryState> walls_state = ws->GetFactory ();
  walls_state->AddInsideBox (csVector3 (-size, 0, -size),
  	csVector3 (size, 4*size, size));
  walls_state->SetPolygonMaterial (CS_POLYRANGE_LAST, tm);
  walls_state->SetPolygonTextureMapping (CS_POLYRANGE_LAST, 3);

  csRef<iLight> light;
  iLightList* ll = room->GetLights ();
  light = engine->CreateLight (0, csVector3 (-3, 10, 0), 10,
  	csColor (0.8f, 0.8f, 0.8f));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (3, 10,  0), 10,
  	csColor (0.8f, 0.8f, 0.8f));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 10, -3), 10,
  	csColor (0.8f, 0.8f, 0.8f));
  ll->Add (light);

  light = engine->CreateLight (0, csVector3 (0, 10,  3), 10,
  	csColor (0.8f, 0.8f, 0.8f));
  ll->Add (light);

  engine->Prepare ();
  Printf (CS_REPORTER_SEVERITY_NOTIFY, "Created.");

  view = csPtr<iView> (new csView (engine, g3d));
  view->GetCamera ()->SetSector (room);
  view->GetCamera ()->GetTransform ().SetOrigin (csVector3 (0, 10, -4));
  view->SetRectangle (0, 0, g2d->GetWidth (), g2d->GetHeight ());

  if (scaleTxt != 0)
  {
    sscanf (scaleTxt, "%f", &scale);
    printf ("Scaling: %f\n", scale);
  }
  else
      scale = 1.0F;

  // Load specified Libraries
  Printf (CS_REPORTER_SEVERITY_NOTIFY, "Loading libs...");
  const char *libname;
  int i;
  for (i=0; (libname=cmdline->GetOption("Lib",i)); i++)
  {
    if (!loader->LoadLibraryFile(libname))
    {
      Printf (CS_REPORTER_SEVERITY_ERROR, "Couldn't load lib %s.", libname);
    }
  }
  if (i>0)
    engine->Prepare();

  // Load a texture for our sprite.
  if (texturefilename && texturename)
  {
    iTextureWrapper* txt = loader->LoadTexture (texturename,
  	  texturefilename);
    if (txt == 0)
    {
      Printf (CS_REPORTER_SEVERITY_ERROR, "Error loading texture '%s'!",
      	texturefilename);
      return false;
    }
    txt->Register (txtmgr);
    txt->GetTextureHandle()->Prepare ();
    iMaterialWrapper* mat = engine->GetMaterialList ()->FindByName (
    	texturename);
    mat->Register (txtmgr);
    mat->GetMaterialHandle ()->Prepare ();
  }

  // Load a sprite template from disk.
  if (meshfilename && !LoadSprite(meshfilename,scale))
  {
    Printf (CS_REPORTER_SEVERITY_ERROR,
    	"Error loading mesh object factory '%s'!", meshfilename);
    return false;
  }

  // Create an menu
  ConstructMenu();

  return true;
}

bool ViewMesh::AttachMeshToSocket( int socketNumber, char* meshFile, 
                                   float xRot, float yRot, float zRot )
{
  csRef<iSpriteCal3DState> cal3dstate(
        SCF_QUERY_INTERFACE(sprite->GetMeshObject(), iSpriteCal3DState));
        
  if (cal3dstate)
  {
    iSpriteCal3DSocket* socket = cal3dstate->FindSocket(socketlist.Get(socketNumber));
    if ( !socket )
    {
      Printf (CS_REPORTER_SEVERITY_ERROR,
              "Error getting socket: %d!", socketNumber);  
      return false;                    
    }
  
    csRef<iMeshWrapper> meshWrapOld = socket->GetMeshWrapper();
    if ( meshWrapOld )
    {
        sprite->GetChildren()->Remove( meshWrapOld );
        socket->SetMeshWrapper( NULL );    
    }
    
    csRef<iMeshFactoryWrapper> factory = loader->LoadMeshObjectFactory( meshFile );
    if ( !factory )
    {
      Printf (CS_REPORTER_SEVERITY_ERROR,
              "Error loading mesh object factory '%s'!", meshFile);    
      return false;
    }
    else
    {
      csRef<iMeshWrapper> meshWrap = engine->CreateMeshWrapper( factory, meshFile );
      
      if ( xRot != 0 )          
      {
        meshWrap->GetFactory()->GetMeshObjectFactory()->
            HardTransform( csTransform(csXRotMatrix3(xRot),
                           csVector3(0,0,0) ));                
      }
      
      if ( yRot != 0 )          
      {
        meshWrap->GetFactory()->GetMeshObjectFactory()->
            HardTransform( csTransform(csYRotMatrix3(yRot),
                           csVector3(0,0,0) ));                
      }
      if ( zRot != 0 )          
      {
        meshWrap->GetFactory()->GetMeshObjectFactory()->
            HardTransform( csTransform(csZRotMatrix3(zRot),
                           csVector3(0,0,0) ));                
      }                                  
      sprite->GetChildren()->Add( meshWrap );
      socket->SetMeshWrapper( meshWrap );
    }                
  }
  else
  {
    Printf (CS_REPORTER_SEVERITY_ERROR,
            "Could not get iSpriteCal3dState");        
    return false;              
  }              
  
  
  return true;
}


void ViewMesh::CreateRotationWindow(int socket, char* filename)
{
  char buffer[100];
                
  dialog = new csWindow (this, "Select Rotation",
                        CSWS_DEFAULTVALUE | CSWS_TOOLBAR | CSWS_CLIENTBORDER);
  dialog->SetRect (10, 10, 200, 270);               
  csComponent *client = new csDialog (dialog);        
  client->id = 1000;
        
  csStatic *stat = new csStatic (client, 0, "RotX", csscsFrameLabel);
  stat->SetRect (10, 10, 150, 50);           
  csInputLine *il = new csInputLine (client, 40, csifsThickRect);
  il->id = 1001;
  il->SetRect (15, 20, 100, 36);        
  sprintf(buffer, "%f", DEFAULT_SOCKET_X_ROTATION ),
  il->SetText (buffer);        
       
  stat = new csStatic (client, 0, "RotY", csscsFrameLabel);
  stat->SetRect (10, 60, 150, 110);                           
  il = new csInputLine (client, 40, csifsThickRect);
  il->id = 1002;
  il->SetRect (15, 80, 100, 96);
  sprintf(buffer, "%f", DEFAULT_SOCKET_Y_ROTATION),
  il->SetText (buffer);        
        
  stat = new csStatic (client, 0, "RotZ", csscsFrameLabel);
  stat->SetRect (10, 120, 150, 170);                           
  il = new csInputLine (client, 40, csifsThickRect);
  il->id = 1003;
  il->SetRect (15, 140, 100, 157);
  sprintf(buffer, "%f", DEFAULT_SOCKET_Z_ROTATION),
  il->SetText (buffer);        
        
  csButton* b = new csButton (client, cscmdStopModal, CSBS_DEFAULTVALUE | CSBS_DISMISS);
  b->SetText ("~Ok");
  b->SetRect (10, 180, 150, 200);
      
  ModalData *newData=new ModalData;
  newData->meshName = filename;
  newData->socket   = socket;
       
  newData->code = VIEWMESH_COMMAND_ATTACH_SOCKET;
  StartModal (dialog, newData);               
}


/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/

// define a skin for csws
CSWS_SKIN_DECLARE_DEFAULT (DefaultSkin);

int main (int argc, char* argv[])
{
  srand (time (0));

  iObjectRegistry *object_reg = csInitializer::CreateEnvironment(argc, argv);
  if (!object_reg)
    return 1;

  if (!csInitializer::SetupConfigManager (object_reg, 0))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.application.viewmesh", "Couldn't load config file!");
    return 1;
  }
	
  if (!csInitializer::RequestPlugins (object_reg,
	      CS_REQUEST_VFS,
	      CS_REQUEST_OPENGL3D,
	      CS_REQUEST_ENGINE,
	      CS_REQUEST_FONTSERVER,
	      CS_REQUEST_IMAGELOADER,
	      CS_REQUEST_LEVELLOADER,
	      CS_REQUEST_REPORTER,
	      CS_REQUEST_REPORTERLISTENER,
	      CS_REQUEST_END))
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.application.viewmesh", "Couldn't find plugins!\n"
	"Is your CRYSTAL environment var properly set?");
    return 1;
  }

  if (csCommandLineHelper::CheckHelp (object_reg))
  {
    ViewMesh::Help();
    csCommandLineHelper::Help(object_reg);
    return 0;
  }

  if (!csInitializer::OpenApplication(object_reg))
  {
    return 1;
  }

  // Create our main class.
  ViewMesh *app = new ViewMesh (object_reg, DefaultSkin);

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!app->Initialize ())
  {
    csReport (object_reg, CS_REPORTER_SEVERITY_ERROR,
	"crystalspace.application.viewmesh", "Error initializing system!");
    csInitializer::DestroyApplication(object_reg);
    return 1;
  }

  // Main loop.
  csDefaultRunLoop(object_reg);

  // Cleanup.
  delete app;
  csInitializer::DestroyApplication(object_reg);

  return 0;
}

