/*
    Copyright (C) 2000 by David Durant

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
#include "csutil/csdllist.h"

csDLinkList::csDLinkList()
   {
   firstItem    = NULL;
   currentItem  = NULL;
   }


csDLinkList::~csDLinkList()
   {
   csDLListItem *nextItem;

   if (firstItem == NULL)
      return;

   currentItem = firstItem->nextItem;
   while (currentItem != firstItem)
      {
      nextItem = currentItem->nextItem;
      delete currentItem;
      currentItem = nextItem;
      }

   delete firstItem;
   }


void csDLinkList::RemoveItem(void *theObj)
   {
   currentItem = FindListItem(theObj);
   RemoveItem();
   }


csDLListItem * csDLinkList::FindListItem(void *anObj)
   {
   csDLListItem *listItemPtr;

   listItemPtr = firstItem;

   if (firstItem == NULL)
      return NULL;

   if (anObj == listItemPtr->theObject)
      return listItemPtr;

   while ((listItemPtr = listItemPtr->nextItem) != firstItem)
      {
      if (anObj == listItemPtr->theObject)
         return listItemPtr;
      }

   return NULL;
   }


void * csDLinkList::SetCurrentItem(void *theObj)
   {
   csDLListItem *it = FindListItem (theObj);
   if (it)
   {
     currentItem = it;
     return currentItem;
   }
   else
     return NULL;
}


bool csDLinkList::AddItem(void *theObj)
   {
   csDLListItem *newItem;

   newItem = new csDLListItem;
   if (!newItem)
      return false;

   newItem->theObject = theObj;

   if (firstItem == NULL)
      {
      // this is the first object in the list...
      firstItem = newItem;
      currentItem = newItem;
      newItem->prevItem = newItem;
      newItem->nextItem = newItem;
      }
   else
      {
      // this is NOT the first object in the list...
      newItem->prevItem = firstItem->prevItem;
      newItem->nextItem = firstItem;
      firstItem->prevItem->nextItem = newItem;
      firstItem->prevItem = newItem;
      }

   return true;
   }


bool csDLinkList::AddCurrentItem(void *theObj)
   {
   csDLListItem *newItem;

   if(!currentItem || !firstItem) return false;

   newItem = new csDLListItem;
   if (!newItem)
      return false;

   newItem->theObject = theObj;

   newItem->nextItem = currentItem->nextItem;
   currentItem->nextItem = newItem;
   newItem->prevItem = currentItem;
   newItem->nextItem->prevItem = newItem;

   return true;
   }

void csDLinkList::RemoveItem()
   {
   if (!currentItem)
      return;

   if (currentItem->nextItem == currentItem)
      {
      // this is the last object in the list
      firstItem = NULL;
      currentItem = NULL;
      delete currentItem;
      }
   else
      {
      // this is NOT the last object in the list
      csDLListItem *deadItem;

      deadItem = currentItem;
      currentItem->nextItem->prevItem = currentItem->prevItem;
      currentItem->prevItem->nextItem = currentItem->nextItem;
      if (firstItem == currentItem)
         firstItem = currentItem->nextItem;
      currentItem = currentItem->nextItem;
      delete deadItem;
      }
   }


void * csDLinkList::GetFirstItem()
   {
   if (!currentItem)
      return NULL;

   currentItem = firstItem;
   return currentItem->theObject;
   }


void * csDLinkList::PeekFirstItem()
   {
     if (!firstItem)
       return NULL;

     return firstItem->theObject;
   }


void * csDLinkList::GetPrevItem()
   {
   if (!currentItem)
      return NULL;

   currentItem = currentItem->prevItem;
   return currentItem->theObject;
   }


void * csDLinkList::GetNextItem()
   {
   if (!currentItem)
      return NULL;

   currentItem = currentItem->nextItem;
   return currentItem->theObject;
   }


void * csDLinkList::GetCurrentItem()
   {
   if (!currentItem)
      return NULL;

   return currentItem->theObject;
   }
