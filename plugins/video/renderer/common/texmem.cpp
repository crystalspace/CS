/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Xavier Trochu.

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
#include "cssysdef.h"
#include "texmem.h"
#include "isys/system.h"

/**
  * This function provides a state of the current fragmentation of the memory.
  *
  * I have not implemented any unfragmentation procedure, mainly because it is
  * not always possible to read Data from Texture Memory.
  * 
  * With this current implementation, it will be difficult to add.
  */
int FixedTextureMemoryManager::getFragmentationState(void)
{
	int n=0;
	chunck * c;
	c=freeSpace;
	while(c)
	{
		n++;
		c=c->next;
	}
	c=allocatedSpace;
	while(c)
	{
		n++;
		c=c->next;
	}
	return n/2;
}

/**
  * Constructor of the FixedTextureMemoryManager Class.
  *
  * It takes the total size of the memory as a parameter.
  * 
  * It initialise the memory as free space.
  */
FixedTextureMemoryManager::FixedTextureMemoryManager(size_t _size) : size(_size)
{
	allocatedSpace=0;
	freeSpace=new chunck;
	freeSpace->prev=freeSpace->next=0;
	freeSpace->self.offset=0;
	freeSpace->self.size=size;
};

/**
  * This function return true if there is enough linear space in memory for the
  * required size.
  *
  */
bool FixedTextureMemoryManager::hasFreeSpace(size_t blocksize)
{
	// For each free chunck...
	chunck * f=freeSpace;
	while(f)
	{
		//   If it has enough space
		if(f->self.size>=blocksize)
			//     return true
			return true;
		f=f->next;
		// return false
	}
	return false;
}

/**
  * This function allocates a bloc of memory of the required size. It also put the
  * memory bloc in the allocated state. It return a pointer to a 
  */
textMemSpace FixedTextureMemoryManager::allocSpaceMem(size_t blocksize) 
{
	// If required mem size is strange
	if(blocksize<=0)
		// Return an error
		return NULL;
	// For each free chunck...
	chunck * f=freeSpace;
	while(f)
	{
		//   If it has enough space
		if(f->self.size>=blocksize)
			//     return true
			goto found;
		f=f->next;
	}
	// return false, error, hasFreeSpace not called before!
	return 0;
found:
	textMemSpace entry = new TextureMemoryEntry;
	entry->offset=f->self.offset;
	entry->size=blocksize;
	// reduce chunck
	f->self.size-=blocksize;
	f->self.offset+=blocksize;
	// if there is no space left on chunck
	if(!f->self.size)
	{
		//   Remove chunck
		if(f->prev)
			f->prev->next=f->next;
		if(f->next)
			f->next->prev=f->prev;
		//   if first chunck, change chunck
		if(f==freeSpace)
			freeSpace=f->next;
		delete f;
	}
	f=allocatedSpace;
    /* initialize a to something to avoid compiler warning */
	chunck *a[2]={NULL};
	int nb=0;
	while(f)
	{
		// Find allocated chunck(s) involved
		if((entry->offset)==(f->self.offset+f->self.size))
		{
			a[0]=f;
			nb|=1;
		}
		if((entry->offset+entry->size)==(f->self.offset))
		{
			a[1]=f;
			nb|=2;
		}
		f=f->next;
	}
	switch(nb)
	{
	case 0:
		// if none 
		f=new chunck;
		//   add a chunck
		f->self.offset=entry->offset;
		f->self.size=entry->size;
		
		f->next=allocatedSpace;
		if(allocatedSpace)
			allocatedSpace->prev=f;
		f->prev=0;
		allocatedSpace=f;
		break;
	case 1:
		// else if one
		//   add the mem to the chunck
		a[0]->self.size+=entry->size;
		break;
	case 2:
		a[1]->self.offset=entry->offset;
		a[1]->self.size+=entry->size;
		break;
		// else //(two)
	case 3:
		// merge chuncks.
		a[0]->self.size+=entry->size+a[1]->self.size;
		// Remove one chunck
		if(a[1]->prev)
			a[1]->prev->next=a[1]->next;
		if(a[1]->next)
			a[1]->next->prev=a[1]->prev;
		if(allocatedSpace==a[1])
			allocatedSpace=a[1]->next;
		delete a[1];
		break;
	}
	// return Info
	return entry;
}

/// frees a space of mem
void FixedTextureMemoryManager::freeSpaceMem(textMemSpace entry) {
	chunck *c=allocatedSpace;
	while(c)
	{
		// find the allocated chunck involved.
		if((entry->offset>=c->self.offset)&&(entry->offset<(c->self.offset+c->self.size)))
			break;
		c=c->next;
	}
	// if all mem of chunck is for this text
	if((entry->offset==c->self.offset)&&(entry->size==c->self.size))
	{
		//   remove chunck
		if(c->next)
			c->next->prev=c->prev;
		if(c->prev)
			c->prev->next=c->next;
		// if first chunck
		if(c==allocatedSpace)
			allocatedSpace=c->next;
		delete c;
		
	}
	// else if at a border of the chunck
	else if(entry->offset==c->self.offset)
	{
		//   shrink the chunck
		c->self.offset+=entry->size;
		c->self.size-=entry->size;
	}
	else if((entry->offset+entry->size)==(c->self.offset+c->self.size))
	{
		//   shrink the chunck
		c->self.size-=entry->size;
	}
	// else if in the middle of a chunck
	else
	{
		//   split it in two
		chunck *n=new chunck;
		/* In the list:
		 * current <-> after
		 * must become:
		 * current <-> new <-> after
		 */
		n->prev=c;   		  // current <- new 
		n->next=c->next;	  // new -> after
		if(n->next)
			n->next->prev=n;  // new <- after
		c->next=n;		  // current -> new

		n->self.offset=entry->offset+entry->size;
		n->self.size=c->self.offset+c->self.size-n->self.offset;
		c->self.size=entry->offset-c->self.offset;
	}
	
	// find the free chunck(s) involved
    // initialize f to something to avoid compiler warning
	chunck * f[2]={NULL};
	int nb=0;
	c=freeSpace;
	while(c)
	{
		if((entry->offset)==(c->self.offset+c->self.size))
		{
			f[0]=c;	nb|=1;
		}
		if((entry->offset+entry->size)==(c->self.offset))
		{
			f[1]=c;	nb|=2;
		}
		c=c->next;
	}
	switch(nb)
	{
	case 0:
		// if none found 
		{
			chunck * n=new chunck;
			//     Create a new chunck.
			n->self.size=entry->size;
			n->self.offset=entry->offset;
			n->prev=0;
			n->next=freeSpace;
			if(freeSpace)
				freeSpace->prev=n;
			freeSpace=n;
		}
		break;
	case 1:
		// else if one found
		//   Add the mem to this chunck
		f[0]->self.size+=entry->size;
		break;
	case 2:
		f[1]->self.offset=entry->offset;
		f[1]->self.size+=entry->size;
		break;
		// else (//if two found)
	case 3:
		//   merge them.
		f[0]->self.size+=entry->size+f[1]->self.size;
		// Remove one
		if(f[1]->prev)
			f[1]->prev->next=f[1]->next;
		if(f[1]->next)
			f[1]->next->prev=f[1]->prev;
		if(f[1]==freeSpace)
			freeSpace=f[1]->next;
		delete f[1];
		break;
	}
}
