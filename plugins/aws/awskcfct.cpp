/*
    Copyright (C) 2001 by Christopher Nelson

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
#include "awskcfct.h"

awsKeyFactory::awsKeyFactory (iAws* a) : wmgr(a)
{
  SCF_CONSTRUCT_IBASE (0);
}

awsKeyFactory::~awsKeyFactory ()
{
  SCF_DESTRUCT_IBASE ();
}

void awsKeyFactory::Initialize (const char* name, const char* component_type)
{
  awsComponentNode* n = new awsComponentNode (wmgr, name, component_type);
  base = SCF_QUERY_INTERFACE (n, iAwsComponentNode);
  CS_ASSERT (base);
  // We have a ref for n and one for base we don't want the one for n though.
  n->DecRef ();
}

void awsKeyFactory::AddToWindowList (iAwsPrefManager *pm)
{
  if (pm && base)
  {
    /**
     * Following the changes that let us instantiate non-window top
     * level components this check seems un-needed:
     * if (strcmp (base->ComponentTypeName ()->GetData (), "Window") == 0)
     */
    ((awsPrefManager *)pm)->AddWindowDef (base);
  }
}

void awsKeyFactory::AddFactory (iAwsKeyFactory *factory)
{
  if (base && factory)
  {
    base->Add (((awsKeyFactory *)factory)->base);
  }
}

void awsKeyFactory::AddIntKey (const char* name, int v)
{
  if (base)
  {
    awsIntKey* temp = new awsIntKey (wmgr, name, v);
    csRef<iAwsIntKey> key (SCF_QUERY_INTERFACE (temp, iAwsIntKey));
    CS_ASSERT (key);

    base->Add (key);

    temp->DecRef ();
  }
}

void awsKeyFactory::AddStringKey (const char* name, const char* v)
{
  if (base)
  {
    awsStringKey* temp = new awsStringKey (wmgr, name, v);
    csRef<iAwsStringKey> key (SCF_QUERY_INTERFACE (temp, iAwsStringKey));
    CS_ASSERT (key);

    base->Add (key);

    temp->DecRef ();
  }
}

void awsKeyFactory::AddRectKey (const char* name, csRect v)
{
  if (base)
  {
    awsRectKey* temp = new awsRectKey (wmgr, name, v);
    csRef<iAwsRectKey> key (SCF_QUERY_INTERFACE (temp, iAwsRectKey));
    CS_ASSERT (key);

    base->Add (key);

    temp->DecRef ();
  }
}

void awsKeyFactory::AddRGBKey (
  const char* name,
  unsigned char r,
  unsigned char g,
  unsigned char b)
{
  if (base)
  {
    awsRGBKey* temp = new awsRGBKey (wmgr, name, r, g, b);
    csRef<iAwsRGBKey> key (SCF_QUERY_INTERFACE (temp, iAwsRGBKey));
    CS_ASSERT (key);

    base->Add (key);

    temp->DecRef ();
  }
}

void awsKeyFactory::AddPointKey (const char* name, csPoint v)
{
  if (base)
  {
    awsPointKey* temp = new awsPointKey (wmgr, name, v);
    csRef<iAwsPointKey> key (SCF_QUERY_INTERFACE (temp, iAwsPointKey));
    CS_ASSERT (key);

    base->Add (key);

    temp->DecRef ();
  }
}

void awsKeyFactory::AddConnectionKey (
  const char* name,
  iAwsSink *s,
  unsigned long t,
  unsigned long sig)
{
  if (base)
  {
    awsConnectionKey* temp = new awsConnectionKey (wmgr, name, s, t, sig);
    csRef<iAwsConnectionKey> key(SCF_QUERY_INTERFACE(temp, iAwsConnectionKey));
    CS_ASSERT (key);

    base->Add (key);

    temp->DecRef ();
  }
}

void awsKeyFactory::AddConnectionNode (iAwsConnectionNodeFactory *node)
{
  if (base && node && node->GetThisNode ())
  {
    base->Add ((awsKey*)node->GetThisNode ());
    ((awsConnectionNodeFactory *)node)->base_in_use = true;
  }
}

iAwsComponentNode *awsKeyFactory::GetThisNode ()
{
  return base;
}

awsConnectionNodeFactory::awsConnectionNodeFactory (iAws* a) : wmgr(a)
{
  base = 0;
  base_in_use = false;
}

awsConnectionNodeFactory::~awsConnectionNodeFactory ()
{
  // THIS LEAKS!! Key containers do not yet clean up after themselves!
  if (base && !base_in_use) delete base;
}

void awsConnectionNodeFactory::Initialize ()
{
  base=new awsConnectionNode (wmgr);
}

void awsConnectionNodeFactory::AddConnectionKey (
  const char* name,
  iAwsSink *s,
  unsigned long t,
  unsigned long sig)
{
  if (base)
  {
    base->Add ((awsKey*)(new awsConnectionKey (wmgr, name, s, t, sig)));
  }
}

awsConnectionNode *awsConnectionNodeFactory::GetThisNode ()
{
  return base;
}
