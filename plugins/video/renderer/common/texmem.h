/*
    Copyright (C) 1998 by Jorrit Tyberghein
    Written by Xavier Trochu

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

#ifndef TEXMEM_H
#define TEXMEM_H

/// An entry in memory : Pointer and Size
typedef struct TextureMemoryEntry
{
	/// Offset of this bloc from the start of memory
	unsigned int offset;
	/// Size of this bloc.
	size_t size;
} * textMemSpace;

/// The Memory manager Base Class
class TextureMemoryManager
{
public:
	/// Check for free space for this size
	virtual bool hasFreeSpace(size_t blocksize) = 0;
	/// Allocation of space for this size
	virtual textMemSpace allocSpaceMem(size_t blocksize) = 0;
	/// Free this bloc of mem
	virtual void freeSpaceMem(textMemSpace entry) = 0;
	/**
	 * Gives an idea about the current fragmentation state
	 * The more it is, the more the mem is fragmented !
	 */
	virtual int getFragmentationState(void) =0;
};

/// This class implements all abstract function of TextureMemoryManager, but does nothing.
class DummyTextureMemoryManager : public TextureMemoryManager
{
public:
	virtual bool hasFreeSpace(size_t blocksize) { return true; };
	virtual textMemSpace allocSpaceMem(size_t blocksize) { return 0; };
	virtual void freeSpaceMem(textMemSpace) {};
	virtual int getFragmentationState(void) {return 0; };
};

/// This class implements a TextureMemoryManager based on a global linear bloc of memory.
class FixedTextureMemoryManager : public TextureMemoryManager
{
protected:
	/// A structure for the linked list of chuncks.
	struct chunck
	{
		/// The offset and size of this chunck.
		TextureMemoryEntry self;
		/// The previous and the next chunk (NULL if none).
		chunck * prev, *next;
	};
	/// Total size of the memory
	size_t size;
	/// Linked list of free chuncks.
	chunck * freeSpace;
	/// Linked list of allocated chuncks.
	chunck * allocatedSpace;
public:
	/// The constructor
	FixedTextureMemoryManager(size_t _size);
	/// return true if enough space available.
	virtual bool hasFreeSpace(size_t blocksize);
	/// allocate a chunck of memory.
	virtual textMemSpace allocSpaceMem(size_t blocksize);
	/// frees a previously allocated bloc of memory.
	virtual void freeSpaceMem(textMemSpace entry);
	/// return information about the fragmentation of the memory.
	virtual int getFragmentationState(void);
};


#endif // TEXMEM_H

