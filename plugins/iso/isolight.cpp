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
#include "isolight.h"
#include "qint.h"

IMPLEMENT_IBASE (csIsoLight)
  IMPLEMENTS_INTERFACE (iIsoLight)
IMPLEMENT_IBASE_END

csIsoLight::csIsoLight (iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
  grid = NULL;
  attenuation = CSISO_ATTN_REALISTIC;
  position.Set(0,0,0);
  color.Set(1.,1.,1.);
  radius = 10;
  vismap = NULL;
  visw = vish = 0;
  recalc_vis = true;
}

csIsoLight::~csIsoLight ()
{
  delete[] vismap;
}

void csIsoLight::SetGrid(iIsoGrid *grid)
{
  if(csIsoLight::grid) csIsoLight::grid->UnRegisterLight(this);
  csIsoLight::grid = grid;
  grid->RegisterLight(this);
  delete[] vismap;
  visw = grid->GetWidth() * grid->GetGroundMultX();
  vish = grid->GetHeight() * grid->GetGroundMultY();
  vismap = new uint8 [ (visw * vish + 7) / 8 ];
  recalc_vis = true;
}

void csIsoLight::SetPosition(const csVector3& pos)
{
  position = pos;
  recalc_vis = true;
}

void csIsoLight::SetRadius(float radius)
{
  csIsoLight::radius = radius;
  inv_radius = 1./radius;
  recalc_vis = true;
}

float csIsoLight::GetAttenuation(float distance)
{
  float res;
  switch(attenuation)
  {
    case CSISO_ATTN_NONE:
      res = 1.0; 
      break;
    case CSISO_ATTN_LINEAR:
      if(distance >= radius) return 0.;
      res = (radius - distance) * inv_radius; 
      break;
    case CSISO_ATTN_INVERSE:
      res = radius / distance; 
      break;
    case CSISO_ATTN_REALISTIC:
    default:
      res = radius / distance; 
      res *= res;
      break;
  };
  return res;
}


void csIsoLight::CalcVis()
{
  // calculate the visibility of all ground cells in the grid.
  // only using the ground height (for speed)
  if(!grid) return;
  recalc_vis = false;
  
  // set all to invisible
  int size = (visw*vish+7)/8;
  memset(vismap, 0, size*sizeof(uint8));
  int mingridx, mingridy;
  grid->GetGridOffset(mingridx, mingridy);
  int multx = grid->GetGroundMultX();
  int multy = grid->GetGroundMultY();
  
  // scan each possibly visible ground cell
  // start with min,max at center cell;
  int xmin = QInt(position.z * float(multx)) - mingridx*multx; 
  int ymin = QInt(position.x * float(multy)) - mingridy*multy;
  int xmax = xmin;
  int ymax = ymin;
  // shift min/max radius.
  xmin -= QInt(MaxRadius() * float(multx))+1;
  xmax += QInt(MaxRadius() * float(multx))+1;
  ymin -= QInt(MaxRadius() * float(multy))+1;
  ymax += QInt(MaxRadius() * float(multy))+1;
  if(xmin < 0) xmin = 0;
  if(ymin < 0) ymin = 0;
  if(xmax >= visw) xmax = visw-1;
  if(ymax >= vish) ymax = vish-1;
  // the position to check in world space
  csVector3 pos;
  float zinc = 1./float(multx);
  float xinc = 1./float(multy);
  for(int y = ymin; y<=ymax; y++)
  {
    // set pos to the start of the x line
    // set it to the center of that square
    pos.Set(
      float(y+mingridy*multy)/float(multy) + xinc*0.5,
      0.,
      float(xmin+mingridx*multx)/float(multx) + zinc*0.5 
    );
    for(int x = xmin; x<=xmax; x++)
    {
      // set posy to groundy
      pos.y = grid->GetGroundValue(x,y);
      // check if line from my position to groudn cell pos is possible
      if(grid->GroundHitBeam(position, pos))
        SetVis(x,y,1);
      pos.z += zinc;
    }
  }
}


static uint8 bitmasks[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

void csIsoLight::SetVis(int x, int y, bool val)
{
  int pos = y*visw+x;
  if(val) vismap[pos>>3] |= bitmasks[pos&0x7];
  else vismap[pos>>3] &= ~bitmasks[pos&0x7];
}

bool csIsoLight::GetVis(int x, int y) const
{
  if(x<0) x=0;
  else if(x>=visw) x=visw-1;
  if(y<0) y=0;
  else if(y>=vish) y=vish-1;
  int pos = y*visw+x;
  return vismap[pos>>3] & bitmasks[pos&0x7];
}

static void lightfunc(iIsoSprite *sprite, void *data)
{
  iIsoLight *light = (iIsoLight*)data;
  light->ShineSprite(sprite);
}

void csIsoLight::ShineGrid()
{
  if(!grid) return;
  if(recalc_vis) CalcVis();
  int mingridx, mingridy;
  grid->GetGridOffset(mingridx, mingridy);
  /// x in cell numbers
  int xmin = QInt(position.z) - mingridx; 
  int ymin = QInt(position.x) - mingridy;
  int xmax = xmin;
  int ymax = ymin;
  // shift min/max radius.
  xmin -= QInt(MaxRadius())+1;
  xmax += QInt(MaxRadius())+1;
  ymin -= QInt(MaxRadius())+1;
  ymax += QInt(MaxRadius())+1;
  if(xmin < 0) xmin = 0;
  if(ymin < 0) ymin = 0;
  if(xmax >= visw) xmax = grid->GetWidth()-1;
  if(ymax >= vish) ymax = grid->GetHeight()-1;
  for(int y = ymin; y<=ymax; y++)
  {
    for(int x = xmin; x<=xmax; x++)
    {
      grid->GetGridCell(x,y)->Traverse(lightfunc, (iIsoLight*)this);
    }
  }
}

void csIsoLight::ShineSprite(iIsoSprite *sprite)
{
  if(!grid) return;
  if(recalc_vis) CalcVis();
  int mingridx, mingridy;
  grid->GetGridOffset(mingridx, mingridy);
  int multx = grid->GetGroundMultX();
  int multy = grid->GetGroundMultY();
  // light position in the objects space
  csVector3 relpos = position - sprite->GetPosition();
  int sprx = QInt(sprite->GetPosition().z * float(multx)) - mingridx*multx; 
  int spry = QInt(sprite->GetPosition().x * float(multy)) - mingridy*multy;
  for(int i=0; i<sprite->GetNumVertices(); i++)
  {
    csVector3 vpos = sprite->GetVertexPosition(i);
    int x = QInt(vpos.z * float(multx)) + sprx;
    int y = QInt(vpos.x * float(multy)) + spry;
    if(!GetVis(x,y)) continue;
    csVector3 path = vpos - relpos;
    float dist = path.Norm();
    float light = GetAttenuation(dist);
    /// no normal vectors taken into account atm.
    // light *= path.Unit()*normal;
    sprite->AddToVertexColor(i, color*light);
  }
}

float csIsoLight::MaxRadius() const
{
  switch(attenuation)
  {
    case CSISO_ATTN_NONE:
      return 999999.0;
      break;
    case CSISO_ATTN_LINEAR:
      return radius;
      break;
    case CSISO_ATTN_INVERSE:
      return 999999.0;
      break;
    case CSISO_ATTN_REALISTIC:
    default:
      return 999999.0;
      break;
  };
}

