<!--
  Copyright (C) 2006 by Frank Richter
	    (C) 2006 by Jorrit Tyberghein

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
-->
<include>

  <?SIfNDef __SHADERCOND_TOOLS_INC__ ?>
  <?Define __SHADERCOND_TOOLS_INC__ ?>

  <?! Some basic utility stuff for shaders. ?>

  <?! "Lists" - uses templates to realize "semi-dynamic" lists to which 
      templates can be added. Later on, everything that was added to the list 
      can be inserted at one or more places in the source. ?>
  <?! Create a list with name LIST. ?>
  <?Template CreateList LIST?>
    <?Template _List_$LIST$ ?><?Endtemplate?>
  <?Endtemplate?>
  <?! Add an item to LIST. ITEM must be the name of a template that contains
      the code to be added. ?>
  <?Template AddToList LIST ITEM?>
    <?Template _List_$LIST$ ?>
      <?_List_$LIST$?>
      <?$ITEM$?>
    <?Endtemplate?>
  <?Endtemplate?>
  <?! Insert the contents of LIST into the code. ?>
  <?Template InsertList LIST?>
    <?_List_$LIST$?>
  <?Endtemplate?>
  
  <?SEndIf?>
</include>