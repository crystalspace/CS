/*
    Copyright (C) 2000-2001 by Jorrit Tyberghein

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
#include "walktest.h"
#include "bot.h"
#include "infmaze.h"
#include "hugeroom.h"
#include "wentity.h"
#include "command.h"
#include "ivaria/view.h"
#include "csutil/scanstr.h"
#include "csutil/csstring.h"
#include "csutil/util.h"
#include "cstool/impexp.h"
#include "csutil/dataobj.h"
#include "cstool/crossbld.h"
#include "csgeom/math3d.h"
#include "cssys/system.h"
#include "cssys/csendian.h"
#include "cstool/keyval.h"
#include "cstool/collider.h"
#include "cstool/cspixmap.h"
#include "qint.h"
#include "isound/handle.h"
#include "isound/source.h"
#include "isound/listener.h"
#include "isound/source.h"
#include "isound/renderer.h"
#include "isound/wrapper.h"
#include "imesh/skeleton.h"
#include "iengine/skelbone.h"
#include "ivideo/graph3d.h"
#include "ivideo/graph2d.h"
#include "isys/vfs.h"
#include "iengine/motion.h"
#include "iengine/light.h"
#include "ivaria/perfstat.h"
#include "imesh/sprite3d.h"

#include "csengine/polygon.h"
#include "csengine/radiosty.h"
#include "csengine/light.h"

extern WalkTest* Sys;

/// Save recording
void SaveRecording (iVFS* vfs, const char* fName)
{
  iFile* cf;
  cf = vfs->Open (fName, VFS_FILE_WRITE);
  long l = convert_endian (Sys->recording.Length ());
  cf->Write ((char*)&l, sizeof (l));
  int i;
  csRecordedCameraFile camint;
  iSector* prev_sector = NULL;
  for (i = 0 ; i < Sys->recording.Length () ; i++)
  {
    csRecordedCamera* reccam = (csRecordedCamera*)Sys->recording[i];
    camint.m11 = convert_endian (float2long (reccam->mat.m11));
    camint.m12 = convert_endian (float2long (reccam->mat.m12));
    camint.m13 = convert_endian (float2long (reccam->mat.m13));
    camint.m21 = convert_endian (float2long (reccam->mat.m21));
    camint.m22 = convert_endian (float2long (reccam->mat.m22));
    camint.m23 = convert_endian (float2long (reccam->mat.m23));
    camint.m31 = convert_endian (float2long (reccam->mat.m31));
    camint.m32 = convert_endian (float2long (reccam->mat.m32));
    camint.m33 = convert_endian (float2long (reccam->mat.m33));
    camint.x = convert_endian (float2long (reccam->vec.x));
    camint.y = convert_endian (float2long (reccam->vec.y));
    camint.z = convert_endian (float2long (reccam->vec.z));
    camint.ax = convert_endian (float2long (reccam->angle.x));
    camint.ay = convert_endian (float2long (reccam->angle.y));
    camint.az = convert_endian (float2long (reccam->angle.z));
    camint.mirror = reccam->mirror;
    cf->Write ((char*)&camint, sizeof (camint));
    unsigned char len;
    if (prev_sector == reccam->sector)
    {
      len = 255;
      cf->Write ((char*)&len, 1);
    }
    else
    {
      len = strlen (reccam->sector->QueryObject ()->GetName ());
      cf->Write ((char*)&len, 1);
      cf->Write (reccam->sector->QueryObject ()->GetName (),
      	1+strlen (reccam->sector->QueryObject ()->GetName ()));
    }
    prev_sector = reccam->sector;
    if (reccam->cmd)
    {
      len = strlen (reccam->cmd);
      cf->Write ((char*)&len, 1);
      cf->Write (reccam->cmd, 1+strlen(reccam->cmd));
    }
    else
    {
      len = 100;
      cf->Write ((char*)&len, 1);
    }
    if (reccam->arg)
    {
      len = strlen (reccam->arg);
      cf->Write ((char*)&len, 1);
      cf->Write (reccam->arg, 1+strlen(reccam->arg));
    }
    else
    {
      len = 100;
      cf->Write ((char*)&len, 1);
    }
  }
  cf->DecRef ();
}

/// Load recording
void LoadRecording (iVFS* vfs, const char* fName)
{
  iFile* cf;
  cf = vfs->Open (fName, VFS_FILE_READ);
  if (!cf) return;
  Sys->recording.DeleteAll ();
  Sys->recording.SetLength (0);
  long l;
  cf->Read ((char*)&l, sizeof (l));
  l = convert_endian (l);
  csRecordedCameraFile camint;
  iSector* prev_sector = NULL;
  int i;
  for (i = 0 ; i < l ; i++)
  {
    csRecordedCamera* reccam = new csRecordedCamera ();
    cf->Read ((char*)&camint, sizeof (camint));
    reccam->mat.m11 = long2float (convert_endian (camint.m11));
    reccam->mat.m12 = long2float (convert_endian (camint.m12));
    reccam->mat.m13 = long2float (convert_endian (camint.m13));
    reccam->mat.m21 = long2float (convert_endian (camint.m21));
    reccam->mat.m22 = long2float (convert_endian (camint.m22));
    reccam->mat.m23 = long2float (convert_endian (camint.m23));
    reccam->mat.m31 = long2float (convert_endian (camint.m31));
    reccam->mat.m32 = long2float (convert_endian (camint.m32));
    reccam->mat.m33 = long2float (convert_endian (camint.m33));
    reccam->vec.x = long2float (convert_endian (camint.x));
    reccam->vec.y = long2float (convert_endian (camint.y));
    reccam->vec.z = long2float (convert_endian (camint.z));
    reccam->angle.x = long2float (convert_endian (camint.ax));
    reccam->angle.y = long2float (convert_endian (camint.ay));
    reccam->angle.z = long2float (convert_endian (camint.az));
    reccam->mirror = camint.mirror;
    unsigned char len;
    cf->Read ((char*)&len, 1);
    iSector* s;
    if (len == 255)
    {
      s = prev_sector;
    }
    else
    {
      char buf[100];
      cf->Read (buf, 1+len);
      s = Sys->Engine->FindSector (buf);
    }
    reccam->sector = s;
    prev_sector = s;

    cf->Read ((char*)&len, 1);
    if (len == 100)
    {
      reccam->cmd = NULL;
    }
    else
    {
      reccam->cmd = new char[len+1];
      cf->Read (reccam->cmd, 1+len);
    }
    cf->Read ((char*)&len, 1);
    if (len == 100)
    {
      reccam->arg = NULL;
    }
    else
    {
      reccam->arg = new char[len+1];
      cf->Read (reccam->arg, 1+len);
    }
    Sys->recording.Push ((void*)reccam);
  }
  cf->DecRef ();
}

/// Save/load camera functions
void SaveCamera (iVFS* vfs, const char *fName)
{
  if (!Sys->view) return;
  iCamera *c = Sys->view->GetCamera ();
  csOrthoTransform& camtrans = c->GetTransform ();
  if (!c) return;
  const csMatrix3& m_o2t = camtrans.GetO2T ();
  const csVector3& v_o2t = camtrans.GetOrigin ();
  csString s;
  s << v_o2t.x << ' ' << v_o2t.y << ' ' << v_o2t.z << '\n'
    << m_o2t.m11 << ' ' << m_o2t.m12 << ' ' << m_o2t.m13 << '\n'
    << m_o2t.m21 << ' ' << m_o2t.m22 << ' ' << m_o2t.m23 << '\n'
    << m_o2t.m31 << ' ' << m_o2t.m32 << ' ' << m_o2t.m33 << '\n'
    << '"' << c->GetSector ()->QueryObject ()->GetName () << "\"\n"
    << c->IsMirrored () << '\n'
    << Sys->angle.x << ' ' << Sys->angle.y << ' ' << Sys->angle.z << '\n';
  vfs->WriteFile (fName, s.GetData(), s.Length());
}

bool LoadCamera (iVFS* vfs, const char *fName)
{
  if (!vfs->Exists (fName))
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "Could not open coordinate file '%s'!\n", fName);
    return false;
  }

  iDataBuffer *data = vfs->ReadFile(fName);
  if (!data)
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "Could not read coordinate file '%s'!\n", fName);
    return false;
  }

  csMatrix3 m;
  csVector3 v;
  int imirror = false;
  char* sector_name = new char [data->GetSize ()];

  csScanStr (**data,
    "%f %f %f\n"
    "%f %f %f\n"
    "%f %f %f\n"
    "%f %f %f\n"
    "%S\n"
    "%d\n"
    "%f %f %f",
    &v.x, &v.y, &v.z,
    &m.m11, &m.m12, &m.m13,
    &m.m21, &m.m22, &m.m23,
    &m.m31, &m.m32, &m.m33,
    sector_name,
    &imirror,
    &Sys->angle.x, &Sys->angle.y, &Sys->angle.z);

  iSector* s = Sys->Engine->FindSector (sector_name);
  delete[] sector_name;
  data->DecRef ();
  if (!s)
  {
    CsPrintf (CS_MSG_FATAL_ERROR, "Sector `%s' in coordinate file does not "
      "exist in this map!\n", sector_name);
    return false;
  }

  iCamera *c = Sys->view->GetCamera ();
  c->SetSector (s);
  c->SetMirrored ((bool)imirror);
  c->GetTransform ().SetO2T (m);
  c->GetTransform ().SetOrigin (v);
  return true;
}

void move_mesh (iMeshWrapper* sprite, iSector* where, csVector3 const& pos)
{
  sprite->GetMovable ()->SetPosition (pos);
  sprite->GetMovable ()->SetSector (where);
  sprite->GetMovable ()->UpdateMove ();
}

// Load a mesh object factory from a general format (3DS, MD2, ...)
// This creates a mesh object factory which you can then add using add_meshobj ().
void load_meshobj (char *filename, char *templatename, char* txtname)
{
  // First check if the texture exists.
  if (!Sys->view->GetEngine ()->FindMaterial (txtname))
  {
    Sys->Printf (CS_MSG_CONSOLE, "Can't find material '%s' in memory!\n", txtname);
    return;
  }

  // read in the model file
  converter * filedata = new converter;
  if (filedata->ivcon (filename, true, false, NULL, Sys->myVFS) == ERROR)
  {
    Sys->Printf (CS_MSG_CONSOLE, "There was an error reading the data!\n");
    delete filedata;
    return;
  }

  // convert data from the 'filedata' structure into a CS sprite template
  csCrossBuild_SpriteTemplateFactory builder (Sys);
  iMeshObjectFactory *result = (iMeshObjectFactory *)builder.CrossBuild (*filedata);
  delete filedata;

  // Add this sprite template to the engine.
  iSprite3DFactoryState* fstate = SCF_QUERY_INTERFACE (result, iSprite3DFactoryState);
  fstate->SetMaterialWrapper (Sys->Engine->FindMaterial (txtname));
  fstate->DecRef ();

  Sys->Engine->CreateMeshFactory (result, templatename);
}

iMeshWrapper* add_meshobj (char* tname, char* sname, iSector* where,
	csVector3 const& pos, float size)
{
  iMeshFactoryWrapper* tmpl = Sys->Engine->FindMeshFactory (tname);
  if (!tmpl)
  {
    Sys->Printf (CS_MSG_CONSOLE, "Unknown mesh factory '%s'!\n", tname);
    return NULL;
  }
  iSector* isector = SCF_QUERY_INTERFACE (where, iSector);
  iMeshWrapper* spr = Sys->Engine->CreateMeshObject (tmpl, sname,
  	isector, pos);
  isector->DecRef ();
  csMatrix3 m; m.Identity (); m = m * size;
  spr->GetMovable ()->SetTransform (m);
  spr->GetMovable ()->UpdateMove ();

  spr->DeferUpdateLighting (CS_NLIGHT_STATIC|CS_NLIGHT_DYNAMIC, 10);
  return spr;
}


void list_meshes (void)
{
  int num_meshes;
  const char* mesh_name;
  iMeshWrapper* mesh;

  num_meshes = Sys->Engine->GetMeshObjectCount ();

  for(int i = 0; i < num_meshes; i++)
  {
    mesh = Sys->Engine->GetMeshObject (i);
    mesh_name = mesh->QueryObject ()->GetName();

    if (mesh_name)
      Sys->Printf (CS_MSG_CONSOLE, "%s\n", mesh_name);
    else
      Sys->Printf (CS_MSG_CONSOLE, "A mesh with no name.\n");
  }
  Sys->Printf (CS_MSG_CONSOLE, "There are:%d meshes\n",
	       Sys->Engine->GetMeshObjectCount ());
}

//===========================================================================

void SetConfigOption (iBase* plugin, const char* optName, const char* optValue)
{
  iConfig* config = SCF_QUERY_INTERFACE (plugin, iConfig);
  if (!config)
    CsPrintf (CS_MSG_CONSOLE, "No config interface for this plugin.\n");
  else
  {
    int i;
    for (i = 0 ; ; i++)
    {
      csOptionDescription odesc;
      if (!config->GetOptionDescription (i, &odesc)) break;
      if (strcmp (odesc.name, optName) == 0)
      {
	CsPrintf (CS_MSG_CONSOLE, "Set option %s to %s\n", odesc.name, optValue);
	csVariant var;
	switch (odesc.type)
	{
	  case CSVAR_LONG: sscanf (optValue, "%ld", &var.v.l); break;
	  case CSVAR_BOOL: csScanStr (optValue, "%b", &var.v.b); break;
	  case CSVAR_FLOAT: csScanStr (optValue, "%f", &var.v.f); break;
	  default: break;
	}
	config->SetOption (i, &var);
	return;
      }
    }
    config->DecRef ();
  }
}

void SetConfigOption (iBase* plugin, const char* optName, csVariant& optValue)
{
  iConfig* config = SCF_QUERY_INTERFACE (plugin, iConfig);
  if (!config)
    CsPrintf (CS_MSG_CONSOLE, "No config interface for this plugin.\n");
  else
  {
    int i;
    for (i = 0 ; ; i++)
    {
      csOptionDescription odesc;
      if (!config->GetOptionDescription (i, &odesc)) break;
      if (strcmp (odesc.name, optName) == 0)
	config->SetOption (i, &optValue);
	config->DecRef ();
	return;
    }
    config->DecRef ();
  }
}

bool GetConfigOption (iBase* plugin, const char* optName, csVariant& optValue)
{
  iConfig* config = SCF_QUERY_INTERFACE (plugin, iConfig);
  if (!config)
    CsPrintf (CS_MSG_CONSOLE, "No config interface for this plugin.\n");
  else
  {
    int i;
    for (i = 0 ; ; i++)
    {
      csOptionDescription odesc;
      if (!config->GetOptionDescription (i, &odesc)) break;
      if (strcmp (odesc.name, optName) == 0)
      {
	config->GetOption (i, &optValue);
	config->DecRef ();
	return true;
      }
    }
    config->DecRef ();
  }
  return false;
}

//===========================================================================

void WalkTest::ParseKeyCmds (iObject* src)
{
  iObjectIterator *it = src->GetIterator ();
  while (!it->IsFinished ())
  {
    iKeyValuePair* kp = SCF_QUERY_INTERFACE (it->GetObject (), iKeyValuePair);
    if (!kp)
    {
      it->Next ();
      continue;
    }
    if (!strcmp (kp->GetKey (), "cmd_AnimateSky"))
    {
      iSector *Sector = SCF_QUERY_INTERFACE_FAST (src, iSector);
      if (Sector)
      {
        char name[100], rot[100];
        csScanStr (kp->GetValue (), "%s,%s,%f", name, rot, &anim_sky_speed);
        if (rot[0] == 'x') anim_sky_rot = 0;
        else if (rot[0] == 'y') anim_sky_rot = 1;
        else anim_sky_rot = 2;
        anim_sky = Sector->GetMesh (name);
	Sector->DecRef ();
      }
    }
    else if (!strcmp (kp->GetKey (), "cmd_AnimateDirLight"))
    {
      iMeshWrapper *wrap = SCF_QUERY_INTERFACE_FAST (src, iMeshWrapper);
      if (wrap)
      {
        anim_dirlight = wrap;
	wrap->DecRef ();
      }
    }
    else if (!strcmp (kp->GetKey (), "cmd_AnimateDynLight"))
    {
      iLight* l = SCF_QUERY_INTERFACE_FAST (src, iLight);
      if (l)
      {
        anim_dynlight = l;
	l->DecRef ();
      }
    }
    else if (!strcmp (kp->GetKey (), "entity_Door"))
    {
      iMeshWrapper *wrap = SCF_QUERY_INTERFACE_FAST (src, iMeshWrapper);
      if (wrap)
      {
        csVector3 hinge;
        csScanStr (kp->GetValue (), "%f,%f,%f", &hinge.x, &hinge.y, &hinge.z);
	csDoor* door = new csDoor (wrap);
	door->SetHinge (hinge);
        src->ObjAdd (door);
	door->DecRef ();
	wrap->DecRef ();
      }
    }
    else if (!strcmp (kp->GetKey (), "entity_Rotate"))
    {
      csVector3 angle;
      bool always;
      csScanStr (kp->GetValue (), "%f,%f,%f,%b", &angle.x, &angle.y, &angle.z,
		&always);
      csRotatingObject* rotobj = new csRotatingObject (src);
      rotobj->SetAngles (angle);
      rotobj->SetAlways (always);
      src->ObjAdd (rotobj);
      if (always)
	Sys->busy_entities.Push (rotobj);
    }
    else if (!strcmp (kp->GetKey (), "entity_Light"))
    {
      iMeshWrapper *wrap = SCF_QUERY_INTERFACE_FAST (src, iMeshWrapper);
      if (wrap)
      {
	csColor start_col, end_col;
	float act_time;
	char sector_name[100];
	char light_name[100];
        csScanStr (kp->GetValue (), "%s,%s,%f,%f,%f,%f,%f,%f,%f",
	  sector_name, light_name,
	  &start_col.red, &start_col.green, &start_col.blue,
	  &end_col.red, &end_col.green, &end_col.blue, &act_time);
	iSector* sect = Engine->FindSector (sector_name);
	if (!sect)
	{
	  CsPrintf (CS_MSG_WARNING, "Sector '%s' not found! 'entity_Light' is ignored!\n",
	  	sector_name);
	}
	else
	{
	  iStatLight* l = sect->GetLight (light_name);
	  if (!l)
	  {
	    CsPrintf (CS_MSG_WARNING, "Light '%s' not found! 'entity_Light' is ignored!\n",
	  	light_name);
	  }
	  else
	  {
	    iLight* il = SCF_QUERY_INTERFACE (l, iLight);
	    csLightObject* lobj = new csLightObject (il);
	    il->DecRef ();
	    lobj->SetColors (start_col, end_col);
	    lobj->SetTotalTime (act_time);
            src->ObjAdd (lobj);
	    lobj->DecRef ();
	  }
	}
	wrap->DecRef ();
      }
    }
    else
    {
      // Unknown command. Ignore for the moment.
    }
    kp->DecRef ();
    it->Next ();
  }
  it->DecRef ();

  iMeshWrapper *mesh = SCF_QUERY_INTERFACE_FAST (src, iMeshWrapper);
  if (mesh)
  {
    int k;
    for (k = 0 ; k < mesh->GetChildCount () ; k++)
    {
      iMeshWrapper* spr = mesh->GetChild (k);
      ParseKeyCmds (spr->QueryObject ());
    }
    mesh->DecRef ();
  }

}

void WalkTest::ParseKeyCmds ()
{
  int i;
  for (i = 0 ; i < Engine->GetSectorCount () ; i++)
  {
    iSector* sector = Engine->GetSector (i);
    ParseKeyCmds (sector->QueryObject ());

    int j;
    for (j = 0 ; j < sector->GetMeshCount () ; j++)
    {
      iMeshWrapper* sprite = sector->GetMesh (j);
      ParseKeyCmds (sprite->QueryObject ());
    }
    for (j = 0 ; j < sector->GetLightCount () ; j++)
    {
      iStatLight* l = sector->GetLight (j);
      ParseKeyCmds (l->QueryObject ());
    }
  }
}

void WalkTest::ActivateObject (iObject* src)
{
  iObjectIterator *it = src->GetIterator ();
  while (!it->IsFinished ())
  {
    csWalkEntity* wentity = SCF_QUERY_INTERFACE_FAST (it->GetObject (), csWalkEntity);
    if (wentity)
    {
      wentity->Activate ();
      wentity->DecRef ();
    }
    it->Next ();
  }
  it->DecRef ();
}

//===========================================================================

float safe_atof (const char* arg)
{
  if (arg) return atof (arg);
  else return 1;
}

//--//--//--//--//--//--//--//--//--//--//-- Handle our additional commands --//

// Command recording
#define RECORD_ARGS(CMD, ARG) \
if (Sys->cfg_recording >= 0)                        \
{                                                   \
  Sys->recorded_cmd = new char[strlen(CMD)+1];      \
  strcpy (Sys->recorded_cmd, CMD);                  \
  if (ARG)                                          \
  {                                                 \
    Sys->recorded_arg = new char[strlen(ARG)+1];    \
    strcpy (Sys->recorded_arg, ARG);                \
  }                                                 \
}
#define RECORD_CMD(CMD) \
if (Sys->cfg_recording >= 0)                        \
{                                                   \
  Sys->recorded_cmd = new char[strlen(CMD)+1];      \
  strcpy (Sys->recorded_cmd, CMD);                  \
}

bool CommandHandler (const char *cmd, const char *arg)
{
  if (!strcasecmp (cmd, "help"))
  {
    csCommandProcessor::perform (cmd, arg);
#   undef CONPRI
#   define CONPRI(m) Sys->Printf (CS_MSG_CONSOLE, m);
    CONPRI("-*- Additional commands -*-\n");
    CONPRI("Visibility:\n");
    CONPRI("  emode pvs freezepvs pvsonly\n");
    CONPRI("  db_octree, db_osolid, db_dumpstubs, db_cbuffer, db_frustum\n");
    CONPRI("  db_curleaf\n");
    CONPRI("Lights:\n");
    CONPRI("  addlight dellight dellights picklight droplight\n");
    CONPRI("  clrlights setlight\n");
    CONPRI("Movement:\n");
    CONPRI("  step_forward step_backward strafe_left strafe_right\n");
    CONPRI("  look_up look_down rotate_left rotate_right jump move3d\n");
    CONPRI("  i_forward i_backward i_left i_right i_up i_down i_rotleftc\n");
    CONPRI("  i_rotleftw i_rotrightc i_rotrightw i_rotleftx i_rotleftz\n");
    CONPRI("  i_rotrightx i_rotrightz do_gravity colldet freelook\n");
    CONPRI("Statistics:\n");
    CONPRI("  stats fps perftest coordshow\n");
    CONPRI("Special effects:\n");
    CONPRI("  addbot delbot addskel addghost fire explosion spiral rain\n");
    CONPRI("  snow fountain flame portal fs_inter fs_fadeout fs_fadecol\n");
    CONPRI("  fs_fadetxt fs_red fs_green fs_blue fs_whiteout fs_shadevert\n");
    CONPRI("Debugging:\n");
    CONPRI("  hi zbuf debug0 debug1 debug2 palette bugplug\n");
    CONPRI("  db_boxshow db_boxcam1 db_boxcam2 db_boxsize1 db_boxsize2\n");
    CONPRI("  db_boxnode1 db_boxnode2 db_boxvis db_radstep db_radhi db_radtodo\n");
    CONPRI("Meshes:\n");
    CONPRI("  loadmesh addmesh delmesh listmeshes\n");
    CONPRI("  listactions setaction setmotion\n");
    CONPRI("Various:\n");
    CONPRI("  coordsave coordload bind capture map mapproj p_alpha s_fog\n");
    CONPRI("  snd_play snd_volume record play clrrec saverec\n");
    CONPRI("  loadrec action plugins conflist confset\n");

#   undef CONPRI
  }
  else if (!strcasecmp (cmd, "coordsave"))
  {
    Sys->Printf (CS_MSG_CONSOLE, "Saved camera location in /temp/walktest.cam\n");
    SaveCamera (Sys->myVFS, "/temp/walktest.cam");
  }
  else if (!strcasecmp (cmd, "coordload"))
  {
    Sys->Printf (CS_MSG_CONSOLE, "Loaded camera location from /temp/walktest.cam\n");
    LoadCamera (Sys->myVFS, "/temp/walktest.cam");
  }
  else if (!strcasecmp (cmd, "plugins"))
  {
    int num = Sys->GetPlugInCount ();
    int i;
    for (i = 0 ; i < num ; i++)
    {
      iBase* plugin = Sys->GetPlugIn (i);
      iFactory* fact = SCF_QUERY_INTERFACE (plugin, iFactory);
      CsPrintf (CS_MSG_CONSOLE, "%d: %s\n", i, fact->QueryDescription ());
      fact->DecRef ();
    }
  }
  else if (!strcasecmp (cmd, "conflist"))
  {
    if (arg)
    {
      int idx;
      sscanf (arg, "%d", &idx);
      if (idx < 0 || idx >= Sys->GetPlugInCount ())
	CsPrintf (CS_MSG_CONSOLE, "Bad value for plugin (see 'plugins' command)!\n");
      else
      {
        iBase* plugin = Sys->GetPlugIn (idx);
        iConfig* config = SCF_QUERY_INTERFACE (plugin, iConfig);
	if (!config)
	  CsPrintf (CS_MSG_CONSOLE, "No config interface for this plugin.\n");
	else
	{
          int i;
          for (i = 0 ; ; i++)
          {
	    csOptionDescription odesc;
	    if (!config->GetOptionDescription (i, &odesc)) break;
	    CsPrintf (CS_MSG_CONSOLE, "Option %s (%s) ", odesc.name,
	        odesc.description);
	    csVariant var;
	    config->GetOption (i, &var);
	    switch (odesc.type)
	    {
	      case CSVAR_LONG: CsPrintf (CS_MSG_CONSOLE, "LONG=%ld\n",
				   var.v.l);
			       break;
	      case CSVAR_BOOL: CsPrintf (CS_MSG_CONSOLE, "BOOL=%d\n",
				   var.v.b);
			       break;
	      case CSVAR_CMD: CsPrintf (CS_MSG_CONSOLE, "CMD\n");
			       break;
	      case CSVAR_FLOAT: CsPrintf (CS_MSG_CONSOLE, "FLOAT=%g\n",
				   var.v.f);
			       break;
	      default: CsPrintf (CS_MSG_CONSOLE, "<unknown type>\n");
		       break;
	    }
          }
	  config->DecRef ();
	}
      }
    }
    else
      Sys->Printf (CS_MSG_CONSOLE, "Expected index to plugin (from 'plugins' command)!\n");
  }
  else if (!strcasecmp (cmd, "confset"))
  {
    if (arg)
    {
      char name[50];
      char val[256];
      int idx;
      csScanStr (arg, "%d,%s,%s", &idx, name, val);
      if (idx < 0 || idx >= Sys->GetPlugInCount ())
	CsPrintf (CS_MSG_CONSOLE, "Bad value for plugin (see 'plugins' command)!\n");
      else
      {
        iBase* plugin = Sys->GetPlugIn (idx);
	SetConfigOption (plugin, name, val);
      }
    }
    else
      Sys->Printf (CS_MSG_CONSOLE, "Expected index to plugin (from 'plugins' command)!\n");
  }
  else if (!strcasecmp (cmd, "action"))
  {
    csVector3 where = Sys->view->GetCamera ()->GetTransform ().This2Other(3.0f*VEC_FORWARD);
    csPolygon3D* p = Sys->view->GetCamera ()->GetPrivateObject ()->GetHit (where);
    if (p)
    {
      CsPrintf (CS_MSG_CONSOLE, "Action polygon '%s' ", p->GetName ());
      csThing* ob = p->GetParent ();
      CsPrintf (CS_MSG_CONSOLE, "in set '%s'\n", ob->GetName ());
      printf ("ACTION\n");
      Sys->ActivateObject ((csObject*)ob);
    }
  }
  else if (!strcasecmp (cmd, "saverec"))
  {
    if (arg)
    {
      char buf[255];
      sprintf (buf, "/this/%s.rec", arg);
      SaveRecording (Sys->myVFS, buf);
    }
    else
      SaveRecording (Sys->myVFS, "/this/record");
  }
  else if (!strcasecmp (cmd, "loadrec"))
  {
    if (Sys->perf_stats && Sys->recorded_perf_stats)
      Sys->perf_stats->FinishSubsection ();
    Sys->recorded_perf_stats = NULL;
    delete [] Sys->recorded_perf_stats_name;
    Sys->recorded_perf_stats_name = NULL;
    if (arg)
    {
      char buf[255];
      sprintf (buf, "/this/%s.rec", arg);
      LoadRecording (Sys->myVFS, buf);
      Sys->recorded_perf_stats_name = csStrNew (arg);
    }
    else
      LoadRecording (Sys->myVFS, "/this/record");
  }
  else if (!strcasecmp (cmd, "clrrec"))
  {
    Sys->recording.DeleteAll ();
    Sys->recording.SetLength (0);
  }
  else if (!strcasecmp (cmd, "record"))
  {
    if (Sys->cfg_recording == -1)
    {
      Sys->cfg_playrecording = -1;
      Sys->cfg_recording = 0;
      Sys->Printf (CS_MSG_CONSOLE, "Start recording camera movement...\n");
      if (Sys->perf_stats)
        Sys->recorded_perf_stats = Sys->perf_stats->StartNewSubsection (NULL);
    }
    else
    {
      Sys->cfg_recording = -1;
      Sys->Printf (CS_MSG_CONSOLE, "Stop recording.\n");
      if (Sys->perf_stats)
        Sys->perf_stats->FinishSubsection ();
      Sys->recorded_perf_stats = NULL;
    }
  }
  else if (!strcasecmp (cmd, "play"))
  {
    if (Sys->cfg_playrecording == -1)
    {
      Sys->cfg_recording = -1;
      Sys->cfg_playrecording = 0;
      Sys->Printf (CS_MSG_CONSOLE, "Start playing back camera movement...\n");
      Sys->cfg_playloop = true;
      if (arg)
      {
	bool summary = true;
	char name[50], option[50];
	int resolution = 0;
	csScanStr (arg, "%s,%d,%s", option, &resolution, name);
	if (Sys->perf_stats)
	  Sys->recorded_perf_stats = Sys->perf_stats->StartNewSubsection (name);
	if (!strcasecmp (option, "res") && (resolution >= 1))
	{
	  if (Sys->perf_stats)
	    Sys->perf_stats->SetResolution (resolution);
	  summary = false;
	}
	else if (!strcasecmp (option, "break") && (resolution >= 1))
	{
	  Sys->recorded_perf_stats->DebugSetBreak (resolution);
	  return true;
	}
	else
    {
	  strcpy(name, option);
    }
	char buf[255];
	sprintf (buf, "/this/%s.rps", name);
	Sys->recorded_perf_stats->SetOutputFile (buf, summary);
	if (Sys->recorded_perf_stats_name)
	  Sys->recorded_perf_stats->SetName (Sys->recorded_perf_stats_name);
	Sys->recorded_perf_stats->DebugSetBreak (-1);
	Sys->cfg_playloop = false;
      }
    }
    else
    {
      Sys->cfg_playrecording = -1;
      Sys->Printf (CS_MSG_CONSOLE, "Stop playback.\n");
      if (Sys->perf_stats)
        Sys->perf_stats->FinishSubsection ();
      Sys->recorded_perf_stats = NULL;
    }
  }
  else if (!strcasecmp (cmd, "recsubperf"))
  {
    RECORD_ARGS (cmd, arg);
    if (Sys->recorded_perf_stats)
    {
      if (!Sys->recorded_perf_stats->IsSubsection ())
      {
	if (!arg)
	  Sys->Printf (CS_MSG_CONSOLE, "Error: Expecting name of subsection.\n");
	else
	{
	  Sys->recorded_perf_stats->StartNewSubsection (arg);
	  Sys->Printf (CS_MSG_CONSOLE, "Performance subsection '%s'\n",arg);
	}
      }
      else
      {
	Sys->Printf (CS_MSG_CONSOLE, "Finished performance subsection.\n");
	Sys->recorded_perf_stats->PrintSubsectionStats (CS_MSG_CONSOLE);
	Sys->recorded_perf_stats->FinishSubsection ();
      }
    }
    else
      Sys->Printf (CS_MSG_CONSOLE, "Error: Option valid only when recording.\n");
  }
  else if (!strcasecmp (cmd, "bind"))
  {
    extern void bind_key (const char* arg);
    bind_key (arg);
  }
  else if (!strcasecmp (cmd, "bugplug"))
  {
    (void)_CS_LOAD_PLUGIN (Sys, "crystalspace.utilities.bugplug", "BugPlug");
  }
  else if (!strcasecmp (cmd, "fps"))
    csCommandProcessor::change_boolean (arg, &Sys->do_fps, "fps");
  else if (!strcasecmp (cmd, "do_gravity"))
    csCommandProcessor::change_boolean (arg, &Sys->do_gravity, "do_gravity");
  else if (!strcasecmp (cmd, "inverse_mouse"))
    csCommandProcessor::change_boolean (arg, &Sys->inverse_mouse, "inverse_mouse");
  else if (!strcasecmp (cmd, "colldet"))
    csCommandProcessor::change_boolean (arg, &Sys->do_cd, "colldet");
  else if (!strcasecmp (cmd, "zbuf"))
    csCommandProcessor::change_boolean (arg, &Sys->do_show_z, "zbuf");
  else if (!strcasecmp (cmd, "db_boxshow"))
    csCommandProcessor::change_boolean (arg, &Sys->do_show_debug_boxes, "show debug boxes");
  else if (!strcasecmp (cmd, "db_boxcam1"))
    Sys->debug_box1.SetCenter (Sys->view->GetCamera ()->GetTransform ().GetOrigin ());
  else if (!strcasecmp (cmd, "db_boxcam2"))
    Sys->debug_box2.SetCenter (Sys->view->GetCamera ()->GetTransform ().GetOrigin ());
  else if (!strcasecmp (cmd, "db_boxsize1"))
  {
    float size = Sys->debug_box1.MaxX ()-Sys->debug_box1.MinX ();
    csCommandProcessor::change_float (arg, &size, "box1 size", 0.01, 1000);
    Sys->debug_box1.SetSize (csVector3 (size, size, size));
  }
  else if (!strcasecmp (cmd, "db_boxsize2"))
  {
    float size = Sys->debug_box2.MaxX ()-Sys->debug_box2.MinX ();
    csCommandProcessor::change_float (arg, &size, "box2 size", 0.01, 1000);
    Sys->debug_box2.SetSize (csVector3 (size, size, size));
  }
  else if (!strcasecmp (cmd, "db_boxnode1"))
  {
#if 0
//@@@
    iSector* room = Sys->view->GetCamera ()->GetSector ();
    csThing* stat = room->GetStaticThing ();
    if (stat)
    {
      csPolygonTree* tree = stat->GetStaticTree ();
      csOctree* otree = (csOctree*)tree;
      csOctreeNode* l = otree->GetLeaf (Sys->view->GetCamera ()->GetOrigin ());
      Sys->debug_box1 = l->GetBox ();
      Sys->Printf (CS_MSG_CONSOLE, "Box 1: (%f,%f,%f)-(%f,%f,%f)\n",
	  	Sys->debug_box1.MinX (), Sys->debug_box1.MinY (),
	  	Sys->debug_box1.MinZ (), Sys->debug_box1.MaxX (),
	  	Sys->debug_box1.MaxY (), Sys->debug_box1.MaxZ ());
    }
    else
      Sys->Printf (CS_MSG_CONSOLE, "There is no octree in this sector!\n");
#endif
  }
  else if (!strcasecmp (cmd, "db_boxnode2"))
  {
#if 0
//@@@
    iSector* room = Sys->view->GetCamera ()->GetSector ();
    csThing* stat = room->GetStaticThing ();
    if (stat)
    {
      csPolygonTree* tree = stat->GetStaticTree ();
      csOctree* otree = (csOctree*)tree;
      csOctreeNode* l = otree->GetLeaf (Sys->view->GetCamera ()->GetOrigin ());
      Sys->debug_box2 = l->GetBox ();
      Sys->Printf (CS_MSG_CONSOLE, "Box 2: (%f,%f,%f)-(%f,%f,%f)\n",
	  	Sys->debug_box2.MinX (), Sys->debug_box2.MinY (),
	  	Sys->debug_box2.MinZ (), Sys->debug_box2.MaxX (),
	  	Sys->debug_box2.MaxY (), Sys->debug_box2.MaxZ ());
    }
    else
      Sys->Printf (CS_MSG_CONSOLE, "There is no octree in this sector!\n");
#endif
  }
  else if (!strcasecmp (cmd, "db_boxvis"))
  {
#if 0
//@@@
    iSector* room = Sys->view->GetCamera ()->GetSector ();
    csThing* stat = room->GetStaticThing ();
    if (stat)
    {
      //csPolygonTree* tree = stat->GetStaticTree ();
      //csOctree* otree = (csOctree*)tree;
      //bool vis1 = otree->BoxCanSeeBox (Sys->debug_box1, Sys->debug_box2);
      //bool vis2 = otree->BoxCanSeeBox (Sys->debug_box2, Sys->debug_box1);
      //CsPrintf (CS_MSG_CONSOLE, "Box1->box2:%d box2->box1:%d\n", vis1, vis2);
    }
    else
      CsPrintf (CS_MSG_CONSOLE, "No octree in this sector!\n");
#endif
  }
  else if (!strcasecmp (cmd, "db_cbuffer"))
    csCommandProcessor::change_boolean (arg, &Sys->do_show_cbuffer, "debug cbuffer");
  else if (!strcasecmp (cmd, "db_frustum"))
    csCommandProcessor::change_int (arg, &Sys->cfg_debug_check_frustum, "debug check frustum", 0, 2000000000);
  else if (!strcasecmp (cmd, "db_octree"))
    csCommandProcessor::change_int (arg, &Sys->cfg_draw_octree, "debug octree", -1, 10);
  else if (!strcasecmp (cmd, "db_curleaf"))
  {
#if 0
//@@@
    iSector* room = Sys->view->GetCamera ()->GetSector ();
    csThing* stat = room->GetStaticThing ();
    if (stat)
    {
      csPolygonTree* tree = stat->GetStaticTree ();
      csOctree* otree = (csOctree*)tree;
      csOctreeNode* l = otree->GetLeaf (Sys->view->GetCamera ()->GetOrigin ());
      const csBox3& b = l->GetBox ();
      printf ("Leaf (%d): %f,%f,%f %f,%f,%f\n", l->IsLeaf (),
      	b.MinX (), b.MinY (), b.MinZ (), b.MaxX (), b.MaxY (), b.MaxZ ());
      printf ("Solid Mask x: %04x\n", l->GetSolidMask (BOX_SIDE_x));
      printf ("Solid Mask X: %04x\n", l->GetSolidMask (BOX_SIDE_X));
      printf ("Solid Mask y: %04x\n", l->GetSolidMask (BOX_SIDE_y));
      printf ("Solid Mask Y: %04x\n", l->GetSolidMask (BOX_SIDE_Y));
      printf ("Solid Mask z: %04x\n", l->GetSolidMask (BOX_SIDE_z));
      printf ("Solid Mask Z: %04x\n", l->GetSolidMask (BOX_SIDE_Z));
    }
    else
      Sys->Printf (CS_MSG_CONSOLE, "There is no octree in this sector!\n");
#endif
  }
  else if (!strcasecmp (cmd, "db_dumpstubs"))
  {
#if 0
//@@@
    iSector* room = Sys->view->GetCamera ()->GetSector ();
    csThing* stat = room->GetStaticThing ();
    if (stat)
    {
      csPolygonTree* tree = stat->GetStaticTree ();
      csOctree* otree = (csOctree*)tree;
      printf ("1\n");
      //Dumper::dump_stubs (otree);
    }
    csMeshWrapper* spr = (csMeshWrapper*)Sys->engine->meshes[0];
    if (spr)
    {
      //@@@@@Dumper::dump_stubs (spr->GetPolyTreeObject ());
    }
    CsPrintf (CS_MSG_DEBUG_0F, "======\n");
#endif
  }
  else if (!strcasecmp (cmd, "db_osolid"))
  {
#if 0
//@@@
    extern void CreateSolidThings (csEngine*, iSector*, csOctreeNode*, int);
    iSector* room = Sys->view->GetCamera ()->GetSector ();
    csThing* stat = room->GetStaticThing ();
    if (stat)
    {
      csPolygonTree* tree = stat->GetStaticTree ();
      csOctree* otree = (csOctree*)tree;
      CreateSolidThings (Sys->Engine, room, otree->GetRoot (), 0);
    }
#endif
  }
  else if (!strcasecmp (cmd, "db_radstep"))
  {
    csRadiosity* rad;
    csEngine* engine = (csEngine*)(Sys->Engine);
    if ((rad = engine->GetRadiosity ()) != NULL)
    {
      int steps;
      if (arg)
        sscanf (arg, "%d", &steps);
      else
        steps = 1;
      if (steps < 1) steps = 1;
      rad->DoRadiosityStep (steps);
      engine->InvalidateLightmaps ();
    }
  }
  else if (!strcasecmp (cmd, "db_radtodo"))
  {
    csRadiosity* rad;
    csEngine* engine = (csEngine*)(Sys->Engine);
    if ((rad = engine->GetRadiosity ()) != NULL)
    {
      rad->ToggleShowDeltaMaps ();
      engine->InvalidateLightmaps ();
    }
  }
  else if (!strcasecmp (cmd, "db_radhi"))
  {
    csRadiosity* rad;
    csEngine* engine = (csEngine*)(Sys->Engine);
    if ((rad = engine->GetRadiosity ()) != NULL)
    {
      Sys->selected_polygon = SCF_QUERY_INTERFACE (rad->GetNextPolygon (),
      	iPolygon3D);
      Sys->selected_polygon->DecRef ();
    }
  }
  else if (!strcasecmp (cmd, "palette"))
    csCommandProcessor::change_boolean (arg, &Sys->do_show_palette, "palette");
  else if (!strcasecmp (cmd, "move3d"))
    csCommandProcessor::change_boolean (arg, &Sys->move_3d, "move3d");
  else if (!strcasecmp (cmd, "pvs"))
  {
    csEngine* engine = (csEngine*)(Sys->Engine);
    bool en = engine->IsPVS ();
    csCommandProcessor::change_boolean (arg, &en, "pvs");
    if (en) 
      engine->EnablePVS ();
    else
      engine->DisablePVS ();
  }
  else if (!strcasecmp (cmd, "pvsonly"))
  {
    csEngine* engine = (csEngine*)(Sys->Engine);
    bool en = engine->IsPVSOnly ();
    csCommandProcessor::change_boolean (arg, &en, "pvs only");
    if (en) 
      engine->EnablePVSOnly ();
    else
      engine->DisablePVSOnly ();
  }
  else if (!strcasecmp (cmd, "freezepvs"))
  {
    csEngine* engine = (csEngine*)(Sys->Engine);
    bool en = engine->IsPVSFrozen ();
    csCommandProcessor::change_boolean (arg, &en, "freeze pvs");
    if (en) 
      engine->FreezePVS (Sys->view->GetCamera ()->GetTransform ().GetOrigin ());
    else
      engine->UnfreezePVS ();
  }
  else if (!strcasecmp (cmd, "emode"))
  {
    const char* const choices[5] = { "auto", "back2front", "front2back",
    	"zbuffer", NULL };
    int emode = Sys->Engine->GetEngineMode ();
    csCommandProcessor::change_choice (arg, &emode, "engine mode", choices, 4);
    csEngine* engine = (csEngine*)(Sys->Engine);
    engine->SetEngineMode (emode);
  }
  else if (!strcasecmp (cmd, "freelook"))
  {
    csCommandProcessor::change_boolean (arg, &Sys->do_freelook, "freelook");
    if (Sys->do_freelook)
      Sys->myG2D->SetMousePosition (FRAME_WIDTH / 2, FRAME_HEIGHT / 2);
  }
  else if (!strcasecmp (cmd, "stats"))
  {
    csCommandProcessor::change_boolean (arg, &Sys->do_stats, "stats");
    if (Sys->do_stats) Sys->do_show_coord = false;
  }
  else if (!strcasecmp (cmd, "coordshow"))
  {
    csCommandProcessor::change_boolean (arg, &Sys->do_show_coord, "coordshow");
    if (Sys->do_show_coord) Sys->do_stats = false;
  }
  else if (!strcasecmp (cmd, "hi"))
  {
#if 0
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    csPolygon3D* hi = arg ? Sys->view->GetCamera ()->GetSector ()->GetPolygon3D (arg) : (csPolygon3D*)NULL;
    if (hi) Sys->Printf (CS_MSG_CONSOLE, "Hilighting polygon: '%s'\n", arg);
    else Sys->Printf (CS_MSG_CONSOLE, "Disabled hilighting.\n");
    Sys->selected_polygon = hi;
#endif
  }
  else if (!strcasecmp (cmd, "p_alpha"))
  {
#if 0
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    csPolygon3D* hi = Sys->selected_polygon;
    if (hi)
    {
      if (hi->GetPortal ())
      {
        int a = hi->GetAlpha ();
        csCommandProcessor::change_int (arg, &a, "portal alpha", 0, 100);
	hi->SetAlpha (a);
      }
      else Sys->Printf (CS_MSG_CONSOLE, "Only for portals!\n");
    }
    else Sys->Printf (CS_MSG_CONSOLE, "No polygon selected!\n");
#endif
  }
  else if (!strcasecmp (cmd, "s_fog"))
  {
    csFog* f = Sys->view->GetCamera ()->GetSector ()->GetFog ();
    if (!arg)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Fog in current sector (%f,%f,%f) density=%f\n",
      	f->red, f->green, f->blue, f->density);
    }
    else
    {
      float r, g, b, dens;
      if (csScanStr (arg, "%f,%f,%f,%f", &r, &g, &b, &dens) != 4)
      {
        Sys->Printf (CS_MSG_CONSOLE, "Expected r,g,b,density. Got something else!\n");
        return false;
      }
      f->enabled = true;
      f->density = dens;
      f->red = r;
      f->green = g;
      f->blue = b;
    }
  }
  else if (!strcasecmp (cmd, "capture"))
    CaptureScreen ();
  else if (!strcasecmp (cmd, "portal"))
  {
    if (arg)
    {
      char level[100];
      csScanStr (arg, "%s", level);
      void OpenPortal (iLoader*, iView* view, char* lev);
      OpenPortal (Sys->LevelLoader, Sys->view, level);
    }
    else
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameter 'level'!\n");
  }
  else if (!strcasecmp (cmd, "fs_inter"))
  {
    Sys->do_fs_inter = !Sys->do_fs_inter;
    if (Sys->do_fs_inter)
    {
      Sys->fs_inter_amount = .3;
      Sys->fs_inter_length = 30;
      Sys->fs_inter_anim = 0;
      if (arg)
        csScanStr (arg, "%f,%f", &Sys->fs_inter_amount, &Sys->fs_inter_length);
    }
  }
  else if (!strcasecmp (cmd, "fs_fadeout"))
  {
    Sys->do_fs_fadeout = !Sys->do_fs_fadeout;
    if (Sys->do_fs_fadeout)
    {
      Sys->fs_fadeout_fade = 0;
      Sys->fs_fadeout_dir = true;
    }
  }
  else if (!strcasecmp (cmd, "fs_fadecol"))
  {
    Sys->do_fs_fadecol = !Sys->do_fs_fadecol;
    if (Sys->do_fs_fadecol)
    {
      Sys->fs_fadecol_fade = 0;
      Sys->fs_fadecol_dir = true;
      float r = 1, g = 0, b = 0;
      if (arg) csScanStr (arg, "%f,%f,%f", &r, &g, &b);
      Sys->fs_fadecol_color.Set (r, g, b);
    }
  }
  else if (!strcasecmp (cmd, "fs_fadetxt"))
  {
    Sys->do_fs_fadetxt = !Sys->do_fs_fadetxt;
    if (Sys->do_fs_fadetxt)
    {
      Sys->fs_fadetxt_fade = 0;
      Sys->fs_fadetxt_dir = true;
      char buf[255];
      *buf = 0;
      if (arg) csScanStr (arg, "%s", buf);
      iMaterialWrapper* mat = Sys->view->GetEngine ()->FindMaterial (buf);
      if (mat)
      {
        Sys->fs_fadetxt_mat = mat->GetMaterialHandle ();
      }
      else
      {
        Sys->Printf (CS_MSG_CONSOLE, "Can't find material!\n");
	Sys->do_fs_fadetxt = false;
      }
    }
  }
  else if (!strcasecmp (cmd, "fs_red"))
  {
    Sys->do_fs_red = !Sys->do_fs_red;
    if (Sys->do_fs_red)
    {
      Sys->fs_red_fade = 0;
      Sys->fs_red_dir = true;
    }
  }
  else if (!strcasecmp (cmd, "fs_green"))
  {
    Sys->do_fs_green = !Sys->do_fs_green;
    if (Sys->do_fs_green)
    {
      Sys->fs_green_fade = 0;
      Sys->fs_green_dir = true;
    }
  }
  else if (!strcasecmp (cmd, "fs_blue"))
  {
    Sys->do_fs_blue = !Sys->do_fs_blue;
    if (Sys->do_fs_blue)
    {
      Sys->fs_blue_fade = 0;
      Sys->fs_blue_dir = true;
    }
  }
  else if (!strcasecmp (cmd, "fs_whiteout"))
  {
    Sys->do_fs_whiteout = !Sys->do_fs_whiteout;
    if (Sys->do_fs_whiteout)
    {
      Sys->fs_whiteout_fade = 0;
      Sys->fs_whiteout_dir = true;
    }
  }
  else if (!strcasecmp (cmd, "fs_shadevert"))
  {
    Sys->do_fs_shadevert = !Sys->do_fs_shadevert;
    if (Sys->do_fs_shadevert)
    {
      float tr = 1, tg = 0, tb = 0, br = 0, bg = 0, bb = 1;
      if (arg) csScanStr (arg, "%f,%f,%f,%f,%f,%f", &tr, &tg, &tb, &br, &bg, &bb);
      Sys->fs_shadevert_topcol.Set (tr, tg, tb);
      Sys->fs_shadevert_botcol.Set (br, bg, bb);
    }
  }
  else if (!strcasecmp (cmd, "perftest"))
  {
    int num = 100;
    if (arg) csScanStr (arg, "%d", &num);
    perf_test (num);
  }
  else if (!strcasecmp (cmd, "debug0"))
  {
#if 0
//@@@
    csCamera* c = Sys->view->GetCamera ();
    if (c->GetSector ()->GetStaticThing ())
    {
      csOctree* octree = (csOctree*)(c->GetSector ()->GetStaticThing ()->GetStaticTree ());
      //Dumper::dump_stubs (octree);
#if 0
      csNamedObjVector& meshes = Sys->view->GetEngine ()->meshes;
      int i;
      for (i = 0 ; i < meshes.Length () ; i++)
      {
        csMeshWrapper* spr = (csMeshWrapper*)meshes[i];
	@@@@Dumper::dump_stubs (spr->GetPolyTreeObject ());
      }
#endif
    }
    //Sys->Printf (CS_MSG_CONSOLE, "No debug0 implementation in this version.\n");
#endif
  }
  else if (!strcasecmp (cmd, "debug1"))
  {
    extern int covtree_level;
    covtree_level++;
    if (covtree_level > 25) covtree_level = 1;
    printf ("covtree_level=%d\n", covtree_level);
    //Sys->Printf (CS_MSG_CONSOLE, "No debug1 implementation in this version.\n");
  }
  else if (!strcasecmp (cmd, "debug2"))
  {
#   if 0
    extern bool do_covtree_dump;
    do_covtree_dump = !do_covtree_dump;
#   else
    Sys->Printf (CS_MSG_CONSOLE, "No debug2 implementation in this version.\n");
#   endif
  }
  else if (!strcasecmp (cmd, "strafe_left"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_left (.1, false, false); }
    else Sys->strafe (-1*f,0);
  }
  else if (!strcasecmp (cmd, "strafe_right"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_right (.1, false, false); }
    else Sys->strafe (1*f,0);
  }
  else if (!strcasecmp (cmd, "step_forward"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_forward (.1, false, false); }
    else Sys->step (1*f,0);
  }
  else if (!strcasecmp (cmd, "step_backward"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_backward (.1, false, false); }
    else Sys->step (-1*f,0);
  }
  else if (!strcasecmp (cmd, "rotate_left"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_left_camera (.1, false, false); }
    else Sys->rotate (-1*f,0);
  }
  else if (!strcasecmp (cmd, "rotate_right"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_right_camera (.1, false, false); }
    else Sys->rotate (1*f,0);
  }
  else if (!strcasecmp (cmd, "look_up"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_right_xaxis (.1, false, false); }
    else Sys->look (-1*f,0);
  }
  else if (!strcasecmp (cmd, "look_down"))
  {
    float f = safe_atof (arg);
    if (Sys->move_3d || Sys->map_mode) { if (f) Sys->imm_rot_left_xaxis (.1, false, false); }
    else Sys->look (1*f,0);
  }
  else if (!strcasecmp (cmd, "jump"))
  {
    if (Sys->do_gravity && Sys->on_ground)
      Sys->velocity.y = Sys->cfg_jumpspeed;
  }
  else if (!strcasecmp (cmd, "i_forward"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_forward (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_backward"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_backward (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_left"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_left (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_right"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_right (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_up"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_up (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_down"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_down (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftc"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_camera (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftw"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_world (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightc"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_camera (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightw"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_world (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftx"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_xaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotleftz"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_left_zaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightx"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_xaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "i_rotrightz"))
  {
    int slow = 0, fast = 0; if (arg) csScanStr (arg, "%d,%d", &slow, &fast);
    Sys->imm_rot_right_zaxis (.1, (bool)slow, (bool)fast);
  }
  else if (!strcasecmp (cmd, "fire"))
  {
    RECORD_CMD (cmd);
    extern void fire_missile ();
    fire_missile ();
  }
  else if (!strcasecmp (cmd, "rain"))
  {
    char txtname[100];
    int cnt = 0;
    /* speed and num must be preset to prevent compiler warnings
     * on some systems. */
    int num = 0;
    float speed = 0;
    if (arg) cnt = csScanStr (arg, "%s,%d,%f", txtname, &num, &speed);
    extern void add_particles_rain (iSector* sector, char* txtname,
    	int num, float speed);
    if (cnt <= 2) speed = 2;
    if (cnt <= 1) num = 500;
    if (cnt <= 0) strcpy (txtname, "raindrop");
    add_particles_rain (Sys->view->GetCamera ()->GetSector (),
    	txtname, num, speed);
  }
  else if (!strcasecmp (cmd, "snow"))
  {
    char txtname[100];
    int cnt = 0;
    /* speed and num must be preset to prevent compiler warnings
     * on some systems. */
    int num = 0;
    float speed = 0;
    if (arg) cnt = csScanStr (arg, "%s,%d,%f", txtname, &num, &speed);
    extern void add_particles_snow (iSector* sector, char* txtname,
    	int num, float speed);
    if (cnt <= 2) speed = .3;
    if (cnt <= 1) num = 500;
    if (cnt <= 0) strcpy (txtname, "snow");
    add_particles_snow (Sys->view->GetCamera ()->GetSector (),
    	txtname, num, speed);
  }
  else if (!strcasecmp (cmd, "flame"))
  {
    RECORD_ARGS (cmd, arg);
    char txtname[100];
    int cnt = 0;
    int num = 0;
    if (arg) cnt = csScanStr (arg, "%s,%d", txtname, &num);
    extern void add_particles_fire (iSector* sector, char* txtname,
    	int num, const csVector3& origin);
    if (cnt <= 1) num = 200;
    if (cnt <= 0) strcpy (txtname, "raindrop");
    add_particles_fire (Sys->view->GetCamera ()->GetSector (),
    	txtname, num, Sys->view->GetCamera ()->GetTransform ().GetOrigin ()-
	csVector3 (0, Sys->cfg_body_height, 0));
  }
  else if (!strcasecmp (cmd, "fountain"))
  {
    RECORD_ARGS (cmd, arg);
    char txtname[100];
    int cnt = 0;
    int num = 0;
    if (arg) cnt = csScanStr (arg, "%s,%d", txtname, &num);
    extern void add_particles_fountain (iSector* sector, char* txtname,
    	int num, const csVector3& origin);
    if (cnt <= 1) num = 400;
    if (cnt <= 0) strcpy (txtname, "spark");
    add_particles_fountain (Sys->view->GetCamera ()->GetSector (),
    	txtname, num, Sys->view->GetCamera ()->GetTransform ().GetOrigin ()-
	csVector3 (0, Sys->cfg_body_height, 0));
  }
  else if (!strcasecmp (cmd, "explosion"))
  {
    char txtname[100];
    int cnt = 0;
    if (arg) cnt = csScanStr (arg, "%s", txtname);
    extern void add_particles_explosion (iSector* sector, const csVector3& center,
    	char* txtname);
    if (cnt != 1)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameter 'texture'!\n");
    }
    else
      add_particles_explosion (Sys->view->GetCamera ()->GetSector (),
    	Sys->view->GetCamera ()->GetTransform ().GetOrigin (), txtname);
  }
  else if (!strcasecmp (cmd, "spiral"))
  {
    char txtname[100];
    int cnt = 0;
    if (arg) cnt = csScanStr (arg, "%s", txtname);
    extern void add_particles_spiral (iSector* sector, const csVector3& bottom,
    	char* txtname);
    if (cnt != 1)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameter 'texture'!\n");
    }
    else
      add_particles_spiral (Sys->view->GetCamera ()->GetSector (),
    	Sys->view->GetCamera ()->GetTransform ().GetOrigin (), txtname);
  }
  else if (!strcasecmp (cmd, "loadmesh"))
  {
    char filename[100], tempname[100], txtname[100];
    int cnt = 0;
    if (arg) cnt = csScanStr (arg, "%s,%s,%s", filename, tempname, txtname);
    if (cnt != 3)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameters 'file','template','texture'!\n");
    }
    else load_meshobj (filename, tempname, txtname);
  }
  else if (!strcasecmp (cmd, "addmesh"))
  {
    char tname[100];
    char sname[100];
    float size = 9;
    int cnt = 0;
    if (arg) cnt = csScanStr (arg, "%s,%s,%f", tname, sname, &size);
    if(cnt != 3)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameters 'templatename',");
      Sys->Printf (CS_MSG_CONSOLE, "'meshname','size'!\n");
    }
    else
    {
      add_meshobj (tname, sname, Sys->view->GetCamera ()->GetSector (),
    	          Sys->view->GetCamera ()->GetTransform ().GetOrigin (), size);
    }
  }
  else if (!strcasecmp (cmd, "delmesh"))
  {
    char name[100];
    if (arg)
    {
      csScanStr (arg, "%s", name);
      csObject* obj = Sys->view->GetEngine ()->GetCsEngine ()->meshes.FindByName (name);
      if (obj)
        Sys->view->GetEngine ()->GetCsEngine ()->RemoveMesh ((csMeshWrapper*)obj);
      else
        CsPrintf (CS_MSG_CONSOLE, "Can't find mesh with that name!\n");
    }
    else
      CsPrintf (CS_MSG_CONSOLE, "Missing mesh name!\n");
  }
  else if (!strcasecmp (cmd, "listmeshes"))
  {
    list_meshes ();
  }
  else if(!strcasecmp(cmd, "listactions"))
  {
    char name[100];
    char action[100];
    int cnt = 0;
    if (arg) cnt = csScanStr (arg, "%s,%s", name, action);
    if(cnt != 1)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameters 'meshname'!\n");
      Sys->Printf (CS_MSG_CONSOLE, "To get the names use 'listmeshes'\n");
    }
    else
    {
      // See if the mesh exists.
      iMeshWrapper *wrap = Sys->Engine->FindMeshObject (name);
      if (wrap)
      {
	iSprite3DFactoryState* fstate = SCF_QUERY_INTERFACE (
		wrap->GetMeshObject ()->GetFactory (),
		iSprite3DFactoryState);
	iSpriteAction* aspr_act;
	int i;

	for (i = 0; i < (fstate->GetActionCount ()); i ++)
	{
	  aspr_act = fstate->GetAction (i);
	  Sys->Printf (CS_MSG_CONSOLE, "%s\n", aspr_act->GetName ());
	}
	fstate->DecRef ();
      }
      else
      {
        Sys->Printf (CS_MSG_CONSOLE, "Expected parameters 'meshname'!\n");
        Sys->Printf (CS_MSG_CONSOLE, "To get the names use 'listmeshes'\n");
      }
    }
  }
  else if (!strcasecmp (cmd, "setaction"))
  {
    char name[100];
    char action[100];
    int cnt = 0;
    if (arg) cnt = csScanStr (arg, "%s,%s", name, action);
    if(cnt != 2)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameters 'meshname,action'!\n");
      Sys->Printf (CS_MSG_CONSOLE, "To get the names use 'listmeshes'\n");
    }
    else
    {
      // Test to see if the mesh exists.
      iMeshWrapper* wrap = Sys->Engine->FindMeshObject (name);
      if (!wrap)
        Sys->Printf (CS_MSG_CONSOLE, "No such mesh!\n");
      else
      {
        iSprite3DState* state = SCF_QUERY_INTERFACE (wrap->GetMeshObject (),
		iSprite3DState);
        if (state)
        {
          // Test to see if the action exists for that sprite.
          if (!state->SetAction (action))
	  {
            Sys->Printf (CS_MSG_CONSOLE,
		         "Expected parameters 'meshname,action'!\n");
	    Sys->Printf (CS_MSG_CONSOLE,
		   "That mesh does not have that action.\n");
	  }
	  state->DecRef ();
        }
        else
          Sys->Printf (CS_MSG_CONSOLE, "Mesh is not a 3D sprite!");
      }
    }
  }
  else if (!strcasecmp (cmd, "setmotion"))
  {
    char name[100];
    char motion[100];
    int cnt = 0;
    if (arg) cnt = csScanStr (arg, "%s,%s", name, motion);
    if(cnt != 2)
    {
      Sys->Printf (CS_MSG_CONSOLE, "Expected parameters 'meshname,motion'!\n");
      Sys->Printf (CS_MSG_CONSOLE, "To get the names use 'listmeshes'\n");
    }
    else
    {
      // Test to see if the mesh exists.
      iMeshWrapper* wrap = Sys->Engine->FindMeshObject (name);
      if (!wrap)
        Sys->Printf (CS_MSG_CONSOLE, "No such mesh!\n");
      else
      {
	iSprite3DState* state = SCF_QUERY_INTERFACE (wrap->GetMeshObject (),
		iSprite3DState);
        if (state)
        {
	  iSkeletonBone *sb=SCF_QUERY_INTERFACE(state->GetSkeletonState(),
	  	iSkeletonBone);
	  if (sb)
	  {
	    if (Sys->myMotionMan)
	    {
	      if (!Sys->myMotionMan->ApplyMotion(sb, motion, motion, true,
	      	false, 1.0, 0, false))
	        Sys->Printf (CS_MSG_CONSOLE, "That motion does not exist!\n");
	    }
	    else
	      Sys->Printf (CS_MSG_CONSOLE, "No motion manager exists to animate the skeleton!\n");
	    sb->DecRef ();
	  }
	  else
	    Sys->Printf (CS_MSG_CONSOLE, "That sprite does not contain a skeleton!\n");
	  state->DecRef ();
        }
	else
	{
          Sys->Printf (CS_MSG_CONSOLE, "Mesh is not a 3D sprite!\n");
	}
      }
    }
  }
  else if (!strcasecmp (cmd, "addskel"))
  {
    RECORD_ARGS (cmd, arg);
    int depth = 0, width = 0;
    if (arg) csScanStr (arg, "%d,%d", &depth, &width);
    if (depth < 1) depth = 3; 
    if (width < 1) width = 3;
    extern void add_skeleton_tree (iSector* where, csVector3 const& pos,
    	int depth, int width);
    add_skeleton_tree (Sys->view->GetCamera ()->GetSector (),
    	Sys->view->GetCamera ()->GetTransform ().GetOrigin (), depth, width);
  }
  else if (!strcasecmp (cmd, "addghost"))
  {
    RECORD_ARGS (cmd, arg);
    int depth, width;
    if (arg) csScanStr (arg, "%d,%d", &depth, &width);
    else { depth = 5; width = 8; }
    extern void add_skeleton_ghost (iSector* where, csVector3 const& pos,
    	int maxdepth, int width);
    add_skeleton_ghost (Sys->view->GetCamera ()->GetSector (),
    	Sys->view->GetCamera ()->GetTransform ().GetOrigin (), depth, width);
  }
  else if (!strcasecmp (cmd, "addbot"))
  {
    RECORD_ARGS (cmd, arg);
    float radius = 0;
    if (arg) csScanStr (arg, "%f", &radius);
    extern void add_bot (float size, iSector* where, csVector3 const& pos,
	float dyn_radius);
    add_bot (2, Sys->view->GetCamera ()->GetSector (),
    	Sys->view->GetCamera ()->GetTransform ().GetOrigin (), radius);
  }
  else if (!strcasecmp (cmd, "delbot"))
  {
    extern void del_bot ();
    del_bot ();
  }
  else if (!strcasecmp (cmd, "clrlights"))
  {
    RECORD_CMD (cmd);
    csLightIt* lit = Sys->view->GetEngine ()->GetCsEngine ()
    	->NewLightIterator ();
    csLight* l;
    while ((l = lit->Fetch ()) != NULL)
    {
      l->SetColor (csColor (0, 0, 0));
    }
  }
  else if (!strcasecmp (cmd, "setlight"))
  {
    if (Sys->selected_light)
    {
      float r, g, b;
      if (arg && csScanStr (arg, "%f,%f,%f", &r, &g, &b) == 3)
        Sys->selected_light->SetColor (csColor (r, g, b));
      else
        CsPrintf (CS_MSG_CONSOLE, "Arguments missing or invalid!\n");
    }
    else
      CsPrintf (CS_MSG_CONSOLE, "No light selected!\n");
  }
  else if (!strcasecmp (cmd, "addlight"))
  {
    RECORD_ARGS (cmd, arg);
    csVector3 dir (0,0,0);
    csVector3 pos = Sys->view->GetCamera ()->GetTransform ().This2Other (dir);
    csDynLight* dyn;

    bool rnd;
    float r, g, b, radius, thing_shadows;
    if (arg && csScanStr (arg, "%f,%f,%f,%f,%d", &r, &g, &b, &radius,
    	&thing_shadows) == 5)
    {
      dyn = new csDynLight (pos.x, pos.y, pos.z, radius, r, g, b);
      if (thing_shadows) dyn->flags.Set (CS_LIGHT_THINGSHADOWS,
      	CS_LIGHT_THINGSHADOWS);
      rnd = false;
    }
    else
    {
      dyn = new csDynLight (pos.x, pos.y, pos.z, 6, 1, 1, 1);
      rnd = true;
    }
    Sys->view->GetEngine ()->GetCsEngine ()->AddDynLight (dyn);
    dyn->SetSector (Sys->view->GetCamera ()->GetSector ()->GetPrivateObject ());
    dyn->Setup ();
    extern void AttachRandomLight (csDynLight* light);
    if (rnd)
      AttachRandomLight (dyn);
    Sys->Printf (CS_MSG_CONSOLE, "Dynamic light added.\n");
  }
  else if (!strcasecmp (cmd, "dellight"))
  {
    csDynLight* dyn;
    if ((dyn = Sys->view->GetEngine ()->GetCsEngine ()->GetFirstDynLight ()) != NULL)
    {
      Sys->view->GetEngine ()->GetCsEngine ()->RemoveDynLight (dyn);
      delete dyn;
      Sys->Printf (CS_MSG_CONSOLE, "Dynamic light deleted.\n");
    }
  }
  else if (!strcasecmp (cmd, "dellights"))
  {
    RECORD_CMD (cmd);
    csDynLight* dyn;
    while ((dyn = Sys->view->GetEngine ()->GetCsEngine ()->GetFirstDynLight ()) != NULL)
    {
      Sys->view->GetEngine ()->GetCsEngine ()->RemoveDynLight (dyn);
      delete dyn;
    }
    Sys->Printf (CS_MSG_CONSOLE, "All dynamic lights deleted.\n");
  }
  else if (!strcasecmp (cmd, "picklight"))
  {
#   if 0
    pickup_light = Sys->view->GetEngine ()->GetFirstFltLight ();
    if (pickup_light) Sys->Printf (CS_MSG_CONSOLE, "Floating light taken.\n");
    else Sys->Printf (CS_MSG_CONSOLE, "No floating light to take.\n");
#   endif
  }
  else if (!strcasecmp (cmd, "droplight"))
  {
#   if 0
    if (pickup_light) Sys->Printf (CS_MSG_CONSOLE, "Floating light dropped.\n");
    else Sys->Printf (CS_MSG_CONSOLE, "No floating light to drop.\n");
    pickup_light = NULL;
#   endif
  }
  else if (!strcasecmp (cmd, "map"))
  {
    const char* const choices[5] = { "off", "overlay", "on", "txt", NULL };
    csCommandProcessor::change_choice (arg, &Sys->map_mode, "map", choices, 4);
  }
  else if (!strcasecmp (cmd, "mapproj"))
  {
    const char* const choices[5] = { "persp", "x", "y", "z", NULL };
    Sys->map_projection++;
    csCommandProcessor::change_choice (arg, &Sys->map_projection, "map projection", choices, 4);
    Sys->map_projection--;
  }
  else if (!strcasecmp (cmd, "snd_play"))
  {
    if (Sys->mySound)
    {
      iSoundWrapper *sb = GET_NAMED_CHILD_OBJECT_FAST (Sys->view->GetEngine ()->
        QueryObject (), iSoundWrapper, arg);
      if (sb)
        sb->GetSound ()->Play();
      else
        Sys->Printf (CS_MSG_CONSOLE, "Sound '%s' not found!\n", arg);
    }
  }
  else if (!strcasecmp (cmd, "snd_volume"))
  {
    if (Sys->mySound)
    {
      float vol = Sys->mySound->GetVolume ();
      csCommandProcessor::change_float (arg, &vol, "snd_volume", 0.0, 1.0);
      Sys->mySound->SetVolume (vol);
    }
  }
  else if (!strcasecmp (cmd, "fullscreen"))
    Sys->myG2D->PerformExtension("fullscreen");
  else
    return false;

  return true;
}

#undef RECORD_CMD
#undef RECORD_ARGS
