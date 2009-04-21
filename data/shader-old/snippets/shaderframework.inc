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
  <?SIfNDef __SHADERFRAMEWORK_INC__ ?>
  <?Define __SHADERFRAMEWORK_INC__ ?>

  <?! Framework templates for shaders. ?>

  <?Include /shader/snippets/shadercond-tools.inc ?>
  
  <?! Glue - ie per-pass, per-technique bindings. Ensures that the glue
      is only evaluated once. ?>
  <?Template BeginGlue component?>
    <?SIfNDef _glue_$component$?>
    <?Define _glue_$component$?>
  <?Endtemplate?>
  <?Template EndGlue?>
    <?SEndIf?>
  <?Endtemplate?>
  
  <?! Helper to share a resource NAME. Emits the statement STATEMENT and
      adds PASS and VARIABLEMAP to the glue. ?>
  <?Template ShareResource NAME STATEMENT PASS VARIABLEMAP ?>
    $STATEMENT$
    <?SIfNDef _resshared_$NAME$?>
      <?Define _resshared_$NAME$?>
      <?BeginGlue _resshared_$NAME$?>
	<?$PASS$?>
	<?$VARIABLEMAP$?>
      <?EndGlue?>
    <?SEndIf?>
  <?Endtemplate?>
  
  <?! Create default lists ?>
  <?CreateList PassMappings ?>
  <?CreateList ProgramMappings ?>
  <?CreateList ShaderVars ?>
  
  <?! Write out a single pass, given the current mappings ?>
  <?Template WriteShaderPass?>
    <pass>
      <?TemplateWeak WriteVertexProgram?><?Endtemplate?>
      <?WriteVertexProgram?>
      <?TemplateWeak WriteFragmentProgram?><?Endtemplate?>
      <?WriteFragmentProgram?>
      <?InsertList PassMappings?>
    </pass>
  <?Endtemplate?>
  <?! Write out a shader's techniques ?>
  <?Template WriteShaderTechnique PRIO?>
    <technique priority="$PRIO$">
      <?WriteShaderPass?>
    </technique>
  <?Endtemplate?>
  
  <?! Write out shader variables ?>
  <?Template WriteShaderVars?>
    <shadervars><?InsertList ShaderVars?></shadervars>
  <?Endtemplate?>

  <?SEndIf?>
</include>
