/*
    Copyright (C) 2003 by Christopher Nelson

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
#include "csgeom/csrect.h"
#include "csutil/scfstr.h"
#include "iaws/aws.h"
#include "iutil/string.h"
#include "awsprefs.h"

iAwsKey* awsKeyContainer::Find (iString *n) const
{
  return Find (ComputeKeyID(n->GetData()));
}

iAwsKey* awsKeyContainer::Find (const char* n) const
{
  return Find (ComputeKeyID(n));
}

iAwsKey *awsKeyContainer::Find (unsigned long idname) const
{
  if (aws_debug)
    printf (
      "aws-debug: searching for %lu (%zu items)\n",
      idname,
      children.Length ());

  size_t i;
  for (i = 0; i < children.Length (); ++i)
  {
    iAwsKey *key = children[i];

    if (aws_debug)
      printf ("aws-debug: item %zu=%lu ? %lu\n", i,
	      key->Name (), idname);

    if (key && key->Name () == idname) return key;
  }

  if (aws_debug) printf ("aws-debug: search failed.\n");

  return 0;
}

void awsKeyContainer::Remove (iString* name)
{
  iAwsKey* key = Find (name);
  
  if (key)
    Remove (key);
}

void awsKeyContainer::Remove (const char* name)
{
  iAwsKey* key = Find (name);

  if (key)
    Remove (key);
}

void awsKeyContainer::Remove (iAwsKey* key)
{
  children.Delete (key);
}

void awsKeyContainer::RemoveAll ()
{
  children.DeleteAll ();
}

void awsKeyContainer::Consume (iAwsKeyContainer *c)
{
  if (aws_debug)
  {
    printf (
      "aws-debug: Consuming %d items (%zu items currently).\n",
      c->Length (),
      children.Length ());
  }

  int i;
  /**
   * c->Length() will change as we go through the loop so don't
   * try the usual i = 0; i < c->Length(); i++.
   */
  for (i = c->Length ()-1; i >= 0; --i)
  {
    /**
     * Everytime we remove the key from c so the next key is always
     * key in index 0.
     */
    iAwsKey *k = c->GetAt (0);
    Add (k);
    c->Remove (k);
  }

  if (aws_debug)
    printf ("aws-debug: Now contains %zu items.\n",
	    children.Length ());

  /**
   * Do NOT delete awsKeyContainer!  This is NOT a memory leak! The
   * caller is responsible for cleaning up the container!
   */
}

awsConnectionNode::awsConnectionNode (iAws* a) : awsKeyContainer (a,"Connect")
{
}

awsConnectionNode::~awsConnectionNode ()
{
}
