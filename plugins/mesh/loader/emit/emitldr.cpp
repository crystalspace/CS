/*
    Copyright (C) 2001 by Jorrit Tyberghein
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
#include "csgeom/math3d.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "emitldr.h"
#include "imesh/object.h"
#include "iengine/mesh.h"
#include "iengine/engine.h"
#include "isys/system.h"
#include "imesh/partsys.h"
#include "imesh/emit.h"
#include "ivideo/graph3d.h"
#include "qint.h"
#include "iutil/strvec.h"
#include "csutil/util.h"
#include "iutil/object.h"
#include "iengine/material.h"

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (TRANSPARENT)

  CS_TOKEN_DEF (AGING)
  CS_TOKEN_DEF (ATTRACTORFORCE)
  CS_TOKEN_DEF (ATTRACTOR)
  CS_TOKEN_DEF (EMITBOX)
  CS_TOKEN_DEF (EMITCONE)
  CS_TOKEN_DEF (EMITCYLINDERTANGENT)
  CS_TOKEN_DEF (EMITCYLINDER)
  CS_TOKEN_DEF (EMITFIXED)
  CS_TOKEN_DEF (EMITLINE)
  CS_TOKEN_DEF (EMITMIX)
  CS_TOKEN_DEF (EMITSPHERETANGENT)
  CS_TOKEN_DEF (EMITSPHERE)
  CS_TOKEN_DEF (FACTORY)
  CS_TOKEN_DEF (LIGHTING)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MIXMODE)
  CS_TOKEN_DEF (NUMBER)
  CS_TOKEN_DEF (RECTPARTICLES)
  CS_TOKEN_DEF (REGULARPARTICLES)
  CS_TOKEN_DEF (STARTACCEL)
  CS_TOKEN_DEF (STARTPOS)
  CS_TOKEN_DEF (STARTSPEED)
  CS_TOKEN_DEF (TOTALTIME)
  CS_TOKEN_DEF (WEIGHT)
CS_TOKEN_DEF_END

IMPLEMENT_IBASE (csEmitFactoryLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csEmitFactorySaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csEmitLoader)
  IMPLEMENTS_INTERFACE (iLoaderPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_IBASE (csEmitSaver)
  IMPLEMENTS_INTERFACE (iSaverPlugIn)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csEmitFactoryLoader)
IMPLEMENT_FACTORY (csEmitFactorySaver)
IMPLEMENT_FACTORY (csEmitLoader)
IMPLEMENT_FACTORY (csEmitSaver)

EXPORT_CLASS_TABLE (emitldr)
  EXPORT_CLASS (csEmitFactoryLoader, "crystalspace.mesh.loader.factory.emit",
    "Crystal Space Emit Factory Loader")
  EXPORT_CLASS (csEmitFactorySaver, "crystalspace.mesh.saver.factory.emit",
    "Crystal Space Emit Factory Saver")
  EXPORT_CLASS (csEmitLoader, "crystalspace.mesh.loader.emit",
    "Crystal Space Emit Mesh Loader")
  EXPORT_CLASS (csEmitSaver, "crystalspace.mesh.saver.emit",
    "Crystal Space Emit Mesh Saver")
EXPORT_CLASS_TABLE_END

csEmitFactoryLoader::csEmitFactoryLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csEmitFactoryLoader::~csEmitFactoryLoader ()
{
}

bool csEmitFactoryLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

iBase* csEmitFactoryLoader::Parse (const char* /*string*/,
	iEngine* /*engine*/, iBase* /* context */)
{
  iMeshObjectType* type = QUERY_PLUGIN_CLASS (sys,
  	"crystalspace.mesh.object.emit", "MeshObj", iMeshObjectType);
  if (!type)
  {
    type = LOAD_PLUGIN (sys, "crystalspace.mesh.object.emit",
    	"MeshObj", iMeshObjectType);
    printf ("Load TYPE plugin crystalspace.mesh.object.emit\n");
  }
  iMeshObjectFactory* fact = type->NewFactory ();
  type->DecRef ();
  return fact;
}

//---------------------------------------------------------------------------

csEmitFactorySaver::csEmitFactorySaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csEmitFactorySaver::~csEmitFactorySaver ()
{
}

bool csEmitFactorySaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

#define MAXLINE 100 /* max number of chars per line... */

static void WriteMixmode(iStrVector *str, UInt mixmode)
{
  str->Push(strnew("  MIXMODE ("));
  if(mixmode&CS_FX_COPY) str->Push(strnew(" COPY ()"));
  if(mixmode&CS_FX_ADD) str->Push(strnew(" ADD ()"));
  if(mixmode&CS_FX_MULTIPLY) str->Push(strnew(" MULTIPLY ()"));
  if(mixmode&CS_FX_MULTIPLY2) str->Push(strnew(" MULTIPLY2 ()"));
  if(mixmode&CS_FX_KEYCOLOR) str->Push(strnew(" KEYCOLOR ()"));
  if(mixmode&CS_FX_TRANSPARENT) str->Push(strnew(" TRANSPARENT ()"));
  if(mixmode&CS_FX_ALPHA)
  {
    char buf[MAXLINE];
    sprintf(buf, "ALPHA (%g)", float(mixmode&CS_FX_MASK_ALPHA)/255.);
    str->Push(strnew(buf));
  }
  str->Push(strnew(")"));
}

void csEmitFactorySaver::WriteDown (iBase* /*obj*/, iStrVector * /*str*/,
  iEngine* /*engine*/)
{
  // nothing to do
}

//---------------------------------------------------------------------------

csEmitLoader::csEmitLoader (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csEmitLoader::~csEmitLoader ()
{
}

bool csEmitLoader::Initialize (iSystem* system)
{
  sys = system;
  return true;
}

static UInt ParseMixmode (char* buf)
{
  CS_TOKEN_TABLE_START (modes)
    CS_TOKEN_TABLE (COPY)
    CS_TOKEN_TABLE (MULTIPLY2)
    CS_TOKEN_TABLE (MULTIPLY)
    CS_TOKEN_TABLE (ADD)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (KEYCOLOR)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  UInt Mixmode = 0;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return 0;
    }
    switch (cmd)
    {
      case CS_TOKEN_COPY: Mixmode |= CS_FX_COPY; break;
      case CS_TOKEN_MULTIPLY: Mixmode |= CS_FX_MULTIPLY; break;
      case CS_TOKEN_MULTIPLY2: Mixmode |= CS_FX_MULTIPLY2; break;
      case CS_TOKEN_ADD: Mixmode |= CS_FX_ADD; break;
      case CS_TOKEN_ALPHA:
	Mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
        ScanStr (params, "%f", &alpha);
	Mixmode |= CS_FX_SETALPHA(alpha);
	break;
      case CS_TOKEN_TRANSPARENT: Mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: Mixmode |= CS_FX_KEYCOLOR; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    printf ("Token '%s' not found while parsing the modes!\n",
    	csGetLastOffender ());
    return 0;
  }
  return Mixmode;
}


static iEmitGen3D* ParseEmit (char* buf, iEmitFactoryState *fstate,
	float* weight)
{
  CS_TOKEN_TABLE_START (emits)
    CS_TOKEN_TABLE (WEIGHT)
    CS_TOKEN_TABLE (EMITBOX)
    CS_TOKEN_TABLE (EMITCONE)
    CS_TOKEN_TABLE (EMITCYLINDERTANGENT)
    CS_TOKEN_TABLE (EMITCYLINDER)
    CS_TOKEN_TABLE (EMITFIXED)
    CS_TOKEN_TABLE (EMITLINE)
    CS_TOKEN_TABLE (EMITMIX)
    CS_TOKEN_TABLE (EMITSPHERETANGENT)
    CS_TOKEN_TABLE (EMITSPHERE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  iEmitGen3D* result = 0;
  iEmitMix *emix = 0;
  csVector3 a,b;
  float p,q,r,s,t;
  if (weight) *weight = 1.;

  while ((cmd = csGetObject (&buf, emits, &name, &params)) > 0)
  {
    if (!params)
    {
      printf ("Expected parameters instead of '%s'!\n", buf);
      return 0;
    }
    switch (cmd)
    {
      case CS_TOKEN_WEIGHT:
	if (weight == NULL)
	{
	  printf ("WEIGHT cannot be given in this context!\n");
	  return 0;
	}
        ScanStr (params, "%f", weight);
	break;
      case CS_TOKEN_EMITFIXED:
        {
          ScanStr (params, "%f,%f,%f", &a.x, &a.y, &a.z);
	  iEmitFixed *efixed = fstate->CreateFixed();
	  efixed->SetValue(a);
	  result = efixed;
	}
	break;
      case CS_TOKEN_EMITBOX:
        {
          ScanStr (params, "%f,%f,%f,%f,%f,%f", &a.x, &a.y, &a.z, 
	    &b.x, &b.y, &b.z);
	  iEmitBox *ebox = fstate->CreateBox();
	  ebox->SetContent(a, b);
	  result = ebox;
	}
	break;
      case CS_TOKEN_EMITSPHERE:
        {
          ScanStr (params, "%f,%f,%f,%f,%f,%f", &a.x, &a.y, &a.z, &p, &q);
	  iEmitSphere *esphere = fstate->CreateSphere();
	  esphere->SetContent(a, p, q);
	  result = esphere;
	}
	break;
      case CS_TOKEN_EMITCONE:
        {
          ScanStr (params, "%f,%f,%f,%f,%f,%f", &a.x, &a.y, &a.z, 
	    &p, &q, &r, &s, &t);
	  iEmitCone *econe = fstate->CreateCone();
	  econe->SetContent(a, p, q, r, s, t);
	  result = econe;
	}
	break;
      case CS_TOKEN_EMITMIX:
        {
	  if(!emix) emix = fstate->CreateMix();
	  float amt;
	  iEmitGen3D *gen;
	  gen = ParseEmit(params, fstate, &amt);
	  emix->AddEmitter(amt, gen);
	  result = emix;
	}
	break;
      case CS_TOKEN_EMITLINE:
        {
          ScanStr (params, "%f,%f,%f,%f,%f,%f", &a.x, &a.y, &a.z, 
	    &b.x, &b.y, &b.z);
	  iEmitLine *eline = fstate->CreateLine();
	  eline->SetContent(a, b);
	  result = eline;
	}
	break;
      case CS_TOKEN_EMITCYLINDER:
        {
          ScanStr (params, "%f,%f,%f,%f,%f,%f,%f,%f", &a.x, &a.y, &a.z, 
	    &b.x, &b.y, &b.z, &p, &q);
	  iEmitCylinder *ecyl = fstate->CreateCylinder();
	  ecyl->SetContent(a, b, p, q);
	  result = ecyl;
	}
	break;
      case CS_TOKEN_EMITCYLINDERTANGENT:
        {
          ScanStr (params, "%f,%f,%f,%f,%f,%f,%f,%f", &a.x, &a.y, &a.z, 
	    &b.x, &b.y, &b.z, &p, &q);
	  iEmitCylinderTangent *ecyltan = fstate->CreateCylinderTangent();
	  ecyltan->SetContent(a, b, p, q);
	  result = ecyltan;
	}
	break;
      case CS_TOKEN_EMITSPHERETANGENT:
        {
          ScanStr (params, "%f,%f,%f,%f,%f", &a.x, &a.y, &a.z, &p, &q);
	  iEmitSphereTangent *espheretan = fstate->CreateSphereTangent();
	  espheretan->SetContent(a, p, q);
	  result = espheretan;
	}
	break;
    }
  }
  return result;
}

iBase* csEmitLoader::Parse (const char* string, iEngine* engine,
	iBase* context)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (AGING)
    CS_TOKEN_TABLE (ATTRACTORFORCE)
    CS_TOKEN_TABLE (ATTRACTOR)
    CS_TOKEN_TABLE (FACTORY)
    CS_TOKEN_TABLE (LIGHTING)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MIXMODE)
    CS_TOKEN_TABLE (NUMBER)
    CS_TOKEN_TABLE (RECTPARTICLES)
    CS_TOKEN_TABLE (REGULARPARTICLES)
    CS_TOKEN_TABLE (STARTACCEL)
    CS_TOKEN_TABLE (STARTPOS)
    CS_TOKEN_TABLE (STARTSPEED)
    CS_TOKEN_TABLE (TOTALTIME)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  char str[255];

  iMeshWrapper* imeshwrap = QUERY_INTERFACE (context, iMeshWrapper);
  imeshwrap->DecRef ();

  iMeshObject* mesh = NULL;
  iParticleState* partstate = NULL;
  iEmitFactoryState* emitfactorystate = NULL;
  iEmitState* emitstate = NULL;

  char* buf = (char*)string;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      // @@@ Error handling!
      if (partstate) partstate->DecRef ();
      if (emitstate) emitstate->DecRef ();
      return NULL;
    }
    switch (cmd)
    {
      case CS_TOKEN_FACTORY:
	{
          ScanStr (params, "%s", str);
	  iMeshFactoryWrapper* fact = engine->FindMeshFactory (str);
	  if (!fact)
	  {
	    // @@@ Error handling!
	    if (partstate) partstate->DecRef ();
	    if (emitstate) emitstate->DecRef ();
	    return NULL;
	  }
	  mesh = fact->GetMeshObjectFactory ()->NewInstance ();
	  imeshwrap->SetFactory (fact);
          partstate = QUERY_INTERFACE (mesh, iParticleState);
          emitstate = QUERY_INTERFACE (mesh, iEmitState);
	  emitfactorystate = QUERY_INTERFACE(fact->GetMeshObjectFactory(),
	    iEmitFactoryState);
	}
	break;
      case CS_TOKEN_MATERIAL:
	{
          ScanStr (params, "%s", str);
          iMaterialWrapper* mat = engine->FindMaterial (str);
	  if (!mat)
	  {
	    printf ("Could not find material '%s'!\n", str);
            // @@@ Error handling!
            mesh->DecRef ();
	    if (partstate) partstate->DecRef ();
	    if (emitstate) emitstate->DecRef ();
            return NULL;
	  }
	  partstate->SetMaterialWrapper (mat);
	}
	break;
      case CS_TOKEN_MIXMODE:
        partstate->SetMixMode (ParseMixmode (params));
	break;
      case CS_TOKEN_NUMBER:
	{
	  int num = 10;
          ScanStr (params, "%d", &num);
	  emitstate->SetNumberParticles (num);
	}
	break;
      case CS_TOKEN_LIGHTING:
	{
	  bool light = false;
          ScanStr (params, "%b", &light);
	  emitstate->SetLighting (light);
	}
	break;
      case CS_TOKEN_TOTALTIME:
	{
	  int totaltime = 100;
          ScanStr (params, "%d", &totaltime);
	  emitstate->SetParticleTime (totaltime);
	}
	break;
      case CS_TOKEN_RECTPARTICLES:
	{
	  float w,h;
          ScanStr (params, "%f,%f", &w, &h);
	  emitstate->SetRectParticles (w, h);
	}
	break;
      case CS_TOKEN_REGULARPARTICLES:
	{
	  int sides;
	  float radius;
          ScanStr (params, "%d,%f", &sides, &radius);
	  emitstate->SetRegularParticles (sides, radius);
	}
	break;
      case CS_TOKEN_AGING:
        {
	  int time;
	  csColor col;
	  float alpha, swirl, rotspeed, scale;
          ScanStr (params, "%d,%f,%f,%f,%f,%f,%f,%f", &time, &col.red,
	    &col.green, &col.blue, &alpha, &swirl, &rotspeed, &scale);
	  emitstate->AddAge (time, col, alpha, swirl, rotspeed, scale);
	}
	break;
      case CS_TOKEN_STARTPOS:
	{
	  emitstate->SetStartPosEmit (ParseEmit(params, emitfactorystate,
	  	NULL));
	}
	break;
      case CS_TOKEN_STARTSPEED:
	{
	  emitstate->SetStartSpeedEmit (ParseEmit(params, emitfactorystate,
	  	NULL));
	}
	break;
      case CS_TOKEN_STARTACCEL:
	{
	  emitstate->SetStartAccelEmit (ParseEmit(params, emitfactorystate,
	  	NULL));
	}
	break;
      case CS_TOKEN_ATTRACTORFORCE:
	{
	  float force;
          ScanStr (params, "%f", &force);
	  emitstate->SetAttractorForce (force);
	}
	break;
      case CS_TOKEN_ATTRACTOR:
	{
	  emitstate->SetAttractorEmit (ParseEmit(params, emitfactorystate,
	  	NULL));
	}
	break;
    }
  }

  if (partstate) partstate->DecRef ();
  if (emitstate) emitstate->DecRef ();
  if (emitfactorystate) emitfactorystate->DecRef ();
  return mesh;
}

//---------------------------------------------------------------------------


csEmitSaver::csEmitSaver (iBase* pParent)
{
  CONSTRUCT_IBASE (pParent);
}

csEmitSaver::~csEmitSaver ()
{
}

bool csEmitSaver::Initialize (iSystem* system)
{
  sys = system;
  return true;
}


/// write emitter to string
static void WriteEmit(iStrVector *str, iEmitGen3D *emit)
{
  char buf[MAXLINE];
  csVector3 a,b;
  float p, q, r, s, t;
  iEmitFixed *efixed = QUERY_INTERFACE(emit, iEmitFixed);
  if(efixed)
  {
    /// b is ignored
    efixed->GetValue(a, b);
    sprintf(buf, "  EMITFIXED(%g, %g, %g)\n", a.x, a.y, a.z);
    str->Push(strnew(buf));
    efixed->DecRef();
    return;
  }
  iEmitSphere *esphere = QUERY_INTERFACE(emit, iEmitSphere);
  if(esphere)
  {
    esphere->GetContent(a, p, q);
    sprintf(buf, "  EMITSPHERE(%g,%g,%g, %g, %g)\n", a.x, a.y, a.z, p, q);
    str->Push(strnew(buf));
    esphere->DecRef();
    return;
  }
  iEmitBox *ebox = QUERY_INTERFACE(emit, iEmitBox);
  if(ebox)
  {
    ebox->GetContent(a, b);
    sprintf(buf, "  EMITBOX(%g,%g,%g, %g,%g,%g)\n", a.x,a.y,a.z, b.x,b.y,b.z);
    str->Push(strnew(buf));
    ebox->DecRef();
    return;
  }
  iEmitCone *econe = QUERY_INTERFACE(emit, iEmitCone);
  if(econe)
  {
    econe->GetContent(a, p, q, r, s, t);
    sprintf(buf, "  EMITCONE(%g,%g,%g, %g, %g, %g, %g, %g)\n", a.x,a.y,a.z, 
      p, q, r, s, t);
    str->Push(strnew(buf));
    econe->DecRef();
    return;
  }
  iEmitMix *emix = QUERY_INTERFACE(emit, iEmitMix);
  if(emix)
  {
    for(int i=0; i<emix->GetNumberEmitters(); i++)
    {
      float w;
      iEmitGen3D *gen;
      emix->GetContent(i, w, gen);
      sprintf(buf, "  EMITMIX( WEIGHT(%g)\n", w);
      str->Push(strnew(buf));
      WriteEmit(str, gen);
      str->Push(strnew("  )\n"));
    }
    emix->DecRef();
    return;
  }
  iEmitLine *eline = QUERY_INTERFACE(emit, iEmitLine);
  if(eline)
  {
    eline->GetContent(a, b);
    sprintf(buf, "  EMITLINE(%g,%g,%g, %g,%g,%g)\n", a.x,a.y,a.z, b.x,b.y,b.z);
    str->Push(strnew(buf));
    eline->DecRef();
    return;
  }
  iEmitCylinder *ecyl = QUERY_INTERFACE(emit, iEmitCylinder);
  if(ecyl)
  {
    ecyl->GetContent(a, b, p, q);
    sprintf(buf, "  EMITCYLINDER(%g,%g,%g, %g,%g,%g, %g, %g)\n", a.x,a.y,a.z, 
      b.x,b.y,b.z, p, q);
    str->Push(strnew(buf));
    ecyl->DecRef();
    return;
  }
  iEmitCylinderTangent *ecyltan = QUERY_INTERFACE(emit, iEmitCylinderTangent);
  if(ecyltan)
  {
    ecyltan->GetContent(a, b, p, q);
    sprintf(buf, "  EMITCYLINDERTANGENT(%g,%g,%g, %g,%g,%g, %g, %g)\n", 
      a.x,a.y,a.z, b.x,b.y,b.z, p, q);
    str->Push(strnew(buf));
    ecyltan->DecRef();
    return;
  }
  iEmitSphereTangent *espheretan = QUERY_INTERFACE(emit, iEmitSphereTangent);
  if(espheretan)
  {
    espheretan->GetContent(a, p, q);
    sprintf(buf, "  EMITSPHERETANGENT(%g,%g,%g, %g, %g)\n", a.x,a.y,a.z, p, q);
    str->Push(strnew(buf));
    espheretan->DecRef();
    return;
  }
  printf ("Unknown emitter type, cannot writedown!\n");
}

void csEmitSaver::WriteDown (iBase* obj, iStrVector *str,
  iEngine* /*engine*/)
{
  iFactory *fact = QUERY_INTERFACE (this, iFactory);
  iParticleState *partstate = QUERY_INTERFACE (obj, iParticleState);
  iEmitState *state = QUERY_INTERFACE (obj, iEmitState);
  char buf[MAXLINE];
  char name[MAXLINE];

  csFindReplace(name, fact->QueryDescription (), "Saver", "Loader", MAXLINE);
  sprintf(buf, "FACTORY ('%s')\n", name);
  str->Push(strnew(buf));

  if(partstate->GetMixMode() != CS_FX_COPY)
  {
    WriteMixmode(str, partstate->GetMixMode());
  }
  sprintf(buf, "MATERIAL (%s)\n", partstate->GetMaterialWrapper()->
    QueryObject()->GetName());
  str->Push(strnew(buf));
  printf(buf, "NUMBER (%d)\n", state->GetNumberParticles());
  str->Push(strnew(buf));
  sprintf(buf, "LIGHTING (%s)\n", state->GetLighting()?"true":"false");
  str->Push(strnew(buf));
  sprintf(buf, "TOTALTIME (%d)\n", state->GetParticleTime());
  str->Push(strnew(buf));
  if(state->UsingRectParticles())
  {
    float w,h; state->GetRectParticles(w,h);
    sprintf(buf, "RECTPARTICLES (%g, %g)\n", w, h);
  }
  else 
  {
    int n; float r; state->GetRegularParticles(n, r);
    sprintf(buf, "REGULARPARTICLES (%d, %g)\n", n, r);
  }
  str->Push(strnew(buf));

  str->Push(strnew("STARTPOS(\n"));
  WriteEmit(str, state->GetStartPosEmit());
  str->Push(strnew(")\n"));
  
  str->Push(strnew("STARTSPEED(\n"));
  WriteEmit(str, state->GetStartSpeedEmit());
  str->Push(strnew(")\n"));

  str->Push(strnew("STARTACCEL(\n"));
  WriteEmit(str, state->GetStartAccelEmit());
  str->Push(strnew(")\n"));

  if(state->GetAttractorEmit())
  {
    str->Push(strnew("ATTRACTOR(\n"));
    WriteEmit(str, state->GetAttractorEmit());
    str->Push(strnew(")\n"));
    sprintf(buf, "ATTRACTORFORCE (%g)\n", state->GetAttractorForce());
    str->Push(strnew(buf));
  }

  for(int i=0; i<state->GetNumberAging(); i++)
  {
    int time;
    csColor col;
    float alpha, swirl, rotspeed, scale;
    state->GetAgingMoment(i, time, col, alpha, swirl, rotspeed, scale);
    sprintf(buf, "AGING (%d, %g,%g,%g, %g, %g, %g, %g)\n", time, col.red,
      col.green, col.blue, alpha, swirl, rotspeed, scale);
    str->Push(strnew(buf));
  }

  fact->DecRef();
  partstate->DecRef();
  state->DecRef();
}

//---------------------------------------------------------------------------

