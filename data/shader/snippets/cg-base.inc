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
  
  <?! Some common lists used by shaders. ?>
  <?CreateList Passes ?>
  <?CreateList VariableMaps ?>
  <?CreateList Includes ?>

  
  <?! Crude "polymorphism" realized with templates and the Cg preprocessor. 
      
      The shader snippets are sorted into "classes", depending on what purpose
      they serve (fog, lighting, surface...). For a bit of abstraction, 
      programs should not use a specific class "implementation", but the
      abstract "base class" (e.g. Surface instead of Surface_Classic).
      
      To reach that end, the 'SetSnippet' command defines macros that
      expand the "base" to the implementation.
    ?>
  <?Template _SetSnippetHelper CLASS TYPE MODULE?>
    #ifdef $CLASS$_$TYPE$
    #undef $CLASS$_$TYPE$
    #endif
    #define $CLASS$_$TYPE$ $CLASS$_$MODULE$
  <?Endtemplate?>
  <?Template SetSnippet TYPE MODULE?>
    <?_SetSnippetHelper AppToVert $TYPE$ $MODULE$?>
    <?_SetSnippetHelper VertToFrag $TYPE$ $MODULE$?>

    <?_SetSnippetHelper Frag $TYPE$ $MODULE$?>
  <?Endtemplate?>
  
</include>