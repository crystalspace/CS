/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#define SYSDEF_ACCESS
#include "sysdef.h"
#include "cssys/common/system.h"
#include "csparser/csloader.h"
#include "apps/blocks/blocks.h"
#include "csutil/archive.h"
#include "csutil/inifile.h"
#include "csgfxldr/csimage.h"
#include "csgfxldr/gifimage.h"
#include "csengine/dumper.h"
#include "csengine/texture.h"
#include "csengine/sector.h"
#include "csengine/polytext.h"
#include "csengine/polygon.h"
#include "csengine/library.h"
#include "csengine/world.h"
#include "csengine/light.h"
#include "csengine/lghtmap.h"
#include "csengine/thing.h"
#include "csengine/thingtpl.h"
#include "csobject/nameobj.h"
#include "csengine/textrans.h"
#include "csengine/csview.h"
#include "igraph3d.h"
#include "itxtmgr.h"

Blocks* Sys = NULL;
csView* view = NULL;

#define Gfx3D System->piG3D

//-----------------------------------------------------------------------------

Blocks::Blocks ()
{
  rot_px_todo = 0;
  rot_py_todo = 0;
  rot_pz_todo = 0;
  rot_mx_todo = 0;
  rot_my_todo = 0;
  rot_mz_todo = 0;
  move_hor_todo = 0;
  cam_move_dist = 0;

  full_rotate_x = create_rotate_x (M_PI/2);
  full_rotate_y = create_rotate_y (M_PI/2);
  full_rotate_z = create_rotate_z (M_PI/2);

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
  cur_ver_dest = 1;
  move_right_dx = dest_move_right_dx[cur_hor_dest];
  move_right_dy = dest_move_right_dy[cur_hor_dest];
  move_down_dx = dest_move_down_dx[cur_hor_dest];
  move_down_dy = dest_move_down_dy[cur_hor_dest];

  view_origin = csVector3 (0, 3, 0);
}

void Blocks::init_game ()
{
  int i, j, k;
  for (k = 0 ; k < CUBE_SAFETY+CUBE_HEIGHT+CUBE_SAFETY ; k++)
    for (j = 0 ; j < CUBE_SAFETY+CUBE_DIM+CUBE_SAFETY ; j++)
      for (i = 0 ; i < CUBE_SAFETY+CUBE_DIM+CUBE_SAFETY ; i++)
        game_cube[i][j][k] =
	  i < CUBE_SAFETY || j < CUBE_SAFETY || k < CUBE_SAFETY ||
	  i >= CUBE_SAFETY+CUBE_DIM || j >= CUBE_SAFETY+CUBE_DIM || k >= CUBE_SAFETY+CUBE_HEIGHT;
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
  float dim = BLOCK_DIM/2.;
  CHK (pilar_tmpl = new csThingTemplate ());
  csNameObject::AddName (*pilar_tmpl, "pilar");
  pilar_tmpl->AddVertex (-dim, 0, dim);
  pilar_tmpl->AddVertex (dim, 0, dim);
  pilar_tmpl->AddVertex (dim, 0, -dim);
  pilar_tmpl->AddVertex (-dim, 0, -dim);
  pilar_tmpl->AddVertex (-dim, CUBE_HEIGHT*BLOCK_DIM, dim);
  pilar_tmpl->AddVertex (dim, CUBE_HEIGHT*BLOCK_DIM, dim);
  pilar_tmpl->AddVertex (dim, CUBE_HEIGHT*BLOCK_DIM, -dim);
  pilar_tmpl->AddVertex (-dim, CUBE_HEIGHT*BLOCK_DIM, -dim);

  csPolygonTemplate* p;
  float A, B, C;
  csMatrix3 tx_matrix;
  csVector3 tx_vector;

  CHK (p = new csPolygonTemplate (pilar_tmpl, "d", pilar_txt));
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (3);
  p->AddVertex (2);
  p->AddVertex (1);
  p->AddVertex (0);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (0), pilar_tmpl->Vtex (1), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (pilar_tmpl, "b", pilar_txt));
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (0);
  p->AddVertex (1);
  p->AddVertex (5);
  p->AddVertex (4);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (0), pilar_tmpl->Vtex (1), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (pilar_tmpl, "t", pilar_txt));
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (5);
  p->AddVertex (6);
  p->AddVertex (7);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (4), pilar_tmpl->Vtex (5), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (pilar_tmpl, "f", pilar_txt));
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (7);
  p->AddVertex (6);
  p->AddVertex (2);
  p->AddVertex (3);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (7), pilar_tmpl->Vtex (6), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (pilar_tmpl, "l", pilar_txt));
  pilar_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (7);
  p->AddVertex (3);
  p->AddVertex (0);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	pilar_tmpl->Vtex (7), pilar_tmpl->Vtex (3), 1, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (pilar_tmpl, "r", pilar_txt));
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
  CHK (pilar = new csThing ());
  csNameObject::AddName (*pilar, "pilar");
  pilar->SetSector (room);
  pilar->SetFlags (CS_ENTITY_MOVEABLE, 0);
  pilar->MergeTemplate (pilar_tmpl, pilar_txt, 1);
  room->AddThing (pilar);
  csVector3 v ((x-CUBE_DIM/2)*BLOCK_DIM, 0, (y-CUBE_DIM/2)*BLOCK_DIM);
  pilar->SetMove (room, v);
  pilar->Transform ();
}


void Blocks::add_cube_template ()
{
  float dim = BLOCK_DIM/2.;
  CHK (cube_tmpl = new csThingTemplate ());
  csNameObject::AddName (*cube_tmpl, "cube");
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

  CHK (p = new csPolygonTemplate (cube_tmpl, "d1", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (3);
  p->AddVertex (2);
  p->AddVertex (1);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "d2", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (3);
  p->AddVertex (1);
  p->AddVertex (0);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "b1", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (0);
  p->AddVertex (1);
  p->AddVertex (5);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "b2", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (0);
  p->AddVertex (5);
  p->AddVertex (4);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "t1", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (5);
  p->AddVertex (6);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "t2", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (6);
  p->AddVertex (7);
  p->SetTextureSpace (tx_matrix, tx_vector);

#if 0
  CHK (p = new csPolygonTemplate (cube_tmpl, "f", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (7);
  p->AddVertex (6);
  p->AddVertex (2);
  p->AddVertex (3);
  p->PlaneNormal (&A, &B, &C);
  TextureTrans::compute_texture_space (tx_matrix, tx_vector,
      	cube_tmpl->Vtex (7), cube_tmpl->Vtex (6), BLOCK_DIM, A, B, C);
  p->SetTextureSpace (tx_matrix, tx_vector);
#else
  CHK (p = new csPolygonTemplate (cube_tmpl, "f1", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (7);
  p->AddVertex (6);
  p->AddVertex (2);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "f2", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (7);
  p->AddVertex (2);
  p->AddVertex (3);
  p->SetTextureSpace (tx_matrix, tx_vector);
#endif

  CHK (p = new csPolygonTemplate (cube_tmpl, "l1", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (7);
  p->AddVertex (3);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "l2", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (4);
  p->AddVertex (3);
  p->AddVertex (0);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "r1", cube_txt));
  cube_tmpl->AddPolygon (p);
  p->AddVertex (6);
  p->AddVertex (5);
  p->AddVertex (1);
  p->SetTextureSpace (tx_matrix, tx_vector);

  CHK (p = new csPolygonTemplate (cube_tmpl, "r2", cube_txt));
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

void Blocks::add_cube (int dx, int dy, int dz, int x, int y, int z)
{
  csThing* cube;
  CHK (cube = new csThing ());
  csNameObject::AddName (*cube, "cube");
  cube->SetSector (room);
  cube->SetFlags (CS_ENTITY_MOVEABLE, CS_ENTITY_MOVEABLE);
  csVector3 shift (dx*BLOCK_DIM, dy*BLOCK_DIM, dz*BLOCK_DIM);
  cube->MergeTemplate (cube_tmpl, cube_txt, 1, NULL, &shift, NULL);

  csPolygon3D* p;

  p = cube->GetPolygon ("f1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon ("f2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon ("t1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon ("t2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon ("b1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon ("b2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon ("d1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon ("d2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon ("l1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon ("l2"); set_uv (p, 0, 0, 1, 1, 0, 1);
  p = cube->GetPolygon ("r1"); set_uv (p, 0, 0, 1, 0, 1, 1);
  p = cube->GetPolygon ("r2"); set_uv (p, 0, 0, 1, 1, 0, 1);

  room->AddThing (cube);
  csVector3 v ((x-CUBE_DIM/2)*BLOCK_DIM, z*BLOCK_DIM+1, (y-CUBE_DIM/2)*BLOCK_DIM);
  cube->SetMove (room, v);
  cube->Transform ();
  cube->InitLightMaps (false);
  room->ShineLights (cube);
  cube->CreateLightMaps (Gfx3D);
  cube_info[num_cubes].thing = cube;
  cube_info[num_cubes].dx = dx;
  cube_info[num_cubes].dy = dy;
  cube_info[num_cubes].dz = dz;
  num_cubes++;
}

void Blocks::start_shape (BlShapeType type, int x, int y, int z)
{
  num_cubes = 0;
  switch (type)
  {
    case SHAPE_R1: add_cube (0, 0, 0, x, y, z); break;
    case SHAPE_R2: add_cube (0, 0, 0, x, y, z); add_cube (1, 0, 0, x, y, z); break;
    case SHAPE_R3: add_cube (-1, 0, 0, x, y, z); add_cube (0, 0, 0, x, y, z); add_cube (1, 0, 0, x, y, z); break;
    case SHAPE_R4: add_cube (-1, 0, 0, x, y, z); add_cube (0, 0, 0, x, y, z); add_cube (1, 0, 0, x, y, z); add_cube (2, 0, 0, x, y, z); break;
    case SHAPE_L: add_cube (-1, 0, 1, x, y, z); add_cube (-1, 0, 0, x, y, z); add_cube (0, 0, 0, x, y, z); add_cube (1, 0, 0, x, y, z); break;
    case SHAPE_T1: add_cube (0, 0, 1, x, y, z); add_cube (-1, 0, 0, x, y, z); add_cube (0, 0, 0, x, y, z); add_cube (1, 0, 0, x, y, z); break;
    case SHAPE_T2: add_cube (0, 0, 2, x, y, z); add_cube (0, 0, 1, x, y, z); add_cube (-1, 0, 0, x, y, z); add_cube (0, 0, 0, x, y, z); add_cube (1, 0, 0, x, y, z); break;
    case SHAPE_CUBE:
    	add_cube (0, 0, 0, x, y, z); add_cube (1, 0, 0, x, y, z); add_cube (0, 1, 0, x, y, z); add_cube (1, 1, 0, x, y, z);
    	add_cube (0, 0, 1, x, y, z); add_cube (1, 0, 1, x, y, z); add_cube (0, 1, 1, x, y, z); add_cube (1, 1, 1, x, y, z);
	break;
    default: break;
  }
  move_down_todo = 0;
  cube_x = x;
  cube_y = y;
  cube_z = z;
  speed = .2;
}

void Blocks::start_rotation (BlRotType type)
{
  if (rot_px_todo || rot_mx_todo || rot_py_todo || rot_my_todo || rot_pz_todo || rot_mz_todo ||
      move_hor_todo) return;
  switch (type)
  {
    case ROT_PX: if (!check_new_shape_rotation (full_rotate_x)) return; rot_px_todo = M_PI/2; break;
    case ROT_MX: if (!check_new_shape_rotation (-full_rotate_x)) return; rot_mx_todo = M_PI/2; break;
    case ROT_PY: if (!check_new_shape_rotation (full_rotate_z)) return; rot_py_todo = M_PI/2; break;
    case ROT_MY: if (!check_new_shape_rotation (-full_rotate_z)) return; rot_my_todo = M_PI/2; break;
    case ROT_PZ: if (!check_new_shape_rotation (full_rotate_y)) return; rot_pz_todo = M_PI/2; break;
    case ROT_MZ: if (!check_new_shape_rotation (-full_rotate_y)) return; rot_mz_todo = M_PI/2; break;
  }
}

void Blocks::start_horizontal_move (int dx, int dy)
{
  if (rot_px_todo || rot_mx_todo || rot_py_todo || rot_my_todo || rot_pz_todo || rot_mz_todo ||
      move_hor_todo) return;
  if (!check_new_shape_location (dx, dy, 0)) return;
  move_hor_todo = BLOCK_DIM;
  move_hor_dx = dx;
  move_hor_dy = dy;
}

void Blocks::move_camera ()
{
  csVector3 pos = cam_move_dist*cam_move_src + (1-cam_move_dist)*cam_move_dest;
  view->GetCamera ()->SetPosition (pos); view->GetCamera ()->LookAt (view_origin-pos, cam_move_up);
}

void Blocks::eatkeypress (int key, bool /*shift*/, bool /*alt*/, bool /*ctrl*/)
{
  switch (key)
  {
    case '1':
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
    case '2':
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
    case '3':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      if (cur_ver_dest > 0) cur_ver_dest--;
      cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case '4':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      if (cur_ver_dest < 3) cur_ver_dest++;
      cam_move_dest = destinations[cur_hor_dest][cur_ver_dest];
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case 'z':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      cam_move_dest = cam_move_src + .3 * (view_origin - cam_move_src);
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case 'Z':
      if (cam_move_dist) break;
      cam_move_dist = 1;
      cam_move_src = view->GetCamera ()->GetW2CTranslation ();
      cam_move_dest = cam_move_src - .3 * (view_origin - cam_move_src);
      cam_move_up = csVector3 (0, -1, 0);
      break;
    case CSKEY_F1: piG2D->PerformExtension ("sim_pal"); break;
    case 'q': start_rotation (ROT_PX); break;
    case 'a': start_rotation (ROT_MX); break;
    case 'w': start_rotation (ROT_PY); break;
    case 's': start_rotation (ROT_MY); break;
    case 'e': start_rotation (ROT_PZ); break;
    case 'd': start_rotation (ROT_MZ); break;
    case CSKEY_UP: start_horizontal_move (-move_down_dx, -move_down_dy); break;
    case CSKEY_DOWN: start_horizontal_move (move_down_dx, move_down_dy); break;
    case CSKEY_LEFT: start_horizontal_move (-move_right_dx, -move_right_dy); break;
    case CSKEY_RIGHT: start_horizontal_move (move_right_dx, move_right_dy); break;
    case ' ': speed = 7; break;
    case 'p': pause = !pause; break;
    case CSKEY_ESC: System->Shutdown = true;
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
  for (i = 0 ; i < num_cubes ; i++)
  {
    x = cube_x+cube_info[i].dx;
    y = cube_y+cube_info[i].dy;
    z = cube_z+cube_info[i].dz;
    set_cube (x, y, z, true);
  }
}

void Blocks::dump_shape ()
{
  int i;
  int x, y, z;
  printf ("Dump shape:\n");
  for (i = 0 ; i < num_cubes ; i++)
  {
    x = cube_info[i].dx;
    y = cube_info[i].dy;
    z = cube_info[i].dz;
    printf ("    %d: (%d,%d,%d) d=(%d,%d,%d)\n", i, cube_x+x, cube_y+y, cube_z+z, x, y, z);
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

void Blocks::move_cubes (long elapsed_time)
{
  int i;
  float elapsed = (float)elapsed_time/1000.;
  float elapsed_rot = 3 * elapsed * (M_PI/2);
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

  if (!move_down_todo)
  {
    //dump_shape ();
    bool stop = !check_new_shape_location (0, 0, -1);
    if (stop)
    {
      freeze_shape ();
      start_shape ((BlShapeType)(rand () % SHAPE_TOTAL), 3, 3, CUBE_HEIGHT-3);
      return;
    }
    else
    {
      move_shape_internal (0, 0, -1);
      move_down_todo = BLOCK_DIM;
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
    if (!rot_px_todo) rotate_shape_internal (full_rotate_x);
  }
  else if (rot_mx_todo)
  {
    if (elapsed_rot > rot_mx_todo) elapsed_rot = rot_mx_todo;
    rot_mx_todo -= elapsed_rot;
    rot = create_rotate_x (-elapsed_rot);
    do_rot = true;
    if (!rot_mx_todo) rotate_shape_internal (-full_rotate_x);
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
    if (!rot_my_todo) rotate_shape_internal (-full_rotate_z);
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
    if (!rot_mz_todo) rotate_shape_internal (-full_rotate_y);
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

void Blocks::NextFrame (long elapsed_time, long current_time)
{
  SysSystemDriver::NextFrame (elapsed_time, current_time);

  csEvent *Event;
  while ((Event = Sys->EventQueue->Get ()))
  {
    switch (Event->Type)
    {
      case csevKeyDown:
        eatkeypress (Event->Key.Code, Event->Key.ShiftKeys & CSMASK_SHIFT,
            Event->Key.ShiftKeys & CSMASK_ALT, Event->Key.ShiftKeys & CSMASK_CTRL);
        break;
      case csevBroadcast:
        //if ((Event->Command.Code == cscmdFocusChanged)
         //&& (Event->Command.Info == NULL))
          //memset (&Keyboard->Key, 0, sizeof (Keyboard->Key));
        break;
      case csevMouseDown:
        break;
      case csevMouseMove:
        break;
      case csevMouseUp:
        break;
    }
    CHK (delete Event);
  }

  move_cubes (elapsed_time);

  // Tell Gfx3D we're going to display 3D things
  if (Gfx3D->BeginDraw (CSDRAW_3DGRAPHICS) != S_OK) return;
  view->Draw ();

  // Start drawing 2D graphics
  //if (Gfx3D->BeginDraw (CSDRAW_2DGRAPHICS) != S_OK) return;

  // Drawing code ends here
  Gfx3D->FinishDraw ();
  // Print the output.
  Gfx3D->Print (NULL);
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
  view->GetCamera ()->SaveFile ("coord.bug");
  Sys->Printf (MSG_DEBUG_0, "Camera saved in coord.bug\n");
  Dumper::dump (view->GetCamera ());
  Sys->Printf (MSG_DEBUG_0, "Camera dumped in debug.txt\n");
  Dumper::dump (Sys->world);
  Sys->Printf (MSG_DEBUG_0, "World dumped in debug.txt\n");
}

//---------------------------------------------------------------------------

void cleanup ()
{
  pprintf ("Cleaning up...\n");
  CHK (delete view);
  CHK (delete Sys);
  CHK (delete config);
  pprintf_close();              // DAN: this closes the pprintf routine
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  srand (time (NULL));

  // Create our main class which is the driver for Blocks.
  CHK (Sys = new Blocks ());

  // Open our configuration file.
  CHK (config = new csIniFile ("blocks.cfg"));

  // Create our world. The world is the representation of
  // the 3D engine.
  CHK (Sys->world = new csWorld ());

  // Initialize the main system. This will load all needed
  // COM drivers (3D, 2D, network, sound, ...) and initialize them.
  if (!Sys->Initialize (argc, argv, Sys->world->GetEngineConfigCOM ()))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error initializing system!\n");
    cleanup ();
    fatal_exit (0, false);
  }

  // Open the main system. This will open all the previously loaded
  // COM drivers.
  if (!Sys->Open ("3D Blocks"))
  {
    Sys->Printf (MSG_FATAL_ERROR, "Error opening system!\n");
    cleanup ();
    fatal_exit (0, false);
  }

  // Some settings.
  Gfx3D->SetRenderState (G3DRENDERSTATE_INTERLACINGENABLE, (long)false);

  // Some commercials...
  Sys->Printf (MSG_INITIALIZATION, "3D Blocks version 0.1.\n");
  Sys->Printf (MSG_INITIALIZATION, "Created by Jorrit Tyberghein and others...\n\n");
  ITextureManager* txtmgr;
  Gfx3D->GetTextureManager (&txtmgr);
  txtmgr->SetVerbose (true);

  // Initialize our world.
  Sys->world->Initialize (GetISystemFromSystem (System), Gfx3D, config);

  // csView is a view encapsulating both a camera and a clipper.
  // You don't have to use csView as you can do the same by
  // manually creating a camera and a clipper but it makes things a little
  // easier.
  CHK (view = new csView (Sys->world, Gfx3D));

  // Create our world.
  csSector* room;
  Sys->Printf (MSG_INITIALIZATION, "Creating world!...\n");
  Sys->world->EnableLightingCache (false);

  char const* path = config->GetStr ("Blocks", "ARCHIVE", "data/blocks.zip");
  Archive* archive = new Archive (path, true);

  Sys->set_pilar_texture (CSLoader::LoadTexture (Sys->world, "txt", "stone4.gif", archive));
  Sys->set_cube_texture (CSLoader::LoadTexture (Sys->world, "txt", "cube.gif", archive));
  csTextureHandle* tm = CSLoader::LoadTexture (Sys->world, "txt2", "mystone2.gif", archive);

  delete archive; archive = 0;

  room = Sys->world->NewSector ();
  csNameObject::AddName (*room, "room");
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
  Sys->init_game ();
  Sys->add_pilar (-1, -1);
  Sys->add_pilar (CUBE_DIM, -1);
  Sys->add_pilar (-1, CUBE_DIM);
  Sys->add_pilar (CUBE_DIM, CUBE_DIM);
  Sys->start_shape (SHAPE_T2, 3, 3, CUBE_HEIGHT-3);

  csStatLight* light;
  CHK (light = new csStatLight (-3, 5, 0, 10, 1, 0, 0, false));
  room->AddLight (light);
  CHK (light = new csStatLight (3, 5, 0, 10, 0, 0, 1, false));
  room->AddLight (light);
  CHK (light = new csStatLight (0, 5, -3, 10, 0, 1, 0, false));
  room->AddLight (light);
  CHK (light = new csStatLight (0, (CUBE_HEIGHT-3-3)*BLOCK_DIM+1, 0, BLOCK_DIM*10, .5, .5, .5, false));
  room->AddLight (light);
  CHK (light = new csStatLight (0, (CUBE_HEIGHT-3+3)*BLOCK_DIM+1, 0, BLOCK_DIM*10, .5, .5, .5, false));
  room->AddLight (light);

  Sys->world->Prepare (Gfx3D);

  Sys->Printf (MSG_INITIALIZATION, "--------------------------------------\n");

  // Wait one second before starting.
  long t = Sys->Time ()+1000;
  while (Sys->Time () < t) ;

  view->SetSector (room);
  view->GetCamera ()->SetPosition (csVector3 (0, 5, -3));
  view->SetRectangle (2, 2, Sys->FrameWidth - 4, Sys->FrameHeight - 4);

  txtmgr->AllocPalette ();

  Sys->Loop ();

  cleanup ();

  return 0;
}
