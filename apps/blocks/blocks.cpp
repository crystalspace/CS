/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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

/*
  TODO:
    Make nice startup screen with moving blocks as demo.
    Better textures.
    Mark game-over height so that you can see it.
    Improve 'Game Over' screen!
    Cleanup of several 'Screens' in Blocks (code cleanup).
    Psuedo-AI module to play automatically.
    Add confirmation before quitting game.
    Fix bugs with marker on floor (wrong size...).
 */

#include "cssysdef.h"
#include "blocks.h"
#include "cssys/system.h"
#include "csparser/csloader.h"
#include "csutil/csstring.h"
#include "csgfxldr/csimage.h"
#include "csengine/dumper.h"
#include "csengine/engine.h"
#include "csengine/texture.h"
#include "csengine/material.h"
#include "csengine/sector.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "csengine/thing.h"
#include "csengine/textrans.h"
#include "csengine/csview.h"
#include "ivfs.h"
#include "igraph3d.h"
#include "itxtmgr.h"
#include "isndrdr.h"
#include "csparser/snddatao.h"
#include "isnddata.h"
#include "ifontsrv.h"
#include "icfgmgr.h"

#if defined(BLOCKS_NETWORKING)
#include "inetdrv.h"
#endif

//-----------------------------------------------------------------------------
// Networking stuff.

#if defined(BLOCKS_NETWORKING)
static bool do_network = false;
// Maybe move these into Blocks class.
static iNetworkListener* Listener = NULL;
static iNetworkConnection* Connection = NULL;
// static long LastAcceptTime = 0;
static long LastConnectTime = 0;
#endif

// End networking stuff.
//-----------------------------------------------------------------------------

Blocks* Sys = NULL;

csView* view = NULL;

#define Gfx3D System->G3D
#define Gfx2D System->G2D

//------------------------------------------------- We need the 3D engine -----

REGISTER_STATIC_LIBRARY (engine)

int Blocks::white, Blocks::black, Blocks::red;

//-----------------------------------------------------------------------------

HighScore::HighScore ()
{
  int i;
  for (i = 0 ; i < 10 ; i++)
  {
    scores[i] = -1;
    names[i] = NULL;
  }
}

HighScore::~HighScore ()
{
  int i;
  for (i = 0 ; i < 10 ; i++)
    delete[] names[i];
}

bool HighScore::RegisterScore (const char* name, int score)
{
  int i, j;
  for (i = 0 ; i < 10 ; i++)
  {
    if (score > scores[i])
    {
      delete names[9];
      for (j = 9 ; j > i ; j--)
      {
        scores[j] = scores[j-1];
	names[j] = names[j-1];
      }
      scores[i] = score;
      names[i] = NULL;
      SetName (i, name);
      return true;
    }
  }
  return false;
}

bool HighScore::CheckScore (int score)
{
  int i;
  for (i = 0 ; i < 10 ; i++)
  {
    if (score > scores[i]) return true;
  }
  return false;
}

void HighScore::SetName (int i, const char* name)
{
  delete names[i];
  names[i] = NULL;
  if (!name) return;
  names[i] = new char [strlen (name)+1];
  strcpy (names[i], name);
}

//-----------------------------------------------------------------------------

TextEntryMenu::TextEntryMenu ()
{
  entries = last = NULL;
  num_entries = 0;
  selected = 0;
  hisel = false;
  time_left = 0;
}

TextEntryMenu::~TextEntryMenu ()
{
  Clear ();
}

void TextEntryMenu::Clear ()
{
  while (entries)
  {
    TextEntry* n = entries->next;
    delete entries->txt;
    delete entries->entry;
    delete entries;
    entries = n;
  }
  last = NULL;
  num_entries = 0;
  hisel = false;
  selected = 0;
  time_left = 0;
}

void TextEntryMenu::Add (const char* txt, const char* entry, void* userdata)
{
  TextEntry* n = new TextEntry;
  n->txt = new char [strlen (txt)+1];
  strcpy (n->txt, txt);
  n->entry = new char [strlen (entry)+1];
  strcpy (n->entry, entry);
  n->userdata = userdata;
  n->next = NULL;
  if (last)
  {
    last->next = n;
    last = n;
  }
  else
  {
    entries = last = n;
  }
  num_entries++;
}

void TextEntryMenu::ReplaceSelected (const char* txt, const char* entry,
  void* userdata)
{
  TextEntry* n = GetEntry (selected);
  // Make sure to delete old pointers to text AFTER allocating new ones.
  char* old_txt = n->txt;
  char* old_entry = n->entry;
  n->txt = new char [strlen (txt)+1];
  strcpy (n->txt, txt);
  n->entry = new char [strlen (entry)+1];
  strcpy (n->entry, entry);
  n->userdata = userdata;
  delete old_txt;
  delete old_entry;
}

void TextEntryMenu::Draw (cs_time elapsed_time)
{
  float elapsed = (float)elapsed_time/1000.;
  if (elapsed > time_left) elapsed = time_left;
  time_left -= elapsed;
  if (time_left <= 0) time_left = 1;

  int tot_lines = (Sys->FrameHeight-20)/12;

  Gfx2D->Clear (0);
  int i;
  TextEntry* entry = entries;
  for (i = selected-tot_lines/2 ; i < selected+tot_lines/2 ; i++)
  {
    if (i < 0 || i >= num_entries) continue;
    entry = GetEntry (i);

    int selcol = selected == i ? Blocks::red : Blocks::black;
    int y = 10+(i-selected+tot_lines/2)*12;
    Gfx2D->DrawBox (10, y-2, 300, 12, selcol);
    Gfx2D->Write (Sys->font, 10, y, Blocks::white, selcol, entry->txt);
    if (hisel && selected == i)
    {
      int col;
      if (time_left < .5) col = Blocks::white;
      else col = Blocks::red;
      Gfx2D->DrawBox (190, y-2, 120, 12, col);
      Gfx2D->Write (Sys->font, 200, y, Blocks::black, col, entry->entry);
    }
    else
    {
      Gfx2D->Write (Sys->font, 200, y, Blocks::white, selcol, entry->entry);
    }
  }
}

TextEntryMenu::TextEntry* TextEntryMenu::GetEntry (int num)
{
  int i;
  TextEntry* entry = entries;
  for (i = 0 ; i < num ; i++) entry = entry->next;
  return entry;
}

void* TextEntryMenu::GetSelectedData ()
{
  return GetEntry (selected)->userdata;
}

char* TextEntryMenu::GetSelectedText ()
{
  return GetEntry (selected)->txt;
}

char* TextEntryMenu::GetSelectedEntry ()
{
  return GetEntry (selected)->entry;
}

//-----------------------------------------------------------------------------
Blocks::Blocks ()
{
  engine = NULL;

  full_rotate_x = create_rotate_x (M_PI/2);
  full_rotate_y = create_rotate_y (M_PI/2);
  full_rotate_z = create_rotate_z (M_PI/2);
  full_rotate_x_reverse = create_rotate_x (-M_PI/2);
  full_rotate_y_reverse = create_rotate_y (-M_PI/2);
  full_rotate_z_reverse = create_rotate_z (-M_PI/2);

  pause = false;

  destinations[0][0] = csVector3 (0, 3, -7);
  destinations[1][0] = csVector3 (7, 3, 0);
  destinations[2][0] = csVector3 (0, 3, 7);
  destinations[3][0] = csVector3 (-7, 3, 0);
  destinations[0][1] = csVector3 (0, 7, -5);
  destinations[1][1] = csVector3 (5, 7, 0);
  destinations[2][1] = csVector3 (0, 7, 5);
  destinations[3][1] = csVector3 (-5, 7, 0);
  destinations[0][2] = csVector3 (0, 8, -.4);
  destinations[1][2] = csVector3 (.4, 8, 0);
  destinations[2][2] = csVector3 (0, 8, .4);
  destinations[3][2] = csVector3 (-.4, 8, 0);
  dest_move_right_dx[0] = 1; dest_move_right_dy[0] = 0;
  dest_move_right_dx[1] = 0; dest_move_right_dy[1] = 1;
  dest_move_right_dx[2] = -1; dest_move_right_dy[2] = 0;
  dest_move_right_dx[3] = 0; dest_move_right_dy[3] = -1;
  dest_move_down_dx[0] = 0; dest_move_down_dy[0] = -1;
  dest_move_down_dx[1] = 1; dest_move_down_dy[1] = 0;
  dest_move_down_dx[2] = 0; dest_move_down_dy[2] = 1;
  dest_move_down_dx[3] = -1; dest_move_down_dy[3] = 0;
  cur_hor_dest = 0;
  cur_ver_dest = 2;
  move_right_dx = dest_move_right_dx[cur_hor_dest];
  move_right_dy = dest_move_right_dy[cur_hor_dest];
  move_down_dx = dest_move_down_dx[cur_hor_dest];
  move_down_dy = dest_move_down_dy[cur_hor_dest];

  view_origin = csVector3 (0, 3, 0);

  initscreen = true;
  screen = SCREEN_STARTUP;
  dynlight = NULL;

#if defined(BLOCKS_NETWORKING)
  // Network stuff.
  since_last_check = 0;
#endif

  keyconf_menu = NULL;

  // State changes.
  player1 = new States();
  
#if defined(BLOCKS_NETWORKING)
  player1_net = new NetworkStates();
#endif

  //menu
  cur_menu = 0;
  old_cur_menu = 0;
  menu_todo = 0.0;
  menu_hor_todo = 0.0;
  menu_hor_old_x_src = 0.0;
  menu_hor_old_x_dst = 0.0;
  menu_hor_new_x_src = 0.0;
  menu_hor_new_x_dst = 0.0;

  menu_hor_old_menu = NULL;
}

Blocks::~Blocks ()
{
  delete dynlight;
  if (engine) engine->Clear ();
  delete keyconf_menu;
#if defined(BLOCKS_NETWORKING)
  TerminateConnection();
#endif
}

void Blocks::InitGame ()
{
  player1->InitStates();

  initscreen = false;

  rot_px_todo = 0;
  rot_py_todo = 0;
  rot_pz_todo = 0;
  rot_mx_todo = 0;
  rot_my_todo = 0;
  rot_mz_todo = 0;
  queue_rot_todo = ROT_NONE;
  queue_move_dx_todo = 0;
  queue_move_dy_todo = 0;
  move_hor_todo = 0;
  cam_move_dist = 0;

  cur_hor_dest = 0;
  cur_ver_dest = 2;
  move_right_dx = dest_move_right_dx[cur_hor_dest];
  move_right_dy = dest_move_right_dy[cur_hor_dest];
  move_down_dx = dest_move_down_dx[cur_hor_dest];
  move_down_dy = dest_move_down_dy[cur_hor_dest];
  cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];

  pause = false;
}

void reset_vertex_colors (csThing* th)
{
  int i;
  for (i = 0 ; i < th->GetNumPolygons () ; i++)
  {
    csPolygon3D* p = th->GetPolygon3D (i);
    p->UpdateVertexLighting (NULL, csColor (0, 0, 0), true, true);
    p->UpdateVertexLighting (NULL, csColor (0, 0, 0), false, true);
  }
}

csMatrix3 Blocks::create_rotate_x (float angle)
{
  csMatrix3 rotate_x;
  rotate_x.m11 = 1; rotate_x.m12 = 0;           rotate_x.m13 = 0;
  rotate_x.m21 = 0; rotate_x.m22 = cos (angle); rotate_x.m23 = -sin (angle);
  rotate_x.m31 = 0; rotate_x.m32 = sin (angle); rotate_x.m33 =  cos (angle);
  return rotate_x;
}

csMatrix3 Blocks::create_rotate_y (float angle)
{
  csMatrix3 rotate_y;
  rotate_y.m11 = cos (angle); rotate_y.m12 = 0; rotate_y.m13 = -sin (angle);
  rotate_y.m21 = 0;           rotate_y.m22 = 1; rotate_y.m23 = 0;
  rotate_y.m31 = sin (angle); rotate_y.m32 = 0; rotate_y.m33 =  cos (angle);
  return rotate_y;
}

csMatrix3 Blocks::create_rotate_z (float angle)
{
  csMatrix3 rotate_z;
  rotate_z.m11 = cos (angle); rotate_z.m12 = -sin (angle); rotate_z.m13 = 0;
  rotate_z.m21 = sin (angle); rotate_z.m22 =  cos (angle); rotate_z.m23 = 0;
  rotate_z.m31 = 0;           rotate_z.m32 = 0;            rotate_z.m33 = 1;
  return rotate_z;
}

csPolygon3D* add_polygon_template (csThing* tmpl,
	char* name, csMaterialWrapper* material,
	int vt0, int vt1, int vt2, int vt3 = -1)
{
  csPolygon3D* p;
  p = tmpl->NewPolygon (material);
  p->SetName (name);
  p->AddVertex (vt0);
  p->AddVertex (vt1);
  p->AddVertex (vt2);
  if (vt3 != -1) p->AddVertex (vt3);
  p->ComputeNormal ();
  return p;
}

void Blocks::add_pillar_template ()
{
  float dim = CUBE_DIM/2.;
  pillar_tmpl = new csThing (engine);
  pillar_tmpl->SetName ("pillar");
  pillar_tmpl->AddVertex (-dim, 0, dim);
  pillar_tmpl->AddVertex (dim, 0, dim);
  pillar_tmpl->AddVertex (dim, 0, -dim);
  pillar_tmpl->AddVertex (-dim, 0, -dim);
  pillar_tmpl->AddVertex (-dim, ZONE_HEIGHT*CUBE_DIM, dim);
  pillar_tmpl->AddVertex (dim, ZONE_HEIGHT*CUBE_DIM, dim);
  pillar_tmpl->AddVertex (dim, ZONE_HEIGHT*CUBE_DIM, -dim);
  pillar_tmpl->AddVertex (-dim, ZONE_HEIGHT*CUBE_DIM, -dim);

  csPolygon3D* p;
  csVector3 norm;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = add_polygon_template (pillar_tmpl, "d", pillar_mat, 3, 2, 1, 0);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pillar_tmpl->Vobj (0), pillar_tmpl->Vobj (1), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (pillar_tmpl, "b", pillar_mat, 0, 1, 5, 4);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pillar_tmpl->Vobj (0), pillar_tmpl->Vobj (1), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (pillar_tmpl, "t", pillar_mat, 4, 5, 6, 7);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pillar_tmpl->Vobj (4), pillar_tmpl->Vobj (5), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (pillar_tmpl, "f", pillar_mat, 7, 6, 2, 3);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pillar_tmpl->Vobj (7), pillar_tmpl->Vobj (6), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (pillar_tmpl, "l", pillar_mat, 4, 7, 3, 0);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pillar_tmpl->Vobj (7), pillar_tmpl->Vobj (3), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (pillar_tmpl, "r", pillar_mat, 6, 5, 1, 2);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pillar_tmpl->Vobj (6), pillar_tmpl->Vobj (5), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

  engine->thing_templates.Push (pillar_tmpl);
}

void Blocks::add_vrast_template ()
{
  float dim = RAST_DIM;
  vrast_tmpl = new csThing (engine);
  vrast_tmpl->SetName ("vrast");
  vrast_tmpl->AddVertex (-dim, 0, dim);
  vrast_tmpl->AddVertex (dim, 0, dim);
  vrast_tmpl->AddVertex (-dim, ZONE_HEIGHT*CUBE_DIM, dim);
  vrast_tmpl->AddVertex (dim, ZONE_HEIGHT*CUBE_DIM, dim);

  csPolygon3D* p;
  csVector3 norm;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = add_polygon_template (vrast_tmpl, "f", raster_mat, 0, 1, 3, 2);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	vrast_tmpl->Vobj (0), vrast_tmpl->Vobj (1), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

#if 0
  p = add_polygon_template (vrast_tmpl, "b", raster_mat, 2, 3, 1, 0);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	vrast_tmpl->Vobj (0), vrast_tmpl->Vobj (1), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);
#endif

  engine->thing_templates.Push (vrast_tmpl);
}

void Blocks::add_hrast_template ()
{
  float dim = RAST_DIM;
  hrast_tmpl = new csThing (engine);
  hrast_tmpl->SetName ("hrast");

  // zone_dim s were BIG here changed.

  //hrast_tmpl->AddVertex ((-(float)(player1->zone_dim)/2.)*CUBE_DIM,.02,-dim);
  //hrast_tmpl->AddVertex ((-(float)(player1->zone_dim)/2.)*CUBE_DIM,.02,dim);
  //hrast_tmpl->AddVertex (((float)(player1->zone_dim)/2.)*CUBE_DIM,.02,-dim);
  //hrast_tmpl->AddVertex (((float)(player1->zone_dim)/2.)*CUBE_DIM,.02,dim);

  hrast_tmpl->AddVertex ((-(float)ZONE_DIM/2.)*CUBE_DIM, .02, -dim);
  hrast_tmpl->AddVertex ((-(float)ZONE_DIM/2.)*CUBE_DIM, .02, dim);
  hrast_tmpl->AddVertex (((float)ZONE_DIM/2.)*CUBE_DIM, .02, -dim);
  hrast_tmpl->AddVertex (((float)ZONE_DIM/2.)*CUBE_DIM, .02, dim);

  csPolygon3D* p;
  csVector3 norm;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = add_polygon_template (hrast_tmpl, "f", raster_mat, 0, 1, 3, 2);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	hrast_tmpl->Vobj (0), hrast_tmpl->Vobj (1), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);

#if 0
  p = add_polygon_template (hrast_tmpl, "b", raster_mat, 2, 3, 1, 0);
  norm = p->GetPolyPlane ()->GetNormal ();
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	hrast_tmpl->Vobj (0), hrast_tmpl->Vobj (1), 1, norm.x, norm.y, norm.z);
  p->SetTextureSpace (tx_matrix, tx_vector);
#endif

  engine->thing_templates.Push (hrast_tmpl);
}


void Blocks::add_pillar (int x, int y)
{
  csThing* pillar;
  pillar = new csThing (engine);
  engine->things.Push (pillar);
  pillar->SetName ("pillar");
  pillar->GetMovable ().SetSector (room);
  pillar->MergeTemplate (pillar_tmpl, room, pillar_mat, 1);
  csVector3 v ( (x-(player1->zone_dim)/2)*CUBE_DIM, 0,
	       (y-(player1->zone_dim)/2)*CUBE_DIM);
  pillar->HardTransform (csTransform (csMatrix3 (), v));
  pillar->GetMovable ().SetSector (room);
  pillar->GetMovable ().UpdateMove ();
}

void Blocks::add_vrast (int x, int y, float dx, float dy, float rot_z)
{
  csThing* vrast;
  vrast = new csThing (engine);
  engine->things.Push (vrast);
  vrast->SetName ("vrast");
  vrast->GetMovable ().SetSector (room);
  vrast->MergeTemplate (vrast_tmpl, room, raster_mat, 1);
  vrast->GetMovable ().SetSector (room);
  csVector3 v ((x-(player1->zone_dim)/2)*CUBE_DIM+dx, 0,
	       (y-(player1->zone_dim)/2)*CUBE_DIM+dy);
  csMatrix3 rot = create_rotate_y (rot_z);
  vrast->HardTransform (csTransform (rot, v));
  vrast->GetMovable ().UpdateMove ();
}

void Blocks::add_hrast (int x, int y, float dx, float dy, float rot_z)
{
  csThing* hrast;
  hrast = new csThing (engine);
  engine->things.Push (hrast);
  hrast->SetName ("hrast");
  hrast->GetMovable ().SetSector (room);
  hrast->MergeTemplate (hrast_tmpl, room, raster_mat, 1);
  hrast->GetMovable ().SetSector (room);
  csVector3 v ((x-(player1->zone_dim)/2)*CUBE_DIM+dx, 0,
	       (y-(player1->zone_dim)/2)*CUBE_DIM+dy);
  csMatrix3 rot = create_rotate_y (rot_z);
  hrast->HardTransform (csTransform (rot, v));
  hrast->GetMovable ().UpdateMove ();
}

void Blocks::ChangeThingMaterial (csThing* thing, csMaterialWrapper* mat)
{
  for (int i = 0 ; i < thing->GetNumPolygons () ; i++)
  {
    csPolygon3D* p = thing->GetPolygon3D (i);
    p->SetMaterial (mat);
  }
}

void Blocks::add_cube_template ()
{
  float dim = CUBE_DIM/2.;
  cube_tmpl = new csThing (engine);
  cube_tmpl->SetName ("cube");
  cube_tmpl->AddVertex (-dim, -dim, dim);
  cube_tmpl->AddVertex (dim, -dim, dim);
  cube_tmpl->AddVertex (dim, -dim, -dim);
  cube_tmpl->AddVertex (-dim, -dim, -dim);
  cube_tmpl->AddVertex (-dim, dim, dim);
  cube_tmpl->AddVertex (dim, dim, dim);
  cube_tmpl->AddVertex (dim, dim, -dim);
  cube_tmpl->AddVertex (-dim, dim, -dim);

  csPolygon3D* p;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = add_polygon_template (cube_tmpl, "d1", cube_mat, 3, 2, 1);
  p->SetTextureSpace (tx_matrix, tx_vector);
  p = add_polygon_template (cube_tmpl, "d2", cube_mat, 3, 1, 0);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (cube_tmpl, "b1", cube_mat, 0, 1, 5);
  p->SetTextureSpace (tx_matrix, tx_vector);
  p = add_polygon_template (cube_tmpl, "b2", cube_mat, 0, 5, 4);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (cube_tmpl, "t1", cube_mat, 4, 5, 6);
  p->SetTextureSpace (tx_matrix, tx_vector);
  p = add_polygon_template (cube_tmpl, "t2", cube_mat, 4, 6, 7);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (cube_tmpl, "f1", cube_mat, 7, 6, 2);
  p->SetTextureSpace (tx_matrix, tx_vector);
  p = add_polygon_template (cube_tmpl, "f2", cube_mat, 7, 2, 3);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (cube_tmpl, "l1", cube_mat, 4, 7, 3);
  p->SetTextureSpace (tx_matrix, tx_vector);
  p = add_polygon_template (cube_tmpl, "l2", cube_mat, 4, 3, 0);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = add_polygon_template (cube_tmpl, "r1", cube_mat, 6, 5, 1);
  p->SetTextureSpace (tx_matrix, tx_vector);
  p = add_polygon_template (cube_tmpl, "r2", cube_mat, 6, 1, 2);
  p->SetTextureSpace (tx_matrix, tx_vector);

  engine->thing_templates.Push (cube_tmpl);
}

void set_uv (csPolygon3D* p, float u1, float v1, float u2, float v2,
	float u3, float v3)
{
  p->SetTextureType (POLYTXT_GOURAUD);
  csPolyTexGouraud* gs = p->GetGouraudInfo ();
  gs->Setup (p);
  gs->SetUV (0, u1, v1);
  gs->SetUV (1, u2, v2);
  gs->SetUV (2, u3, v3);
}

// dx,dy,dz are logical coordinates (Z vertical).
csThing* Blocks::create_cube_thing (float dx, float dy, float dz,
	csThing* tmpl)
{
  csThing* cube;
  cube = new csThing (engine);
  engine->things.Push (cube);
  cube->SetName ("cubexxx");
  cube->GetMovable ().SetSector (room);
  csVector3 shift (
  	(dx-shift_rotate.x)*CUBE_DIM,
  	(dz-shift_rotate.z)*CUBE_DIM,
	(dy-shift_rotate.y)*CUBE_DIM);
  cube->MergeTemplate (tmpl, room, cube_mat, 1, &shift, NULL);
  cube->SetMovingOption (CS_THING_MOVE_OCCASIONAL); // @@@ should be OFTEN!

  csPolygon3D* p;
  p = cube->GetPolygon3D ("f1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon3D ("f2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon3D ("t1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon3D ("t2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon3D ("b1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon3D ("b2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon3D ("d1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon3D ("d2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon3D ("l1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon3D ("l2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon3D ("r1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon3D ("r2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  return cube;
}

// dx,dy,dz and x,y,z are logical coordinates (Z vertical).
void Blocks::add_cube (float dx, float dy, float dz, float x, float y, float z,
	csThing* tmpl)
{
  csThing* cube = add_cube_thing (room, dx, dy, dz,
  	(x-(player1->zone_dim)/2+shift_rotate.x)*CUBE_DIM,
	(z+shift_rotate.z)*CUBE_DIM+CUBE_DIM/2,
  	(y-(player1->zone_dim)/2+shift_rotate.y)*CUBE_DIM, tmpl);
  cube_info[num_cubes].thing = cube;
  cube_info[num_cubes].dx = dx;
  cube_info[num_cubes].dy = dy;
  cube_info[num_cubes].dz = dz;
  num_cubes++;
}

// dx,dy,dz are logical coordinates (Z vertical).
// x,y,z are physical coordinates (Y vertical).
csThing* Blocks::add_cube_thing (csSector* sect, float dx, float dy, float dz,
	float x, float y, float z, csThing* tmpl)
{
  csThing* cube = create_cube_thing (dx, dy, dz, tmpl);
  cube->GetMovable ().SetSector (sect);
  csVector3 v (x, y, z);
  cube->GetMovable ().SetPosition (sect, v);
  cube->GetMovable ().UpdateMove ();
  cube->InitLightMaps (false);
  room->ShineLights (cube);
  cube->CreateLightMaps (Gfx3D);
  return cube;
}

void Blocks::StartNewShape ()
{
  int x, y, z;
  if ((player1->zone_dim) <= 4) x = y = 1;
  else if ((player1->zone_dim) <= 5) x = y = 2;
  else x = y = 3;
  z = ZONE_HEIGHT-3;
  num_cubes = 0;
  int difficulty = 0;
  switch (diff_level)
  {
    case 0: difficulty = NUM_EASY_SHAPE; break;
    case 1: difficulty = NUM_MEDIUM_SHAPE; break;
    case 2: difficulty = NUM_HARD_SHAPE; break;
  }

again:
  shift_rotate.Set (0, 0, 0);
  BlShapeType type = (BlShapeType)(rand () % difficulty);
  switch (type)
  {
    case SHAPE_R1:
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_R2:
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_R3:
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_R4:
      if ((player1->zone_dim) <= 3) goto again;
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (2, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_L1:
      shift_rotate.Set (.5, .5, .5);
      add_cube (0, 0, 1, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_L2:
      add_cube (-1, 0, 1, x, y, z, cube_tmpl);
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_T1:
      add_cube (0, 0, 1, x, y, z, cube_tmpl);
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_T2:
      add_cube (0, 0, 1, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (-1, 0, -1, x, y, z, cube_tmpl);
      add_cube (0, 0, -1, x, y, z, cube_tmpl);
      add_cube (1, 0, -1, x, y, z, cube_tmpl);
      break;
    case SHAPE_FLAT:
      shift_rotate.Set (.5, .5, .5);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 1, 0, x, y, z, cube_tmpl);
      add_cube (1, 1, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_CUBE:
      shift_rotate.Set (.5, .5, .5);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 1, 0, x, y, z, cube_tmpl);
      add_cube (1, 1, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 1, x, y, z, cube_tmpl);
      add_cube (1, 0, 1, x, y, z, cube_tmpl);
      add_cube (0, 1, 1, x, y, z, cube_tmpl);
      add_cube (1, 1, 1, x, y, z, cube_tmpl);
      break;
    case SHAPE_U:
      add_cube (-1, 0, 1, x, y, z, cube_tmpl);
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 1, x, y, z, cube_tmpl);
      break;
    case SHAPE_S:
      add_cube (-1, 0, 1, x, y, z, cube_tmpl);
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, -1, x, y, z, cube_tmpl);
      break;
    case SHAPE_L3:
      if ((player1->zone_dim) <= 3) goto again;
      add_cube (-1, 0, 1, x, y, z, cube_tmpl);
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (2, 0, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_T1X:
      add_cube (0, 0, 1, x, y, z, cube_tmpl);
      add_cube (-1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 1, 0, x, y, z, cube_tmpl);
      break;
    case SHAPE_FLATX:
      shift_rotate.Set (.5, .5, .5);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 1, 0, x, y, z, cube_tmpl);
      add_cube (1, 1, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 1, x, y, z, cube_tmpl);
      break;
    case SHAPE_FLATXX:
      shift_rotate.Set (.5, .5, .5);
      add_cube (0, 0, 0, x, y, z, cube_tmpl);
      add_cube (1, 0, 0, x, y, z, cube_tmpl);
      add_cube (0, 1, 0, x, y, z, cube_tmpl);
      add_cube (1, 1, 0, x, y, z, cube_tmpl);
      add_cube (0, 0, 1, x, y, z, cube_tmpl);
      add_cube (1, 1, 1, x, y, z, cube_tmpl);
      break;
    default: break;
  }
  player1->move_down_todo = 0;

  player1->cube_x = x;
  player1->cube_y = y;
  player1->cube_z = z;

  player1->speed = player1->cur_speed;
  int i;
  for (i = 0 ; i < num_cubes ; i++)
  {
    csThing* t = cube_info[i].thing;
    reset_vertex_colors (t);
    room->ShineLights (t);
  }
  csThing::current_light_frame_number++;
}

void Blocks::start_demo_shape (BlShapeType type, float x, float y, float z)
{
  shift_rotate.Set (0, 0, 0);
  switch (type)
  {
    case SHAPE_DEMO_B:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  1, x, y, z, cube_tmpl);
      break;
    case SHAPE_DEMO_L:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  -2, x, y, z, cube_tmpl);
      break;
    case SHAPE_DEMO_O:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z, cube_tmpl);
      break;
    case SHAPE_DEMO_C:
      add_cube_thing (demo_room, -1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z, cube_tmpl);
      break;
    case SHAPE_DEMO_K:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z, cube_tmpl);
      break;
    case SHAPE_DEMO_S:
      add_cube_thing (demo_room,  1, 0, -1, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0, -2, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z, cube_tmpl);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z, cube_tmpl);
      break;
    default: break;
  }
}

void Blocks::start_rotation (BlRotType type)
{
  if (rot_px_todo || rot_mx_todo || rot_py_todo ||
      rot_my_todo || rot_pz_todo || rot_mz_todo)
  {
    queue_rot_todo = type;
    return;
  }
  switch (type)
  {
    case ROT_PX:
      if (!check_new_shape_rotation (full_rotate_x_reverse)) return;
      rotate_shape_internal (full_rotate_x_reverse);
      rot_px_todo = M_PI/2;
      break;
    case ROT_MX:
      if (!check_new_shape_rotation (full_rotate_x)) return;
      rotate_shape_internal (full_rotate_x);
      rot_mx_todo = M_PI/2;
      break;
    case ROT_PY:
      if (!check_new_shape_rotation (full_rotate_z)) return;
      rotate_shape_internal (full_rotate_z);
      rot_py_todo = M_PI/2;
      break;
    case ROT_MY:
      if (!check_new_shape_rotation (full_rotate_z_reverse)) return;
      rotate_shape_internal (full_rotate_z_reverse);
      rot_my_todo = M_PI/2;
      break;
    case ROT_PZ:
      if (!check_new_shape_rotation (full_rotate_y)) return;
      rotate_shape_internal (full_rotate_y);
      rot_pz_todo = M_PI/2;
      break;
    case ROT_MZ:
      if (!check_new_shape_rotation (full_rotate_y_reverse)) return;
      rotate_shape_internal (full_rotate_y_reverse);
      rot_mz_todo = M_PI/2;
      break;
    case ROT_NONE: break;
  }
}

void Blocks::start_horizontal_move (int dx, int dy)
{
  if (move_hor_todo)
  {
    queue_move_dx_todo = dx;
    queue_move_dy_todo = dy;
    return;
  }
  if (!check_new_shape_location (dx, dy, 0)) return;
  move_hor_todo = CUBE_DIM;
  move_hor_dx = dx;
  move_hor_dy = dy;
  move_shape_internal (move_hor_dx, move_hor_dy, 0);
}

void Blocks::HandleCameraMovement ()
{
  csVector3 pos = cam_move_dist*cam_move_src + (1-cam_move_dist)*cam_move_dest;
  view->GetCamera ()->SetPosition (pos);
  view->GetCamera ()->LookAt (view_origin-pos, cam_move_up);
}

void Blocks::HandleGameOverKey (int key, bool /*shift*/, bool /*alt*/,
  bool /*ctrl*/)
{
  if (strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ",
      key))
  {
    if (hs_pos >= 20) return;
    hs_name[hs_pos++] = key;
    hs_name[hs_pos] = 0;
  }
  else if (key == CSKEY_ENTER)
  {
    highscores[diff_level][(player1->zone_dim)-3].RegisterScore (
      hs_name, player1->score);
    WriteConfig ();
    enter_highscore = false;
  }
  else if (key == CSKEY_BACKSPACE)
  {
    if (hs_pos <= 0) return;
    hs_pos--;
    hs_name[hs_pos] = 0;
  }
}

void Blocks::HandleGameKey (int key, bool shift, bool alt, bool ctrl)
{
  if (key_pause.Match (key, shift, alt, ctrl)) pause = !pause;
  if (pause) return;

  if (key_viewleft.Match (key, shift, alt, ctrl))
  {
    if (cam_move_dist) return;
    cam_move_dist = 1;
    cam_move_src = view->GetCamera ()->GetW2CTranslation ();
    cur_hor_dest = (cur_hor_dest-1+4)%4;
    cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
    cam_move_up = csVector3 (0, -1, 0);
    move_right_dx = dest_move_right_dx[cur_hor_dest];
    move_right_dy = dest_move_right_dy[cur_hor_dest];
    move_down_dx = dest_move_down_dx[cur_hor_dest];
    move_down_dy = dest_move_down_dy[cur_hor_dest];
  }
  else if (key_viewright.Match (key, shift, alt, ctrl))
  {
    if (cam_move_dist) return;
    cam_move_dist = 1;
    cam_move_src = view->GetCamera ()->GetW2CTranslation ();
    cur_hor_dest = (cur_hor_dest+1)%4;
    cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
    cam_move_up = csVector3 (0, -1, 0);
    move_right_dx = dest_move_right_dx[cur_hor_dest];
    move_right_dy = dest_move_right_dy[cur_hor_dest];
    move_down_dx = dest_move_down_dx[cur_hor_dest];
    move_down_dy = dest_move_down_dy[cur_hor_dest];
  }
  else if (key_viewdown.Match (key, shift, alt, ctrl))
  {
    if (cam_move_dist) return;
    cam_move_dist = 1;
    cam_move_src = view->GetCamera ()->GetW2CTranslation ();
    if (cur_ver_dest > 0) cur_ver_dest--;
    cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
    cam_move_up = csVector3 (0, -1, 0);
  }
  else if (key_viewup.Match (key, shift, alt, ctrl))
  {
    if (cam_move_dist) return;
    cam_move_dist = 1;
    cam_move_src = view->GetCamera ()->GetW2CTranslation ();
    if (cur_ver_dest < 2) cur_ver_dest++;
    cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
    cam_move_up = csVector3 (0, -1, 0);
  }
  else if (key_zoomin.Match (key, shift, alt, ctrl))
  {
    if (cam_move_dist) return;
    cam_move_dist = 1;
    cam_move_src = view->GetCamera ()->GetW2CTranslation ();
    cam_move_dest = cam_move_src + .3 * (view_origin - cam_move_src);
    cam_move_up = csVector3 (0, -1, 0);
  }
  else if (key_zoomout.Match (key, shift, alt, ctrl))
  {
    if (cam_move_dist) return;
    cam_move_dist = 1;
    cam_move_src = view->GetCamera ()->GetW2CTranslation ();
    cam_move_dest = cam_move_src - .3 * (view_origin - cam_move_src);
    cam_move_up = csVector3 (0, -1, 0);
  }
  else if (key_rotpx.Match (key, shift, alt, ctrl))
  {
    switch (cur_hor_dest)
    {
      case 0: start_rotation (ROT_PX); break;
      case 1: start_rotation (ROT_PZ); break;
      case 2: start_rotation (ROT_MX); break;
      case 3: start_rotation (ROT_MZ); break;
    }
  }
  else if (key_rotmx.Match (key, shift, alt, ctrl))
  {
    switch (cur_hor_dest)
    {
      case 0: start_rotation (ROT_MX); break;
      case 1: start_rotation (ROT_MZ); break;
      case 2: start_rotation (ROT_PX); break;
      case 3: start_rotation (ROT_PZ); break;
    }
  }
  else if (key_rotpy.Match (key, shift, alt, ctrl)) start_rotation (ROT_PY);
  else if (key_rotmy.Match (key, shift, alt, ctrl)) start_rotation (ROT_MY);
  else if (key_rotpz.Match (key, shift, alt, ctrl))
  {
    switch (cur_hor_dest)
    {
      case 0: start_rotation (ROT_PZ); break;
      case 1: start_rotation (ROT_MX); break;
      case 2: start_rotation (ROT_MZ); break;
      case 3: start_rotation (ROT_PX); break;
    }
  }
  else if (key_rotmz.Match (key, shift, alt, ctrl))
  {
    switch (cur_hor_dest)
    {
      case 0: start_rotation (ROT_MZ); break;
      case 1: start_rotation (ROT_PX); break;
      case 2: start_rotation (ROT_PZ); break;
      case 3: start_rotation (ROT_MX); break;
    }
  }
  else if (key_up.Match (key, shift, alt, ctrl))
    start_horizontal_move (-move_down_dx, -move_down_dy);
  else if (key_down.Match (key, shift, alt, ctrl))
    start_horizontal_move (move_down_dx, move_down_dy);
  else if (key_left.Match (key, shift, alt, ctrl))
    start_horizontal_move (-move_right_dx, -move_right_dy);
  else if (key_right.Match (key, shift, alt, ctrl))
    start_horizontal_move (move_right_dx, move_right_dy);
  else if (key_drop.Match (key, shift, alt, ctrl))
  {
    if ((player1->speed) == MAX_FALL_SPEED)
      (player1->speed) = (player1->cur_speed);
    else
      player1->speed = MAX_FALL_SPEED;
  }
  else if (key_esc.Match (key, shift, alt, ctrl))
  {
    initscreen = true;
    screen = SCREEN_STARTUP;
  }
}

void Blocks::HandleHighscoresKey (int key, bool /*shift*/, bool /*alt*/,
  bool /*ctrl*/)
{
  if (key == CSKEY_ESC)
  {
    screen = SCREEN_STARTUP;
  }
}

void Blocks::HandleKeyConfigKey (int key, bool shift, bool alt, bool ctrl)
{
  if (waiting_for_key)
  {
    if (!key || key == CSKEY_SHIFT || key == CSKEY_ALT || key == CSKEY_CTRL)
      return;
    KeyMapping* map = (KeyMapping*)(keyconf_menu->GetSelectedData ());
    map->key = key;
    map->shift = shift;
    map->alt = alt;
    map->ctrl = ctrl;
    keyconf_menu->ReplaceSelected (keyconf_menu->GetSelectedText (),
    	KeyName (*map), map);
    waiting_for_key = false;
    keyconf_menu->HilightSelected (false);
    return;
  }
  switch (key)
  {
    case CSKEY_UP:
      keyconf_menu->SelUp ();
      break;
    case CSKEY_DOWN:
      keyconf_menu->SelDown ();
      break;
    case CSKEY_ENTER:
      waiting_for_key = true;
      keyconf_menu->HilightSelected (true);
      break;
    case CSKEY_ESC:
      screen = SCREEN_STARTUP;
      WriteConfig ();
      break;
  }
}

void Blocks::HandleDemoKey (int key, bool /*shf*/, bool /*alt*/, bool /*ctl*/)
{
  switch (key)
  {
    case CSKEY_UP:
      if (!menu_todo && !menu_hor_todo)
      {
        old_cur_menu = cur_menu;
        cur_menu = (cur_menu+1)%num_menus;
	menu_todo = 1;
      }
      break;
    case CSKEY_DOWN:
      if (!menu_todo && !menu_hor_todo)
      {
        old_cur_menu = cur_menu;
        cur_menu = (cur_menu-1+num_menus)%num_menus;
	menu_todo = 1;
      }
      break;
    case CSKEY_RIGHT:
      if (!menu_todo && !menu_hor_todo)
      {
        if (idx_menus[cur_menu] >= MENU_NOVICE &&
		idx_menus[cur_menu] <= MENU_EXPERT)
	{
	  menu_hor_old_menu = menus[cur_menu];
	  menu_hor_old_x_src = 0;
	  menu_hor_old_x_dst = -6.;
	  menu_hor_new_x_src = 6.;
	  menu_hor_new_x_dst = 0;
	  menu_hor_todo = 1;
	  ReplaceMenuItem (cur_menu,
	  	MENU_NOVICE + (idx_menus[cur_menu]-MENU_NOVICE + 1)
		% (MENU_EXPERT-MENU_NOVICE+1));
          diff_level = idx_menus[cur_menu]-MENU_NOVICE;
	  WriteConfig ();
	}
        else if (idx_menus[cur_menu] >= MENU_3X3 &&
		idx_menus[cur_menu] <= MENU_6X6)
	{
	  menu_hor_old_menu = menus[cur_menu];
	  menu_hor_old_x_src = 0;
	  menu_hor_old_x_dst = -6.;
	  menu_hor_new_x_src = 6.;
	  menu_hor_new_x_dst = 0;
	  menu_hor_todo = 1;
	  ReplaceMenuItem (cur_menu,
	  	MENU_3X3 + (idx_menus[cur_menu]-MENU_3X3 + 1)
		% (MENU_6X6-MENU_3X3+1));
	  player1->new_zone_dim = idx_menus[cur_menu]-MENU_3X3+3;
	  WriteConfig ();
	}
      }
      break;
    case CSKEY_LEFT:
      if (!menu_todo && !menu_hor_todo)
      {
        if (idx_menus[cur_menu] >= MENU_NOVICE &&
		idx_menus[cur_menu] <= MENU_EXPERT)
	{
	  menu_hor_old_menu = menus[cur_menu];
	  menu_hor_old_x_src = 0;
	  menu_hor_old_x_dst = 6.;
	  menu_hor_new_x_src = -6.;
	  menu_hor_new_x_dst = 0;
	  menu_hor_todo = 1;
	  ReplaceMenuItem (cur_menu,
	  	MENU_NOVICE + (idx_menus[cur_menu]-MENU_NOVICE - 1
		+ (MENU_EXPERT-MENU_NOVICE+1))
		% (MENU_EXPERT-MENU_NOVICE+1));
          diff_level = idx_menus[cur_menu]-MENU_NOVICE;
	  WriteConfig ();
	}
        else if (idx_menus[cur_menu] >= MENU_3X3 &&
		idx_menus[cur_menu] <= MENU_6X6)
	{
	  menu_hor_old_menu = menus[cur_menu];
	  menu_hor_old_x_src = 0;
	  menu_hor_old_x_dst = 6.;
	  menu_hor_new_x_src = -6.;
	  menu_hor_new_x_dst = 0;
	  menu_hor_todo = 1;
	  ReplaceMenuItem (cur_menu,
	  	MENU_3X3 + (idx_menus[cur_menu]-MENU_3X3 - 1
		+ (MENU_6X6-MENU_3X3+1))
		% (MENU_6X6-MENU_3X3+1));
	  player1->new_zone_dim = idx_menus[cur_menu]-MENU_3X3+3;
	  WriteConfig ();
	}
      }
      break;
    case CSKEY_ENTER:
      switch (idx_menus[cur_menu])
      {
        case MENU_HIGHSCORES:
	  screen = SCREEN_HIGHSCORES;
	  initscreen = true;
	  break;
        case MENU_KEYCONFIG:
	  screen = SCREEN_KEYCONFIG;
	  initscreen = true;
	  break;
	case MENU_STARTGAME:
	  screen = SCREEN_GAME;
	  initscreen = true;
	  break;
	case MENU_QUIT:
#if defined(BLOCKS_NETWORKING)
	  // Start Networking stuff.
	  // Send disconect stuff here.
	  if (Connection != NULL)
            Connection->Send ("BYE", sizeof("BYE"));

	  TerminateConnection ();
	  // Finish networking stuff.
#endif
	  System->Shutdown = true;
	  break;
      }
      break;
  }
}

void Blocks::HandleKey (int key, bool shift, bool alt, bool ctrl)
{
  if (screen == SCREEN_KEYCONFIG)
    HandleKeyConfigKey (key, shift, alt, ctrl);
  else if (screen == SCREEN_HIGHSCORES)
    HandleHighscoresKey (key, shift, alt, ctrl);
  else if (screen == SCREEN_STARTUP)
    HandleDemoKey (key, shift, alt, ctrl);
  else if (screen == SCREEN_GAMEOVER && enter_highscore)
    HandleGameOverKey (key, shift, alt, ctrl);
  else
    HandleGameKey (key, shift, alt, ctrl);
}

void Blocks::move_shape_internal (int dx, int dy, int dz)
{
  player1->cube_x += dx;
  player1->cube_y += dy;
  player1->cube_z += dz;
}

void Blocks::rotate_shape_internal (const csMatrix3& rot)
{
  csVector3 new_shift_rot;
  new_shift_rot = rot * shift_rotate;
  if (new_shift_rot.x < -.25) new_shift_rot.x = .5;
  else if (new_shift_rot.x > .25) new_shift_rot.x = .5;
  else new_shift_rot.x = 0;
  if (new_shift_rot.y < -.25) new_shift_rot.y = .5;
  else if (new_shift_rot.y > .25) new_shift_rot.y = .5;
  else new_shift_rot.y = 0;
  if (new_shift_rot.z < -.25) new_shift_rot.z = .5;
  else if (new_shift_rot.z > .25) new_shift_rot.z = .5;
  else new_shift_rot.z = 0;

  for (int i = 0 ; i < num_cubes ; i++)
  {
    csVector3 v;
    v.x = cube_info[i].dx;
    v.y = cube_info[i].dy;
    v.z = cube_info[i].dz;
    v = (v - shift_rotate) * 10.;
    v = rot * v;
    v = v + (new_shift_rot*10.);

    if (v.x < 0) cube_info[i].dx = (float)((int)(v.x-5)/10);
    else cube_info[i].dx = (float)((int)(v.x+5)/10);
    if (v.y < 0) cube_info[i].dy = (float)((int)(v.y-5)/10);
    else cube_info[i].dy = (float)((int)(v.y+5)/10);
    if (v.z < 0) cube_info[i].dz = (float)((int)(v.z-5)/10);
    else cube_info[i].dz = (float)((int)(v.z+5)/10);
  }

  shift_rotate = new_shift_rot;
}

csMaterialWrapper* Blocks::GetMaterialForHeight (int z)
{
  switch (z % 4)
  {
    case 0: return cubef1_mat;
    case 1: return cubef2_mat;
    case 2: return cubef3_mat;
    case 3: return cubef4_mat;
  }
  return cube_mat;
}

void Blocks::freeze_shape ()
{
  for (int i = 0 ; i < num_cubes ; i++)
  {
    int x = player1->cube_x+(int)cube_info[i].dx;
    int y = player1->cube_y+(int)cube_info[i].dy;
    int z = player1->cube_z+(int)cube_info[i].dz;

    player1->set_cube (x, y, z, true);

    if (screen != SCREEN_GAMEOVER)
    {
      player1->AddScore (10);
    }
    char cubename[20];
    sprintf (cubename, "cubeAt%d%d%d", x, y, z);
    // Before we let go of the shape (lose the pointer to it) we set it's
    // name according to it's position.
    cube_info[i].thing->SetName (cubename);
    ChangeThingMaterial (cube_info[i].thing, GetMaterialForHeight (z));
    if (screen != SCREEN_GAMEOVER && z >= GAMEOVER_HEIGHT)
    {
      screen = SCREEN_GAMEOVER;
      enter_highscore =
              highscores[diff_level][(player1->zone_dim)-3].CheckScore (
							       player1->score);
      hs_pos = 0;
      hs_name[0] = 0;
    }
  }
}

void Blocks::dump_shape ()
{
  CsPrintf (MSG_DEBUG_0,"Dump shape:\n");
  for (int i = 0 ; i < num_cubes ; i++)
  {
    int x = (int)cube_info[i].dx;
    int y = (int)cube_info[i].dy;
    int z = (int)cube_info[i].dz;
    CsPrintf (MSG_DEBUG_0, " %d: (%d,%d,%d) d=(%d,%d,%d)\n",
    	i, player1->cube_x+x, player1->cube_y+y, player1->cube_z+z, x, y, z);
  }
}

bool Blocks::check_new_shape_location (int dx, int dy, int dz)
{
  for (int i = 0 ; i < num_cubes ; i++)
  {
    int x = player1->cube_x+(int)cube_info[i].dx + dx;
    int y = player1->cube_y+(int)cube_info[i].dy + dy;
    int z = player1->cube_z+(int)cube_info[i].dz + dz;
    if (player1->get_cube (x, y, z)) return false;
  }
  return true;
}

bool Blocks::check_new_shape_rotation (const csMatrix3& rot)
{
  csVector3 new_shift_rot;
  new_shift_rot = rot * shift_rotate;
  if (new_shift_rot.x < -.25) new_shift_rot.x = .5;
  else if (new_shift_rot.x > .25) new_shift_rot.x = .5;
  else new_shift_rot.x = 0;
  if (new_shift_rot.y < -.25) new_shift_rot.y = .5;
  else if (new_shift_rot.y > .25) new_shift_rot.y = .5;
  else new_shift_rot.y = 0;
  if (new_shift_rot.z < -.25) new_shift_rot.z = .5;
  else if (new_shift_rot.z > .25) new_shift_rot.z = .5;
  else new_shift_rot.z = 0;

  new_shift_rot *= 10.;

  for (int i = 0 ; i < num_cubes ; i++)
  {
    csVector3 v;

    v.x = cube_info[i].dx;
    v.y = cube_info[i].dy;
    v.z = cube_info[i].dz;
    v = (v - shift_rotate) * 10.;

    v = rot * v + new_shift_rot;

    int dx, dy, dz;
    if (v.x < 0) dx = ((int)(v.x-5)/10);
    else dx = ((int)(v.x+5)/10);
    if (v.y < 0) dy = ((int)(v.y-5)/10);
    else dy = ((int)(v.y+5)/10);
    if (v.z < 0) dz = ((int)(v.z-5)/10);
    else dz = ((int)(v.z+5)/10);

    int x = player1->cube_x + dx;
    int y = player1->cube_y + dy;
    int z = player1->cube_z + dz;
    if (player1->get_cube (x, y, z)) return false;
  }
  return true;
}

void Blocks::HandleStartupMovement (cs_time elapsed_time)
{
  float elapsed = (float)elapsed_time/1000.;
  if (menu_todo)
  {
    float elapsed_menu = elapsed*1.9;
    if (elapsed_menu > menu_todo) elapsed_menu = menu_todo;
    menu_todo -= elapsed_menu;
    DrawMenu (menu_todo, menu_hor_todo, old_cur_menu, cur_menu);
  }
  if (menu_hor_todo)
  {
    float elapsed_menu = elapsed*2.4;
    if (elapsed_menu > menu_hor_todo) elapsed_menu = menu_hor_todo;
    menu_hor_todo -= elapsed_menu;
    DrawMenu (menu_todo, menu_hor_todo, old_cur_menu, cur_menu);
  }

  float old_dyn_x = dynlight_x;
  dynlight_x += dynlight_dx*elapsed;
  if (dynlight_x > 4 || dynlight_x < -4)
  {
    dynlight_dx = -dynlight_dx;
    dynlight_x = old_dyn_x;
  }
  dynlight->Move (demo_room, dynlight_x, dynlight_y, dynlight_z);
  dynlight->Setup ();
}

void Blocks::HandleGameMovement (cs_time elapsed_time)
{
  int i;
  float elapsed = (float)elapsed_time/1000.;
  float elapsed_rot = 5 * elapsed * (M_PI/2);
  float elapsed_fall = elapsed * (player1->speed);
  float elapsed_move = elapsed*2;

  if (!player1->move_down_todo)
  {
    bool stop = !check_new_shape_location (0, 0, -1);
    if ((player1->speed >= MAX_FALL_SPEED) && (screen != SCREEN_GAMEOVER))
      player1->AddScore (1);
    if (stop)
    {
      if (!(rot_px_todo || rot_mx_todo || rot_py_todo || rot_my_todo ||
      	    rot_pz_todo || rot_mz_todo || move_hor_todo))
      {
        freeze_shape ();
        player1->checkForPlane ();

	removePlanesVisual (player1);

        if (!player1->transition) StartNewShape ();
        return;
      }
    }
    else
    {
      move_shape_internal (0, 0, -1);
      player1->move_down_todo = CUBE_DIM;
    }
  }
  if (elapsed_fall > player1->move_down_todo)
    elapsed_fall = player1->move_down_todo;
  player1->move_down_todo -= elapsed_fall;

  float dx = 0, dy = 0;
  csMatrix3 rot;
  bool do_rot = false;
  if (rot_px_todo)
  {
    if (elapsed_rot > rot_px_todo) elapsed_rot = rot_px_todo;
    rot_px_todo -= elapsed_rot;
    rot = create_rotate_x (elapsed_rot);
    do_rot = true;
  }
  else if (rot_mx_todo)
  {
    if (elapsed_rot > rot_mx_todo) elapsed_rot = rot_mx_todo;
    rot_mx_todo -= elapsed_rot;
    rot = create_rotate_x (-elapsed_rot);
    do_rot = true;
  }
  else if (rot_py_todo)
  {
    if (elapsed_rot > rot_py_todo) elapsed_rot = rot_py_todo;
    rot_py_todo -= elapsed_rot;
    rot = create_rotate_y (elapsed_rot);
    do_rot = true;
  }
  else if (rot_my_todo)
  {
    if (elapsed_rot > rot_my_todo) elapsed_rot = rot_my_todo;
    rot_my_todo -= elapsed_rot;
    rot = create_rotate_y (-elapsed_rot);
    do_rot = true;
  }
  else if (rot_pz_todo)
  {
    if (elapsed_rot > rot_pz_todo) elapsed_rot = rot_pz_todo;
    rot_pz_todo -= elapsed_rot;
    rot = create_rotate_z (elapsed_rot);
    do_rot = true;
  }
  else if (rot_mz_todo)
  {
    if (elapsed_rot > rot_mz_todo) elapsed_rot = rot_mz_todo;
    rot_mz_todo -= elapsed_rot;
    rot = create_rotate_z (-elapsed_rot);
    do_rot = true;
  }
  // Check if there is a rotation in the queue.
  if (!(rot_px_todo || rot_mx_todo || rot_py_todo ||
  	rot_my_todo || rot_pz_todo || rot_mz_todo) && queue_rot_todo)
  {
    start_rotation (queue_rot_todo);
    queue_rot_todo = ROT_NONE;
  }

  if (move_hor_todo)
  {
    if (elapsed_move > move_hor_todo) elapsed_move = move_hor_todo;
    move_hor_todo -= elapsed_move;
    dx = elapsed_move*(float)move_hor_dx;
    dy = elapsed_move*(float)move_hor_dy;
    // Check if there is a horizontal move in the queue.
    if (!move_hor_todo && (queue_move_dx_todo || queue_move_dy_todo))
    {
      start_horizontal_move (queue_move_dx_todo, queue_move_dy_todo);
      queue_move_dx_todo = 0;
      queue_move_dy_todo = 0;
    }
  }

  for (i = 0 ; i < num_cubes ; i++)
  {
    csThing* t = cube_info[i].thing;
    if (do_rot)
      t->GetMovable ().Transform (rot);
    t->GetMovable ().MovePosition (csVector3 (dx, -elapsed_fall, dy));
    t->GetMovable ().UpdateMove ();
    reset_vertex_colors (t);
    room->ShineLights (t);
  }
  csThing::current_light_frame_number++;
}

void Blocks::HandleMovement (cs_time elapsed_time)
{
  float elapsed = (float)elapsed_time/1000.;

  if (player1->fog_density)
  {
    float elapsed_fog = elapsed*.8;
    if (elapsed_fog > player1->fog_density) elapsed_fog = player1->fog_density;
    player1->fog_density -= elapsed_fog;
    csSector* s;
    if (screen == SCREEN_STARTUP) s = demo_room;
    else s = room;
    if (player1->fog_density)
      s->SetFog (player1->fog_density, csColor (0, 0, 0));
    else
      s->DisableFog ();
    return;
  }

  if (screen == SCREEN_STARTUP)
  {
    HandleStartupMovement (elapsed_time);
    return;
  }

  if (cam_move_dist)
  {
    float elapsed_cam_move = elapsed*2;
    if (elapsed_cam_move > cam_move_dist) elapsed_cam_move = cam_move_dist;
    cam_move_dist -= elapsed_cam_move;
    HandleCameraMovement ();
  }

  if (player1->transition) { HandleTransition (elapsed_time); return; }

  if (pause) return;
  if (screen == SCREEN_GAMEOVER) return;

  HandleGameMovement (elapsed_time);
}

void Blocks::InitTextures ()
{
  if (engine) engine->Clear ();
  csEngine* const e = Sys->engine;

  csLoader::LoadTexture (e, "pillar", "stone4.png");
  Sys->set_pillar_material (e->GetMaterials ()->FindByName ("pillar"));

  csLoader::LoadTexture (e, "cube", "cube.png");
  Sys->set_cube_material (e->GetMaterials ()->FindByName ("cube"));
  csLoader::LoadTexture (e, "raster", "clouds_thick1.jpg");
  Sys->set_raster_material (e->GetMaterials ()->FindByName ("raster"));
  csLoader::LoadTexture (e, "room", "mystone2.png");
  csLoader::LoadTexture (e, "clouds", "clouds.jpg");

  csLoader::LoadTexture (e, "cubef1", "cubef1.png");
  Sys->set_cube_f1_material (e->GetMaterials ()->FindByName ("cubef1"));
  csLoader::LoadTexture (e, "cubef2", "cubef2.png");
  Sys->set_cube_f2_material (e->GetMaterials ()->FindByName ("cubef2"));
  csLoader::LoadTexture (e, "cubef3", "cubef3.png");
  Sys->set_cube_f3_material (e->GetMaterials ()->FindByName ("cubef3"));
  csLoader::LoadTexture (e, "cubef4", "cubef4.png");
  Sys->set_cube_f4_material (e->GetMaterials ()->FindByName ("cubef4"));

  csLoader::LoadTexture (e, "menu_novice", "novice.png");
  csLoader::LoadTexture (e, "menu_back", "back.png");
  csLoader::LoadTexture (e, "menu_average", "average.png");
  csLoader::LoadTexture (e, "menu_expert", "expert.png");
  csLoader::LoadTexture (e, "menu_high", "high.png");
  csLoader::LoadTexture (e, "menu_quit", "quit.png");
  csLoader::LoadTexture (e, "menu_3x3", "p3x3.png");
  csLoader::LoadTexture (e, "menu_4x4", "p4x4.png");
  csLoader::LoadTexture (e, "menu_5x5", "p5x5.png");
  csLoader::LoadTexture (e, "menu_6x6", "p6x6.png");
  csLoader::LoadTexture (e, "menu_keyconfig", "keys.png");
  csLoader::LoadTexture (e, "menu_start", "start.png");
}

void Blocks::DrawMenu (int menu)
{
  DrawMenu (0, 0, 0, menu);
  menu_todo = 0;
  menu_hor_todo = 0;
}

void Blocks::DrawMenu (float menu_trans, float menu_hor_trans, int old_menu,
  int new_menu)
{
  for (int i = 0 ; i < num_menus ; i++)
  {
    int old_curi = (i-old_menu+num_menus)%num_menus;
    int new_curi = (i-new_menu+num_menus)%num_menus;
    if (old_curi == 0 && new_curi == num_menus-1) old_curi = num_menus;
    if (new_curi == 0 && old_curi == num_menus-1) new_curi = num_menus;
    float curi = menu_trans * ((float)old_curi) +
    	(1-menu_trans) * ((float)new_curi);
    float angle = 2.*M_PI*curi/(float)num_menus;
    float x = 0;
    if (i == cur_menu)
    {
      x = menu_hor_trans * menu_hor_new_x_src +
    		(1-menu_hor_trans) * menu_hor_new_x_dst;
    }
    float y = 3. + sin (angle)*3.;
    float z = 5. - cos (angle)*3.;

    csVector3 v (x, y, z);
    menus[i]->GetMovable ().SetPosition (demo_room, v);
    menus[i]->GetMovable ().UpdateMove ();
  }
  // Move the old menu item away.
  if ((ABS (menu_hor_trans) > SMALL_EPSILON) &&
  	menu_hor_old_menu)
  {
    float x = menu_hor_trans * menu_hor_old_x_src +
    		(1-menu_hor_trans) * menu_hor_old_x_dst;
    float angle = 0;
    float y = 3. + sin (angle)*3.;
    float z = 5. - cos (angle)*3.;
    csVector3 v (x, y, z);
    menu_hor_old_menu->GetMovable ().SetPosition (demo_room, v);
    menu_hor_old_menu->GetMovable ().UpdateMove ();
  }
  else if (menu_hor_old_menu)
  {
    menu_hor_old_menu->GetMovable ().ClearSectors ();
    menu_hor_old_menu->GetMovable ().UpdateMove ();
    menu_hor_old_menu = NULL;
  }

  if (!(ABS (menu_trans) > SMALL_EPSILON) &&
  	!(ABS (menu_hor_trans) > SMALL_EPSILON) &&
  	leftright_menus[cur_menu])
  {
    float angle = 0;
    float x = 0;
    float y = 3. + sin (angle)*3.;
    float z = 5. - cos (angle)*3.;
    csVector3 v (x, y, z);
    arrow_left->GetMovable ().SetSector (demo_room);
    arrow_right->GetMovable ().SetSector (demo_room);
    arrow_left->GetMovable ().SetPosition (demo_room, v);
    arrow_right->GetMovable ().SetPosition (demo_room, v);
    arrow_left->GetMovable ().UpdateMove ();
    arrow_right->GetMovable ().UpdateMove ();
  }
  else
  {
    arrow_left->GetMovable ().ClearSectors ();
    arrow_right->GetMovable ().ClearSectors ();
    arrow_left->GetMovable ().UpdateMove ();
    arrow_right->GetMovable ().UpdateMove ();
  }
}

void Blocks::CreateMenuEntry (const char* mat, int menu_nr)
{
  csMaterialWrapper* tm_front = engine->GetMaterials ()->FindByName (mat);
  csThing* thing = new csThing (engine);
  thing->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
  engine->things.Push (thing);

  thing->AddVertex (-1, .25, 0);
  thing->AddVertex (1, .25, 0);
  thing->AddVertex (1, -.25, 0);
  thing->AddVertex (-1, -.25, 0);

  csPolygon3D* p;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = thing->NewPolygon (tm_front);
  p->AddVertex (0);
  p->AddVertex (1);
  p->AddVertex (3);
  p->SetTextureSpace (tx_matrix, tx_vector);
  set_uv (p, 0, 0, 1, 0, 0, 1);

  p = thing->NewPolygon (tm_front);
  p->AddVertex (1);
  p->AddVertex (2);
  p->AddVertex (3);
  p->SetTextureSpace (tx_matrix, tx_vector);
  set_uv (p, 1, 0, 1, 1, 0, 1);

  src_menus[menu_nr] = thing;
}

csThing* Blocks::CreateMenuArrow (bool left)
{
  csMaterialWrapper* tm_front = engine->GetMaterials ()->FindByName ("menu_back");
  csThing* thing = new csThing (engine);
  thing->SetMovingOption (CS_THING_MOVE_OCCASIONAL);
  engine->things.Push (thing);

  float pointx;
  float rearx;
  if (left)
  {
    pointx = -1-.4;
    rearx = -1-.1;
  }
  else
  {
    pointx = 1+.4;
    rearx = 1+.1;
  }

  thing->AddVertex (pointx, 0, 0);
  thing->AddVertex (rearx, .25, 0);
  thing->AddVertex (rearx, -.25, 0);

  csPolygon3D* p;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = thing->NewPolygon (tm_front);
  if (left)
  {
    p->AddVertex (0);
    p->AddVertex (1);
    p->AddVertex (2);
  }
  else
  {
    p->AddVertex (2);
    p->AddVertex (1);
    p->AddVertex (0);
  }
  p->SetTextureSpace (tx_matrix, tx_vector);
  set_uv (p, 0, 0, 1, 0, 0, 1);

  return thing;
}

void Blocks::InitMenu ()
{
  num_menus = 0;
  for (int i = 0 ; i < MENU_TOTAL ; i++)
  {
    src_menus[i]->GetMovable ().ClearSectors ();
    src_menus[i]->GetMovable ().UpdateMove ();
  }
  arrow_left->GetMovable ().ClearSectors ();
  arrow_right->GetMovable ().ClearSectors ();
  arrow_left->GetMovable ().UpdateMove ();
  arrow_right->GetMovable ().UpdateMove ();
}

void Blocks::AddMenuItem (int menu_nr, bool leftright)
{
  menus[num_menus] = src_menus[menu_nr];
  idx_menus[num_menus] = menu_nr;
  leftright_menus[num_menus] = leftright;
  num_menus++;
  src_menus[menu_nr]->GetMovable ().SetSector (demo_room);
}

void Blocks::ReplaceMenuItem (int idx, int menu_nr)
{
  menus[idx] = src_menus[menu_nr];
  idx_menus[idx] = menu_nr;
  src_menus[menu_nr]->GetMovable ().SetSector (demo_room);
}

void Blocks::ChangePlaySize (int new_size)
{
  int idx = Sys->engine->sectors.Find ((csSome)room);
  Sys->engine->sectors.Delete (idx);
  player1->zone_dim = new_size;
  WriteConfig ();
  InitGameRoom ();
  room->Prepare ();
  room->InitLightMaps (false);
  room->ShineLights ();
  room->CreateLightMaps (Gfx3D);
}

void Blocks::StartKeyConfig ()
{
  initscreen = false;
  waiting_for_key = false;
  delete keyconf_menu;
  keyconf_menu = new TextEntryMenu ();
  keyconf_menu->Add ("shape up", KeyName (key_up), (void*)&key_up);
  keyconf_menu->Add ("shape down", KeyName (key_down), (void*)&key_down);

  keyconf_menu->Add ("shape left", KeyName (key_left), (void*)&key_left);
  keyconf_menu->Add ("shape right", KeyName (key_right), (void*)&key_right);
  keyconf_menu->Add ("pitch down", KeyName (key_rotpx), (void*)&key_rotpx);
  keyconf_menu->Add ("pitch up", KeyName (key_rotmx), (void*)&key_rotmx);
  keyconf_menu->Add ("yaw left", KeyName (key_rotpy), (void*)&key_rotpy);
  keyconf_menu->Add ("yaw right", KeyName (key_rotmy), (void*)&key_rotmy);
  keyconf_menu->Add ("roll left", KeyName (key_rotpz), (void*)&key_rotpz);
  keyconf_menu->Add ("roll right", KeyName (key_rotmz), (void*)&key_rotmz);
  keyconf_menu->Add ("camera left", KeyName (key_viewleft),
    (void*)&key_viewleft);
  keyconf_menu->Add ("camera right", KeyName (key_viewright),
    (void*)&key_viewright);
  keyconf_menu->Add ("camera up", KeyName (key_viewup), (void*)&key_viewup);
  keyconf_menu->Add ("camera down", KeyName (key_viewdown),
    (void*)&key_viewdown);
  keyconf_menu->Add ("zoom in", KeyName (key_zoomin), (void*)&key_zoomin);
  keyconf_menu->Add ("zoom out", KeyName (key_zoomout), (void*)&key_zoomout);
  keyconf_menu->Add ("drop", KeyName (key_drop), (void*)&key_drop);
  keyconf_menu->Add ("pause", KeyName (key_pause), (void*)&key_pause);
  keyconf_menu->SetSelected (0);
}

void Blocks::InitGameRoom ()
{
  csMaterialWrapper* tm = engine->GetMaterials ()->FindByName ("room");
  room = Sys->engine->CreateCsSector ("room");
  csThing* walls = Sys->engine->CreateSectorWalls (room, "walls");
  Sys->set_cube_room (room);
  csPolygon3D* p;
  p = walls->NewPolygon (tm);
  p->AddVertex (-5, 0, 5);
  p->AddVertex (5, 0, 5);
  p->AddVertex (5, 0, -5);
  p->AddVertex (-5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (-5, 20, 5);
  p->AddVertex (5, 20, 5);
  p->AddVertex (5, 0, 5);
  p->AddVertex (-5, 0, 5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (5, 20, 5);
  p->AddVertex (5, 20, -5);
  p->AddVertex (5, 0, -5);
  p->AddVertex (5, 0, 5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (-5, 20, -5);
  p->AddVertex (-5, 20, 5);
  p->AddVertex (-5, 0, 5);
  p->AddVertex (-5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = walls->NewPolygon (tm);
  p->AddVertex (5, 20, -5);
  p->AddVertex (-5, 20, -5);
  p->AddVertex (-5, 0, -5);
  p->AddVertex (5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  Sys->add_pillar_template ();
  Sys->add_cube_template ();
  Sys->add_vrast_template ();
  Sys->add_hrast_template ();

  Sys->add_pillar (-1, -1);
  Sys->add_pillar (player1->zone_dim, -1);
  Sys->add_pillar (-1, player1->zone_dim);
  Sys->add_pillar (player1->zone_dim, player1->zone_dim);

  for (int i = 0 ; i < player1->zone_dim-1 ; i++)
  {
    Sys->add_vrast (-1, i, CUBE_DIM/2, CUBE_DIM/2, -M_PI/2);
    Sys->add_vrast ((player1->zone_dim)-1, i, CUBE_DIM/2, CUBE_DIM/2, M_PI/2);
    Sys->add_vrast (i, -1, CUBE_DIM/2, CUBE_DIM/2, 0);
    Sys->add_vrast (i, (player1->zone_dim)-1, CUBE_DIM/2, CUBE_DIM/2, M_PI);
  }

  Sys->add_hrast (-1, 2, CUBE_DIM/2, CUBE_DIM/2, -M_PI/2);
  Sys->add_hrast ((player1->zone_dim)-1, 2, CUBE_DIM/2, CUBE_DIM/2, -M_PI/2);
  Sys->add_hrast (2, -1, CUBE_DIM/2, CUBE_DIM/2, 0);
  Sys->add_hrast (2, (player1->zone_dim)-1, CUBE_DIM/2, CUBE_DIM/2, 0);

  room->AddLight (new csStatLight (-3, 5, 0, 10, .8, .4, .4, false));
  room->AddLight (new csStatLight (3, 5, 0, 10, .4, .4, .8, false));
  room->AddLight (new csStatLight (0, 5, -3, 10, .4, .8, .4, false));
  room->AddLight (new csStatLight (0, 5, 3, 10, .8, .4, .8, false));
  room->AddLight (new csStatLight (0, (ZONE_HEIGHT-3-3)*CUBE_DIM+1, 0,
  	CUBE_DIM*10, .5, .5, .5, false));
  room->AddLight (new csStatLight (0, (ZONE_HEIGHT-3+3)*CUBE_DIM+1, 0,
  	CUBE_DIM*10, .5, .5, .5, false));
}

void Blocks::InitDemoRoom ()
{
  csMaterialWrapper* demo_tm = engine->GetMaterials ()->FindByName ("clouds");
  demo_room = Sys->engine->CreateCsSector ("room");
  csThing* walls = Sys->engine->CreateSectorWalls (demo_room, "walls");

  csPolygon3D* p;
  p = walls->NewPolygon (demo_tm);
  p->AddVertex (-50, 50, 50);
  p->AddVertex (50, 50, 50);
  p->AddVertex (50, -50, 50);
  p->AddVertex (-50, -50, 50);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 100);

  demo_room->AddLight (new csStatLight (0, 0, -2, 10, .4, .4, .4, false));

  float char_width = CUBE_DIM*4.;
  float offset_x = -char_width * (6/2)+CUBE_DIM*2;
  start_demo_shape (SHAPE_DEMO_B, offset_x, 3, 4); offset_x += char_width;
  start_demo_shape (SHAPE_DEMO_L, offset_x, 3, 4); offset_x += char_width;
  start_demo_shape (SHAPE_DEMO_O, offset_x, 3, 4); offset_x += char_width;
  start_demo_shape (SHAPE_DEMO_C, offset_x, 3, 4); offset_x += char_width;
  start_demo_shape (SHAPE_DEMO_K, offset_x, 3, 4); offset_x += char_width;
  start_demo_shape (SHAPE_DEMO_S, offset_x, 3, 4); offset_x += char_width;

  CreateMenuEntry ("menu_novice", MENU_NOVICE);
  CreateMenuEntry ("menu_average", MENU_AVERAGE);
  CreateMenuEntry ("menu_expert", MENU_EXPERT);
  CreateMenuEntry ("menu_start", MENU_STARTGAME);
  CreateMenuEntry ("menu_high", MENU_HIGHSCORES);
  CreateMenuEntry ("menu_high", MENU_SETUP);	// Unused!
  CreateMenuEntry ("menu_quit", MENU_QUIT);
  CreateMenuEntry ("menu_3x3", MENU_3X3);
  CreateMenuEntry ("menu_4x4", MENU_4X4);
  CreateMenuEntry ("menu_5x5", MENU_5X5);
  CreateMenuEntry ("menu_6x6", MENU_6X6);
  CreateMenuEntry ("menu_keyconfig", MENU_KEYCONFIG);
  arrow_left = CreateMenuArrow (true);
  arrow_right = CreateMenuArrow (false);
}

void Blocks::InitEngine ()
{
  InitTextures ();
  InitGameRoom ();
  InitDemoRoom ();
  Sys->engine->Prepare ();

//#undef DO_SOUND
#ifdef DO_SOUND
  // Load the blocks.zip library where sound refs are stored
  csLoader::LoadLibraryFile (engine, "/data/blocks/Library");
  iSoundData* w = csSoundDataObject::GetSound(*engine, "background.wav");
  if (w && Sound) Sound->PlaySound (w, true);
#endif
}

void Blocks::StartDemo ()
{
  initscreen = false;

  dynlight_x = 0;
  dynlight_y = 3;
  dynlight_z = 0;
  dynlight_dx = 3;
  delete dynlight;
  dynlight = new csDynLight (dynlight_x, dynlight_y, dynlight_z, 7, 3, 0, 0);

  Sys->engine->AddDynLight (dynlight);
  dynlight->SetSector (demo_room);
  dynlight->Setup ();

  view->SetSector (demo_room);
  csVector3 pos (0, 3, -5);
  view->GetCamera ()->SetPosition (pos);
  cam_move_up = csVector3 (0, -1, 0);
  view->GetCamera ()->LookAt (view_origin-pos, cam_move_up);
  view->SetRectangle (0, 0, Sys->FrameWidth, Sys->FrameHeight);

  player1->fog_density = 1;
  demo_room->SetFog (player1->fog_density, csColor (0, 0, 0));

  InitMainMenu ();
  menu_hor_old_menu = NULL;
}

void Blocks::InitMainMenu ()
{
  InitMenu ();
  AddMenuItem (MENU_STARTGAME, false);
  AddMenuItem (MENU_NOVICE+diff_level, true);
  AddMenuItem (MENU_3X3+(player1->new_zone_dim)-3, true);
  AddMenuItem (MENU_KEYCONFIG, false);
  AddMenuItem (MENU_HIGHSCORES, false);
  AddMenuItem (MENU_QUIT, false);

  cur_menu = 0;
  DrawMenu (cur_menu);
}

void Blocks::StartNewGame ()
{
  if (player1->new_zone_dim != player1->zone_dim)
    ChangePlaySize (player1->new_zone_dim);

  delete dynlight; dynlight = NULL;

  // First delete all cubes that may still be in the engine.
  int i = 0;
  while (i < room->GetNumThings ())
  {
    csThing* cube = room->GetThing (i);
    if (!strncmp (cube->GetName (), "cube", 4))
    {
      room->UnlinkThing (cube);
      delete cube;
    }
    else
      i++;
  }

  Sys->InitGame ();
  Sys->StartNewShape ();

  cam_move_up = csVector3 (0, -1, 0);
  view->SetSector (room);
  Sys->HandleCameraMovement ();
  view->SetRectangle (0, 0, Sys->FrameWidth, Sys->FrameHeight);

  player1->fog_density = 1;
  room->SetFog (player1->fog_density, csColor (0, 0, 0));
}

void Blocks::removePlanesVisual (States* player)
{
  for (int z = 0; z < ZONE_HEIGHT; z++)
  {
    if (player->filled_planes[z])
      for (int x = 0; x < player->zone_dim; x++)
        for (int y = 0; y < player->zone_dim; y++)
        { // Physically remove it.
          char temp[20];
          sprintf (temp, "cubeAt%d%d%d", x, y, z);
	  csThing* th = room->GetThing (temp);
	  th->GetMovable ().ClearSectors ();
	  th->GetMovable ().UpdateMove ();
        }
  }
}

void Blocks::HandleTransition (cs_time elapsed_time)
{
  if (screen == SCREEN_GAMEOVER) return;
  player1->transition = false;

  for (int i=0 ; i<ZONE_HEIGHT ; i++)
  {
    if (player1->filled_planes[i])
    {
      player1->transition = true;
      player1->gone_z = i;
      HandleLoweringPlanes (elapsed_time);
      break;
    }
  }
  if (!player1->transition)
  {
    if (player1->CheckEmptyPlayArea ())
    {
      if(screen != SCREEN_GAMEOVER)
        player1->AddScore (800);
    }
    player1->move_down_todo = 0;
    StartNewShape ();
  }
}

void Blocks::HandleLoweringPlanes (cs_time elapsed_time)
{
  if (pause) return;

  float elapsed = (float)elapsed_time/1000.;
  float elapsed_fall = elapsed* (player1->speed);

  int i;
  int x,y,z;
  char temp[20];
  csThing* t;

  // Finished the transition.
  if (!player1->move_down_todo)
  {
    // We finished handling this plane.
    // We lower the planes (in case the player made more then one plane).
    for (i=player1->gone_z ; i<ZONE_HEIGHT-1 ; i++)
      player1->filled_planes[i] = player1->filled_planes[i+1];
    player1->filled_planes[ZONE_HEIGHT - 1] = false; // And the last one.

    // Now that everything is visually ok we lower(change) their names
    // accordingly and clear them from game_cube[][][].
    for (z=(player1->gone_z)+1 ; z<ZONE_HEIGHT ; z++)
      for (x=0 ; x < player1->zone_dim ; x++)
        for (y=0 ; y < player1->zone_dim ; y++)
	{
	  player1->set_cube (x, y, z-1, player1->get_cube (x, y, z));
          sprintf (temp, "cubeAt%d%d%d", x, y, z);
          t = room->GetThing (temp);
          if (t)
	  {
            sprintf (temp, "cubeAt%d%d%d", x, y, z-1);
	    t->SetName (temp);
	    ChangeThingMaterial (t, GetMaterialForHeight (z-1));
	  }
	}

    // Mustn't forget the topmost level.
    for (x=0 ; x < player1->zone_dim ; x++)
      for (y=0 ; y < player1->zone_dim ; y++)
        player1->set_cube (x, y, ZONE_HEIGHT-1, false);
    // Set movement for next plane (if needed).
    player1->move_down_todo = CUBE_DIM;
    return;
  }

  if (elapsed_fall > player1->move_down_todo)
    elapsed_fall = player1->move_down_todo;
  player1->move_down_todo -= elapsed_fall;

  // Move everything from above the plane that dissapeared a bit lower.
  for (z=(player1->gone_z)+1 ; z<ZONE_HEIGHT ; z++)
    for (x=0 ; x < player1->zone_dim ; x++)
      for (y=0 ; y < player1->zone_dim ; y++)
      {
	sprintf (temp, "cubeAt%d%d%d", x, y, z);
        // Only if there is a thing at that certain position, or less
	// then CUBE_DIM lower.
	t = room->GetThing (temp);
	if (t)
	{
          t->GetMovable ().MovePosition (csVector3 (0, -elapsed_fall, 0));
          t->GetMovable ().UpdateMove ();
          reset_vertex_colors (t);
          room->ShineLights (t);
	}
      }

  // So that the engine knows to update the lights only when the frame
  // has changed (?).
  csThing::current_light_frame_number++;
}

void Blocks::NextFrame ()
{
  SysSystemDriver::NextFrame ();
  cs_time elapsed_time, current_time;
  GetElapsedTime (elapsed_time, current_time);

  // -----------------------------------------------------------------
  // Start network stuff.
  
#if defined(BLOCKS_NETWORKING)

  if (do_network && NUM_FRAMES_CHK_NET <= since_last_check)
  {
    since_last_check = 0;

///    player1->EncodeStates ();
    //    printf("Printing to before.txt\n");
    //    player1->PrintData("before.txt");
    //    printf("********decoding data*********\n");
///    if (!player1->DecodeStates ())
///      printf ("failed to decode\n");
      //    printf("Printing to after.txt\n");
      //    player1->PrintData("after.txt");

///    unsigned char * abuffer = (unsigned char *) malloc(
///				     (ST_ENCODED_LENGTH + ST_CLIENT_EXTRA) 
///				     * sizeof(unsigned char));
///        
///    if(!player1_net->EncodeForNetwork(player1->encodedData, abuffer, 
///				      ST_ENCODED_LENGTH, 
///                                      (ST_ENCODED_LENGTH + ST_CLIENT_EXTRA)))
///      printf("Network don't work\n");

///    if(!player1_net->DecodeFromNetwork(abuffer,
///				       (ST_ENCODED_LENGTH + ST_CLIENT_EXTRA),
///				       player1))
///      printf("Network don't work, decoding\n");
///    else
///      printf("eh!! it did work\n");


    if (IsServer)
    {
//      printf("I am server hear me roar!!!\n");

      if (Connection != NULL)
      {
        CheckConnection();
//        Connection->Send("You suck", sizeof("You suck"));
      }
      else
      {
//	printf("oi!");
        if ((System->Time() - LastConnectTime) > 1000)
        {
          LastConnectTime = System->Time();
	  
          Connection = Listener->Accept();
	  // These slow down blocks too much.
	  if (Connection != NULL)
	    System->Printf(MSG_INITIALIZATION, "Connection accepted\n");
	  else
	    System->Printf(MSG_INITIALIZATION,
			   "Awaiting connect (response %d)\n",
			   Listener->GetLastError());
        }
      
      }
    }
    
    else  // We are not a blocks server.
    {
      
      if (Connection != NULL)
        CheckConnection();
      else
      {
        if ((Sys->Time() - LastConnectTime) > 1000)
        {
          LastConnectTime = Sys->Time ();
          Connect();
        }
      }
      
    }
  }
  else  // aren't up to the number of frames yet.
  {
    since_last_check++;
  }

#endif

  // Finish network stuff.
  // -----------------------------------------------------------------

  if (screen == SCREEN_STARTUP)
  {
    if (initscreen) StartDemo ();
    HandleMovement (elapsed_time);
    if (!Gfx3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
      return;
    view->Draw ();
    Gfx3D->FinishDraw ();
    Gfx3D->Print (NULL);
    return;
  }

  if (screen == SCREEN_KEYCONFIG)
  {
    if (initscreen) StartKeyConfig ();
    if (!Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    keyconf_menu->Draw (elapsed_time);
    Gfx3D->FinishDraw ();
    Gfx3D->Print (NULL);
    return;
  }

  if (screen == SCREEN_HIGHSCORES)
  {
    if (initscreen) { initscreen = false; }
    if (!Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    Gfx2D->Clear (0);
    char buffer[100];
    sprintf (buffer, "Highscores for %s at %dx%d",
    	diff_level == 0 ? "novice" : diff_level == 1 ? "average" : "expert",
	player1->new_zone_dim, player1->new_zone_dim);
    Gfx2D->Write (font, 10, 10, white, black, buffer);
    int i;
    HighScore& hs = highscores[diff_level][(player1->new_zone_dim)-3];
    bool scores = false;
    for (i = 0 ; i < 10 ; i++)
    {
      if (hs.Get (i) >= 0)
      {
        scores = true;
        if (hs.GetName (i))
          Gfx2D->Write (font, 10, 30+i*12, white, black, hs.GetName (i));
        else
          Gfx2D->Write (font, 10, 30+i*12, white, black, "?");
	char scorebuf[30];
	sprintf (scorebuf, "%d", hs.Get (i));
        Gfx2D->Write (font, 200, 30+i*12, white, black, scorebuf);
      }
    }
    if (!scores)
      Gfx2D->Write (font, 10, Sys->FrameHeight/2, white, black,
      	"This screen intentionally left blank");
    Gfx3D->FinishDraw ();
    Gfx3D->Print (NULL);
    return;
  }

  if (initscreen) StartNewGame ();

  // This is where Blocks stuff really happens.
  HandleMovement (elapsed_time);

  if (!pause)
  {
    player1->cur_speed += ((float)elapsed_time)/(300.*1000.);
    if (player1->cur_speed > MAX_SPEED) player1->cur_speed = MAX_SPEED;

    // Tell Gfx3D we're going to display 3D things
    if (!Gfx3D->BeginDraw (engine->GetBeginDrawFlags () | CSDRAW_3DGRAPHICS))
      return;
    view->Draw ();
  }

  // Start drawing 2D graphics
  if (!Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  if (pause)
  {
    Gfx2D->Clear (0);
    Gfx2D->Write (font, 100, Sys->FrameHeight-20, white, black, "PAUSE");
  }

  char scorebuf[50];
  sprintf (scorebuf, "%d", player1->score);
  Gfx2D->Write (font, 10, Sys->FrameHeight-20, white, black, scorebuf);

  // Game over!
  if (screen == SCREEN_GAMEOVER)
  {
    float elapsed = (float)elapsed_time/1000.;
    static float time_left = 0;
    if (elapsed > time_left) elapsed = time_left;
    time_left -= elapsed;
    if (time_left <= 0) time_left = 1;

    int y = Sys->FrameHeight/2;
    int x = (Sys->FrameWidth-300)/2;
    Gfx2D->DrawBox (x, y, 300, 12, black);
    Gfx2D->Write (font, x+10, y+2, white, black, "GAME OVER!");
    if (enter_highscore)
    {
      Gfx2D->DrawBox (x, y+12, 300, 24, black);
      Gfx2D->Write (font, x+10, y+12+2, white, black, "Enter name for highscores:");
      char name[21];
      strcpy (name, hs_name);
      if (time_left < .5) strcat (name, "_");
      Gfx2D->Write (font, x+10, y+24+2, white, black, name);
    }
  }

  // Drawing code ends here
  Gfx3D->FinishDraw ();
  Gfx3D->Print (NULL);
}

bool Blocks::HandleEvent (iEvent &Event)
{
  if (SysSystemDriver::HandleEvent (Event))
    return false;

  switch (Event.Type)
  {
    case csevKeyDown:
      HandleKey (Event.Key.Code, Event.Key.Modifiers & CSMASK_SHIFT,
          Event.Key.Modifiers & CSMASK_ALT, Event.Key.Modifiers & CSMASK_CTRL);
      break;
    case csevMouseDown:
      break;
    case csevMouseMove:
      break;
    case csevMouseUp:
      break;
  }
  return false;
}

//-----------------------------------------------------

void Blocks::NamedKey (const char* keyname, KeyMapping& map)
{
  map.shift = false;
  map.alt = false;
  map.ctrl = false;
  const char* dash = strchr (keyname, '-');
  while (dash)
  {
    const int len = dash - keyname;
    if (len > 0)
      if (!strncmp (keyname, "shift", len)) map.shift = true;
      else if (!strncmp (keyname, "alt", len)) map.alt = true;
      else if (!strncmp (keyname, "ctrl", len)) map.ctrl = true;
    keyname = dash + 1;
    dash = strchr (keyname, '-');
  }

  if (!strcmp (keyname, "tab")) map.key = CSKEY_TAB;
  else if (!strcmp (keyname, "space")) map.key = ' ';
  else if (!strcmp (keyname, "esc")) map.key = CSKEY_ESC;
  else if (!strcmp (keyname, "enter")) map.key = CSKEY_ENTER;
  else if (!strcmp (keyname, "bs")) map.key = CSKEY_BACKSPACE;
  else if (!strcmp (keyname, "up")) map.key = CSKEY_UP;
  else if (!strcmp (keyname, "down")) map.key = CSKEY_DOWN;
  else if (!strcmp (keyname, "right")) map.key = CSKEY_RIGHT;
  else if (!strcmp (keyname, "left")) map.key = CSKEY_LEFT;
  else if (!strcmp (keyname, "pgup")) map.key = CSKEY_PGUP;
  else if (!strcmp (keyname, "pgdn")) map.key = CSKEY_PGDN;
  else if (!strcmp (keyname, "home")) map.key = CSKEY_HOME;
  else if (!strcmp (keyname, "end")) map.key = CSKEY_END;
  else if (!strcmp (keyname, "ins")) map.key = CSKEY_INS;
  else if (!strcmp (keyname, "del")) map.key = CSKEY_DEL;
  else if (!strcmp (keyname, "f1")) map.key = CSKEY_F1;
  else if (!strcmp (keyname, "f2")) map.key = CSKEY_F2;
  else if (!strcmp (keyname, "f3")) map.key = CSKEY_F3;
  else if (!strcmp (keyname, "f4")) map.key = CSKEY_F4;
  else if (!strcmp (keyname, "f5")) map.key = CSKEY_F5;
  else if (!strcmp (keyname, "f6")) map.key = CSKEY_F6;
  else if (!strcmp (keyname, "f7")) map.key = CSKEY_F7;
  else if (!strcmp (keyname, "f8")) map.key = CSKEY_F8;
  else if (!strcmp (keyname, "f9")) map.key = CSKEY_F9;
  else if (!strcmp (keyname, "f10")) map.key = CSKEY_F10;
  else if (!strcmp (keyname, "f11")) map.key = CSKEY_F11;
  else if (!strcmp (keyname, "f12")) map.key = CSKEY_F12;
  else if ((*keyname >= 'A' && *keyname <= 'Z') ||
    strchr ("!@#$%^&*()_+", *keyname))
  {
    map.shift = true;
    map.key = *keyname;
  }
  else if (*keyname >= 'a' && *keyname <= 'z')
  {
    if (map.shift) map.key = (*keyname)+'A'-'a';
    else map.key = *keyname;
  }
  else map.key = *keyname;
}

const char* Blocks::KeyName (const KeyMapping& map)
{
  static char buf[100];
  buf[0] = 0;
  if (map.shift) strcat (buf, "shift-");
  if (map.ctrl) strcat (buf, "ctrl-");
  if (map.alt) strcat (buf, "alt-");
  switch (map.key)
  {
    case CSKEY_TAB: strcat (buf, "tab"); break;
    case ' ': strcat (buf, "space"); break;
    case CSKEY_ESC: strcat (buf, "esc"); break;
    case CSKEY_ENTER: strcat (buf, "enter"); break;
    case CSKEY_BACKSPACE: strcat (buf, "bs"); break;
    case CSKEY_UP: strcat (buf, "up"); break;
    case CSKEY_DOWN: strcat (buf, "down"); break;
    case CSKEY_RIGHT: strcat (buf, "right"); break;
    case CSKEY_LEFT: strcat (buf, "left"); break;
    case CSKEY_PGUP: strcat (buf, "pgup"); break;
    case CSKEY_PGDN: strcat (buf, "pgdn"); break;
    case CSKEY_HOME: strcat (buf, "home"); break;
    case CSKEY_END: strcat (buf, "end"); break;
    case CSKEY_INS: strcat (buf, "ins"); break;
    case CSKEY_DEL: strcat (buf, "del"); break;
    case CSKEY_F1: strcat (buf, "f1"); break;
    case CSKEY_F2: strcat (buf, "f2"); break;
    case CSKEY_F3: strcat (buf, "f3"); break;
    case CSKEY_F4: strcat (buf, "f4"); break;
    case CSKEY_F5: strcat (buf, "f5"); break;
    case CSKEY_F6: strcat (buf, "f6"); break;
    case CSKEY_F7: strcat (buf, "f7"); break;
    case CSKEY_F8: strcat (buf, "f8"); break;
    case CSKEY_F9: strcat (buf, "f9"); break;
    case CSKEY_F10: strcat (buf, "f10"); break;
    case CSKEY_F11: strcat (buf, "f11"); break;
    case CSKEY_F12: strcat (buf, "f12"); break;
    default:
    {
      char* s = strchr (buf, 0);
      *s++ = map.key;
      *s = 0;
    }
  }
  return buf;
}

void Blocks::ReadConfig ()
{
  iConfigFile *keys = System->GetConfig();
  NamedKey (keys->GetStr ("Blocks.Keys.Up", "up"), key_up);
  NamedKey (keys->GetStr ("Blocks.Keys.Down", "down"), key_down);
  NamedKey (keys->GetStr ("Blocks.Keys.Left", "left"), key_left);
  NamedKey (keys->GetStr ("Blocks.Keys.Right", "right"), key_right);
  NamedKey (keys->GetStr ("Blocks.Keys.RotPX", "q"), key_rotpx);
  NamedKey (keys->GetStr ("Blocks.Keys.RotMX", "a"), key_rotmx);
  NamedKey (keys->GetStr ("Blocks.Keys.RotPY", "w"), key_rotpy);
  NamedKey (keys->GetStr ("Blocks.Keys.RotMY", "s"), key_rotmy);
  NamedKey (keys->GetStr ("Blocks.Keys.RotPZ", "e"), key_rotpz);
  NamedKey (keys->GetStr ("Blocks.Keys.RotMZ", "d"), key_rotmz);
  NamedKey (keys->GetStr ("Blocks.Keys.Pause", "p"), key_pause);
  NamedKey (keys->GetStr ("Blocks.Keys.ESC", "esc"), key_esc);
  NamedKey (keys->GetStr ("Blocks.Keys.Drop", "space"), key_drop);
  NamedKey (keys->GetStr ("Blocks.Keys.ViewLeft", "del"), key_viewleft);
  NamedKey (keys->GetStr ("Blocks.Keys.ViewRight", "pgdn"), key_viewright);
  NamedKey (keys->GetStr ("Blocks.Keys.ViewUp", "home"), key_viewup);
  NamedKey (keys->GetStr ("Blocks.Keys.ViewDown", "end"), key_viewdown);
  NamedKey (keys->GetStr ("Blocks.Keys.ZoomIn", "ins"), key_zoomin);
  NamedKey (keys->GetStr ("Blocks.Keys.ZoomOut", "pgup"), key_zoomout);
  player1->new_zone_dim = keys->GetInt ("Blocks.Game.PlaySize", 5);
  player1->zone_dim = player1->new_zone_dim;
  diff_level = keys->GetInt ("Blocks.Game.Level", 0);
  int level, size, i;
  for (level = 0 ; level <= 2 ; level++)
    for (size = 3 ; size <= 6 ; size++)
    {
      char key[50];
      for (i = 0 ; i < 10 ; i++)
      {
        sprintf (key, "Blocks.HighScores.Score%d_%dx%d_%d", level, size, size, i);
        highscores[level][size-3].Set (i, keys->GetInt (key, -1));
        sprintf (key, "Blocks.HighScores.Name%d_%dx%d_%d", level, size, size, i);
        highscores[level][size-3].SetName (i,
	  keys->GetStr (key, NULL));
      }
    }
#if defined(BLOCKS_NETWORKING)
  // Network stuff.
  IsServer = Config->GetYesNo ("Blocks.Network.Server", false);
#endif
}

void Blocks::WriteConfig ()
{
  iConfigFile *keys = System->GetConfig();
  keys->SetStr ("Blocks.Keys.Up", KeyName (key_up));
  keys->SetStr ("Blocks.Keys.Down", KeyName (key_down));
  keys->SetStr ("Blocks.Keys.Left", KeyName (key_left));
  keys->SetStr ("Blocks.Keys.Right", KeyName (key_right));
  keys->SetStr ("Blocks.Keys.RotPX", KeyName (key_rotpx));
  keys->SetStr ("Blocks.Keys.RotMX", KeyName (key_rotmx));
  keys->SetStr ("Blocks.Keys.RotPY", KeyName (key_rotpy));
  keys->SetStr ("Blocks.Keys.RotMY", KeyName (key_rotmy));
  keys->SetStr ("Blocks.Keys.RotPZ", KeyName (key_rotpz));
  keys->SetStr ("Blocks.Keys.RotMZ", KeyName (key_rotmz));
  keys->SetStr ("Blocks.Keys.Pause", KeyName (key_pause));
  keys->SetStr ("Blocks.Keys.ESC", KeyName (key_esc));
  keys->SetStr ("Blocks.Keys.Drop", KeyName (key_drop));
  keys->SetStr ("Blocks.Keys.ViewLeft", KeyName (key_viewleft));
  keys->SetStr ("Blocks.Keys.ViewRight", KeyName (key_viewright));
  keys->SetStr ("Blocks.Keys.ViewUp", KeyName (key_viewup));
  keys->SetStr ("Blocks.Keys.ViewDown", KeyName (key_viewdown));
  keys->SetStr ("Blocks.Keys.ZoomIn", KeyName (key_zoomin));
  keys->SetStr ("Blocks.Keys.ZoomOut", KeyName (key_zoomout));
  keys->SetInt ("Blocks.Game.PlaySize", player1->new_zone_dim);
  keys->SetInt ("Blocks.Game.Level", diff_level);
  int level, size, i;
  for (level = 0 ; level <= 2 ; level++)
    for (size = 3 ; size <= 6 ; size++)
    {
      char key[50];
      for (i = 0 ; i < 10 ; i++)
        if (highscores[level][size-3].Get (i) != -1)
        {
          sprintf (key, "Blocks.HighScores.Score%d_%dx%d_%d", level, size, size, i);
          keys->SetInt (key, highscores[level][size-3].Get (i));
          sprintf (key, "Blocks.HighScores.Name%d_%dx%d_%d", level, size, size, i);
	  keys->SetStr(key,highscores[level][size-3].GetName (i));
        }
    }
  keys->Save ();
}

// -----------------------------------------------------------------
// Network Code
// TODO: add the server CheckConnection stuff in here.

#if defined(BLOCKS_NETWORKING)

void Blocks::CheckConnection()
{
  if (!do_network) return;

  if(IsServer)
    ServerCheckConnection();
  else
    ClientCheckConnection();
}

void Blocks::ClientCheckConnection()
{
  // The buffer is the length of the encoded state plus the STARTSTATE, 
  //  and ENDSTATE parts.
  const int BUFF_SIZE = ST_ENCODED_LENGTH + ST_CLIENT_EXTRA;
  unsigned char buff[BUFF_SIZE];
  const int received = Connection->Receive(buff, BUFF_SIZE);
  if (received > 0)
  {
    if (strcmp((char *)buff, "BYE") == 0)
    {
      Sys->Printf(MSG_INITIALIZATION, "Server disconnected.\n");
      TerminateConnection();
    }
    else
    {
      //Sys->Printf(MSG_INITIALIZATION, "Server responds: %s\n", buff);
      
      
      if(!player1_net->DecodeFromNetwork(buff,
				       (ST_ENCODED_LENGTH + ST_CLIENT_EXTRA),
				       player1))
      {
        printf("CLIENT: Network don't work, decoding\n");
      }
      else
      {
        printf("CLIENT: eh!! it did work\n");
      }
    }
  }
  else
  {
    const csNetworkDriverError err = Connection->GetLastError();
    if (err != CS_NET_ERR_NO_ERROR)
    {
      Sys->Printf(MSG_INITIALIZATION, "Receive error %d\n", err);
      Connection->DecRef();
      Connection = NULL;
    }
  }
}

void Blocks::ServerCheckConnection()
{

  const int BUFF_SIZE = ST_ENCODED_LENGTH + ST_SERVER_EXTRA;
  unsigned char buff[BUFF_SIZE];
  const int received = Connection->Receive(buff, BUFF_SIZE);
  if (received > 0)
  {
    if (strcmp((char *)buff, "BYE") == 0)
    {
      Sys->Printf(MSG_INITIALIZATION, "Other blocks disconnected.\n");
      TerminateConnection();
      InitNet();
    }
    else if (strcmp((char *)buff, "OK") == 0)
    {
      Sys->Printf(MSG_INITIALIZATION, "Received data: %s\n", buff);
      Connection->Send("OK", sizeof("OK"));
    }
    
    else
    {
      Sys->Printf(MSG_INITIALIZATION, "Other blocks responds: %s\n", buff);
      Connection->Send("You", sizeof("You"));
    }
  }
  else
  {
    const csNetworkDriverError err = Connection->GetLastError();
    if (err != CS_NET_ERR_NO_ERROR)
    {
      Sys->Printf(MSG_INITIALIZATION, "Receive error %d\n", err);
      Connection->DecRef();
      Connection = NULL;
    }
    
//    Connection->Send("You are a poo poo", sizeof("You are a poo poo"));

    // Encode the state.
    player1->EncodeStates();
    
    if(!player1_net->EncodeForNetwork(player1->encodedData, buff,
				      ST_ENCODED_LENGTH, 
                                      (ST_ENCODED_LENGTH + ST_CLIENT_EXTRA)))
    {
      printf("SERVER:Network don't work\n");
    }
    else  // send the data.
    {
      Connection->Send(buff, (ST_ENCODED_LENGTH +ST_CLIENT_EXTRA));

    }
  }
}

bool Blocks::InitNet()
{
  LastConnectTime = Sys->Time ();
  
  if (IsServer)
  {
    const char source[] = "2222";

    if (!System->NetDrv) return false;
    Listener = System->NetDrv->NewListener (source, true, false, false);
    if (Listener != NULL)
      Sys->Printf (MSG_INITIALIZATION, "Listening on port %s\n", source);
    else
      Sys->Printf (MSG_INITIALIZATION,"Error creating network listener (%d)\n",
        Sys->NetDrv->GetLastError ());

    return (Listener != NULL);
  }
  else  // We are the client.
  {
    //LastConnectTime = Sys->Time ();
    return true;
  }
  return true;
}

void Blocks::Connect ()
{
  const char target[] = "localhost:2222";
  Connection = NULL;
  if (!Sys->NetDrv) return;
  Sys->Printf(MSG_INITIALIZATION, "Attempting connection to %s...", target);
  Connection = Sys->NetDrv->NewConnection(target, true, false);
  if (Connection == NULL)
    Sys->Printf (MSG_INITIALIZATION,"Error %d\n", Sys->NetDrv->GetLastError());
  else
    Sys->Printf(MSG_INITIALIZATION, "OK\nPress a key [A-Z] to send a"
      "message to the server.\n");
}

void Blocks::TerminateConnection()
{
  if (Connection != NULL)
  {
    Connection->Terminate();
    Connection->DecRef();
    Connection = NULL;
  }
  if (Listener != NULL)
  {
    Listener->Terminate();
    Listener->DecRef();
    Listener = NULL;
  }
}

#endif // BLOCKS_NETWORKING

void cleanup ()
{
  Sys->console_out ("Cleaning up...\n");
  delete view;
  delete Sys;
  Sys = NULL;
}

//----------------------------------------------------------------------------
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class which is the driver for Blocks.
  Sys = new Blocks ();

  // temp hack until we find a better way
  csEngine::System = Sys;

  if (!Sys->Initialize (argc, argv, "/config/blocks.cfg"))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    cleanup ();
    fatal_exit (0, false);
  }

  // Open the main system. This will open all the previously loaded plug-ins.
  if (!Sys->Open ("3D Blocks"))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    fatal_exit (0, false);
  }

  // Find the pointer to engine plugin
  iEngine *engine = QUERY_PLUGIN (Sys, iEngine);
  if (!engine)
  {
    CsPrintf (MSG_FATAL_ERROR, "No iEngine plugin!\n");
    return -1;
  }
  Sys->engine = engine->GetCsEngine ();
  engine->DecRef ();

  // Get a font handle
  Sys->font = Sys->G2D->GetFontServer ()->LoadFont (CSFONT_LARGE);

  // Some settings.
  Gfx3D->SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, (long)false);

  // Some commercials...
  Sys->Printf (MSG_INITIALIZATION, "3D Blocks version 1.0.\n");
  Sys->Printf (MSG_INITIALIZATION,
    "Created by Jorrit Tyberghein and others...\n\n");
  Sys->txtmgr = Gfx3D->GetTextureManager ();
  Sys->txtmgr->SetVerbose (true);

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.

  view = new csView (Sys->engine, Gfx3D);

  // Create our world.
  Sys->Printf (MSG_INITIALIZATION, "Creating world!...\n");
  Sys->engine->EnableLightingCache (false);

  // Change to virtual directory where Blocks data is stored
  //if (!)

  csString world_file(Sys->Config->GetStr ("Blocks.Data", "/data/blocks"));
  world_file.Append("/");
  if (!Sys->VFS->Exists (world_file.GetData()))
  {
    Sys->Printf (MSG_FATAL_ERROR,
      "The directory on VFS (%s) for world file does not exist!\n",
      world_file.GetData());
    return -1;
  }

  Sys->VFS->ChDir (world_file.GetData());
  Sys->ReadConfig ();
  Sys->InitEngine ();

  Sys->txtmgr->SetPalette ();
  Sys->white = Sys->txtmgr->FindRGB (255, 255, 255);
  Sys->black = Sys->txtmgr->FindRGB (0, 0, 0);
  Sys->red = Sys->txtmgr->FindRGB (255, 0, 0);

#if defined(BLOCKS_NETWORKING)
  do_network = Sys->InitNet();
#endif
  Sys->Loop ();
  cleanup ();
  return 0;
}
