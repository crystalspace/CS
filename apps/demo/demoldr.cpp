/*
    Copyright (C) 2001 by Jorrit Tyberghein

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
#include "demo.h"
#include "demoldr.h"
#include "demoseq.h"
#include "demoop.h"
#include "csutil/scanstr.h"
#include "csgeom/vector3.h"
#include "csgeom/path.h"
#include "ivaria/sequence.h"
#include "ivaria/reporter.h"
#include "iutil/databuff.h"
#include "iutil/vfs.h"
#include "csutil/xmltiny.h"
#include "cscsQsqrt.h"

// Define all tokens used through this file
enum
{
  XMLTOKEN_ATTACH = 1,
  XMLTOKEN_DECLARESEQUENCE,
  XMLTOKEN_RUNSEQUENCE,
  XMLTOKEN_SEQUENCE,
  XMLTOKEN_SEQUENCES,
  XMLTOKEN_RECURSE,
  XMLTOKEN_SETUPMESH,
  XMLTOKEN_SHOWMESH,
  XMLTOKEN_HIDEMESH,
  XMLTOKEN_PATH,
  XMLTOKEN_SECTOR,
  XMLTOKEN_FILE,
  XMLTOKEN_TEST,
  XMLTOKEN_DELAY,
  XMLTOKEN_FADE,
  XMLTOKEN_NUM,
  XMLTOKEN_POS,
  XMLTOKEN_FORWARD,
  XMLTOKEN_UP,
  XMLTOKEN_TIMES,
  XMLTOKEN_UNIFORMSPEED,
  XMLTOKEN_ROTPART,
  XMLTOKEN_SPEED,
  XMLTOKEN_V
};

#define SPEED_FACTOR 1

//-----------------------------------------------------------------------------

DemoSequenceLoader::DemoSequenceLoader (Demo* demo,
	DemoSequenceManager* demoseq, iSequenceManager* seqmgr,
	const char* fileName)
{
  DemoSequenceLoader::demo = demo;
  DemoSequenceLoader::demoseq = demoseq;
  DemoSequenceLoader::seqmgr = seqmgr;

  xmltokens.Register ("attach", XMLTOKEN_ATTACH);
  xmltokens.Register ("declaresequence", XMLTOKEN_DECLARESEQUENCE);
  xmltokens.Register ("runsequence", XMLTOKEN_RUNSEQUENCE);
  xmltokens.Register ("sequence", XMLTOKEN_SEQUENCE);
  xmltokens.Register ("sequences", XMLTOKEN_SEQUENCES);
  xmltokens.Register ("recurse", XMLTOKEN_RECURSE);
  xmltokens.Register ("setupmesh", XMLTOKEN_SETUPMESH);
  xmltokens.Register ("showmesh", XMLTOKEN_SHOWMESH);
  xmltokens.Register ("hidemesh", XMLTOKEN_HIDEMESH);
  xmltokens.Register ("path", XMLTOKEN_PATH);
  xmltokens.Register ("sector", XMLTOKEN_SECTOR);
  xmltokens.Register ("file", XMLTOKEN_FILE);
  xmltokens.Register ("test", XMLTOKEN_TEST);
  xmltokens.Register ("delay", XMLTOKEN_DELAY);
  xmltokens.Register ("fade", XMLTOKEN_FADE);
  xmltokens.Register ("num", XMLTOKEN_NUM);
  xmltokens.Register ("pos", XMLTOKEN_POS);
  xmltokens.Register ("forward", XMLTOKEN_FORWARD);
  xmltokens.Register ("up", XMLTOKEN_UP);
  xmltokens.Register ("times", XMLTOKEN_TIMES);
  xmltokens.Register ("uniformspeed", XMLTOKEN_UNIFORMSPEED);
  xmltokens.Register ("rotpart", XMLTOKEN_ROTPART);
  xmltokens.Register ("speed", XMLTOKEN_SPEED);
  xmltokens.Register ("v", XMLTOKEN_V);

  csRef<iDataBuffer> buf (demo->myVFS->ReadFile (fileName));
  if (!buf || !buf->GetSize ())
  {
    demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"Could not open sequence file '%s' on VFS!", fileName);
    exit (0);
  }

  csRef<iDocumentSystem> xml (
  	CS_QUERY_REGISTRY (demo->object_reg, iDocumentSystem));
  if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
  csRef<iDocument> doc = xml->CreateDocument ();
  const char* error = doc->Parse (buf);
  if (error != 0)
  {
    demo->Report (CS_REPORTER_SEVERITY_ERROR,
    	"XML error '%s' for file '%s'!", error, fileName);
    exit (0);
  }
  LoadSequencesMain (doc->GetRoot ());
}

DemoSequenceLoader::~DemoSequenceLoader ()
{
}

iSequence* DemoSequenceLoader::GetSequence (const char* name)
{
  size_t i;
  for (i = 0 ; i < sequences.Length () ; i++)
  {
    NamedSequence* ns = sequences[i];
    if (!strcmp (ns->name, name)) return ns->sequence;
  }
  return 0;
}

void DemoSequenceLoader::LoadSequence (iDocumentNode* node, iSequence* seq)
{
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
      case XMLTOKEN_ROTPART:
      {
        const char* meshName;
	csTicks t;
	float angle_speed;
	csRef<iDocumentNode> meshnode = child->GetNode ("mesh");
	if (!meshnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <mesh> in <rotpart>!");
	  exit (0);
	}
	meshName = meshnode->GetContentsValue ();
	csRef<iDocumentNode> anglenode = child->GetNode ("anglespeed");
	if (!anglenode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <anglespeed> in <rotpart>!");
	  exit (0);
	}
	angle_speed = anglenode->GetContentsValueAsFloat ();
	csRef<iDocumentNode> timenode = child->GetNode ("time");
	if (!timenode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <time> in <rotpart>!");
	  exit (0);
	}
	t = timenode->GetContentsValueAsInt ();
	t = csTicks (float (t) * SPEED_FACTOR);
        RotatePartOp* op = new RotatePartOp (meshName, t, angle_speed);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_SETUPMESH:
      {
        const char* meshName;
	const char* sectName;
	csRef<iDocumentNode> meshnode = child->GetNode ("mesh");
	if (!meshnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <mesh> in <setupmesh>!");
	  exit (0);
	}
	meshName = meshnode->GetContentsValue ();
	csRef<iDocumentNode> sectnode = child->GetNode ("sector");
	if (!sectnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <sector> in <setupmesh>!");
	  exit (0);
	}
	sectName = sectnode->GetContentsValue ();

	csVector3 p;
	csRef<iDocumentNode> posnode = child->GetNode ("position");
	if (!posnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <position> in <setupmesh>!");
	  exit (0);
	}
	p.x = posnode->GetAttributeValueAsFloat ("x");
	p.y = posnode->GetAttributeValueAsFloat ("y");
	p.z = posnode->GetAttributeValueAsFloat ("z");
        SetupMeshOp* op = new SetupMeshOp (meshName, sectName, p);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_SHOWMESH:
      {
        ShowMeshOp* op = new ShowMeshOp (child->GetContentsValue ());
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_HIDEMESH:
      {
        HideMeshOp* op = new HideMeshOp (child->GetContentsValue ());
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_ATTACH:
      {
        const char* meshName;
	const char* pathName;
	csRef<iDocumentNode> meshnode = child->GetNode ("mesh");
	if (!meshnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <mesh> in <attach>!");
	  exit (0);
	}
	meshName = meshnode->GetContentsValue ();
	csRef<iDocumentNode> pathnode = child->GetNode ("path");
	if (!pathnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <path> in <attach>!");
	  exit (0);
	}
	pathName = pathnode->GetContentsValue ();
	const char* name = meshName;
	if (!strcmp ("camera", meshName)) name = 0;
        AttachOp* op = new AttachOp (name, pathName);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_PATH:
      {
        const char* meshName;
	const char* pathName;
	csRef<iDocumentNode> meshnode = child->GetNode ("mesh");
	if (!meshnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <mesh> in <path>!");
	  exit (0);
	}
	meshName = meshnode->GetContentsValue ();
	csRef<iDocumentNode> pathnode = child->GetNode ("path");
	if (!pathnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <path> in <path>!");
	  exit (0);
	}
	pathName = pathnode->GetContentsValue ();

	csTicks t;
	csRef<iDocumentNode> timenode = child->GetNode ("time");
	if (!timenode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <time> in <path>!");
	  exit (0);
	}
	t = timenode->GetContentsValueAsInt ();

	t = csTicks (float (t) * SPEED_FACTOR);
	const char* name = meshName;
	if (!strcmp ("camera", meshName)) name = 0;
        PathOp* op = new PathOp (t, name, pathName);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_TEST:
      {
        TestOp* op = new TestOp ();
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_FADE:
      {
	float start, end;
	csTicks t;

	csRef<iDocumentNode> timenode = child->GetNode ("time");
	if (!timenode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <time> in <fade>!");
	  exit (0);
	}
	t = timenode->GetContentsValueAsInt ();
	csRef<iDocumentNode> startnode = child->GetNode ("start");
	if (!startnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <start> in <fade>!");
	  exit (0);
	}
	start = startnode->GetContentsValueAsFloat ();
	csRef<iDocumentNode> endnode = child->GetNode ("end");
	if (!endnode)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Can't find <end> in <fade>!");
	  exit (0);
	}
	end = endnode->GetContentsValueAsFloat ();
	t = csTicks (float (t) * SPEED_FACTOR);
        FadeOp* op = new FadeOp (start, end, t);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case XMLTOKEN_DELAY:
      {
	csTicks delay;
	delay = csTicks (child->GetContentsValueAsFloat () * SPEED_FACTOR);
	cur_time += delay;
	break;
      }
      case XMLTOKEN_RECURSE:
      {
        //seq->AddRunSequence (cur_time, seq);
        csRef<RecurseOp> op = csPtr<RecurseOp> (new RecurseOp (seq, seqmgr));
	seq->AddOperation (cur_time, op);
        break;
      }
      case XMLTOKEN_RUNSEQUENCE:
      {
        iSequence* newseq = GetSequence (child->GetContentsValue ());
	if (!newseq)
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR, "Can't find sequence '%s'!",
	  	child->GetContentsValue ());
	  exit (0);
	}
	seq->AddRunSequence (cur_time, newseq);
        break;
      }
      case XMLTOKEN_SEQUENCE:
      {
        csRef<iSequence> newseq = seqmgr->NewSequence ();
	LoadSequence (child, newseq);
	seq->AddRunSequence (cur_time, newseq);
	break;
      }
      default:
    	demo->Report (CS_REPORTER_SEVERITY_ERROR,
		"Unexpected token '%s'!", value);
	exit (-1);
    }
  }
}

void DemoSequenceLoader::LoadSequences (iDocumentNode* node)
{
  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_PATH:
      {
	csNamedPath* p = LoadPath (child, child->GetAttributeValue ("name"));
	demoseq->RegisterPath (p);
        break;
      }
      case XMLTOKEN_DECLARESEQUENCE:
      {
        csRef<iSequence> newseq =
		GetSequence (child->GetAttributeValue ("name"));
	if (!newseq)
	{
	  newseq = seqmgr->NewSequence ();
	  NamedSequence* ns = new NamedSequence ();
	  ns->name = csStrNew (child->GetAttributeValue ("name"));
	  ns->sequence = newseq;
	  sequences.Push (ns);
	}
        break;
      }
      case XMLTOKEN_SEQUENCE:
      {
        csRef<iSequence> newseq =
		GetSequence (child->GetAttributeValue ("name"));
	if (!newseq)
	{
          newseq = seqmgr->NewSequence ();
	  NamedSequence* ns = new NamedSequence ();
	  ns->name = csStrNew (child->GetAttributeValue ("name"));
	  ns->sequence = newseq;
	  sequences.Push (ns);
	}
	else if (!newseq->IsEmpty ())
	{
    	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Sequence '%s' is already defined!",
	  	child->GetAttributeValue ("name"));
	  exit (0);
	}
	LoadSequence (child, newseq);
	break;
      }
      default:
    	demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Unexpected token '%s'!", value);
	exit (0);
    }
  }
}

void DemoSequenceLoader::LoadSequencesMain (iDocumentNode* node)
{
  csRef<iDocumentNode> child = node->GetNode ("sequences");
  if (!child)
  {
    demo->Report (CS_REPORTER_SEVERITY_ERROR,
      	"Expected 'sequences'!");
    exit (-1);
  }
  LoadSequences (child);
}

bool DemoSequenceLoader::ParseVectorList (iDocumentNode* node,
	csVector3* list, int num)
{
  int n = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_V:
      {
        csVector3 v;
	v.x = child->GetAttributeValueAsFloat ("x");
	v.y = child->GetAttributeValueAsFloat ("y");
	v.z = child->GetAttributeValueAsFloat ("z");
	list[n++] = v;
	break;
      }
      default:
        return false;
    }
  }
  if (n != num) return false;
  return true;
}

csNamedPath* DemoSequenceLoader::LoadPath (iDocumentNode* node,
	const char* pName)
{
  csNamedPath* np = 0;

  int seq = 0;
  int num = 0;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    csStringID id = xmltokens.Request (value);
    switch (id)
    {
      case XMLTOKEN_FILE:
      {
	const char* fname = child->GetContentsValue ();
  	csRef<iDataBuffer> buf (demo->myVFS->ReadFile (fname));
	if (!buf || !buf->GetSize ())
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	    "Could not open path file '%s' on VFS!", fname);
	  exit (0);
	}
	csRef<iDocumentSystem> xml (
		CS_QUERY_REGISTRY (demo->object_reg, iDocumentSystem));
	if (!xml) xml = csPtr<iDocumentSystem> (new csTinyDocumentSystem ());
	csRef<iDocument> doc = xml->CreateDocument ();
	const char* error = doc->Parse (buf);
	if (error != 0)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	    "Error '%s' reading XML file '%s'!", error, fname);
	  exit (0);
	}
	csRef<iDocumentNode> pathnode = doc->GetRoot ()->GetNode ("path");
	if (!pathnode)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	    "File '%s' does not contain a <path> node!", fname);
	  exit (0);
	}

	np = LoadPath (pathnode, pName);
	return np;
      }
      case XMLTOKEN_NUM:
      {
        if (seq != 0)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"NUM has to come first in path '%s'!", pName);
	  exit (0);
	}
	seq++;
	num = child->GetContentsValueAsInt ();
	np = new csNamedPath (num, pName);
	break;
      }
      case XMLTOKEN_SPEED:
      {
        if (seq < 2)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"First use NUM, POS in path '%s'!", pName);
	  exit (0);
	}
	int n = 0;
	// First get the list of relative speeds.
	float* list = new float[10000];
	csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
	while (child_it->HasNext ())
	{
	  csRef<iDocumentNode> childchild = child_it->Next ();
	  if (childchild->GetType () != CS_NODE_ELEMENT) continue;
	  const char* child_value = childchild->GetValue ();
	  if (!strcmp (child_value, "s"))
	  {
	    ++n; // Don't read in list[0]
	    list[n] = childchild->GetContentsValueAsFloat ();
	  }
	}

	if (n != num-1)
	{
	  delete[] list;
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"SPEED should use %d-1 entries in path '%s'!", num, pName);
	  exit (0);
	}

	// Get the path.
	const float* xv, * yv, * zv;
	xv = np->GetDimensionValues (0);
	yv = np->GetDimensionValues (1);
	zv = np->GetDimensionValues (2);
	csVector3 v0, v1;

	// Calculate the time value for every path segment.
	v0.Set (xv[0], yv[0], zv[0]);
	list[0] = 0;
	float tot = 0;
	int i;
	for (i = 1 ; i < num ; i++)
	{
	  v1.Set (xv[i], yv[i], zv[i]);
	  float d = csQsqrt (csSquaredDist::PointPoint (v0, v1));
	  tot += (1/list[i]) * d;
	  list[i] = tot;
	  v0 = v1;
	}

	// The above procedure will have filled list[i] with times
	// relative to the speed and length. Now we will correct that
	// list so that the last element is equal to 1.
	float correct = 1.0f / tot;
	for (i = 1 ; i < num ; i++) list[i] *= correct;

	np->SetTimeValues (list);
	delete[] list;
        break;
      }
      case XMLTOKEN_UNIFORMSPEED:
      {
        if (seq < 2)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"First use NUM, POS in path '%s'!", pName);
	  exit (0);
	}
	const float* xv, * yv, * zv;
	xv = np->GetDimensionValues (0);
	yv = np->GetDimensionValues (1);
	zv = np->GetDimensionValues (2);
	csVector3 v0, v1;

	// Calculate the total length of the path.
	float totlen = 0;
	int i;
	v0.Set (xv[0], yv[0], zv[0]);
	for (i = 1 ; i < num ; i++)
	{
	  v1.Set (xv[i], yv[i], zv[i]);
	  float d = csQsqrt (csSquaredDist::PointPoint (v0, v1));
	  totlen += d;
	  v0 = v1;
	}

	// Calculate the time value for every path segment,
	// given the total length of the path.
	v0.Set (xv[0], yv[0], zv[0]);
	float* list = new float[10000];
	list[0] = 0;
	float tot = 0.0f;
	for (i = 1 ; i < num ; i++)
	{
	  v1.Set (xv[i], yv[i], zv[i]);
	  float d = csQsqrt (csSquaredDist::PointPoint (v0, v1));
	  tot += d;
	  list[i] = tot / totlen;
	  v0 = v1;
	}
	np->SetTimeValues (list);
	delete[] list;
        break;
      }
      case XMLTOKEN_TIMES:
      {
        if (seq < 1)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"First use NUM in path '%s'!", pName);
	  exit (0);
	}
	int n = 0;
	// First get the list of times.
	float* list = new float[10000];
	csRef<iDocumentNodeIterator> child_it = child->GetNodes ();
	while (child_it->HasNext ())
	{
	  csRef<iDocumentNode> childchild = child_it->Next ();
	  if (childchild->GetType () != CS_NODE_ELEMENT) continue;
	  const char* child_value = childchild->GetValue ();
	  if (!strcmp (child_value, "t"))
	  {
	    list[n] = childchild->GetContentsValueAsFloat ();
	    n++;
	  }
	}

	if (n != num)
	{
	  delete[] list;
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"TIMES should use %d entries in path '%s'!", num, pName);
	  exit (0);
	}
	np->SetTimeValues (list);
        delete[] list;
	break;
      }
      case XMLTOKEN_POS:
      {
        if (seq < 1)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"First use NUM in path '%s'!", pName);
	  exit (0);
	}
	seq++;
	csVector3* v = new csVector3[10000];
	if (!ParseVectorList (child, v, num))
	{
	  delete[] v;
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"POS should use %d entries in path '%s'!", num, pName);
	  exit (0);
	}
	np->SetPositionVectors (v);
	delete[] v;
	break;
      }
      case XMLTOKEN_FORWARD:
      {
        if (seq < 1)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"First use NUM in path '%s'!", pName);
	  exit (0);
	}
	csVector3* v = new csVector3[10000];
	if (!ParseVectorList (child, v, num))
	{
	  delete[] v;
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"FORWARD should use %d entries in path '%s'!", num, pName);
	  exit (0);
	}
	np->SetForwardVectors (v);
	delete[] v;
	break;
      }
      case XMLTOKEN_UP:
      {
        if (seq < 1)
	{
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"First use NUM in path '%s'!", pName);
	  exit (0);
	}
	csVector3* v = new csVector3[10000];
	if (!ParseVectorList (child, v, num))
	{
	  delete[] v;
	  demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"UP should use %d entries in path '%s'!", num, pName);
	  exit (0);
	}
	np->SetUpVectors (v);
	delete[] v;
	break;
      }
      default:
        demo->Report (CS_REPORTER_SEVERITY_ERROR,
	  	"Unexpected token '%s'!", value);
        exit (0);
    }
  }

  return np;
}
