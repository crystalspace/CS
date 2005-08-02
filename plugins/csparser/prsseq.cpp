/*
    Copyright (C) 2002 by Jorrit Tyberghein

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
#include "csqint.h"
#include "csloader.h"

#include "csgeom/sphere.h"
#include "csutil/cscolor.h"
#include "csutil/scanstr.h"
#include "csutil/scf.h"
#include "iengine/engine.h"
#include "iengine/light.h"
#include "iengine/material.h"
#include "iengine/mesh.h"
#include "iengine/region.h"
#include "iengine/sector.h"
#include "iengine/sharevar.h"
#include "imap/ldrctxt.h"
#include "imesh/object.h"
#include "imesh/thing.h"
#include "iutil/document.h"
#include "iutil/object.h"
#include "iutil/objreg.h"
#include "ivaria/sequence.h"

static iSequenceWrapper* FindSequence (iEngineSequenceManager* eseqmgr,
		const char* name)
{
  return eseqmgr->FindSequenceByName (name);
}

static iSequenceWrapper* CreateSequence (iEngineSequenceManager* eseqmgr,
		const char* name)
{
  iSequenceWrapper* sequence = eseqmgr->FindSequenceByName (name);
  if (sequence) return 0;	// Error! Already exists!
  // We don't need the ref returned by CreateSequence().
  csRef<iSequenceWrapper> seqwrap = eseqmgr->CreateSequence (name);
  sequence = seqwrap;
  return sequence;
}

static iSequenceTrigger* FindTrigger (iEngineSequenceManager* eseqmgr,
		const char* name)
{
  return eseqmgr->FindTriggerByName (name);
}

static iSequenceTrigger* CreateTrigger (iEngineSequenceManager* eseqmgr,
		const char* name)
{
  iSequenceTrigger* trigger = eseqmgr->FindTriggerByName (name);
  if (trigger) return 0;	// Error! Already exists!
  // We don't need the ref returned by CreateTrigger().
  csRef<iSequenceTrigger> trigwrap = eseqmgr->CreateTrigger (name);
  trigger = trigwrap;
  return trigger;
}

#define PARTYPE_LIGHT 0
#define PARTYPE_MESH 1
#define PARTYPE_MATERIAL 2
#define PARTYPE_SECTOR 3
#define PARTYPE_TRIGGER 4
#define PARTYPE_SEQUENCE 5
#define PARTYPE_POLYGON 6

csPtr<iParameterESM> csLoader::ResolveOperationParameter (
	iLoaderContext* ldr_context,
	iDocumentNode* opnode,
	int partypeidx, const char* partype, const char* seqname,
	iEngineSequenceParameters* base_params)
{
  bool do_ref = false;
  const char* parname = opnode->GetAttributeValue (partype);
  if (!parname)
  {
    do_ref = true;
    char buf[200];
    strcpy (buf, partype);
    strcat (buf, "_par");
    parname = opnode->GetAttributeValue (buf);
  }
  if (!parname)
  {
    SyntaxService->ReportError (
      "crystalspace.maploader.parse.sequence", opnode,
      "Missing attribute '%s' or '%s_par' in sequence '%s'!",
      partype, partype, seqname);
    return 0;
  }

  if (do_ref)
  {
    if (!base_params)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequence", opnode,
	"No parameters defined in sequence '%s'!",
	seqname);
      return 0;
    }
    csRef<iParameterESM> par = base_params->CreateParameterESM (parname);
    if (par == 0)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequence", opnode,
	"Parameter '%s' is not defined in sequence '%s'!",
	parname, seqname);
      return 0;
    }
    return csPtr<iParameterESM>(par);
  }
  else
  {
    csRef<iBase> value;
    switch (partypeidx)
    {
      case PARTYPE_LIGHT:
	{
	  iLight* l = Engine->FindLight (parname);
	  if (l) value = l;
	}
        break;
      case PARTYPE_MESH:
	value = ldr_context->FindMeshObject (parname);
        break;
      case PARTYPE_MATERIAL:
	value = Engine->FindMaterial (parname);
        break;
      case PARTYPE_SECTOR:
	value = ldr_context->FindSector (parname);
        break;
      case PARTYPE_TRIGGER:
	value = FindTrigger (GetEngineSequenceManager (), parname);
	if (!value)
	{
	  iSequenceTrigger* trig = CreateTrigger (
	  	GetEngineSequenceManager (), parname);
	  AddToRegion (ldr_context, trig->QueryObject ());
	  value = trig;
	}
        break;
      case PARTYPE_SEQUENCE:
	value = FindSequence (GetEngineSequenceManager (), parname);
        break;
      case PARTYPE_POLYGON:
	{
	  const char* meshname = opnode->GetAttributeValue ("mesh");
	  if (!meshname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", opnode,
	      "Missing 'mesh' attribute in sequence '%s'!",
	      seqname);
	    return 0;
	  }
	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", opnode,
	      "Couldn't find mesh '%s' in sequence '%s'!",
	      meshname, seqname);
	    return 0;
	  }
	  csRef<iThingState> st = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
		iThingState);
	  if (!st)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", opnode,
	      "Mesh '%s' is not a thing (sequence '%s')!",
	      meshname, seqname);
	    return 0;
	  }
	  int poly_idx = st->GetFactory ()->FindPolygonByName (parname);
	  if (poly_idx == -1)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", opnode,
	      "Couldn't find polygon '%s' in mesh '%s' (sequence '%s')!",
	      parname, meshname, seqname);
	    return 0;
	  }
	  csRef<iPolygonHandle> h = st->CreatePolygonHandle (poly_idx);
	  value = (iBase*)h;
	}
        break;
    }
    if (!value)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequence",
	opnode, "Couldn't find %s '%s' (sequence '%s)'!",
	partype, parname, seqname);
      return 0;
    }
    csRef<iParameterESM> par = eseqmgr->CreateParameterESM (value);
    return csPtr<iParameterESM>(par);
  }
}

csPtr<iEngineSequenceParameters> csLoader::CreateSequenceParameters (
	iLoaderContext* ldr_context,
	iSequenceWrapper* sequence, iDocumentNode* node,
	const char* parenttype, const char* parentname, bool& error)
{
  error = false;
  csRef<iEngineSequenceParameters> params = sequence->CreateParameterBlock ();
  size_t found_params = 0;
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;

    if (!params)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequenceparams", child,
	"Sequence '%s' doesn't have parameters (%s '%s')!",
	sequence->QueryObject ()->GetName (), parenttype, parentname);
      error = true;
      return 0;
    }

    const char* parname = child->GetAttributeValue ("name");
    if (!parname)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequenceparams", child,
	"Missing 'name' attribute in %s '%s'!",
	parenttype, parentname);
      error = true;
      return 0;
    }
    size_t idx = params->GetParameterIdx (parname);
    if (idx == csArrayItemNotFound)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequenceparams", child,
	"Bad parameter '%s' for sequence '%s' (%s '%s')!",
	parname, sequence->QueryObject ()->GetName (), parenttype, parentname);
      error = true;
      return 0;
    }

    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_MESHOBJ:
	{
	  const char* meshname = child->GetAttributeValue ("mesh");
	  if (!meshname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Missing 'mesh' attribute in %s '%s'!",
	      parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find mesh '%s' in %s '%s'!",
	      meshname, parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  params->SetParameter (idx, mesh);
	  found_params++;
	}
	break;
      case XMLTOKEN_LIGHT:
	{
	  const char* lightname = child->GetAttributeValue ("light");
	  if (!lightname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Missing 'light' attribute in %s '%s'!",
	      parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  iLight* light = Engine->FindLight (lightname);
	  if (!light)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find light '%s' in %s '%s'!",
	      lightname, parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  params->SetParameter (idx, light);
	  found_params++;
	}
	break;
      case XMLTOKEN_SECTOR:
	{
	  const char* sectname = child->GetAttributeValue ("sector");
	  if (!sectname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Missing 'sector' attribute in %s '%s'!",
	      parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  iSector* sector = ldr_context->FindSector (sectname);
	  if (!sector)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find sector '%s' in %s '%s'!",
	      sectname, parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  params->SetParameter (idx, (iBase*)sector);
	  found_params++;
	}
	break;
      case XMLTOKEN_MATERIAL:
	{
	  const char* matname = child->GetAttributeValue ("material");
	  if (!matname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Missing 'material' attribute in %s '%s'!",
	      parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  iMaterialWrapper* mat = Engine->FindMaterial (matname);
	  if (!mat)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find material '%s' in %s '%s'!",
	      matname, parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  params->SetParameter (idx, (iBase*)mat);
	  found_params++;
	}
	break;
      case XMLTOKEN_POLYGON:
	{
	  const char* meshname = child->GetAttributeValue ("mesh");
	  if (!meshname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Missing 'mesh' attribute in %s '%s'!",
	      parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find mesh '%s' in %s '%s'!",
	      meshname, parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  const char* polyname = child->GetAttributeValue ("polygon");
	  if (!polyname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Missing 'polygon' attribute in %s '%s'!",
	      parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  csRef<iThingState> st = SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
		iThingState);
	  if (!st)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Mesh '%s' is not a thing (%s '%s')!",
	      meshname, parenttype, parentname);
	    error = true;
	    return 0;
	  }
	  int polygon = st->GetFactory ()->FindPolygonByName (polyname);
	  if (polygon == -1)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find polygon '%s' in mesh '%s' (%s '%s')!",
	      polyname, meshname, parenttype, parentname);
	    error = true;
	    return 0;
	  }

	  csRef<iPolygonHandle> poly_handle = st->CreatePolygonHandle (polygon);
	  params->SetParameter (idx, (iBase*)poly_handle);
	  found_params++;
	}
	break;
      default:
	SyntaxService->ReportBadToken (child);
	error = true;
	return 0;
    }
  }
  if (params && found_params != params->GetParameterCount ())
  {
    SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequenceparams", node,
	"Missing parameters for firing sequence in %s '%s'!",
	parenttype, parentname);
    error = true;
    return 0;
  }

  return csPtr<iEngineSequenceParameters> (params);
}

iSequenceTrigger* csLoader::LoadTrigger (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  const char* trigname = node->GetAttributeValue ("name");
  // LoadTriggers() already checked for the presence of the engine
  // sequence manager.

  iSequenceTrigger* trigger = FindTrigger (
	GetEngineSequenceManager (), trigname);
  if (!trigger)
  {
    trigger = CreateTrigger (GetEngineSequenceManager (), trigname);
    AddToRegion (ldr_context, trigger->QueryObject ());
  }

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ONCLICK:
	{
	  const char* meshname = child->GetAttributeValue ("mesh");
	  if (!meshname)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find 'mesh' attribute in trigger '%s'!",
		trigname);
	    return 0;
	  }

	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find mesh '%s' in trigger '%s'!", meshname,
		trigname);
	    return 0;
	  }
	  trigger->AddConditionMeshClick (mesh);
	}
	break;
      case XMLTOKEN_LIGHTVALUE:
	{
	  const char *lightname = child->GetAttributeValue ("light");
	  if (!lightname)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find 'light' attribute in trigger '%s'!",
		trigname);
	    return 0;
	  }
	  iLight* light = ldr_context->FindLight (lightname);
	  if (!light)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find light '%s' in trigger '%s'!", lightname,
		trigname);
	    return 0;
	  }
	  int oper = -1;
	  float r,g,b;
	  const char *operation = child->GetAttributeValue ("operator");
	  if (!operation)
	      oper = CS_SEQUENCE_LIGHTCHANGE_NONE;
	  else if (!strcmp (operation,"less"))
	      oper = CS_SEQUENCE_LIGHTCHANGE_LESS;
	  else if (!strcmp (operation,"greater"))
	      oper = CS_SEQUENCE_LIGHTCHANGE_GREATER;
	  r = child->GetAttributeValueAsFloat ("red");
	  g = child->GetAttributeValueAsFloat ("green");
	  b = child->GetAttributeValueAsFloat ("blue");
	  const csColor c(r,g,b);
	  trigger->AddConditionLightChange (light, oper, c);
	}
	break;
      case XMLTOKEN_MANUAL:
	{
	  trigger->AddConditionManual();
	}
	break;
      case XMLTOKEN_SECTORVIS:
	{
	  const char* sectname = child->GetAttributeValue ("sector");
	  if (!sectname)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find 'sector' attribute in trigger '%s'!",
		trigname);
	    return 0;
	  }

	  iSector* sector = ldr_context->FindSector (sectname);
	  if (!sector)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find sector '%s' in trigger '%s'!", sectname,
		trigname);
	    return 0;
	  }
	  bool insideonly = false;
	  csRef<iDocumentNode> ionode = child->GetNode ("insideonly");
	  csRef<iDocumentNode> boxnode = child->GetNode ("box");
	  csRef<iDocumentNode> spherenode = child->GetNode ("sphere");
	  if (ionode)
	    if (!SyntaxService->ParseBool (ionode, insideonly, true))
	      return 0;
	  if (boxnode)
	  {
	    csBox3 box;
	    if (!SyntaxService->ParseBox (boxnode, box))
	      return 0;
	    trigger->AddConditionInSector (sector, box);
	  }
	  else if (spherenode)
	  {
	    csSphere s;
	    s.GetCenter ().x = spherenode->GetAttributeValueAsFloat ("x");
	    s.GetCenter ().y = spherenode->GetAttributeValueAsFloat ("y");
	    s.GetCenter ().z = spherenode->GetAttributeValueAsFloat ("z");
	    s.SetRadius (spherenode->GetAttributeValueAsFloat ("radius"));
	    trigger->AddConditionInSector (sector, s);
	  }
	  else if (insideonly)
	    trigger->AddConditionInSector (sector);
	  else
	    trigger->AddConditionSectorVisible (sector);
	}
        break;
      case XMLTOKEN_FIRE:
	{
	  const char* seqname = child->GetAttributeValue ("sequence");
	  if (!seqname)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find 'sequence' attribute in trigger '%s'!",
		trigname);
	    return 0;
	  }

	  iSequenceWrapper* sequence = FindSequence (
		GetEngineSequenceManager (), seqname);
	  if (!sequence)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find sequence '%s' in trigger '%s'!",
		seqname, trigname);
	    return 0;
	  }

	  csTicks delay = child->GetAttributeValueAsInt ("delay");

	  bool error;
	  csRef<iEngineSequenceParameters> params = CreateSequenceParameters (
	  	ldr_context, sequence, child, "trigger", trigname, error);
	  if (error)
	  {
	    // There was an error already reported by CreateSequenceParameters.
	    return 0;
	  }

	  if (params) trigger->SetParameters (params);
	  trigger->FireSequence (delay, sequence);
	}
        break;
      case XMLTOKEN_DISABLE:
        {
	  trigger->SetEnabled (false);
	}
	break;
      default:
        SyntaxService->ReportBadToken (child);
	return 0;
    }
  }

  return trigger;
}

bool csLoader::LoadTriggers (iLoaderContext* ldr_context, iDocumentNode* node)
{
  iEngineSequenceManager* sm = GetEngineSequenceManager ();
  if (!sm) return false;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_TRIGGER:
        if (!LoadTrigger (ldr_context, child))
	  return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

iSequenceWrapper* csLoader::CreateSequence (iDocumentNode* node)
{
  const char* seqname = node->GetAttributeValue ("name");
  // LoadSequences() already checked for the presence of the engine
  // sequence manager.

  iSequenceWrapper* sequence = ::CreateSequence (
	GetEngineSequenceManager (), seqname);
  if (!sequence)
  {
    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		node, "Duplicate sequence '%s'!", seqname);
    return 0;
  }

  csRef<iDocumentNode> argsnode = node->GetNode ("args");
  if (argsnode)
  {
    iEngineSequenceParameters* params = sequence->CreateBaseParameterBlock ();
    csRef<iDocumentNodeIterator> it = argsnode->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      csStringID id = xmltokens.Request (value);
      switch (id)
      {
        case XMLTOKEN_ARG:
	  {
	    const char* parname = child->GetAttributeValue ("name");
	    params->AddParameter (parname, 0);
	  }
	  break;
        default:
          SyntaxService->ReportBadToken (child);
	  return 0;
      }
    }
  }

  return sequence;
}

iSequenceWrapper* csLoader::LoadSequence (iLoaderContext* ldr_context,
	iDocumentNode* node)
{
  const char* seqname = node->GetAttributeValue ("name");
  // LoadSequences() already checked for the presence of the engine
  // sequence manager.

  iSequenceWrapper* sequence = FindSequence (
	GetEngineSequenceManager (), seqname);
  CS_ASSERT (sequence != 0);

  iEngineSequenceParameters* base_params = sequence->GetBaseParameterBlock ();

  csTicks cur_time = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_ARGS:
	// Already handled in CreateSequence().
	break;
      case XMLTOKEN_RUN:
        {
	  const char* seqname2 = child->GetAttributeValue ("sequence");
	  if (!seqname2)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Missing 'sequence' in sequence '%s'!",
		seqname);
	    return 0;
	  }
	  iSequenceWrapper* sequence2 = FindSequence (
		GetEngineSequenceManager (), seqname2);
	  if (!sequence2)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Can't find sequence '%s' in sequence '%s'!",
		seqname2, seqname);
	    return 0;
	  }
	  bool error;
	  csRef<iEngineSequenceParameters> params2 = CreateSequenceParameters (
	  	ldr_context, sequence2, child, "sequence", seqname, error);
	  if (error)
	  {
	    // There was an error already reported by CreateSequenceParameters.
	    return 0;
	  }

	  sequence->GetSequence ()->AddRunSequence (cur_time,
	  	sequence2->GetSequence (), (iBase*)params2);
	}
	break;
      case XMLTOKEN_DELAY:
        {
	  int min,max,time;
	  min  = child->GetAttributeValueAsInt ("min");
	  max  = child->GetAttributeValueAsInt ("max");
	  time = child->GetAttributeValueAsInt ("time");
	  if (!time && !(min || max))
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequence", child,
	      "Delay tag in sequence '%s' must specify time, or min and max!",
	      seqname);
	    return 0;
	  }
	  if (!time)
	  {
	    sequence->AddOperationRandomDelay (cur_time, min, max);
	    cur_time += min;
	  }
	  else
	    cur_time += time;  // standard delay can be hardcoded into next op
	}
	break;
      case XMLTOKEN_MOVELIGHT:
        {
	  csRef<iParameterESM> light = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_LIGHT, "light", seqname, base_params);
	  if (!light) return 0;

	  int duration = child->GetAttributeValueAsInt ("duration");
	  csVector3 offset;
	  offset.x = child->GetAttributeValueAsFloat ("x");
	  offset.y = child->GetAttributeValueAsFloat ("y");
	  offset.z = child->GetAttributeValueAsFloat ("z");

	  sequence->AddOperationMoveDuration (cur_time, light,
		offset, duration);
        }
	break;
      case XMLTOKEN_MOVE:
        {
	  csRef<iParameterESM> mesh = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return 0;

	  int duration = child->GetAttributeValueAsInt ("duration");
	  csVector3 offset;
	  offset.x = child->GetAttributeValueAsFloat ("x");
	  offset.y = child->GetAttributeValueAsFloat ("y");
	  offset.z = child->GetAttributeValueAsFloat ("z");

	  sequence->AddOperationMoveDuration (cur_time, mesh,
		offset, duration);
        }
	break;
      case XMLTOKEN_ROTATE:
        {
	  csRef<iParameterESM> mesh = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return 0;

	  int nr = 0;
	  int axis1 = -1, axis2 = -1, axis3 = -1;
	  csVector3 offset (0);
	  float tot_angle1 = 0, tot_angle2 = 0, tot_angle3 = 0;
	  int duration = child->GetAttributeValueAsInt ("duration");
	  csRef<iDocumentNodeIterator> it2 = child->GetNodes ();
	  while (it2->HasNext ())
	  {
	    csRef<iDocumentNode> child2 = it2->Next ();
	    if (child2->GetType () != CS_NODE_ELEMENT) continue;
	    const char* value2 = child2->GetValue ();
	    csStringID id2 = xmltokens.Request (value2);
	    switch (id2)
	    {
	      case XMLTOKEN_ROTX:
	      case XMLTOKEN_ROTY:
	      case XMLTOKEN_ROTZ:
	      {
	        int axis = id2 == XMLTOKEN_ROTX ? 0 :
			id2 == XMLTOKEN_ROTY ? 1 : 2;
	        switch (nr)
		{
		  case 0:
		    axis1 = axis;
		    tot_angle1 = child2->GetContentsValueAsFloat ();
		    break;
		  case 1:
		    axis2 = axis;
		    tot_angle2 = child2->GetContentsValueAsFloat ();
		    break;
		  case 2:
		    axis3 = axis;
		    tot_angle3 = child2->GetContentsValueAsFloat ();
		    break;
		  default:
		    SyntaxService->ReportError (
			"crystalspace.maploader.parse.sequence",
			child2, "Maximum 3 rotations in sequence '%s'!",
			seqname);
		    return 0;
		}
	        nr++;
	        break;
	      }
	      case XMLTOKEN_V:
	        if (!SyntaxService->ParseVector (child2, offset))
		  return 0;
		break;
	      case XMLTOKEN_AUTOOFFSET:
		{
		  csRef<iMeshWrapper> mw = SCF_QUERY_INTERFACE (mesh->GetValue(),
		    iMeshWrapper);
		  if (mw)
		  {
		    csBox3 box;
		    mw->GetWorldBoundingBox (box);
		    offset = (box.Min() + box.Max()) / 2.0f;
		  }
		}
		break;
	      default:
		SyntaxService->ReportBadToken (child2);
		return 0;
	    }
	  }

	  sequence->AddOperationRotateDuration (cur_time, mesh,
	  	axis1, tot_angle1, axis2, tot_angle2,
	  	axis3, tot_angle3, offset, duration);
	}
	break;
      case XMLTOKEN_MATERIAL:
        {
	  csRef<iParameterESM> mesh = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return 0;
	  csRef<iParameterESM> mat = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_MATERIAL, "material", seqname, base_params);
	  if (!mat) return 0;

	  // optional polygon parameter.
	  csRef<iParameterESM> polygon = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_POLYGON, "polygon", seqname, base_params);

	  if (polygon)
	    sequence->AddOperationSetPolygonMaterial (cur_time, polygon, mat);
	  else
	    sequence->AddOperationSetMaterial (cur_time, mesh, mat);
	}
        break;
      case XMLTOKEN_FADECOLOR:
        {
	  csRef<iParameterESM> mesh = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return 0;
	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  int duration = child->GetAttributeValueAsInt ("duration");
	  sequence->AddOperationFadeMeshColor (cur_time, mesh, col,
	  	duration);
	}
	break;
      case XMLTOKEN_SETCOLOR:
        {
	  csRef<iParameterESM> mesh = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return 0;
	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  sequence->AddOperationSetMeshColor (cur_time, mesh, col);
	}
        break;
      case XMLTOKEN_FADELIGHT:
        {
	  csRef<iParameterESM> light = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_LIGHT, "light", seqname, base_params);
	  if (!light) return 0;

	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  int duration = child->GetAttributeValueAsInt ("duration");

	  sequence->AddOperationFadeLight (cur_time,
	    	light, col, duration);
	}
	break;
      case XMLTOKEN_SETLIGHT:
        {
	  csRef<iParameterESM> light = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_LIGHT, "light", seqname, base_params);
	  if (!light) return 0;

	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  sequence->AddOperationSetLight (cur_time, light, col);
	}
        break;
      case XMLTOKEN_FADEAMBIENT:
        {
	  csRef<iParameterESM> sector = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_SECTOR, "sector", seqname, base_params);
	  if (!sector) return 0;

	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  int duration = child->GetAttributeValueAsInt ("duration");

	  sequence->AddOperationFadeAmbient (cur_time,
	    	sector, col, duration);
	}
	break;
      case XMLTOKEN_SETVAR:
        {
	  iSharedVariable *var = 0;
	  const char* varname;
	  if ((varname = child->GetAttributeValue ("var")) == 0)
	  {
	    SyntaxService->ReportError (
			"crystalspace.maploader.parse.sequence", child,
			"Please specify a variable name ('var' attribute)!");
	    return 0;
	  }
	  var = FindSharedVariable (varname, iSharedVariable::SV_UNKNOWN);
	  if (!var)
	  {
	    SyntaxService->ReportError (
			"crystalspace.maploader.parse.sequence", child,
			"Shared variable '%s' not found for 'var'!",
			varname);
	    return 0;
	  }
	  csRef<iDocumentAttribute> value_a = child->GetAttribute ("value");
	  if (value_a)
	  {
	    float v = value_a->GetValueAsFloat ();
	    sequence->AddOperationSetVariable (cur_time, var, v, 0);
	    break;
	  }
	  csRef<iDocumentAttribute> add_a = child->GetAttribute ("add");
	  if (add_a)
	  {
	    float v = add_a->GetValueAsFloat ();
	    if (var->GetType () != iSharedVariable::SV_FLOAT)
	    {
	      SyntaxService->ReportError (
			  "crystalspace.maploader.parse.sequence", child,
			  "Variable '%s' is not a float variable!",
			  varname);
	      return 0;
	    }
	    sequence->AddOperationSetVariable (cur_time, var, 0, v);
	    break;
	  }
	  const char* value_var_name = child->GetAttributeValue ("value_var");
	  if (value_var_name)
	  {
	    iSharedVariable* srcvar = FindSharedVariable (value_var_name,
	    	iSharedVariable::SV_UNKNOWN);
	    if (!srcvar)
	    {
	      SyntaxService->ReportError (
			  "crystalspace.maploader.parse.sequence", child,
			  "Shared variable '%s' not found for 'value_var'!",
			  value_var_name);
	      return 0;
	    }
	    sequence->AddOperationSetVariable (cur_time, var, srcvar, 0);
	    break;
	  }
	  const char* add_var_name = child->GetAttributeValue ("add_var");
	  if (add_var_name)
	  {
	    iSharedVariable* addvar = FindSharedVariable (add_var_name,
	    	iSharedVariable::SV_UNKNOWN);
	    if (!addvar)
	    {
	      SyntaxService->ReportError (
			  "crystalspace.maploader.parse.sequence", child,
			  "Shared variable '%s' not found for 'add_var'!",
			  add_var_name);
	      return 0;
	    }
	    if (addvar->GetType () != iSharedVariable::SV_FLOAT)
	    {
	      SyntaxService->ReportError (
			  "crystalspace.maploader.parse.sequence", child,
			  "Variable '%s' is not a float variable!",
			  varname);
	      return 0;
	    }
	    sequence->AddOperationSetVariable (cur_time, var, 0, addvar);
	    break;
	  }
	  csRef<iDocumentAttribute> x_a = child->GetAttribute ("x");
	  if (x_a)
	  {
	    csVector3 v;
	    v.x = child->GetAttributeValueAsFloat ("x");
	    v.y = child->GetAttributeValueAsFloat ("y");
	    v.z = child->GetAttributeValueAsFloat ("z");
	    sequence->AddOperationSetVariable (cur_time, var, v);
	    break;
	  }
	  csRef<iDocumentAttribute> red_a = child->GetAttribute ("red");
	  if (red_a)
	  {
	    csColor c;
	    c.red = child->GetAttributeValueAsFloat ("red");
	    c.green = child->GetAttributeValueAsFloat ("green");
	    c.blue = child->GetAttributeValueAsFloat ("blue");
	    sequence->AddOperationSetVariable (cur_time, var, c);
	    break;
	  }
	  SyntaxService->ReportError (
			  "crystalspace.maploader.parse.sequence", child,
			  "Invalid operation on shared variable '%s'!",
			  varname);
	  return 0;
	}
	break;
      case XMLTOKEN_SETAMBIENT:
        {
	  csRef<iParameterESM> sector = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_SECTOR, "sector", seqname, base_params);
	  if (!sector) return 0;
	  iSharedVariable *var = 0;
	  csColor col;
	  const char *colvar;
	  if ((colvar = child->GetAttributeValue ("color_var")) != 0)
	  { // variable set at run time
	    var = FindSharedVariable (colvar, iSharedVariable::SV_COLOR);
	    if (!var)
	    {
	      SyntaxService->ReportError (
			"crystalspace.maploader.parse.sequence", child,
			"Shared variable '%s' not found or not a color!",
			colvar);
	      return 0;
	    }
	  }
	  else
	  {
	    col.red = child->GetAttributeValueAsFloat ("red");
	    col.green = child->GetAttributeValueAsFloat ("green");
	    col.blue = child->GetAttributeValueAsFloat ("blue");
	  }
	  sequence->AddOperationSetAmbient (cur_time, sector, col, var);
	}
        break;
      case XMLTOKEN_FADEFOG:
        {
	  csRef<iParameterESM> sector = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_SECTOR, "sector", seqname, base_params);
	  if (!sector) return 0;

	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  float density;
	  density = child->GetAttributeValueAsFloat ("density");
	  int duration = child->GetAttributeValueAsInt ("duration");
	  sequence->AddOperationFadeFog (cur_time, sector, col, density,
	  	duration);
	}
	break;
      case XMLTOKEN_SETFOG:
        {
	  csRef<iParameterESM> sector = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_SECTOR, "sector", seqname, base_params);
	  if (!sector) return 0;

	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  float density;
	  density = child->GetAttributeValueAsFloat ("density");
	  sequence->AddOperationSetFog (cur_time, sector, col, density);
	}
        break;
      case XMLTOKEN_ENABLE:
        {
	  csRef<iParameterESM> trigger = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return 0;

	  sequence->AddOperationTriggerState (cur_time,
	  	trigger, true);
	}
        break;
      case XMLTOKEN_DISABLE:
        {
	  csRef<iParameterESM> trigger = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return 0;

	  sequence->AddOperationTriggerState (cur_time,
	  	trigger, false);
	}
        break;
      case XMLTOKEN_CHECK:
        {
	  csRef<iParameterESM> trigger = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return 0;

	  csTicks delay = child->GetAttributeValueAsInt ("delay");
	  sequence->AddOperationCheckTrigger (cur_time,
	  	trigger, delay);
	}
        break;
      case XMLTOKEN_TEST:
        {
	  csRef<iParameterESM> trigger = ResolveOperationParameter (
	  	ldr_context,
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return 0;

	  iSequence* trueseq = 0;
	  const char* trueseqname = child->GetAttributeValue ("truesequence");
	  if (trueseqname)
	  {
	    iSequenceWrapper* trueseqwrap = FindSequence (
		GetEngineSequenceManager (), trueseqname);
	    if (!trueseqwrap)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.sequence",
		  child, "Can't find sequence '%s' in sequence '%s'!",
		  trueseqname, seqname);
	      return 0;
	    }
	    trueseq = trueseqwrap->GetSequence ();
	  }
	  iSequence* falseseq = 0;
	  const char* falseseqname = child->GetAttributeValue("falsesequence");
	  if (falseseqname)
	  {
	    iSequenceWrapper* falseseqwrap = FindSequence (
		GetEngineSequenceManager (), falseseqname);
	    if (!falseseqwrap)
	    {
	      SyntaxService->ReportError (
		  "crystalspace.maploader.parse.sequence",
		  child, "Can't find sequence '%s' in sequence '%s'!",
		  falseseqname, seqname);
	      return 0;
	    }
	    falseseq = falseseqwrap->GetSequence ();
	  }
	  sequence->AddOperationTestTrigger (cur_time,
	  	trigger, trueseq, falseseq);
	}
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return 0;
    }
  }

  return sequence;
}

bool csLoader::LoadSequences (iLoaderContext* ldr_context, iDocumentNode* node)
{
  iEngineSequenceManager* sm = GetEngineSequenceManager ();
  if (!sm) return false;

  // We load sequences in two passes. In the first pass we will
  // create all sequences and also create the parameter blocks.
  // In the second pass we will actually parse the operations.

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SEQUENCE:
        {
          iSequenceWrapper* sequence = CreateSequence (child);
          if (!sequence) return false;
	  AddToRegion (ldr_context, sequence->QueryObject ());
	}
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_SEQUENCE:
        if (!LoadSequence (ldr_context, child))
	  return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

iSharedVariable *csLoader::FindSharedVariable(const char *colvar,
					      int verify_type )
{
    iSharedVariable *found = Engine->GetVariableList ()->
				FindByName (colvar);
    if (found)
    {
      if (verify_type == iSharedVariable::SV_UNKNOWN ||
	  found->GetType() == verify_type)
	return found;
    }
    return 0;
}
