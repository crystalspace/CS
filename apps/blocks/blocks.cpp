/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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
    Print out score.
    Detect end of game when blocks are too high.
    Better rotation of blocks.
    Better textures.
    Better recognition of side of play area.
    Make keys configurable.
    Fix bug with partially shifted blocks.
    Add highscore list.
    Add 'nightmare' level.
 */

#define SYSDEF_ACCESS
#include "sysdef.h"
#include "cssys/system.h"
#include "csparser/csloader.h"
#include "apps/blocks/blocks.h"
#include "csutil/inifile.h"
#include "csutil/vfs.h"
#include "csgfxldr/csimage.h"
#include "csgfxldr/gifimage.h"
#include "csengine/dumper.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"
#include "csengine/world.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "csengine/thing.h"
#include "csengine/thingtpl.h"
#include "csengine/textrans.h"
#include "csengine/csview.h"
#include "igraph3d.h"
#include "itxtmgr.h"

Blocks* Sys = NULL;
csView* view = NULL;

#define Gfx3D System->G3D
#define Gfx2D System->G2D

//------------------------------ We need the VFS plugin and the 3D engine -----

REGISTER_STATIC_LIBRARY (vfs)
REGISTER_STATIC_LIBRARY (engine)

//-----------------------------------------------------------------------------

Blocks::Blocks ()
{
  world = NULL;

  full_rotate_x = create_rotate_x (M_PI/2);
  full_rotate_y = create_rotate_y (M_PI/2);
  full_rotate_z = create_rotate_z (M_PI/2);
  full_rotate_x_reverse = create_rotate_x (-M_PI/2);
  full_rotate_y_reverse = create_rotate_y (-M_PI/2);
  full_rotate_z_reverse = create_rotate_z (-M_PI/2);
  
  pause = false;

  destinations[0][0] = csVector3 (0, 3, -5);
  destinations[1][0] = csVector3 (5, 3, 0);
  destinations[2][0] = csVector3 (0, 3, 5);
  destinations[3][0] = csVector3 (-5, 3, 0);
  destinations[0][1] = csVector3 (0, 6, -5);
  destinations[1][1] = csVector3 (5, 6, 0);
  destinations[2][1] = csVector3 (0, 6, 5);
  destinations[3][1] = csVector3 (-5, 6, 0);
  destinations[0][2] = csVector3 (0, 8, -5);
  destinations[1][2] = csVector3 (5, 8, 0);
  destinations[2][2] = csVector3 (0, 8, 5);
  destinations[3][2] = csVector3 (-5, 8, 0);
  destinations[0][3] = csVector3 (0, 10, -2);
  destinations[1][3] = csVector3 (2, 10, 0);
  destinations[2][3] = csVector3 (0, 10, 2);
  destinations[3][3] = csVector3 (-2, 10, 0);
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

  newgame = true;
  startup_screen = true;
  dynlight = NULL;
}

Blocks::~Blocks ()
{
  delete dynlight;
  if (world) world->Clear ();
}

void Blocks::init_game ()
{
  int i, j, k;
  for (k = 0 ; k < ZONE_SAFETY+ZONE_HEIGHT+ZONE_SAFETY ; k++)
    for (j = 0 ; j < ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY ; j++)
      for (i = 0 ; i < ZONE_SAFETY+ZONE_DIM+ZONE_SAFETY ; i++)
        game_cube[i][j][k] =
	  i < ZONE_SAFETY || j < ZONE_SAFETY || k < ZONE_SAFETY ||
	  i >= ZONE_SAFETY+ZONE_DIM || j >= ZONE_SAFETY+ZONE_DIM ||
	  k >= ZONE_SAFETY+ZONE_HEIGHT;
  transition = false;
  newgame = false;
  score = 0;
  rot_px_todo = 0;
  rot_py_todo = 0;
  rot_pz_todo = 0;
  rot_mx_todo = 0;
  rot_my_todo = 0;
  rot_mz_todo = 0;
  move_hor_todo = 0;
  cam_move_dist = 0;

  cur_hor_dest = 0;
  cur_ver_dest = 2;
  move_right_dx = dest_move_right_dx[cur_hor_dest];
  move_right_dy = dest_move_right_dy[cur_hor_dest];
  move_down_dx = dest_move_down_dx[cur_hor_dest];
  move_down_dy = dest_move_down_dy[cur_hor_dest];
  cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
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

void Blocks::add_pilar_template ()
{
  float dim = CUBE_DIM/2.;
  pilar_tmpl = new csThingTemplate ();
  pilar_tmpl->SetName ("pilar");
  pilar_tmpl->AddVertex (-dim, 0, dim);
  pilar_tmpl->AddVertex (dim, 0, dim);
  pilar_tmpl->AddVertex (dim, 0, -dim);
  pilar_tmpl->AddVertex (-dim, 0, -dim);
  pilar_tmpl->AddVertex (-dim, ZONE_HEIGHT*CUBE_DIM, dim);
  pilar_tmpl->AddVertex (dim, ZONE_HEIGHT*CUBE_DIM, dim);
  pilar_tmpl->AddVertex (dim, ZONE_HEIGHT*CUBE_DIM, -dim);
  pilar_tmpl->AddVertex (-dim, ZONE_HEIGHT*CUBE_DIM, -dim);

  csPolygonTemplate* p;
  float A, B, C;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = new csPolygonTemplate (pilar_tmpl, "d", pilar_txt);
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (3);
  p->AddVertex (2);
  p->AddVertex (1);
  p->AddVertex (0);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (0), pilar_tmpl->Vtex (1), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (pilar_tmpl, "b", pilar_txt);
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (0);
  p->AddVertex (1);
  p->AddVertex (5);
  p->AddVertex (4);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (0), pilar_tmpl->Vtex (1), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (pilar_tmpl, "t", pilar_txt);
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (5);
  p->AddVertex (6);
  p->AddVertex (7);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (4), pilar_tmpl->Vtex (5), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (pilar_tmpl, "f", pilar_txt);
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (7);
  p->AddVertex (6);
  p->AddVertex (2);
  p->AddVertex (3);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (7), pilar_tmpl->Vtex (6), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (pilar_tmpl, "l", pilar_txt);
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (7);
  p->AddVertex (3);
  p->AddVertex (0);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (7), pilar_tmpl->Vtex (3), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (pilar_tmpl, "r", pilar_txt);
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (6);
  p->AddVertex (5);
  p->AddVertex (1);
  p->AddVertex (2);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (6), pilar_tmpl->Vtex (5), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  world->thing_templates.Push (pilar_tmpl);
}

void Blocks::add_pilar (int x, int y)
{
  csThing* pilar;
  pilar = new csThing ();
  pilar->SetName ("pilar");
  pilar->SetSector (room);
  pilar->SetFlags (CS_ENTITY_MOVEABLE, 0);
  pilar->MergeTemplate (pilar_tmpl, pilar_txt, 1);
  room->AddThing (pilar);
  csVector3 v ((x-ZONE_DIM/2)*CUBE_DIM, 0, (y-ZONE_DIM/2)*CUBE_DIM);
  pilar->SetMove (room, v);
  pilar->Transform ();
}


void Blocks::add_cube_template ()
{
  float dim = CUBE_DIM/2.;
  cube_tmpl = new csThingTemplate ();
  cube_tmpl->SetName ("cube");
  cube_tmpl->AddVertex (-dim, -dim, dim);
  cube_tmpl->AddVertex (dim, -dim, dim);
  cube_tmpl->AddVertex (dim, -dim, -dim);
  cube_tmpl->AddVertex (-dim, -dim, -dim);
  cube_tmpl->AddVertex (-dim, dim, dim);
  cube_tmpl->AddVertex (dim, dim, dim);
  cube_tmpl->AddVertex (dim, dim, -dim);
  cube_tmpl->AddVertex (-dim, dim, -dim);

  csPolygonTemplate* p;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  p = new csPolygonTemplate (cube_tmpl, "d1", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (3);
  p->AddVertex (2);
  p->AddVertex (1);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "d2", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (3);
  p->AddVertex (1);
  p->AddVertex (0);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "b1", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (0);
  p->AddVertex (1);
  p->AddVertex (5);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "b2", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (0);
  p->AddVertex (5);
  p->AddVertex (4);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "t1", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (5);
  p->AddVertex (6);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "t2", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (6);
  p->AddVertex (7);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "f1", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (7);
  p->AddVertex (6);
  p->AddVertex (2);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "f2", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (7);
  p->AddVertex (2);
  p->AddVertex (3);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "l1", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (7);
  p->AddVertex (3);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "l2", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (3);
  p->AddVertex (0);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "r1", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (6);
  p->AddVertex (5);
  p->AddVertex (1);
  p->SetTextureSpace (tx_matrix, tx_vector);

  p = new csPolygonTemplate (cube_tmpl, "r2", cube_txt);
  cube_tmpl->AddPolygon (p);
  p->AddVertex (6);
  p->AddVertex (1);
  p->AddVertex (2);
  p->SetTextureSpace (tx_matrix, tx_vector);

  world->thing_templates.Push (cube_tmpl);
}

void set_uv (csPolygon3D* p, float u1, float v1, float u2, float v2,
	float u3, float v3)
{
  p->SetTextureType (POLYTXT_GOURAUD);
  csGouraudShaded* gs = p->GetGouraudInfo ();
  gs->Setup (p->GetVertices ().GetNumVertices ());
  gs->EnableGouraud (true);
  gs->SetUV (0, u1, v1);
  gs->SetUV (1, u2, v2);
  gs->SetUV (2, u3, v3);
}

csThing* Blocks::create_cube_thing (int dx, int dy, int dz)
{
  csThing* cube;
  cube = new csThing ();
  cube->SetName ("cubexxx");
  cube->SetSector (room);
  cube->SetFlags (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
  csVector3 shift (dx*CUBE_DIM, dz*CUBE_DIM, dy*CUBE_DIM);
  cube->MergeTemplate (cube_tmpl, cube_txt, 1, NULL, &shift, NULL);

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

void Blocks::add_cube (int dx, int dy, int dz, int x, int y, int z)
{
  csThing* cube = add_cube_thing (room, dx, dy, dz,
  	(x-ZONE_DIM/2)*CUBE_DIM,
	z*CUBE_DIM+CUBE_DIM/2,
  	(y-ZONE_DIM/2)*CUBE_DIM);
  cube_info[num_cubes].thing = cube;
  cube_info[num_cubes].dx = dx;
  cube_info[num_cubes].dy = dy;
  cube_info[num_cubes].dz = dz;
  num_cubes++;
}

csThing* Blocks::add_cube_thing (csSector* sect, int dx, int dy, int dz,
	float x, float y, float z)
{
  csThing* cube = create_cube_thing (dx, dy, dz);
  sect->AddThing (cube);
  csVector3 v (x, y, z);
  cube->SetMove (sect, v);
  cube->Transform ();
  cube->InitLightMaps (false);
  room->ShineLights (cube);
  cube->CreateLightMaps (Gfx3D);
  return cube;
}

void Blocks::start_shape (BlShapeType type, int x, int y, int z)
{
  num_cubes = 0;
  switch (type)
  {
    case SHAPE_R1:
      add_cube (0, 0, 0, x, y, z);
      break;
    case SHAPE_R2:
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      break;
    case SHAPE_R3:
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      break;
    case SHAPE_R4:
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (2, 0, 0, x, y, z);
      break;
    case SHAPE_L1:
      add_cube (-1, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      break;
    case SHAPE_L2:
      add_cube (-1, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      break;
    case SHAPE_T1:
      add_cube (0, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      break;
    case SHAPE_T2:
      add_cube (0, 0, 2, x, y, z);
      add_cube (0, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      break;
    case SHAPE_FLAT:
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (0, 1, 0, x, y, z);
      add_cube (1, 1, 0, x, y, z);
      break;
    case SHAPE_CUBE:
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (0, 1, 0, x, y, z);
      add_cube (1, 1, 0, x, y, z);
      add_cube (0, 0, 1, x, y, z);
      add_cube (1, 0, 1, x, y, z);
      add_cube (0, 1, 1, x, y, z);
      add_cube (1, 1, 1, x, y, z);
      break;
    case SHAPE_U:
      add_cube (-1, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (1, 0, 1, x, y, z);
      break;
    case SHAPE_S:
      add_cube (-1, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (1, 0, -1, x, y, z);
      break;
    case SHAPE_L3:
      add_cube (-1, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (2, 0, 0, x, y, z);
      break;
    case SHAPE_T1X:
      add_cube (0, 0, 1, x, y, z);
      add_cube (-1, 0, 0, x, y, z);
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (0, 1, 0, x, y, z);
      break;
    case SHAPE_FLATX:
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (0, 1, 0, x, y, z);
      add_cube (1, 1, 0, x, y, z);
      add_cube (0, 0, 1, x, y, z);
      break;
    case SHAPE_FLATXX:
      add_cube (0, 0, 0, x, y, z);
      add_cube (1, 0, 0, x, y, z);
      add_cube (0, 1, 0, x, y, z);
      add_cube (1, 1, 0, x, y, z);
      add_cube (0, 0, 1, x, y, z);
      add_cube (1, 1, 1, x, y, z);
      break;
    default: break;
  }
  move_down_todo = 0;
  cube_x = x;
  cube_y = y;
  cube_z = z;
  speed = .2;
}

void Blocks::start_demo_shape (BlShapeType type, float x, float y, float z)
{
  switch (type)
  {
    case SHAPE_DEMO_B:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z);
      add_cube_thing (demo_room,  0, 0,  0, x, y, z);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z);
      add_cube_thing (demo_room,  1, 0, -1, x, y, z);
      add_cube_thing (demo_room,  1, 0,  1, x, y, z);
      break;
    case SHAPE_DEMO_L:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z);
      add_cube_thing (demo_room,  0, 0,  -2, x, y, z);
      add_cube_thing (demo_room,  1, 0,  -2, x, y, z);
      break;
    case SHAPE_DEMO_O:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z);
      add_cube_thing (demo_room,  1, 0, -2, x, y, z);
      add_cube_thing (demo_room,  1, 0, -1, x, y, z);
      add_cube_thing (demo_room,  1, 0,  0, x, y, z);
      add_cube_thing (demo_room,  1, 0,  1, x, y, z);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z);
      break;
    case SHAPE_DEMO_C:
      add_cube_thing (demo_room, -1, 0, -1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z);
      add_cube_thing (demo_room,  1, 0, -2, x, y, z);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z);
      break;
    case SHAPE_DEMO_K:
      add_cube_thing (demo_room, -1, 0, -2, x, y, z);
      add_cube_thing (demo_room, -1, 0, -1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z);
      add_cube_thing (demo_room, -1, 0,  2, x, y, z);
      add_cube_thing (demo_room,  0, 0, -1, x, y, z);
      add_cube_thing (demo_room,  0, 0,  1, x, y, z);
      add_cube_thing (demo_room,  1, 0, -2, x, y, z);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z);
      break;
    case SHAPE_DEMO_S:
      add_cube_thing (demo_room,  1, 0, -1, x, y, z);
      add_cube_thing (demo_room,  1, 0,  0, x, y, z);
      add_cube_thing (demo_room,  1, 0,  2, x, y, z);
      add_cube_thing (demo_room,  0, 0, -2, x, y, z);
      add_cube_thing (demo_room,  0, 0,  0, x, y, z);
      add_cube_thing (demo_room,  0, 0,  2, x, y, z);
      add_cube_thing (demo_room, -1, 0, -2, x, y, z);
      add_cube_thing (demo_room, -1, 0,  0, x, y, z);
      add_cube_thing (demo_room, -1, 0,  1, x, y, z);
      break;
    default: break;
  }
}

void Blocks::start_rotation (BlRotType type)
{
  if (rot_px_todo || rot_mx_todo || rot_py_todo ||
  	rot_my_todo || rot_pz_todo || rot_mz_todo || move_hor_todo)
    return;
  switch (type)
  {
    case ROT_PX:
      if (!check_new_shape_rotation (full_rotate_x)) return;
      rot_px_todo = M_PI/2;
      break;
    case ROT_MX:
      if (!check_new_shape_rotation (full_rotate_x_reverse)) return;
      rot_mx_todo = M_PI/2;
      break;
    case ROT_PY:
      if (!check_new_shape_rotation (full_rotate_z)) return;
      rot_py_todo = M_PI/2;
      break;
    case ROT_MY:
      if (!check_new_shape_rotation (full_rotate_z_reverse)) return;
      rot_my_todo = M_PI/2;
      break;
    case ROT_PZ:
      if (!check_new_shape_rotation (full_rotate_y)) return;
      rot_pz_todo = M_PI/2;
      break;
    case ROT_MZ:
      if (!check_new_shape_rotation (full_rotate_y_reverse)) return;
      rot_mz_todo = M_PI/2;
      break;
  }
}

void Blocks::start_horizontal_move (int dx, int dy)
{
  if (rot_px_todo || rot_mx_todo || rot_py_todo || rot_my_todo ||
  	rot_pz_todo || rot_mz_todo || move_hor_todo)
    return;
  if (!check_new_shape_location (dx, dy, 0)) return;
  move_hor_todo = CUBE_DIM;
  move_hor_dx = dx;
  move_hor_dy = dy;
}

void Blocks::move_camera ()
{
  csVector3 pos = cam_move_dist*cam_move_src + (1-cam_move_dist)*cam_move_dest;
  view->GetCamera ()->SetPosition (pos);
  view->GetCamera ()->LookAt (view_origin-pos, cam_move_up);
}

void Blocks::eatkeypress (int key, bool /*shift*/, bool /*alt*/, bool /*ctrl*/)
{
  if (startup_screen)
  {
    switch (key)
    {
      case '1':
        difficulty = NUM_EASY_SHAPE;
	startup_screen = false;
	newgame = true;
	break;
      case '2':
        difficulty = NUM_MEDIUM_SHAPE;
	startup_screen = false;
	newgame = true;
	break;
      case '3':
        difficulty = NUM_HARD_SHAPE;
	startup_screen = false;
	newgame = true;
	break;
      case CSKEY_ESC: System->Shutdown = true; break;
    }
    return;
  }

  switch (key)
  {
    case 'u':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      cur_hor_dest = (cur_hor_dest-1+4)%4;
      cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
      cam_move_up = csVector3 (0, -1, 0);
      move_right_dx = dest_move_right_dx[cur_hor_dest];
      move_right_dy = dest_move_right_dy[cur_hor_dest];
      move_down_dx = dest_move_down_dx[cur_hor_dest];
      move_down_dy = dest_move_down_dy[cur_hor_dest];
      break;
    case 'o':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      cur_hor_dest = (cur_hor_dest+1)%4;
      cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
      cam_move_up = csVector3 (0, -1, 0);
      move_right_dx = dest_move_right_dx[cur_hor_dest];
      move_right_dy = dest_move_right_dy[cur_hor_dest];
      move_down_dx = dest_move_down_dx[cur_hor_dest];
      move_down_dy = dest_move_down_dy[cur_hor_dest];
      break;
    case 'h':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      if (cur_ver_dest > 0) cur_ver_dest--;
      cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case 'y':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      if (cur_ver_dest < 3) cur_ver_dest++;
      cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case ';':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      cam_move_dest = cam_move_src + .3 * (view_origin - cam_move_src);
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case 'a':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      cam_move_dest = cam_move_src - .3 * (view_origin - cam_move_src);
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case 'w': start_rotation (ROT_PX); break;
    case 's': start_rotation (ROT_MX); break;
    case 'e': start_rotation (ROT_PY); break;
    case 'd': start_rotation (ROT_MY); break;
    case 'r': start_rotation (ROT_PZ); break;
    case 'f': start_rotation (ROT_MZ); break;
    case 'i': start_horizontal_move (-move_down_dx, -move_down_dy); break;
    case 'k': start_horizontal_move (move_down_dx, move_down_dy); break;
    case 'j': start_horizontal_move (-move_right_dx, -move_right_dy); break;
    case 'l': start_horizontal_move (move_right_dx, move_right_dy); break;
    case ' ': if (speed == 9) speed = 0.2; // Space changes speeds now.
	      else speed = 9; break;
    case 'p': pause = !pause; break;
    case CSKEY_ESC: newgame = true; startup_screen = true; break;
  }
}

void Blocks::move_shape_internal (int dx, int dy, int dz)
{
  cube_x += dx;
  cube_y += dy;
  cube_z += dz;
}

void Blocks::rotate_shape_internal (const csMatrix3& rot)
{
  int i;

  for (i = 0 ; i < num_cubes ; i++)
  {
    csVector3 v;
    v.x = (float)cube_info[i].dx;
    v.y = (float)cube_info[i].dy;
    v.z = (float)cube_info[i].dz;
    v = rot * v;

    if (v.x < 0) cube_info[i].dx = (int)(v.x-.5);
    else cube_info[i].dx = (int)(v.x+.5);
    if (v.y < 0) cube_info[i].dy = (int)(v.y-.5);
    else cube_info[i].dy = (int)(v.y+.5);
    if (v.z < 0) cube_info[i].dz = (int)(v.z-.5);
    else cube_info[i].dz = (int)(v.z+.5);
  }
}

void Blocks::freeze_shape ()
{
  int i;
  int x, y, z;
  char cubename[20];

  for (i = 0 ; i < num_cubes ; i++)
  {
    x = cube_x+cube_info[i].dx;
    y = cube_y+cube_info[i].dy;
    z = cube_z+cube_info[i].dz;
    set_cube (x, y, z, true);
    sprintf (cubename, "cubeAt%d%d%d", x, y, z);
    // Before we let go of the shape (lose the pointer to it) we set it's
    // name according to it's position.
    cube_info[i].thing->SetName (cubename); 
  }
}

void Blocks::dump_shape ()
{
  int i;
  int x, y, z;
  CsPrintf (MSG_DEBUG_0,"Dump shape:\n");
  for (i = 0 ; i < num_cubes ; i++)
  {
    x = cube_info[i].dx;
    y = cube_info[i].dy;
    z = cube_info[i].dz;
    CsPrintf (MSG_DEBUG_0, " %d: (%d,%d,%d) d=(%d,%d,%d)\n",
    	i, cube_x+x, cube_y+y, cube_z+z, x, y, z);
  }
}

bool Blocks::check_new_shape_location (int dx, int dy, int dz)
{
  int i;
  int x, y, z;
  for (i = 0 ; i < num_cubes ; i++)
  {
    x = cube_x+cube_info[i].dx + dx;
    y = cube_y+cube_info[i].dy + dy;
    z = cube_z+cube_info[i].dz + dz;
    if (get_cube (x, y, z)) return false;
  }
  return true;
}

bool Blocks::check_new_shape_rotation (const csMatrix3& rot)
{
  int i;
  int x, y, z;
  int dx, dy, dz;
  for (i = 0 ; i < num_cubes ; i++)
  {
    csVector3 v;
    v.x = (float)cube_info[i].dx;
    v.y = (float)cube_info[i].dy;
    v.z = (float)cube_info[i].dz;
    v = rot * v;
    if (v.x < 0) dx = (int)(v.x-.5);
    else dx = (int)(v.x+.5);
    if (v.y < 0) dy = (int)(v.y-.5);
    else dy = (int)(v.y+.5);
    if (v.z < 0) dz = (int)(v.z-.5);
    else dz = (int)(v.z+.5);
    x = cube_x + dx;
    y = cube_y + dy;
    z = cube_z + dz;
    if (get_cube (x, y, z)) return false;
  }
  return true;
}

void reset_vertex_colors (csThing* th)
{
  int i;
  for (i = 0 ; i < th->GetNumPolygons () ; i++)
  {
    csPolygon3D* p = (csPolygon3D*)(th->GetPolygon (i));
    p->UpdateVertexLighting (NULL, csColor (0, 0, 0), true, true);
    p->UpdateVertexLighting (NULL, csColor (0, 0, 0), false, true);
  }
}

void Blocks::updateScore (void)
{
  int increase = 0;
  int i;
	
  for (i=0 ; i<ZONE_HEIGHT ; i++)
  {
    if (filled_planes[i])
    {
      increase++;
    }
  }

  score+=increase*increase;
}

void Blocks::move_cubes (time_t elapsed_time)
{
  int i;
  float elapsed = (float)elapsed_time/1000.;
  float elapsed_rot = 3 * elapsed * (M_PI/2);
  float elapsed_fall = elapsed*speed;
  float elapsed_move = elapsed*1.6;
  float elapsed_cam_move = elapsed*1.6;

  if (startup_screen)
  {
    float old_dyn_x = dynlight_x;
    dynlight_x += dynlight_dx*elapsed;
    if (dynlight_x > 4 || dynlight_x < -4)
    {
      dynlight_dx = -dynlight_dx;
      dynlight_x = old_dyn_x;
    }
    dynlight->Move (demo_room, dynlight_x, dynlight_y, dynlight_z);
    dynlight->Setup ();
    return;
  }

  if (cam_move_dist)
  {
    if (elapsed_cam_move > cam_move_dist) elapsed_cam_move = cam_move_dist;
    cam_move_dist -= elapsed_cam_move;
    move_camera ();
  }

  if (pause) return;

  if (!move_down_todo)
  {
    // dump_shape ();
    bool stop = !check_new_shape_location (0, 0, -1);
    if (stop)
    {
      freeze_shape ();
      checkForPlane ();
      if (!transition)
        start_shape ((BlShapeType)(rand () % difficulty), 3, 3, ZONE_HEIGHT-3);
      return;
    }
    else
    {
      move_shape_internal (0, 0, -1);
      move_down_todo = CUBE_DIM;
    }
  }
  if (elapsed_fall > move_down_todo) elapsed_fall = move_down_todo;
  move_down_todo -= elapsed_fall;

  float dx = 0, dy = 0;
  csMatrix3 rot;
  bool do_rot = false;
  if (rot_px_todo)
  {
    if (elapsed_rot > rot_px_todo) elapsed_rot = rot_px_todo;
    rot_px_todo -= elapsed_rot;
    rot = create_rotate_x (elapsed_rot);
    do_rot = true;
    if (!rot_px_todo) rotate_shape_internal (full_rotate_x_reverse);
  }
  else if (rot_mx_todo)
  {
    if (elapsed_rot > rot_mx_todo) elapsed_rot = rot_mx_todo;
    rot_mx_todo -= elapsed_rot;
    rot = create_rotate_x (-elapsed_rot);
    do_rot = true;
    if (!rot_mx_todo) rotate_shape_internal (full_rotate_x);
  }
  else if (rot_py_todo)
  {
    if (elapsed_rot > rot_py_todo) elapsed_rot = rot_py_todo;
    rot_py_todo -= elapsed_rot;
    rot = create_rotate_y (elapsed_rot);
    do_rot = true;
    if (!rot_py_todo) rotate_shape_internal (full_rotate_z);
  }
  else if (rot_my_todo)
  {
    if (elapsed_rot > rot_my_todo) elapsed_rot = rot_my_todo;
    rot_my_todo -= elapsed_rot;
    rot = create_rotate_y (-elapsed_rot);
    do_rot = true;
    if (!rot_my_todo) rotate_shape_internal (full_rotate_z_reverse);
  }
  else if (rot_pz_todo)
  {
    if (elapsed_rot > rot_pz_todo) elapsed_rot = rot_pz_todo;
    rot_pz_todo -= elapsed_rot;
    rot = create_rotate_z (elapsed_rot);
    do_rot = true;
    if (!rot_pz_todo) rotate_shape_internal (full_rotate_y);
  }
  else if (rot_mz_todo)
  {
    if (elapsed_rot > rot_mz_todo) elapsed_rot = rot_mz_todo;
    rot_mz_todo -= elapsed_rot;
    rot = create_rotate_z (-elapsed_rot);
    do_rot = true;
    if (!rot_mz_todo) rotate_shape_internal (full_rotate_y_reverse);
  }
  else if (move_hor_todo)
  {
    if (elapsed_move > move_hor_todo) elapsed_move = move_hor_todo;
    move_hor_todo -= elapsed_move;
    dx = elapsed_move*(float)move_hor_dx;
    dy = elapsed_move*(float)move_hor_dy;
    if (!move_hor_todo) move_shape_internal (move_hor_dx, move_hor_dy, 0);
  }

  for (i = 0 ; i < num_cubes ; i++)
  {
    csThing* t = cube_info[i].thing;
    if (do_rot) t->Transform (rot);
    t->Move (dx, -elapsed_fall, dy);
    t->Transform ();
    reset_vertex_colors (t);
    room->ShineLights (t);
  }
  csPolygonSet::current_light_frame_number++;
}

void Blocks::InitTextures ()
{
  if (world) world->Clear ();

  // Maybe we shouldn't load the textures again if this is not the first time.
  Sys->set_pilar_texture (
    csLoader::LoadTexture (Sys->world, "txt1", "stone4.gif"));
  Sys->set_cube_texture (
    csLoader::LoadTexture (Sys->world, "txt2", "cube.gif"));
  //Sys->set_cube_texture (
    //csLoader::LoadTexture (Sys->world, "txt2", "clouds_thick1.jpg"));
  csLoader::LoadTexture (Sys->world, "txt3", "mystone2.gif");
  csLoader::LoadTexture (Sys->world, "clouds", "clouds.gif");
}

void Blocks::InitWorld ()
{
  InitTextures ();

  //-----
  // Prepare the game area.
  //-----
  csTextureHandle* tm = world->GetTextures ()->GetTextureMM ("txt3");
  room = Sys->world->NewSector ();
  room->SetName ("room");
  Sys->set_cube_room (room);
  csPolygon3D* p;
  p = room->NewPolygon (tm);
  p->AddVertex (-5, 0, 5);
  p->AddVertex (5, 0, 5);
  p->AddVertex (5, 0, -5);
  p->AddVertex (-5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = room->NewPolygon (tm);
  p->AddVertex (-5, 20, 5);
  p->AddVertex (5, 20, 5);
  p->AddVertex (5, 0, 5);
  p->AddVertex (-5, 0, 5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = room->NewPolygon (tm);
  p->AddVertex (5, 20, 5);
  p->AddVertex (5, 20, -5);
  p->AddVertex (5, 0, -5);
  p->AddVertex (5, 0, 5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = room->NewPolygon (tm);
  p->AddVertex (-5, 20, -5);
  p->AddVertex (-5, 20, 5);
  p->AddVertex (-5, 0, 5);
  p->AddVertex (-5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  p = room->NewPolygon (tm);
  p->AddVertex (5, 20, -5);
  p->AddVertex (-5, 20, -5);
  p->AddVertex (-5, 0, -5);
  p->AddVertex (5, 0, -5);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 3);

  Sys->add_pilar_template ();
  Sys->add_cube_template ();
  Sys->add_pilar (-1, -1);
  Sys->add_pilar (ZONE_DIM, -1);
  Sys->add_pilar (-1, ZONE_DIM);
  Sys->add_pilar (ZONE_DIM, ZONE_DIM);

  room->AddLight (new csStatLight (-3, 5, 0, 10, .8, .4, .4, false));
  room->AddLight (new csStatLight (3, 5, 0, 10, .4, .4, .8, false));
  room->AddLight (new csStatLight (0, 5, -3, 10, .4, .8, .4, false));
  room->AddLight (new csStatLight (0, (ZONE_HEIGHT-3-3)*CUBE_DIM+1, 0,
  	CUBE_DIM*10, .5, .5, .5, false));
  room->AddLight (new csStatLight (0, (ZONE_HEIGHT-3+3)*CUBE_DIM+1, 0,
  	CUBE_DIM*10, .5, .5, .5, false));

  //-----
  // Prepare the demo area.
  //-----
  csTextureHandle* demo_tm = world->GetTextures ()->GetTextureMM ("clouds");
  demo_room = Sys->world->NewSector ();
  demo_room->SetName ("room");

  p = demo_room->NewPolygon (demo_tm);
  p->AddVertex (-50, 50, 50);
  p->AddVertex (50, 50, 50);
  p->AddVertex (50, -50, 50);
  p->AddVertex (-50, -50, 50);
  p->SetTextureSpace (p->Vobj (0), p->Vobj (1), 100);
  p->SetFlags (CS_POLY_MIPMAP, 0);

  demo_room->AddLight (new csStatLight (0, 0, -2, 10, .4, .4, .4, false));

  float char_width = CUBE_DIM*4.;
  float offset_x = -char_width * (6/2)+CUBE_DIM*2;
  Sys->start_demo_shape (SHAPE_DEMO_B, offset_x, 3, 4); offset_x += char_width;
  Sys->start_demo_shape (SHAPE_DEMO_L, offset_x, 3, 4); offset_x += char_width;
  Sys->start_demo_shape (SHAPE_DEMO_O, offset_x, 3, 4); offset_x += char_width;
  Sys->start_demo_shape (SHAPE_DEMO_C, offset_x, 3, 4); offset_x += char_width;
  Sys->start_demo_shape (SHAPE_DEMO_K, offset_x, 3, 4); offset_x += char_width;
  Sys->start_demo_shape (SHAPE_DEMO_S, offset_x, 3, 4); offset_x += char_width;

  Sys->world->Prepare ();
}


void Blocks::StartDemo ()
{
  newgame = false;

  dynlight_x = 0;
  dynlight_y = 3;
  dynlight_z = 0;
  dynlight_dx = 3;
  delete dynlight;
  dynlight = new csDynLight (dynlight_x, dynlight_y, dynlight_z, 7, 3, 0, 0);

  Sys->world->AddDynLight (dynlight);
  dynlight->SetSector (demo_room);
  dynlight->Setup ();

  view->SetSector (demo_room);
  csVector3 pos (0, 3, -5);
  view->GetCamera ()->SetPosition (pos);
  cam_move_up = csVector3 (0, -1, 0);
  view->GetCamera ()->LookAt (view_origin-pos, cam_move_up);
  view->SetRectangle (0, 0, Sys->FrameWidth, Sys->FrameHeight);
}

void Blocks::StartNewGame ()
{
  delete dynlight; dynlight = NULL;

  // First delete all cubes that may still be in the world.
  csThing* cube = room->GetFirstThing ();
  while (cube)
  {
    csThing* next_cube = (csThing*)cube->GetNext ();
    if (!strncmp (cube->GetName (), "cube", 4))
      room->RemoveThing (cube);
    cube = next_cube;
  }

  Sys->init_game ();
  Sys->start_shape ((BlShapeType)(rand () % difficulty), 3, 3, ZONE_HEIGHT-3);

  cam_move_up = csVector3 (0, -1, 0);
  view->SetSector (room);
  Sys->move_camera ();
  view->SetRectangle (0, 0, Sys->FrameWidth, Sys->FrameHeight);
}


void Blocks::checkForPlane ()
{
  bool plane_hit;
  int x,y,z,i;

  // We know nothing yet.
  for (i=0 ; i<ZONE_HEIGHT ; i++) filled_planes[i] = false;

  for (z=0 ; z<ZONE_HEIGHT ; z++)
  {
    plane_hit = true;
    for (x=0 ; x<ZONE_DIM ; x++)
      for (y=0 ; y<ZONE_DIM ; y++)
      {
	// If one cube is missing we don't have a plane.
	if (!get_cube (x,y,z)) plane_hit = false; 
      }
      if (plane_hit)
      {
	// We've got at least one plane, switch to transition mode.
	transition = true;
	filled_planes[z] = true;
        removePlane (z);
        // That's how much all the cubes above the plane will lower.
        move_down_todo = CUBE_DIM;
        updateScore ();
        Sys->Printf (MSG_DEBUG_0, "\nthe score is %i", score);
      }
  }
}

void Blocks::removePlane (int z)
{
  int x,y;
  char temp[20];

  for (x=0 ; x<ZONE_DIM ; x++)
    for (y=0 ; y<ZONE_DIM ; y++)
    {
      sprintf (temp, "cubeAt%d%d%d", x, y, z);
      // Physically remove it.
      room->RemoveThing (room->GetThing (temp));
    }

  for (x=0 ; x<ZONE_DIM ; x++)
    for (y=0 ; y<ZONE_DIM ; y++)
      // And then remove it from game_cube[][][].
      set_cube (x, y, z, false);
}

void Blocks::handleTransition (time_t elapsed_time)
{
  int i;
  transition = false;

  for (i=0 ; i<ZONE_HEIGHT ; i++)
  {
    if (filled_planes[i])
    {
      transition = true;
      gone_z = i;
      lower (elapsed_time);
      break;
    }
  }
  if (!transition)
    start_shape ((BlShapeType)(rand () % difficulty), 3, 3, ZONE_HEIGHT-3);
}



void Blocks::lower (time_t elapsed_time)
{
  float elapsed = (float)elapsed_time/1000.;
  float elapsed_fall = elapsed*speed;
  float elapsed_move = elapsed*1.6;
  float elapsed_cam_move = elapsed*1.6;

  if (cam_move_dist)
  {
    if (elapsed_cam_move > cam_move_dist) elapsed_cam_move = cam_move_dist;
    cam_move_dist -= elapsed_cam_move;
    move_camera ();
  }

  if (pause) return;

  int i;
  int x,y,z;
  char temp[20];
  csThing* t;

  // Finished the transition.
  if (!move_down_todo)
  {
    // We finished handling this plane.
    filled_planes[gone_z] = false;

    // We lower the planes (in case the player made more then one plane).
    for (i=gone_z+1 ; i<ZONE_HEIGHT-1 ; i++)
      filled_planes[i] = filled_planes[i+1];
    filled_planes[ZONE_HEIGHT] = false; // And the last one.

    // Now that everything is visually ok we lower(change) their names
    // accordingly and clear them from game_cube[][][].
    for (z=gone_z+1 ; z<ZONE_HEIGHT ; z++)
      for (x=0 ; x<ZONE_DIM ; x++)
        for (y=0 ; y<ZONE_DIM ; y++)
	{
	  set_cube (x, y, z-1, get_cube (x, y, z));
          sprintf (temp, "cubeAt%d%d%d", x, y, z);
          t = room->GetThing (temp);
          if (t)
	  {
            sprintf (temp, "cubeAt%d%d%d", x, y, z-1);
	    t->SetName (temp);
	  }
	}

    // Mustn't forget the topmost level.
    for (x=0 ; x<ZONE_DIM ; x++)
      for (y=0 ; y<ZONE_DIM ; y++)
        set_cube (x, y, ZONE_HEIGHT-1, false);
  }

  if (elapsed_fall > move_down_todo) elapsed_fall = move_down_todo;
  move_down_todo -= elapsed_fall;

  // Move everything from above the plane that dissapeared a bit lower.
  for (z=gone_z+1 ; z<ZONE_HEIGHT ; z++)
    for (x=0 ; x<ZONE_DIM ; x++)
      for (y=0 ; y<ZONE_DIM ; y++)
      {
	sprintf (temp, "cubeAt%d%d%d", x, y, z);
        // Only if there is a thing at that certain position, or less
	// then CUBE_DIM lower.
	t = room->GetThing (temp);
	if (t)
	{
          t->Move (0, -elapsed_fall, 0);
          t->Transform ();
          reset_vertex_colors (t);
          room->ShineLights (t);
	}
      }


  // So that the engine knows to update the lights only when the frame
  // has changed (?).
  csPolygonSet::current_light_frame_number++; 
}



void Blocks::NextFrame (time_t elapsed_time, time_t current_time)
{
  SysSystemDriver::NextFrame (elapsed_time, current_time);

  if (startup_screen)
  {
    if (newgame) StartDemo ();
    move_cubes (elapsed_time);
    if (!Gfx3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;
    view->Draw ();
    if (!Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;
    Gfx2D->Write (100, 100, 0xffff, 0, "1: novice");
    Gfx2D->Write (100, 120, 0xffff, 0, "2: normal");
    Gfx2D->Write (100, 140, 0xffff, 0, "3: expert");
    Gfx3D->FinishDraw ();
    Gfx3D->Print (NULL);
    return;
  }

  if (newgame) StartNewGame ();

  // This is where Blocks stuff really happens.
  if (!transition) move_cubes (elapsed_time);
  // This is where the transition is handled.
  else handleTransition (elapsed_time);
  
  // Tell Gfx3D we're going to display 3D things
  if (!Gfx3D->BeginDraw (CSDRAW_3DGRAPHICS)) return;
  view->Draw ();

  // Start drawing 2D graphics
  //if (!Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS)) return;

  // Drawing code ends here
  Gfx3D->FinishDraw ();
  // Print the output.
  Gfx3D->Print (NULL);
}

bool Blocks::HandleEvent (csEvent &Event)
{
  if (SysSystemDriver::HandleEvent (Event))
    return false;

  switch (Event.Type)
  {
    case csevKeyDown:
      eatkeypress (Event.Key.Code, Event.Key.ShiftKeys & CSMASK_SHIFT,
          Event.Key.ShiftKeys & CSMASK_ALT, Event.Key.ShiftKeys & CSMASK_CTRL);
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

int cnt = 1;
long time0 = -1;

/*---------------------------------------------
 * Our main event loop.
 *---------------------------------------------*/

/*
 * Do a large debug dump just before the program
 * exits. This function can be installed as a last signal
 * handler if the program crashes (for systems that support
 * this).
 */
void debug_dump ()
{
  Dumper::dump (view->GetCamera ());
  Sys->Printf (MSG_DEBUG_0, "Camera dumped in debug.txt\n");
  Dumper::dump (Sys->world);
  Sys->Printf (MSG_DEBUG_0, "World dumped in debug.txt\n");
}

//---------------------------------------------------------------------------

void cleanup ()
{
  Sys->console_out ("Cleaning up...\n");
  delete view;
  delete Sys;
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class which is the driver for Blocks.
  Sys = new Blocks ();
  // temp hack until we find a better way
  csWorld::System = Sys;

  // Initialize the main system. This will load all needed plug-ins
  // (3D, 2D, network, sound, ...) and initialize them.
  if (!Sys->Initialize (argc, argv, "blocks.cfg"))
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

  // Find the pointer to world plugin
  iWorld *world = QUERY_PLUGIN (Sys, iWorld);
  if (!world)
  {
    CsPrintf (MSG_FATAL_ERROR, "No iWorld plugin!\n");
    return -1;
  }
  Sys->world = world->GetCsWorld ();
  world->DecRef ();

  // Some settings.
  Gfx3D->SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, (long)false);

  // Some commercials...
  Sys->Printf (MSG_INITIALIZATION, "3D Blocks version 0.2.\n");
  Sys->Printf (MSG_INITIALIZATION, "Created by Jorrit Tyberghein and others...\n\n");
  iTextureManager* txtmgr = Gfx3D->GetTextureManager ();
  txtmgr->SetVerbose (true);

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  view = new csView (Sys->world, Gfx3D);

  // Create our world.
  Sys->Printf (MSG_INITIALIZATION, "Creating world!...\n");
  Sys->world->EnableLightingCache (false);

  // Change to virtual directory where Blocks data is stored
  Sys->VFS->ChDir (Sys->Config->GetStr ("Blocks", "DATA", "/data/blocks"));

  Sys->InitWorld ();  

  txtmgr->AllocPalette ();

  Sys->Loop ();

  cleanup ();

  return 0;
}
