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
#include "iengine/material.h"
#include "csgeom/polyclip.h"
#include "qint.h"

SCF_IMPLEMENT_IBASE (csIsoView)
  SCF_IMPLEMENTS_INTERFACE (iIsoView)
SCF_IMPLEMENT_IBASE_END

csIsoView::csIsoView (iBase *iParent, iIsoEngine *eng, iIsoWorld *world)
{
  SCF_CONSTRUCT_IBASE (iParent);
  engine = eng;
  csIsoView::world = world;
  rect.Set(1,1,engine->GetG3D()->GetWidth()-1, engine->GetG3D()->GetHeight()-1);
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
  rview = new csIsoRenderView(0);
  fakecam = new csIsoFakeCamera();
}

csIsoView::~csIsoView ()
{
  delete rview;
  delete fakecam;
  SCF_DESTRUCT_IBASE();
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
  rview->GetG3D()->SetClipper( rview->GetClipper(), CS_CLIPPER_TOPLEVEL);
  rview->GetG3D()->ResetNearPlane ();
  iMaterialList* ml = engine->GetMaterialList ();
  if(rview->GetNumBuckets() < ml->GetCount ())
    rview->CreateBuckets(ml->GetCount ());
  PreCalc();

  int pass;
  for(pass = CSISO_RENDERPASS_PRE; pass <= CSISO_RENDERPASS_POST; pass++)
  {
    //printf("Renderpass %d\n", pass);
    rview->SetRenderPass(pass);
    world->Draw(rview);
    //if(pass == CSISO_RENDERPASS_MAIN)
      rview->DrawBuckets();
    if(pass == CSISO_RENDERPASS_PRE)
    {
      /// precalc camera
      fakecam->SetIsoView(scroll, x_axis, y_axis, z_axis);
    }
  }

  rview->GetG3D()->SetClipper(0, CS_CLIPPER_NONE);
  delete clipper;
  //printf("IsoView::Draw done\n");
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
  csVector3 topleft, topright, botright;
  csVector2 topright2(rect.xmax, rect.ymax);
  csVector2 topleft2(rect.xmin, rect.ymax);
  csVector2 botright2(rect.xmax, rect.ymin);
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


iCamera* csIsoView::GetFakeCamera(const csVector3& center,
    iIsoRenderView *rview)
{
  fakecam->IsoReady(center, rview);
  return fakecam;
}


//------------- csIsoFakeCamera -----------------------------------

SCF_IMPLEMENT_IBASE (csIsoFakeCamera)
  SCF_IMPLEMENTS_INTERFACE (iCamera)
SCF_IMPLEMENT_IBASE_END


csIsoFakeCamera::csIsoFakeCamera()
{
  view = 0;
  mirror = false;
  camnum = 0;
  scale = 1.0;
}

csIsoFakeCamera::~csIsoFakeCamera()
{
  //printf("delete camera!\n");
}

void csIsoFakeCamera::SetIsoView(const csVector2& scroll,
  const csVector2& x_axis, const csVector2& y_axis, const csVector2& z_axis)
{
  //printf("SetIsoView\n");
  mirror = false;
  fovangle = 180.;
  camnum ++;

  //'other' is world space and 'this' is camera space.
  csMatrix3 m;
  // based on: (W2S)
  //screen.x=scroll.x+ world.x*x_axis.x + world.z*z_axis.x;
  //screen.y=scroll.y+ world.x*x_axis.y + world.y*y_axis.y + world.z*z_axis.y;
  //screen.z= 1.0*world.z - 1.0*world.x;

  m.Set(
    x_axis.x, 0.0, z_axis.x,
    x_axis.y, y_axis.y, z_axis.y,
    -1.0, 0.0, 1.0
  );

  /// adjust to make camera space lengths about 1.0
  /// divide the camera x,y rows of the matrix by the scale,
  /// later multiply the fov by the scale, so that iz = fov/z,
  /// will the scale times larger - and thus transform nicely.
  scale = (x_axis.x + y_axis.y)*0.5;
  float div = 1./scale;

  m.m11 *= div;
  //m.m12 *= div; // is zero
  m.m13 *= div;
  m.m21 *= div;
  m.m22 *= div;
  m.m23 *= div;

  //printf("M (%7g %7g %7g)\n", m.m11, m.m12, m.m13);
  //printf("M (%7g %7g %7g)\n", m.m21, m.m22, m.m23);
  //printf("M (%7g %7g %7g)\n", m.m31, m.m32, m.m33);

  //// testing m.
  /*
  csVector3 pos = csVector3(12, 1, 4) - csVector3(.25, .25, .25);
  csVector3 res = m * pos;
  printf("Scroll %g,%g  \n", scroll.x, scroll.y);
  printf("MTEST %g, %g, %g   ->  %g, %g, %g\n", pos.x, pos.y, pos.z,
    scroll.x + res.x, scroll.y + res.y, res.z);
  res.x=scroll.x+ pos.x*x_axis.x + pos.z*z_axis.x;
  res.y=scroll.y+ pos.x*x_axis.y + pos.y*y_axis.y + pos.z*z_axis.y;
  res.z= 1.0*pos.z - 1.0*pos.x;
  printf("MTEST %g, %g, %g   ->  %g, %g, %g\n", pos.x, pos.y, pos.z,
    res.x, res.y, res.z);
  */

  trans.SetO2T( m );
  trans.SetO2TTranslation( csVector3(0, 0, 0) );
  //res = trans * pos;
  //printf("MTEST %g, %g, %g   ->  %g, %g, %g\n", pos.x, pos.y, pos.z,
    //res.x + scroll.x, res.y + scroll.y, res.z);
  shiftx = scroll.x;
  shifty = scroll.y;
}

void csIsoFakeCamera::IsoReady(const csVector3& position,
  iIsoRenderView *rview)
{
  //printf("IsoReady %g,%g,%g\n", position.x, position.y, position.z);
  camnum++;
  /// correct for position and renderview (minimum z bound)
  float minz = rview->GetMinZ();
  //// fov is an int!
  //fov = QInt(position.z - position.x - minz);
  /// adjust fov by scale, to make the iz=fov/z scaled larger.
  float ffov = (position.z - position.x - minz)*scale;
  fov = QInt( ffov );
  invfov = 1. / ffov;
  //trans.SetO2TTranslation( csVector3(0, 0, -minz) );
  //trans.SetO2TTranslation( trans.This2Other(csVector3(0, 0, -minz)) );

  /*
  // shift Z by the zlowerbound
  trans.SetO2TTranslation( csVector3(0,0, +minz) );
  // compensate for the z shift in the x,y shift in screenspace.
  shiftx = scroll.x + scale * minz * trans.GetO2T().m13;
  shifty = scroll.y + scale * minz * trans.GetO2T().m23;
  */
  //csVector3 move( -minz/2.,-minz/2., +minz/2. );
  //csVector3 move( -minz*45./60.,-minz/4., +minz*15./60. );
  csVector3 move( 0, 0, 0 );
  const csMatrix3 &m = trans.GetO2T();
  //move.x = position.x*m.m11 + position.y*m.m12 + position.z*m.m13;
  //move.z = minz;
  move = position; //// move back to 0,0,0 origin for draw
  minz -= position.z - position.x; /// but give correct depth
  move += csVector3( -minz/2., 0, +minz/2. );
  move.y -= ( -minz/2. * m.m21 + +minz/2.*m.m23 ) / m.m22;

  /// +1 leads to smaller y.
  //move.y = position.x*m.m21 + position.y*m.m22 + position.z*m.m23;

  trans.SetO2TTranslation( move );

  //shiftx = scroll.x + scale*move.x*trans.GetO2T().m11 +
    //scale*move.y*trans.GetO2T().m12 + scale*move.z*trans.GetO2T().m13;
  //shifty = scroll.y + scale*move.x*trans.GetO2T().m21 +
    //scale*move.y*trans.GetO2T().m22 + scale*move.z*trans.GetO2T().m23;

  /// 0,0,0 must display at object's position, use isometric transform
  /// to figure out where
  csVector2 screenpos;
  rview->GetView()->W2S(position, screenpos);
  shiftx = screenpos.x;
  shifty = screenpos.y;

  rview->GetG3D()->SetPerspectiveCenter(QInt(shiftx), QInt(shifty));
  rview->GetG3D()->SetPerspectiveAspect( ffov );

  /*
  csMatrix3 bb = trans.GetO2T();
  bb.Transpose();
  csVector3 b = bb * csVector3(0,0,-1);
  printf("diff is %g, %g, %g ?\n", b.x, b.y, b.z);
  printf("minz %g\n", minz);
  csVector3 pos = csVector3(12, 1, 4) - csVector3(.25, .25, .25);
  csVector3 res = trans * pos;
  printf("MTESTB %g, %g, %g  ->  %g, %g, %g\n", pos.x, pos.y, pos.z,
    res.x + shiftx, res.y + shifty, res.z);
  */
}

