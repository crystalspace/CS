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
#include "qint.h"
#include "csloader.h"
#include "imap/ldrctxt.h"
#include "csutil/scanstr.h"
#include "csutil/cscolor.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/object.h"
#include "iengine/engine.h"
#include "iengine/region.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "iengine/mesh.h"
#include "imesh/thing/thing.h"
#include "imesh/object.h"
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
  if (sequence) return NULL;	// Error! Already exists!
  // We don't need the ref returned by CreateSequence().
  csRef<iSequenceWrapper> seqwrap = eseqmgr->CreateSequence (name);
  sequence = seqwrap;
  return sequence;
}

static iSequenceTrigger* FindCreateTrigger (iEngineSequenceManager* eseqmgr,
		const char* name)
{
  iSequenceTrigger* trigger = eseqmgr->FindTriggerByName (name);
  if (!trigger)
  {
    // We don't need the ref returned by CreateTrigger().
    csRef<iSequenceTrigger> trigwrap = eseqmgr->CreateTrigger (name);
    trigger = trigwrap;
  }
  return trigger;
}

#define PARTYPE_LIGHT 0
#define PARTYPE_MESH 1
#define PARTYPE_MATERIAL 2
#define PARTYPE_SECTOR 3
#define PARTYPE_TRIGGER 4
#define PARTYPE_SEQUENCE 5
#define PARTYPE_POLYGON 6

csPtr<iParameterESM> csLoader::ResolveOperationParameter (iDocumentNode* opnode,
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
    return NULL;
  }

  if (do_ref)
  {
    if (!base_params)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequence", opnode,
	"No parameters defined in sequence '%s'!",
	seqname);
      return NULL;
    }
    csRef<iParameterESM> par = base_params->CreateParameterESM (parname);
    if (par == NULL)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequence", opnode,
	"Parameter '%s' is not defined in sequence '%s'!",
	parname, seqname);
      return NULL;
    }
    return csPtr<iParameterESM>(par);
  }
  else
  {
    iBase* value = NULL;
    switch (partypeidx)
    {
      case PARTYPE_LIGHT:
	{
	  iStatLight* l = Engine->FindLight (parname);
	  if (l) value = l->QueryLight ();
	}
        break;
      case PARTYPE_MESH:
	value = ldr_context->FindMeshObject (parname);
        break;
      case PARTYPE_MATERIAL:
	value = (iBase*)Engine->FindMaterial (parname);
        break;
      case PARTYPE_SECTOR:
	value = (iBase*)ldr_context->FindSector (parname);
        break;
      case PARTYPE_TRIGGER:
	value = FindCreateTrigger (GetEngineSequenceManager (), parname);
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
	    return NULL;
	  }
	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", opnode,
	      "Couldn't find mesh '%s' in sequence '%s'!",
	      meshname, seqname);
	    return NULL;
	  }
	  csRef<iThingState> st (SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
		iThingState));
	  if (!st)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", opnode,
	      "Mesh '%s' is not a thing (sequence '%s')!",
	      meshname, seqname);
	    return NULL;
	  }
	  value = (iBase*)st->GetPolygon (parname);
	  if (!value)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", opnode,
	      "Couldn't find polygon '%s' in mesh '%s' (sequence '%s')!",
	      parname, meshname, seqname);
	    return NULL;
	  }
	}
        break;
    }
    if (!value)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequence",
	opnode, "Couldn't find %s '%s' (sequence '%s)'!",
	partype, parname, seqname);
      return NULL;
    }
    csRef<iParameterESM> par = eseqmgr->CreateParameterESM (value);
    return csPtr<iParameterESM>(par);
  }
}

csPtr<iEngineSequenceParameters> csLoader::CreateSequenceParameters (
	iSequenceWrapper* sequence, iDocumentNode* node,
	const char* parenttype, const char* parentname, bool& error)
{
  error = false;
  csRef<iEngineSequenceParameters> params = sequence->CreateParameterBlock ();
  int found_params = 0;
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
      return NULL;
    }

    const char* parname = child->GetAttributeValue ("name");
    if (!parname)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequenceparams", child,
	"Missing 'name' attribute in %s '%s'!",
	parenttype, parentname);
      error = true;
      return NULL;
    }
    int idx = params->GetParameterIdx (parname);
    if (idx == -1)
    {
      SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequenceparams", child,
	"Bad parameter '%s' for sequence '%s' (%s '%s')!",
	parname, sequence->QueryObject ()->GetName (), parenttype, parentname);
      error = true;
      return NULL;
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
	    return NULL;
	  }
	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find mesh '%s' in %s '%s'!",
	      meshname, parenttype, parentname);
	    error = true;
	    return NULL;
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
	    return NULL;
	  }
	  iStatLight* light = Engine->FindLight (lightname);
	  if (!light)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find light '%s' in %s '%s'!",
	      lightname, parenttype, parentname);
	    error = true;
	    return NULL;
	  }
	  params->SetParameter (idx, light->QueryLight ());
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
	    return NULL;
	  }
	  iSector* sector = ldr_context->FindSector (sectname);
	  if (!sector)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find sector '%s' in %s '%s'!",
	      sectname, parenttype, parentname);
	    error = true;
	    return NULL;
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
	    return NULL;
	  }
	  iMaterialWrapper* mat = Engine->FindMaterial (matname);
	  if (!mat)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find material '%s' in %s '%s'!",
	      matname, parenttype, parentname);
	    error = true;
	    return NULL;
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
	    return NULL;
	  }
	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find mesh '%s' in %s '%s'!",
	      meshname, parenttype, parentname);
	    error = true;
	    return NULL;
	  }
	  const char* polyname = child->GetAttributeValue ("polygon");
	  if (!polyname)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Missing 'polygon' attribute in %s '%s'!",
	      parenttype, parentname);
	    error = true;
	    return NULL;
	  }
	  csRef<iThingState> st (SCF_QUERY_INTERFACE (mesh->GetMeshObject (),
		iThingState));
	  if (!st)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Mesh '%s' is not a thing (%s '%s')!",
	      meshname, parenttype, parentname);
	    error = true;
	    return NULL;
	  }
	  iPolygon3D* polygon = st->GetPolygon (polyname);
	  if (!polygon)
	  {
	    SyntaxService->ReportError (
	      "crystalspace.maploader.parse.sequenceparams", child,
	      "Couldn't find polygon '%s' in mesh '%s' (%s '%s')!",
	      polyname, meshname, parenttype, parentname);
	    error = true;
	    return NULL;
	  }

	  params->SetParameter (idx, (iBase*)polygon);
	  found_params++;
	}
	break;
      default:
	SyntaxService->ReportBadToken (child);
	error = true;
	return NULL;
    }
  }
  if (params && found_params != params->GetParameterCount ())
  {
    SyntaxService->ReportError (
	"crystalspace.maploader.parse.sequenceparams", node,
	"Missing parameters for firing sequence in %s '%s'!",
	parenttype, parentname);
    error = true;
    return NULL;
  }

  return csPtr<iEngineSequenceParameters> (params);
}

iSequenceTrigger* csLoader::LoadTrigger (iDocumentNode* node)
{
  const char* trigname = node->GetAttributeValue ("name");
  // LoadTriggers() already checked for the presence of the engine
  // sequence manager.

  iSequenceTrigger* trigger = FindCreateTrigger (
	GetEngineSequenceManager (), trigname);

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
	    return NULL;
	  }

	  iMeshWrapper* mesh = ldr_context->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find mesh '%s' in trigger '%s'!", meshname,
		trigname);
	    return NULL;
	  }
	  trigger->AddConditionMeshClick (mesh);
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
	    return NULL;
	  }

	  iSector* sector = ldr_context->FindSector (sectname);
	  if (!sector)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find sector '%s' in trigger '%s'!", sectname,
		trigname);
	    return NULL;
	  }
	  bool insideonly = false;
	  csRef<iDocumentNode> ionode = child->GetNode ("insideonly");
	  csRef<iDocumentNode> boxnode = child->GetNode ("box");
	  csRef<iDocumentNode> spherenode = child->GetNode ("sphere");
	  if (ionode)
	    if (!SyntaxService->ParseBool (ionode, insideonly, true))
	      return NULL;
	  if (boxnode)
	  {
	    csBox3 box;
	    if (!SyntaxService->ParseBox (boxnode, box))
	      return NULL;
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
	    return NULL;
	  }

	  iSequenceWrapper* sequence = FindSequence (
		GetEngineSequenceManager (), seqname);
	  if (!sequence)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find sequence '%s' in trigger '%s'!",
		seqname, trigname);
	    return NULL;
	  }

	  csTicks delay = child->GetAttributeValueAsInt ("delay");

	  bool error;
	  csRef<iEngineSequenceParameters> params = CreateSequenceParameters (
	  	sequence, child, "trigger", trigname, error);
	  if (error)
	  {
	    // There was an error already reported by CreateSequenceParameters.
	    return NULL;
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
	return NULL;
    }
  }

  return trigger;
}

bool csLoader::LoadTriggers (iDocumentNode* node)
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
        if (!LoadTrigger (child))
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
    return NULL;
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
	    params->AddParameter (parname, NULL);
	  }
	  break;
        default:
          SyntaxService->ReportBadToken (child);
	  return NULL;
      }
    }
  }

  return sequence;
}

iSequenceWrapper* csLoader::LoadSequence (iDocumentNode* node)
{
  const char* seqname = node->GetAttributeValue ("name");
  // LoadSequences() already checked for the presence of the engine
  // sequence manager.

  iSequenceWrapper* sequence = FindSequence (
	GetEngineSequenceManager (), seqname);
  CS_ASSERT (sequence != NULL);

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
	    return NULL;
	  }
	  iSequenceWrapper* sequence2 = FindSequence (
		GetEngineSequenceManager (), seqname2);
	  if (!sequence2)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Can't find sequence '%s' in sequence '%s'!",
		seqname2, seqname);
	    return NULL;
	  }
	  bool error;
	  csRef<iEngineSequenceParameters> params2 = CreateSequenceParameters (
	  	sequence2, child, "sequence", seqname, error);
	  if (error)
	  {
	    // There was an error already reported by CreateSequenceParameters.
	    return NULL;
	  }

	  sequence->GetSequence ()->AddRunSequence (cur_time,
	  	sequence2->GetSequence (), (iBase*)params2);
	}
	break;
      case XMLTOKEN_DELAY:
        {
	  int delay = child->GetContentsValueAsInt ();
	  cur_time += delay;
	}
	break;
      case XMLTOKEN_MOVE:
        {
	  csRef<iParameterESM> mesh = ResolveOperationParameter (
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return NULL;

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
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return NULL;

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
		    return NULL;
		}
	        nr++;
	        break;
	      }
	      case XMLTOKEN_V:
	        if (!SyntaxService->ParseVector (child2, offset))
		  return NULL;
		break;
	      default:
		SyntaxService->ReportBadToken (child2);
		return NULL;
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
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return NULL;
	  csRef<iParameterESM> mat = ResolveOperationParameter (
	  	child, PARTYPE_MATERIAL, "material", seqname, base_params);
	  if (!mat) return NULL;

	  // optional polygon parameter.
	  csRef<iParameterESM> polygon = ResolveOperationParameter (
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
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return NULL;
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
	  	child, PARTYPE_MESH, "mesh", seqname, base_params);
	  if (!mesh) return NULL;
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
	  	child, PARTYPE_LIGHT, "light", seqname, base_params);
	  if (!light) return NULL;

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
	  	child, PARTYPE_LIGHT, "light", seqname, base_params);
	  if (!light) return NULL;

	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  sequence->AddOperationSetLight (cur_time, light, col);
	}
        break;
      case XMLTOKEN_FADEFOG:
        {
	  csRef<iParameterESM> sector = ResolveOperationParameter (
	  	child, PARTYPE_SECTOR, "sector", seqname, base_params);
	  if (!sector) return NULL;

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
	  	child, PARTYPE_SECTOR, "sector", seqname, base_params);
	  if (!sector) return NULL;

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
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return NULL;

	  sequence->AddOperationTriggerState (cur_time,
	  	trigger, true);
	}
        break;
      case XMLTOKEN_DISABLE:
        {
	  csRef<iParameterESM> trigger = ResolveOperationParameter (
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return NULL;

	  sequence->AddOperationTriggerState (cur_time,
	  	trigger, false);
	}
        break;
      case XMLTOKEN_CHECK:
        {
	  csRef<iParameterESM> trigger = ResolveOperationParameter (
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return NULL;

	  csTicks delay = child->GetAttributeValueAsInt ("delay");
	  sequence->AddOperationCheckTrigger (cur_time,
	  	trigger, delay);
	}
        break;
      case XMLTOKEN_TEST:
        {
	  csRef<iParameterESM> trigger = ResolveOperationParameter (
	  	child, PARTYPE_TRIGGER, "trigger", seqname, base_params);
	  if (!trigger) return NULL;

	  iSequence* trueseq = NULL;
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
	      return NULL;
	    }
	    trueseq = trueseqwrap->GetSequence ();
	  }
	  iSequence* falseseq = NULL;
	  const char* falseseqname = child->GetAttributeValue ("falsesequence");
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
	      return NULL;
	    }
	    falseseq = falseseqwrap->GetSequence ();
	  }
	  sequence->AddOperationTestTrigger (cur_time,
	  	trigger, trueseq, falseseq);
	}
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return NULL;
    }
  }

  return sequence;
}

bool csLoader::LoadSequences (iDocumentNode* node)
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
        if (!CreateSequence (child))
	  return false;
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
        if (!LoadSequence (child))
	  return false;
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return false;
    }
  }

  return true;
}

