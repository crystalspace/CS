/*
    Debugging tools.
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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cssysdef.h"
#include "csutil/scf.h"
#include "csutil/util.h"
#include "csutil/debug.h"
#include "csutil/scf.h"
#include "iutil/objreg.h"

//-----------------------------------------------------------------------------

struct csDGEL
{
  void* object;		// Pointer to the object.
  bool scf;		// If true 'object' is an iBase.
  bool used;		// If true the object is currently allocated.
  uint32 timestamp;	// Timestamp of last allocation.
  char* description;
  char* file;
  int linenr;
  int num_parents;
  csDGEL** parents;	// Pointer to parents.
  uint32* p_stamps;	// Pointer to array of timestamps for parent creation.
  int num_children;
  csDGEL** children;	// Pointer to children.
  uint32* c_stamps;	// Pointer to array of timestamps for child creation.
  bool marker;		// To see what we already dumped.
  bool recurse_marker;	// To see what we're dumping in this recursion.

  csDGEL ()
  {
    object = NULL;
    scf = false;
    used = false;
    timestamp = 0;
    description = NULL;
    file = NULL;
    num_parents = 0;
    parents = NULL;
    p_stamps = NULL;
    num_children = 0;
    children = NULL;
    c_stamps = NULL;
  }
  void Clear ()
  {
    delete[] description; description = NULL;
    delete[] file; file = NULL;
    delete[] parents; parents = NULL; num_parents = 0;
    delete[] p_stamps; p_stamps = NULL;
    delete[] children; children = NULL; num_children = 0;
    delete[] c_stamps; c_stamps = NULL;
  }
  ~csDGEL ()
  {
    Clear ();
  }

  void AddChild (csDGEL* child, uint32 timestamp)
  {
    if (!children)
    {
      CS_ASSERT (c_stamps == NULL);
      children = new csDGEL*[1];
      c_stamps = new uint32[1];
    }
    else
    {
      CS_ASSERT (c_stamps != NULL);
      csDGEL** new_children = new csDGEL*[num_children+1];
      uint32* new_c_stamps = new uint32[num_children+1];
      memcpy (new_children, children, sizeof (csDGEL*)*num_children);
      memcpy (new_c_stamps, c_stamps, sizeof (uint32)*num_children);
      delete[] children; children = new_children;
      delete[] c_stamps; c_stamps = new_c_stamps;
    }
    children[num_children] = child;
    c_stamps[num_children++] = timestamp;
  }
  void RemoveChild (csDGEL* child)
  {
    if (!children)
    {
      CS_ASSERT (c_stamps == NULL);
      return;
    }
    CS_ASSERT (c_stamps != NULL);
    if (num_children == 1)
    {
      if (child == children[0])
      {
        delete[] children; children = NULL;
	delete[] c_stamps; c_stamps = NULL;
	num_children = 0;
      }
      return;
    }
    int i, j = 0;
    for (i = 0 ; i < num_children ; i++)
    {
      if (child != children[i])
      {
	children[j] = children[i];
	c_stamps[j++] = c_stamps[i];
      }
    }
    num_children = j;
  }
  void AddParent (csDGEL* parent, uint32 timestamp)
  {
    if (!parents)
    {
      CS_ASSERT (p_stamps == NULL);
      parents = new csDGEL*[1];
      p_stamps = new uint32[1];
    }
    else
    {
      CS_ASSERT (p_stamps != NULL);
      csDGEL** new_parents = new csDGEL*[num_parents+1];
      uint32* new_p_stamps = new uint32[num_parents+1];
      memcpy (new_parents, parents, sizeof (csDGEL*)*num_parents);
      memcpy (new_p_stamps, p_stamps, sizeof (uint32)*num_parents);
      delete[] parents; parents = new_parents;
      delete[] p_stamps; p_stamps = new_p_stamps;
    }
    parents[num_parents] = parent;
    p_stamps[num_parents++] = timestamp;
  }
  void RemoveParent (csDGEL* parent)
  {
    if (!parents)
    {
      CS_ASSERT (p_stamps == NULL);
      return;
    }
    CS_ASSERT (p_stamps != NULL);
    if (num_parents == 1)
    {
      if (parent == parents[0])
      {
        delete[] parents; parents = NULL;
	delete[] p_stamps; p_stamps = NULL;
	num_parents = 0;
      }
      return;
    }
    int i, j = 0;
    for (i = 0 ; i < num_parents ; i++)
    {
      if (parent != parents[i])
      {
	parents[j] = parents[i];
	p_stamps[j++] = p_stamps[i];
      }
    }
    num_parents = j;
  }
};

class csDebugGraph : public iBase
{
public:
  int num_els;
  int max_els;
  csDGEL** els;
  uint32 last_timestamp;

  csDebugGraph ()
  {
    SCF_CONSTRUCT_IBASE (NULL);
    num_els = 0;
    max_els = 100;
    els = new csDGEL* [max_els];
    last_timestamp = 1;
  }
  virtual ~csDebugGraph ()
  {
    Clear ();
  }
  void Clear ()
  {
    int i;
    for (i = 0 ; i < num_els ; i++)
    {
      delete els[i];
    }
    delete[] els;
    num_els = 0;
    max_els = 100;
    els = new csDGEL* [max_els];
    last_timestamp = 1;
  }

  csDGEL* AddEl (void* object)
  {
    if (num_els >= max_els)
    {
      max_els += 100;
      csDGEL** new_els = new csDGEL* [max_els];
      memcpy (new_els, els, sizeof (csDGEL*) * num_els);
      delete[] els;
      els = new_els;
    }

    csDGEL* el = new csDGEL ();
    els[num_els++] = el;
    el->used = false;
    el->object = object;
    return el;
  }

  csDGEL* FindEl (void* object)
  {
    int i;
    for (i = 0 ; i < num_els ; i++)
    {
      if (els[i]->object == object) return els[i];
    }
    return NULL;
  }

  SCF_DECLARE_IBASE;
};

SCF_IMPLEMENT_IBASE (csDebugGraph)
SCF_IMPLEMENT_IBASE_END

static csDebugGraph* SetupDebugGraph (iObjectRegistry* object_reg)
{
  iBase* idg = CS_QUERY_REGISTRY_TAG (object_reg, "__Debug_Graph__");
  if (!idg)
  {
    idg = new csDebugGraph ();
    object_reg->Register (idg, "__Debug_Graph__");
  }
  idg->DecRef ();
  return (csDebugGraph*)idg;
}

//-----------------------------------------------------------------------------

void csDebuggingGraph::SetupGraph (iObjectRegistry* object_reg)
{
  SetupDebugGraph (object_reg);
#ifdef CS_DEBUG
  iSCF::SCF->object_reg = object_reg;
#endif
}

void csDebuggingGraph::AddObject (iObjectRegistry* object_reg,
	void* object, bool scf, char* file, int linenr,
  	char* description, ...)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);

  csDGEL* el = dg->FindEl (object);
  if (el)
  {
    // The element is already there. This either means that
    // the object was freed first and now a new object happens
    // to be allocated on the same position (this is a valid
    // situation), or else it means that the object is allocated
    // twice! This is not a valid situation because it means
    // that DG_ADD or DG_ADDI is used with a missing DG_REM
    // in between.
    if (el->used)
    {
      printf ("ERROR! Object is added twice to the debug graph!\n");
      printf ("%p %s", el->object, el->description);
      fflush (stdout);
      CS_ASSERT (false);
      return;
    }

    // Reinitialize the element. We will also clear the list of
    // parents and children here since this is a new element and the
    // previous lists are certainly invalid. Note that it is possible
    // that other elements still point to this element from a previous
    // incarnation. That case can be detected with the timestamp: timestamp
    // of this creation will be bigger than the timestamp of the creation
    // of the link to this item. The Dump will show this anomaly.
    el->Clear ();
  }
  else
  {
    // We have a new element.
    el = dg->AddEl (object);
  }

  el->used = true;
  el->timestamp = dg->last_timestamp++;
  el->scf = scf;

  if (description)
  {
    char buf[1000];
    va_list arg;
    va_start (arg, description);
    vsprintf (buf, description, arg);
    va_end (arg);
    el->description = csStrNew (buf);
  }
  else el->description = NULL;

  el->file = file ? csStrNew (file) : NULL;
  el->linenr = linenr;
}

void csDebuggingGraph::AttachDescription (iObjectRegistry* object_reg,
  	void* object, char* description, ...)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);

  csDGEL* el = dg->FindEl (object);
  if (el == NULL)
  {
    printf ("ERROR! Cannot find object %p to add description:\n'", object);
    va_list arg;
    va_start (arg, description);
    vprintf (description, arg);
    va_end (arg);
    printf ("'\n");
    fflush (stdout);
    CS_ASSERT (false);
    return;
  }

  delete[] el->description;
  if (description)
  {
    char buf[1000];
    va_list arg;
    va_start (arg, description);
    vsprintf (buf, description, arg);
    va_end (arg);
    el->description = csStrNew (buf);
  }
  else el->description = NULL;
}

void csDebuggingGraph::RemoveObject (iObjectRegistry* object_reg,
	void* object, char* file, int linenr)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  (void)file;
  (void)linenr;
  csDebugGraph* dg = SetupDebugGraph (object_reg);

  csDGEL* el = dg->FindEl (object);
  if (!el)
  {
    printf ("ERROR! Cannot find element for object %p!\n", object);
    fflush (stdout);
    CS_ASSERT (false);
    return;
  }
  if (!el->used)
  {
    printf ("ERROR! Element for object %p is not allocated!\n", object);
    fflush (stdout);
    CS_ASSERT (false);
    return;
  }

  el->used = false;
}

void csDebuggingGraph::AddParent (iObjectRegistry* object_reg,
	void* child, void* parent)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);
  csDGEL* p_el = dg->FindEl (parent);
  // If parent could not be found. Create a dummy place holder for later.
  if (!p_el) p_el = dg->AddEl (parent);
  csDGEL* c_el = dg->FindEl (child);
  // If child could not be found. Create a dummy place holder for later.
  if (!c_el) c_el = dg->AddEl (child);

  c_el->AddParent (p_el, dg->last_timestamp++);
}

void csDebuggingGraph::AddChild (iObjectRegistry* object_reg,
	void* parent, void* child)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);
  csDGEL* p_el = dg->FindEl (parent);
  // If parent could not be found. Create a dummy place holder for later.
  if (!p_el) p_el = dg->AddEl (parent);
  csDGEL* c_el = dg->FindEl (child);
  // If child could not be found. Create a dummy place holder for later.
  if (!c_el) c_el = dg->AddEl (child);

  p_el->AddChild (c_el, dg->last_timestamp++);
}

void csDebuggingGraph::RemoveParent (iObjectRegistry* object_reg,
	void* child, void* parent)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);
  csDGEL* c_el = dg->FindEl (child);
  if (!c_el) return;	// Nothing to do if child is not there.
  csDGEL* p_el = dg->FindEl (parent);
  if (!p_el) return;	// Nothing to do if parent doesn't exist either.

  c_el->RemoveParent (p_el);
}

void csDebuggingGraph::RemoveChild (iObjectRegistry* object_reg,
	void* parent, void* child)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);
  csDGEL* p_el = dg->FindEl (parent);
  if (!p_el) return;	// Nothing to do.
  csDGEL* c_el = dg->FindEl (child);
  if (!c_el) return;	// Nothing to do.

  p_el->RemoveChild (c_el);
}

void csDebuggingGraph::Clear (iObjectRegistry* object_reg)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);
  dg->Clear ();
}

void csDebuggingGraph::Dump (iObjectRegistry* object_reg)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);

  csDGEL** els = dg->els;
  // First mark all elements as unused and count the number
  // of elements we have.
  int i, cnt = 0;
  for (i = 0 ; i < dg->num_els ; i++)
  {
    if (els[i]->used)
    {
      cnt++;
      els[i]->marker = false;
    }
    else
      els[i]->marker = true;
    els[i]->recurse_marker = false;
  }

  printf ("====================================================\n");
  printf ("Total number of used objects in graph: %d\n", cnt);

  // Find the first unmarked object and dump it.
  i = 0;
  while (i < dg->num_els)
  {
    if (!els[i]->marker)
    {
      Dump (object_reg, els[i]->object, false);
      i = 0;	// Restart scan.
      printf ("----------------------------------------------------\n");
    }
    else i++;
  }
  fflush (stdout);
}

static void DumpSubTree (int indent, const char* type, uint32 link_timestamp,
	csDGEL* el)
{
  // link_timestamp is the timestamp when the link was created.

  char spaces[1000];
  int ind = indent;
  if (ind > 999) ind = 999;
  char* sp = spaces;
  while (ind >= 10)
  {
    strcpy (sp, "          ");
    sp += 10;
    ind -= 10;
  }
  while (ind >= 1)
  {
    *sp++ = ' ';
    ind--;
  }
  *sp = 0;

  if (el->recurse_marker && *type == 'P')
  {
    // We already encountered this object in this recursion. So we just
    // put a short-hand here.
    printf ("%s%s(%d) %p <-\n", spaces, type, link_timestamp, el->object);
    return;
  }

  // Show the ref count if it is an scf interface. If the object
  // is no longer used then show '?' instead of ref count to avoid
  // calling an invalid pointer.
  printf ("%s%s(%d) %p(", spaces, type, link_timestamp, el->object);
  if (el->scf)
  {
    if (el->used)
      printf ("r%d,", ((iBase*)(el->object))->GetRefCount ());
    else
      printf ("r?,-");
  }
  else if (!el->used)
    printf ("-");
  printf ("t%d) %s", el->timestamp, el->description);

  // If the object is used but the link to this object was created
  // BEFORE the object (i.e. timestamps) then this is at least very
  // suspicious and is also marked as such.
  if (el->used && link_timestamp > 0 && link_timestamp < el->timestamp)
  {
    printf (" (SUSPICIOUS!)");
  }

  if (el->marker || *type == 'P' || !el->used)
  {
    if (el->used)
    {
      if (el->marker)
        printf (" (REF)\n");
      else
        printf ("\n");
    }
    else
      printf (" (BAD LINK!)\n");

    if (*type != 'P') el->marker = true;
  }
  else
  {
    el->recurse_marker = true;
    el->marker = true;
    printf (" (%s,%d) #p=%d #c=%d\n",
    	el->file, el->linenr, el->num_parents, el->num_children);
    int i;
    for (i = 0 ; i < el->num_parents ; i++)
    {
      DumpSubTree (indent+2, "P", el->p_stamps[i], el->parents[i]);
    }
    for (i = 0 ; i < el->num_children ; i++)
    {
      DumpSubTree (indent+2, "C", el->c_stamps[i], el->children[i]);
    }
    el->recurse_marker = false;
  }
  fflush (stdout);
}

static int compare_el (const void* vel1, const void* vel2)
{
  csDGEL* el1 = *(csDGEL**)vel1;
  csDGEL* el2 = *(csDGEL**)vel2;
  if (el1->num_parents < el2->num_parents) return -1;
  else if (el1->num_parents > el2->num_parents) return 1;
  else return 0;
}

void csDebuggingGraph::Dump (iObjectRegistry* object_reg, void* object,
	bool reset_mark)
{
#ifdef CS_DEBUG
  if (!object_reg) object_reg = iSCF::SCF->object_reg;
#endif
  CS_ASSERT (object_reg != NULL);
  csDebugGraph* dg = SetupDebugGraph (object_reg);
  int i;
  if (reset_mark)
  {
    // First mark all elements as unused.
    csDGEL** els = dg->els;
    for (i = 0 ; i < dg->num_els ; i++)
    {
      if (els[i]->used) els[i]->marker = false;
      else els[i]->marker = true;
      els[i]->recurse_marker = false;
    }
  }

  csDGEL* el = dg->FindEl (object);
  CS_ASSERT (el != NULL);

  // First copy all elements that belong to this sub-graph
  // to a local array.
  csDGEL** local_els = new csDGEL* [dg->num_els];
  int done = 0, num = 0;
  local_els[num++] = el; el->marker = true;
  while (done < num)
  {
    csDGEL* lel = local_els[done++];
    if (lel->used)
    {
      for (i = 0 ; i < lel->num_parents ; i++)
      {
        if (!lel->parents[i]->marker)
        {
          local_els[num++] = lel->parents[i];
	  lel->parents[i]->marker = true;
        }
      }	
      for (i = 0 ; i < lel->num_children ; i++)
        if (!lel->children[i]->marker)
        {
          local_els[num++] = lel->children[i];
	  lel->children[i]->marker = true;
        }
    }
  }

  // Now mark all elements as unused again.
  for (i = 0 ; i < num ; i++)
    local_els[i]->marker = false;

  // Sort all elements based on the number of parents.
  // This means that 'root' like elements will come first in the
  // array.
  qsort (local_els, num, sizeof (csDGEL*), compare_el);

  // Now dump all parents here until all are marked.
  for (i = 0 ; i < num ; i++)
  {
    if (!local_els[i]->used)
    {
      local_els[i]->marker = true;
    }
    else if (!local_els[i]->marker)
    {
      DumpSubTree (0, "R", 0, local_els[i]);
    }
  }

  delete[] local_els;
}

//-----------------------------------------------------------------------------

