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
#include "iengine/engine.h"
#include "iengine/region.h"
#include "iengine/light.h"
#include "iengine/statlght.h"
#include "ivaria/sequence.h"

static iSequenceWrapper* FindCreateSequence (iEngineSequenceManager* eseqmgr,
		const char* name)
{
  iSequenceWrapper* sequence = eseqmgr->FindSequenceByName (name);
  if (!sequence)
  {
    // We don't need the ref returned by CreateSequence().
    csRef<iSequenceWrapper> seqwrap = eseqmgr->CreateSequence (name);
    sequence = seqwrap;
  }
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
      case XMLTOKEN_SECTORVIS:
	{
	  const char* sectname = child->GetContentsValue ();
	  iSector* sector = ldr_context->FindSector (sectname);
	  if (!sector)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.trigger",
		child, "Couldn't find sector '%s' in trigger '%s'!", sectname,
		trigname);
	    return NULL;
	  }
	  trigger->AddConditionSectorVisible (sector);
	}
        break;
      case XMLTOKEN_FIRE:
	{
	  const char* seqname = child->GetContentsValue ();
	  iSequenceWrapper* sequence = FindCreateSequence (
		GetEngineSequenceManager (), seqname);
	  csTicks delay = child->GetAttributeValueAsInt ("delay");
	  trigger->FireSequence (delay, sequence);
	}
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return NULL;
    }
  }

  return trigger;	// DecRef() is ok here.
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

iSequenceWrapper* csLoader::LoadSequence (iDocumentNode* node)
{
  const char* seqname = node->GetAttributeValue ("name");
  // LoadSequences() already checked for the presence of the engine
  // sequence manager.

  iSequenceWrapper* sequence = FindCreateSequence (
	GetEngineSequenceManager (), seqname);

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
      case XMLTOKEN_RUNSEQUENCE:
        {
	  const char* seqname2 = child->GetContentsValue ();
	  iSequenceWrapper* sequence2 = FindCreateSequence (
		GetEngineSequenceManager (), seqname2);
	  sequence->GetSequence ()->AddRunSequence (cur_time,
	  	sequence2->GetSequence ());
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
	  const char* meshname = child->GetAttributeValue ("mesh");
	  iMeshWrapper* mesh = Engine->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Couldn't find mesh '%s' in sequence '%s'!", meshname,
		seqname);
	    return NULL;
	  }
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
	  const char* meshname = child->GetAttributeValue ("mesh");
	  iMeshWrapper* mesh = Engine->FindMeshObject (meshname);
	  if (!mesh)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Couldn't find mesh '%s' in sequence '%s'!", meshname,
		seqname);
	    return NULL;
	  }
	  int nr = 0;
	  int axis1 = -1, axis2 = -1, axis3 = -1;
	  csVector3 offset (0);
	  float tot_angle1, tot_angle2, tot_angle3;
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
	  	axis1, tot_angle1,
	  	axis2, tot_angle2,
	  	axis3, tot_angle3,
		offset, duration);
	}
	break;
      case XMLTOKEN_FADELIGHT:
        {
	  const char* lightname = child->GetAttributeValue ("light");
	  iStatLight* light = Engine->FindLight (lightname);
	  if (!light)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Couldn't find light '%s' in sequence '%s'!", lightname,
		seqname);
	    return NULL;
	  }
	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  int duration = child->GetAttributeValueAsInt ("duration");
	  sequence->AddOperationFadeLight (cur_time, light->QueryLight (), col,
	  	duration);
	}
	break;
      case XMLTOKEN_SETLIGHT:
        {
	  const char* lightname = child->GetAttributeValue ("light");
	  iStatLight* light = Engine->FindLight (lightname);
	  if (!light)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Couldn't find light '%s' in sequence '%s'!", lightname,
		seqname);
	    return NULL;
	  }
	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  sequence->AddOperationSetLight (cur_time, light->QueryLight (), col);
	}
        break;
      case XMLTOKEN_FADEFOG:
        {
	  const char* sectname = child->GetAttributeValue ("sector");
	  iSector* sector = ldr_context->FindSector (sectname);
	  if (!sector)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Couldn't find sector '%s' in sequence '%s'!", sectname,
		seqname);
	    return NULL;
	  }
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
	  const char* sectname = child->GetAttributeValue ("sector");
	  iSector* sector = ldr_context->FindSector (sectname);
	  if (!sector)
	  {
	    SyntaxService->ReportError (
		"crystalspace.maploader.parse.sequence",
		child, "Couldn't find sector '%s' in sequence '%s'!", sectname,
		seqname);
	    return NULL;
	  }
	  csColor col;
	  col.red = child->GetAttributeValueAsFloat ("red");
	  col.green = child->GetAttributeValueAsFloat ("green");
	  col.blue = child->GetAttributeValueAsFloat ("blue");
	  float density;
	  density = child->GetAttributeValueAsFloat ("density");
	  sequence->AddOperationSetFog (cur_time, sector, col, density);
	}
        break;
      case XMLTOKEN_ENABLETRIGGER:
        {
	  const char* trigname = child->GetContentsValue ();
	  iSequenceTrigger* trigger = FindCreateTrigger (
		GetEngineSequenceManager (), trigname);
	  sequence->AddOperationTriggerState (cur_time,
	  	trigger, true);
	}
        break;
      case XMLTOKEN_DISABLETRIGGER:
        {
	  const char* trigname = child->GetContentsValue ();
	  iSequenceTrigger* trigger = FindCreateTrigger (
		GetEngineSequenceManager (), trigname);
	  sequence->AddOperationTriggerState (cur_time,
	  	trigger, false);
	}
        break;
      default:
        SyntaxService->ReportBadToken (child);
	return NULL;
    }
  }

  return sequence;	// DecRef() is ok here.
}

bool csLoader::LoadSequences (iDocumentNode* node)
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

