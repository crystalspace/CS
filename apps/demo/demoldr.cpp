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
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csgeom/vector3.h"
#include "csgeom/path.h"
#include "ivaria/sequence.h"
#include "qsqrt.h"

// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ATTACH)
  CS_TOKEN_DEF (DECLARESEQUENCE)
  CS_TOKEN_DEF (RUNSEQUENCE)
  CS_TOKEN_DEF (SEQUENCE)
  CS_TOKEN_DEF (SEQUENCES)
  CS_TOKEN_DEF (RECURSE)
  CS_TOKEN_DEF (SETUPMESH)
  CS_TOKEN_DEF (SHOWMESH)
  CS_TOKEN_DEF (HIDEMESH)
  CS_TOKEN_DEF (PATH)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (TEST)
  CS_TOKEN_DEF (DELAY)
  CS_TOKEN_DEF (FADE)
  CS_TOKEN_DEF (NUM)
  CS_TOKEN_DEF (POS)
  CS_TOKEN_DEF (FORWARD)
  CS_TOKEN_DEF (UP)
  CS_TOKEN_DEF (TIMES)
  CS_TOKEN_DEF (UNIFORMSPEED)
  CS_TOKEN_DEF (ROTPART)
  CS_TOKEN_DEF (SPEED)
  CS_TOKEN_DEF (V)
CS_TOKEN_DEF_END

//-----------------------------------------------------------------------------

DemoSequenceLoader::DemoSequenceLoader (Demo* demo,
	DemoSequenceManager* demoseq, iSequenceManager* seqmgr,
	const char* fileName)
{
  DemoSequenceLoader::demo = demo;
  DemoSequenceLoader::demoseq = demoseq;
  DemoSequenceLoader::seqmgr = seqmgr;
  iDataBuffer* buf = demo->VFS->ReadFile (fileName);
  if (!buf || !buf->GetSize ())
  {
    if (buf) buf->DecRef ();
    demo->Printf (MSG_FATAL_ERROR,
    	"Could not open sequence file '%s' on VFS!\n", fileName);
    exit (0);
  }

  LoadSequencesMain (**buf);
  buf->DecRef ();
}

DemoSequenceLoader::~DemoSequenceLoader ()
{
  int i;
  for (i = 0 ; i < sequences.Length () ; i++)
  {
    NamedSequence* ns = (NamedSequence*)sequences[i];
    delete[] ns->name;
    if (ns->sequence) ns->sequence->DecRef ();
  }
}

iSequence* DemoSequenceLoader::GetSequence (const char* name)
{
  int i;
  for (i = 0 ; i < sequences.Length () ; i++)
  {
    NamedSequence* ns = (NamedSequence*)sequences[i];
    if (!strcmp (ns->name, name)) return ns->sequence;
  }
  return NULL;
}

void DemoSequenceLoader::LoadSequence (char* buf, iSequence* seq)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (SETUPMESH)
    CS_TOKEN_TABLE (SHOWMESH)
    CS_TOKEN_TABLE (HIDEMESH)
    CS_TOKEN_TABLE (ATTACH)
    CS_TOKEN_TABLE (PATH)
    CS_TOKEN_TABLE (TEST)
    CS_TOKEN_TABLE (DELAY)
    CS_TOKEN_TABLE (FADE)
    CS_TOKEN_TABLE (ROTPART)
    CS_TOKEN_TABLE (SEQUENCE)
    CS_TOKEN_TABLE (RUNSEQUENCE)
    CS_TOKEN_TABLE (RECURSE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;
  cs_time cur_time = 0;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      demo->Printf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n",
		buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_ROTPART:
      {
        char meshName[100];
	cs_time t;
	float angle_speed;
	ScanStr (params, "%d,%s,%f", &t, meshName, &angle_speed);
        RotatePartOp* op = new RotatePartOp (meshName, t, angle_speed);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_SETUPMESH:
      {
        char meshName[100];
	char sectName[100];
	csVector3 p;
	ScanStr (params, "%s,%s,%f,%f,%f", meshName, sectName,
		&p.x, &p.y, &p.z);
        SetupMeshOp* op = new SetupMeshOp (meshName, sectName, p);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_SHOWMESH:
      {
        char meshName[100];
	ScanStr (params, "%s", meshName);
        ShowMeshOp* op = new ShowMeshOp (meshName);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_HIDEMESH:
      {
        char meshName[100];
	ScanStr (params, "%s", meshName);
        HideMeshOp* op = new HideMeshOp (meshName);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_ATTACH:
      {
        char meshName[100];
	char pathName[100];
	ScanStr (params, "%s,%s", meshName, pathName);
	char* name = meshName;
	if (!strcmp ("camera", meshName)) name = NULL;
        AttachOp* op = new AttachOp (name, pathName);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_PATH:
      {
        char meshName[100];
	char pathName[100];
	cs_time t;
	ScanStr (params, "%d,%s,%s", &t, meshName, pathName);
	char* name = meshName;
	if (!strcmp ("camera", meshName)) name = NULL;
        PathOp* op = new PathOp (t, name, pathName);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_TEST:
      {
        TestOp* op = new TestOp ();
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_FADE:
      {
	float start, end;
	cs_time t;
	ScanStr (params, "%d,%f,%f,%d", &t, &start, &end);
        FadeOp* op = new FadeOp (start, end, t);
	seq->AddOperation (cur_time, op);
	op->DecRef ();
	break;
      }
      case CS_TOKEN_DELAY:
      {
	cs_time delay;
        ScanStr (params, "%d", &delay);
	cur_time += delay;
	break;
      }
      case CS_TOKEN_RECURSE:
      {
        seq->AddRunSequence (cur_time, seq);
        break;
      }
      case CS_TOKEN_RUNSEQUENCE:
      {
	char seqName[100];
	ScanStr (params, "%s", seqName);
        iSequence* newseq = GetSequence (seqName);
	if (!newseq)
	{
    	  demo->Printf (MSG_FATAL_ERROR, "Can't find sequence '%s'!\n",
	  	seqName);
	  exit (0);
	}
	seq->AddRunSequence (cur_time, newseq);
        break;
      }
      case CS_TOKEN_SEQUENCE:
      {
        iSequence* newseq = seqmgr->NewSequence ();
	LoadSequence (params, newseq);
	seq->AddRunSequence (cur_time, newseq);
	newseq->DecRef ();
	break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    demo->Printf (MSG_FATAL_ERROR,
      	"Token '%s' not found while parsing a sequence!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }
}

void DemoSequenceLoader::LoadSequences (char* buf)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (PATH)
    CS_TOKEN_TABLE (DECLARESEQUENCE)
    CS_TOKEN_TABLE (SEQUENCE)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      demo->Printf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n",
		buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_PATH:
      {
	csNamedPath* p = LoadPath (params, name);
	demoseq->RegisterPath (p);
        break;
      }
      case CS_TOKEN_DECLARESEQUENCE:
      {
        iSequence* newseq = GetSequence (name);
	if (!newseq)
	{
	  newseq = seqmgr->NewSequence ();
	  NamedSequence* ns = new NamedSequence ();
	  ns->name = strnew (name);
	  ns->sequence = newseq;
	  sequences.Push (ns);
	}
        break;
      }
      case CS_TOKEN_SEQUENCE:
      {
        iSequence* newseq = GetSequence (name);
	if (!newseq)
	{
          newseq = seqmgr->NewSequence ();
	  NamedSequence* ns = new NamedSequence ();
	  ns->name = strnew (name);
	  ns->sequence = newseq;
	  sequences.Push (ns);
	}
	else if (!newseq->IsEmpty ())
	{
    	  demo->Printf (MSG_FATAL_ERROR, "Sequence '%s' is already defined!\n",
	  	name);
	  exit (0);
	}
	LoadSequence (params, newseq);
	break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    demo->Printf (MSG_FATAL_ERROR,
      	"Token '%s' not found while parsing sequences!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }
}

void DemoSequenceLoader::LoadSequencesMain (char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (SEQUENCES)
  CS_TOKEN_TABLE_END

  csResetParserLine ();
  char *name, *data;
  if (csGetObject (&buf, tokens, &name, &data))
  {
    if (!data)
    {
      demo->Printf (MSG_FATAL_ERROR,
      	"Expected parameters instead of '%s'!\n", buf);
      fatal_exit (0, false);
    }
    LoadSequences (data);
  }
}

static bool ParseVectorList (char* buf, csVector3* list, int num)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (V)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  int n = 0;
  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params) return false;
    switch (cmd)
    {
      case CS_TOKEN_V:
      {
        csVector3 v;
	ScanStr (params, "%f,%f,%f", &v.x, &v.y, &v.z);
	list[n++] = v;
	break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND) return false;
  if (n != num) return false;
  return true;
}

csNamedPath* DemoSequenceLoader::LoadPath (char* buf, const char* pName)
{
  CS_TOKEN_TABLE_START (commands)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (NUM)
    CS_TOKEN_TABLE (POS)
    CS_TOKEN_TABLE (FORWARD)
    CS_TOKEN_TABLE (UP)
    CS_TOKEN_TABLE (TIMES)
    CS_TOKEN_TABLE (UNIFORMSPEED)
    CS_TOKEN_TABLE (SPEED)
  CS_TOKEN_TABLE_END

  csNamedPath* np = NULL;

  char* name;
  long cmd;
  char* params;
  int seq = 0;
  int num = 0;

  while ((cmd = csGetObject (&buf, commands, &name, &params)) > 0)
  {
    if (!params)
    {
      demo->Printf (MSG_FATAL_ERROR, "Expected parameters instead of '%s'!\n",
		buf);
      fatal_exit (0, false);
    }
    switch (cmd)
    {
      case CS_TOKEN_FILE:
      {
	char fname[255];
	ScanStr (params, "%s", fname);
  	iDataBuffer* buf = demo->VFS->ReadFile (fname);
	if (!buf || !buf->GetSize ())
	{
	  if (buf) buf->DecRef ();
	  demo->Printf (MSG_FATAL_ERROR,
	    "Could not open path file '%s' on VFS!\n", fname);
	  exit (0);
	}
	np = LoadPath (**buf, pName);
	buf->DecRef ();
	return np;
      }
      case CS_TOKEN_NUM:
      {
        if (seq != 0)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"NUM has to come first in path '%s'!\n", pName);
	  exit (0);
	}
	seq++;
	ScanStr (params, "%d", &num);
	np = new csNamedPath (num, pName);
	break;
      }
      case CS_TOKEN_SPEED:
      {
        if (seq < 2)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"First use NUM, POS in path '%s'!\n", pName);
	  exit (0);
	}
	int i;

	// First get the list of relative speeds.
	float list[10000];
	int n;
	ScanStr (params, "%F", list+1, &n);	// Don't read in list[0]
	if (n != num-1)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"SPEED should use %d-1 entries in path '%s'!\n", num, pName);
	  exit (0);
	}

	// Get the path.
	float* xv, * yv, * zv;
	xv = np->GetDimensionValues (0);
	yv = np->GetDimensionValues (1);
	zv = np->GetDimensionValues (2);
	csVector3 v0, v1;

	// Calculate the time value for every path segment.
	v0.Set (xv[0], yv[0], zv[0]);
	list[0] = 0;
	float tot = 0;
	for (i = 1 ; i < num ; i++)
	{
	  v1.Set (xv[i], yv[i], zv[i]);
	  float d = qsqrt (csSquaredDist::PointPoint (v0, v1));
	  tot += (1/list[i]) * d;
	  list[i] = tot;
	  v0 = v1;
	}

	// The above procedure will have filled list[i] with times
	// relative to the speed and length. Now we will correct that
	// list so that the last element is equal to 1.
	float correct = 1. / tot;
	for (i = 1 ; i < num ; i++) list[i] *= correct;

	np->SetTimeValues (list);
        break;
      }
      case CS_TOKEN_UNIFORMSPEED:
      {
        if (seq < 2)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"First use NUM, POS in path '%s'!\n", pName);
	  exit (0);
	}
	float list[10000];
	float* xv, * yv, * zv;
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
	  float d = qsqrt (csSquaredDist::PointPoint (v0, v1));
	  totlen += d;
	  v0 = v1;
	}

	// Calculate the time value for every path segment,
	// given the total length of the path.
	v0.Set (xv[0], yv[0], zv[0]);
	list[0] = 0;
	float tot = 0;
	for (i = 1 ; i < num ; i++)
	{
	  v1.Set (xv[i], yv[i], zv[i]);
	  float d = qsqrt (csSquaredDist::PointPoint (v0, v1));
	  tot += d;
	  list[i] = tot / totlen;
	  v0 = v1;
	}
	np->SetTimeValues (list);
        break;
      }
      case CS_TOKEN_TIMES:
      {
        if (seq < 1)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"First use NUM in path '%s'!\n", pName);
	  exit (0);
	}
	int n;
	float list[10000];
	ScanStr (params, "%F", list, &n);
	if (n != num)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"TIMES should use %d entries in path '%s'!\n", num, pName);
	  exit (0);
	}
	np->SetTimeValues (list);
	break;
      }
      case CS_TOKEN_POS:
      {
        if (seq < 1)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"First use NUM in path '%s'!\n", pName);
	  exit (0);
	}
	seq++;
	csVector3 v[10000];
	if (!ParseVectorList (params, v, num))
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"POS should use %d entries in path '%s'!\n", num, pName);
	  exit (0);
	}
	np->SetPositionVectors (v);
	break;
      }
      case CS_TOKEN_FORWARD:
      {
        if (seq < 1)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"First use NUM in path '%s'!\n", pName);
	  exit (0);
	}
	csVector3 v[10000];
	if (!ParseVectorList (params, v, num))
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"FORWARD should use %d entries in path '%s'!\n", num, pName);
	  exit (0);
	}
	np->SetForwardVectors (v);
	break;
      }
      case CS_TOKEN_UP:
      {
        if (seq < 1)
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"First use NUM in path '%s'!\n", pName);
	  exit (0);
	}
	csVector3 v[10000];
	if (!ParseVectorList (params, v, num))
	{
	  demo->Printf (MSG_FATAL_ERROR,
	  	"UP should use %d entries in path '%s'!\n", num, pName);
	  exit (0);
	}
	np->SetUpVectors (v);
	break;
      }
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    demo->Printf (MSG_FATAL_ERROR,
      	"Token '%s' not found while parsing a path!\n",
	csGetLastOffender ());
    fatal_exit (0, false);
  }

  return np;
}

//-----------------------------------------------------------------------------
