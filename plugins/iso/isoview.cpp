/*
    Copyright (C) 2001 by W.C.A. Wijngaards
  
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
#include "isoview.h"
#include "isorview.h"
#include "ivideo/graph3d.h"
#include "csgeom/polyclip.h"
#include "qint.h"

IMPLEMENT_IBASE (csIsoView)
  IMPLEMENTS_INTERFACE (iIsoView)
IMPLEMENT_IBASE_END

csIsoView::csIsoView (iBase *iParent, iIsoEngine *eng, iIsoWorld *world)
{
  CONSTRUCT_IBASE (iParent);
  engine = eng;
  csIsoView::world = world;
  rect.Set(0,0,engine->GetG3D()->GetWidth(), engine->GetG3D()->GetHeight());
  scroll.Set(0,0);
  // set directions
  // now assumes that screeny 0=bottom, max=top
  x_axis.Set(1.0,-1.0);
  y_axis.Set(0.0,1.0);
  z_axis.Set(1.0,1.0);
  // default scale.
  float startscale = float(engine->GetG3D()->GetHeight())/16.0; 
  x_axis *= startscale;
  y_axis *= startscale;
  z_axis *= startscale;
  invx_axis_y = 1.0 / x_axis.y;

  // prealloc a renderview
  rview = new csIsoRenderView(this);
}

csIsoView::~csIsoView ()
{
  delete rview;
}

void csIsoView::W2S(const csVector3& world, csVector2& screen) const
{
  screen.x = world.x*x_axis.x + world.z*z_axis.x;
  screen.y = world.x*x_axis.y + world.y*y_axis.y + world.z*z_axis.y;
  screen += scroll;
}

void csIsoView::W2S(const csVector3& world, csVector3& screen) const
{
  screen.x = scroll.x+ world.x*x_axis.x + world.z*z_axis.x;
  screen.y = scroll.y+ world.x*x_axis.y + world.y*y_axis.y + world.z*z_axis.y;
  // the depth is the same, no matter the particular scale on the screen
  // or the slightly differnt directions of the axis,
  // since the z axis always is 'towards further away' (topright),
  // and the x axis always 'closer to the camera' (bottomright).
  screen.z = world.z - world.x;
}

void csIsoView::S2W(const csVector2& screen, csVector3& world) const
{
  // make x_axis.x + alpha*x_axis.y = 0.
  float alpha = -x_axis.x * invx_axis_y;
  csVector2 off = screen - scroll;
  // so that screen.x + alpha*screen.y has only z component
  world.z = (off.x + alpha*off.y) / (z_axis.x + alpha*z_axis.y);
  world.x = (off.y - z_axis.y * world.z) * invx_axis_y;
  world.y = 0.0;
}

void csIsoView::Draw()
{
  //printf("IsoView::Draw\n");
  rview->SetView(this);
  rview->SetG3D(engine->GetG3D());
  csBoxClipper* clipper = new csBoxClipper(rect.xmin, rect.ymin, 
    rect.xmax, rect.ymax);
  rview->SetClipper(clipper);
  PreCalc();
  
  for(int pass = CSISO_RENDERPASS_PRE; pass <= CSISO_RENDERPASS_POST; pass++)
  {
    rview->SetRenderPass(pass);
    world->Draw(rview);
  }
  delete clipper;
}

void csIsoView::SetScroll(const csVector3& worldpos, const csVector2& coord)
{
  csVector2 screenpos;
  W2S(worldpos, screenpos);
  // pos now shown at screenpos, but should be at 'coord'.
  // shift accordingly
  scroll += coord - screenpos;
}

void csIsoView::MoveScroll(const csVector3& delta)
{
  csVector2 screen;
  screen.x = delta.x*x_axis.x + delta.z*z_axis.x;
  screen.y = delta.x*x_axis.y + delta.y*y_axis.y + delta.z*z_axis.y;
  scroll -= screen;
}


csVector3 csIsoView::GetViewScroll() const
{
  csVector2 screen((rect.xmin+rect.xmax)/2, (rect.ymin+rect.ymax)/2);
  csVector3 worldpos;
  S2W(screen, worldpos);
  return csVector3(worldpos);
}

void csIsoView::PreCalc()
{
  csVector3 topleft, topright, botleft, botright;
  csVector2 botleft2(rect.xmin, rect.ymin);
  csVector2 topright2(rect.xmax, rect.ymax);
  csVector2 topleft2(rect.xmin, rect.ymax);
  csVector2 botright2(rect.xmax, rect.ymin);
  S2W(botleft2, botleft);
  S2W(topleft2, topleft);
  S2W(botright2, botright);
  S2W(topright2, topright);

  int startx = QInt(topright.z)+1;
  int starty = QInt(topright.x)-1;
  int scanw = QInt(topright.z) - QInt(topleft.z) + 2;
  int scanh = QInt(botright.x) - QInt(topright.x) + 2;

  // nr of grid cells per 1.0 world y,
  // the number of cells that fit in y_axis.y
  float celpery = y_axis.y / (z_axis.y - x_axis.y);

  //printf("scanw, scasnh %d %d\n", scanw, scanh);
  rview->SetPrecalcGrid(startx, starty, scanw, scanh, celpery);
  rview->SetMinZ(0.0);
}

void csIsoView::SetAxes(float xscale, float yscale, float zscale, float zskew, 
  float xskew)
{
  x_axis.Set(1.0,-xskew);
  y_axis.Set(0.0,1.0);
  z_axis.Set(1.0,zskew);
  x_axis *= xscale;
  y_axis *= yscale;
  z_axis *= zscale;
  invx_axis_y = 1.0 / x_axis.y;
}
