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

#ifndef __ISOLIGHT_H__
#define __ISOLIGHT_H__

#include "ivaria/iso.h"
#include "iengine/light.h"

class csIsoFakeLight;

/**
 * iso light
*/
class csIsoLight : public iIsoLight {
private:
  /// the grid the light shines to
  iIsoGrid *grid;
  /// the attenuation type of the light
  int attenuation;
  /// position of the light
  csVector3 position;
  /// the color of the light
  csColor color;
  /// radius of the light, 1./radius
  float radius, inv_radius;
  /// visibility map
  float *vismap;
  /// visibility map size
  int visw, vish;
  /// force recalc of vismap
  bool recalc_vis;
  /// my flags
  csFlags flags;
  /// fake iLight
  csIsoFakeLight *fakelight;

public:
  DECLARE_IBASE;

  ///
  csIsoLight (iBase *iParent);
  ///
  virtual ~csIsoLight ();

  /// get the attenuation light multiplier for given distance.
  float GetAttenuation(float distance);
  /// precalc the visible and shadowed portions of the grid.
  void CalcVis();
  /// set visibility value 
  void SetVis(int x, int y, float val);
  /// get the maximum radius when this light still has effect
  float MaxRadius() const;

  //----- iIsoLight -----------------------------------------------
  virtual void SetGrid(iIsoGrid *grid);
  virtual iIsoGrid* GetGrid() const { return grid; }
  virtual void SetAttenuation(int attn) {attenuation = attn; }
  virtual int GetAttenuation() const { return attenuation; }
  virtual void SetPosition(const csVector3& pos);
  virtual const csVector3& GetPosition() const { return position; }
  virtual void SetColor(const csColor& col) {color = col; }
  virtual const csColor& GetColor() const { return color; }
  virtual void SetRadius(float radius);
  virtual float GetRadius() const { return radius; }
  virtual void ShineGrid();
  virtual void ShineSprite(iIsoSprite *sprite);
  virtual csFlags& Flags() {return flags;}
  virtual iLight* GetFakeLight();
  virtual float GetVis(int x, int y) const;

};


/// class to fake a iLight interface
class csIsoFakeLight : public iLight {
  csIsoLight *isolight;

public:
  DECLARE_IBASE;

  csIsoFakeLight(csIsoLight *par) {isolight = par;}
  virtual ~csIsoFakeLight() {}

  //------------ iLight ------------------------------------------
  virtual csLight* GetPrivateObject () {return 0;}
  virtual iObject *QueryObject() {return 0;}
  virtual const csVector3& GetCenter () {return isolight->GetPosition();}
  virtual void SetCenter (const csVector3& pos) {isolight->SetPosition(pos);}
  virtual iSector *GetSector () {return 0;}
  virtual void SetSector (iSector* ) {}
  virtual float GetRadius () {return isolight->GetRadius();}
  virtual float GetSquaredRadius () 
  //{ return isolight->GetRadius()*isolight->GetRadius(); }
  // for use with checking if lighting needs to be done
  { return isolight->MaxRadius()*isolight->MaxRadius(); }
  virtual float GetInverseRadius () {return 1./isolight->GetRadius();}
  virtual void SetRadius (float r) {isolight->SetRadius(r);}
  virtual const csColor& GetColor () {return isolight->GetColor();}
  virtual void SetColor (const csColor& col) {isolight->SetColor(col);}
  virtual int GetAttenuation () {return isolight->GetAttenuation();}
  virtual void SetAttenuation (int a) {isolight->SetAttenuation(a);}
  virtual iCrossHalo* CreateCrossHalo (float, float) {return 0;}
  virtual iNovaHalo* CreateNovaHalo (int, int, float) {return 0;}
  virtual iFlareHalo* CreateFlareHalo () {return 0;}
  virtual float GetBrightnessAtDistance (float d)
  { return isolight->GetAttenuation(d); }
};

#endif
