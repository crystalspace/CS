/*
    Metaballs Demo
    Copyright (C) 1999 by Denis Dmitriev

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

#include <stdarg.h>
#include <string.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "meta.h"
#include "igraph3d.h"
#include "igraph2d.h"
#include "icfgmgr.h"
#include "isystem.h"

IMPLEMENT_FACTORY (csMetaBalls)

EXPORT_CLASS_TABLE (metaball)
  EXPORT_CLASS (csMetaBalls, "crystalspace.graphics.metaballs",
    "Metaball renderer for crystal space")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csMetaBalls)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iMetaBalls)
IMPLEMENT_IBASE_END

#define MAP_RESOLUTION  256
static float asin_table[2*MAP_RESOLUTION+1];

csMetaBalls::csMetaBalls (iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
  th = NULL;
  poly = new G3DPolygonDPFX ();
  memset(poly, 0, sizeof(*poly));
  poly->num = 3;
  alpha = frame = 0;
  triangles_array = NULL;
  meta_balls = NULL;
}

csMetaBalls::~csMetaBalls ()
{
  delete [] triangles_array;
  delete [] meta_balls;
  delete poly;
}

float csMetaBalls::map (float x)
{
  return asin_table[(int)(MAP_RESOLUTION*(1+x))];
}

void csMetaBalls::InitTables(void)
{
  for (int i = -MAP_RESOLUTION, j = 0; i <= MAP_RESOLUTION; i++, j++)
  {
    float c = 1.0 * i / MAP_RESOLUTION;
    switch(env_mapping)
    {
      case TRUE_ENV_MAP:
        asin_table[j] = env_map_mult * (0.5+asin(c)/M_PI);
        break;
      case FAKE_ENV_MAP:
        asin_table[j] = 0.5 * env_map_mult * (1 + c);
        break;
    }
  }
}

bool csMetaBalls::Initialize (iSystem *sys)
{
  Sys = sys;
  iConfigFileNew *Config = Sys->GetConfig ();
  // Read the config here: earlier than Sys->Initialize we can't
  mp.d_alpha = Config->GetFloat ("MetaBalls.InitialSpeed", 0.03);

  num_meta_balls = Config->GetInt ("MetaBalls.NumMetaBalls", 3);
  
  mp.iso_level = Config->GetFloat ("MetaBalls.IsoLevel", 1.0);
  max_triangles = Config->GetInt ("MetaBalls.MaxTriangles", 2000);

  env_map_mult = Config->GetFloat ("MetaBalls.EnvMapMultiplier", 1.0);
  env_mapping = Config->GetBool ("MetaBalls.UseTrueEnvMap", true) 
                ? TRUE_ENV_MAP : FAKE_ENV_MAP;

  meta_balls = new MetaBall[num_meta_balls];
  triangles_array = new Triangle[max_triangles];

  mp.charge = Config->GetFloat ("MetaBalls.Charge", 3.5);
  InitTables();
  return true;
}

void csMetaBalls::SetContext (iGraphics3D *g3d)
{
  G3D = g3d;
}

void csMetaBalls::SetNumberMetaBalls (int number)
{
  if (number < 1 || number  == num_meta_balls)
    return;
  num_meta_balls = number;
  delete [] meta_balls;
  meta_balls = new MetaBall [num_meta_balls];
}

void csMetaBalls::SetQualityEnvironmentMapping (bool toggle)
{
  env_mapping = toggle ? TRUE_ENV_MAP : FAKE_ENV_MAP;;
  InitTables ();
}

void csMetaBalls::SetEnvironmentMappingFactor (float env_mult)
{
  env_map_mult = env_mult;
  InitTables ();
}



void LitVertex(const csVector3 &n, G3DTexturedVertex &c)
{
  if (n.z > 0)
    c.r = c.g = c.b = 0;
  else
  {
    float l = n.z*n.z;
    c.r = c.g = c.b = l;
  }
}

void csMetaBalls::DrawSomething(void)
{
  int i,j;
  int h_height = G3D->GetHeight () / 2;
  int h_width = G3D->GetWidth () / 2;

  for (i = 0; i < num_meta_balls; i++)
  {
    float m = fmod((i + 1) / 3.0, 1.5) + 0.5;

    csVector3 &c = meta_balls[i].center;
    c.x = 4 * m * sin (m * alpha + i * M_PI / 4);
    c.y = 3 * m * cos (1.4 * m * alpha + m * M_PI / 6);
    c.z = 11 + 2 * sin (m * alpha * 1.3214);
  }
  
  CalculateMetaBalls();

  for (i = 0; i < triangles_tesselated; i++)
  {
    Triangle& t = triangles_array[i];

    for (j = 0; j < 3; j++)
    {
      int m = 2 - j;

      // Projecting.
      poly->vertices[j].sx = h_width + h_height * t.p[m].x / (1 + t.p[m].z);
      poly->vertices[j].sy = h_height * (1 + t.p[m].y / (1 + t.p[m].z));

      // Computing normal at point.
      csVector3 n(0, 0, 0);
      for(int k = 0; k < num_meta_balls; k++)
      {
        csVector3 rv(t.p[m].x - meta_balls[k].center.x,
          t.p[m].y - meta_balls[k].center.y,
          t.p[m].z - meta_balls[k].center.z);

        float r = rv.Norm();
        float c = mp.charge / (r*r*r);

        n += rv * c;
      }

      // Lighting
      if (n.z > 0)
        break;

      n = n.Unit();
      LitVertex (n, poly->vertices[j]);

      // Environment mapping.
      poly->vertices[j].u = map (n.x);
      poly->vertices[j].v = map (n.y);
      poly->vertices[j].z = 1 / t.p[m].z;
    }

    // We really want to draw this triangle
    if (j == 3)
      G3D->DrawPolygonFX (*poly);
  }
}

bool csMetaBalls::Draw ()
{
  alpha += mp.d_alpha;
  poly->mat_handle = th;
  G3D->StartPolygonFX(th, CS_FX_COPY | CS_FX_GOURAUD);
  G3D->SetRenderState (G3DRENDERSTATE_ZBUFFERMODE, CS_ZBUF_USE);
  DrawSomething();
  G3D->FinishPolygonFX();
  return true;
}
