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
  <?SIfNDef __SHADERFRAMEWORK_CG_INC__ ?>
  <?Define __SHADERFRAMEWORK_CG_INC__ ?>
  
  <?Include /shader/snippets/shaderframework.inc ?>

  <?! Framework templates for Cg inshaders. ?>
  
  <?! Set up lists used by the framework ?>
  <?CreateList _CgAddV2F ?>
  <?CreateList _CgInitV2F_V ?>
  <?CreateList _CgInitV2F_F ?>
  <?CreateList _CgProgramPrepend?>

  <?Template _CgShareBuffer TYPE SVAR CGNAME SEM ?>
    <?Template _gen_pass_$CGNAME$?>
      <?Template _pass_$CGNAME$ ?>
	<buffer source="$SVAR$" destination="$CGNAME$" />
      <?Endtemplate?>
      <?AddToList PassMappings _pass_$CGNAME$ ?>
    <?Endtemplate?>
    <?Template _gen_varmap_$CGNAME$?>
    <?Endtemplate?>
    <?Template _prepend_buf_$CGNAME$?>
      #ifndef __SHARED_BUFFER_$CGNAME$__
      #define __SHARED_BUFFER_$CGNAME$__
      <?ShareResource $CGNAME$ "$TYPE$ $CGNAME$ $SEM$;" _gen_pass_$CGNAME$ _gen_varmap_$CGNAME$?>
      #endif
    <?Endtemplate?>
    <?AddToList _CgProgramPrepend _prepend_buf_$CGNAME$ ?>
  <?Endtemplate?>
  
  <?! Share a buffer, which is in the shader variable SVAR, with the Cg
      variable of type TYPE being named CGNAME. ?>
  <?Template CgShareBuffer TYPE SVAR CGNAME ?>
    <?_CgShareBuffer $TYPE$ $SVAR$ $CGNAME$ "" ?>
  <?Endtemplate?>
  <?! Share a buffer, which is in the shader variable SVAR, with the Cg
      variable of type TYPE with semantics SEM being named CGNAME. ?>
  <?Template CgShareBufferSem TYPE SVAR CGNAME SEM ?>
    <?_CgShareBuffer $TYPE$ $SVAR$ $CGNAME$ ": $SEM$" ?>
  <?Endtemplate?>
  
  <?Template _CgShareV2F TYPE CGNAME COND SEM?>
    <?Template _v2f_$CGNAME$?>
      <?if $COND$?>
      #ifndef _resshared_cgv2f_$CGNAME$
      #define _resshared_cgv2f_$CGNAME$
      $TYPE$ local_$CGNAME$ $SEM$;
      #endif
      <?endif?>
    <?Endtemplate?>
    <?AddToList _CgAddV2F _v2f_$CGNAME$ ?>
    
    <?Template _v2f_$CGNAME$_init_v?>
      <?if $COND$?>
      #ifndef _resshared_cgv2f_v_$CGNAME$
      #define _resshared_cgv2f_v_$CGNAME$
      local_$CGNAME$ = $CGNAME$;
      #endif
      <?endif?>
    <?Endtemplate?>
    <?AddToList _CgInitV2F_V _v2f_$CGNAME$_init_v ?>
    
    <?Template _v2f_$CGNAME$_init_f?>
      <?if $COND$?>
      #ifndef _resshared_cgv2f_f_$CGNAME$
      #define _resshared_cgv2f_f_$CGNAME$
      $CGNAME$ = local_$CGNAME$;
      #endif
      <?endif?>
    <?Endtemplate?>
    <?AddToList _CgInitV2F_F _v2f_$CGNAME$_init_f ?>
  <?Endtemplate?>
  <?! Share vertex-to-fragment data, accessible by a variable CGNAME of type
      TYPE. ?>
  <?Template CgShareV2F TYPE CGNAME ?>
    <?_CgShareV2F "$TYPE$" "$CGNAME$" consts.true?>
  <?Endtemplate?>
  <?Template CgShareV2FSem TYPE CGNAME SEM?>
    <?_CgShareV2F "$TYPE$" "$CGNAME$" consts.true ": $SEM$"?>
  <?Endtemplate?>
  <?! Share vertex-to-fragment data, when the condition COND is true, 
      accessible by a variable CGNAME of type TYPE. ?>
  <?Template CgShareV2FCond TYPE CGNAME COND ?>
    <?_CgShareV2F "$TYPE$" "$CGNAME$" $COND$?>
  <?Endtemplate?>
  <?Template CgShareV2FCondSem TYPE CGNAME COND SEM ?>
    <?_CgShareV2F "$TYPE$" "$CGNAME$" $COND$ ": $SEM$"?>
  <?Endtemplate?>

  <?! Set the vertex program function ?>
  <?Template CgSetVertexFunc FUNC ?>
    <?Template WriteVertexProgram?>
      <?Template CgProgram?>
	struct VertToFrag_Framework 
	{
	  <?InsertList _CgAddV2F?>
	  
	  void SetupVert ()
	  {
	    <?InsertList _CgInitV2F_V?>
	  }
	  
	  void SetupFrag ()
	  {
	    <?InsertList _CgInitV2F_F?>
	  }
	};
	<?CgProgram?>
      <?Endtemplate?>
      <vp plugin="glcg">
	<cgvp>
	  <entry>$FUNC$</entry>
	  <?TemplateWeak CgVertexOptions?><?Endtemplate?>
	  <?CgVertexOptions?>
	  <program>
	  <?InsertList _CgProgramPrepend?>
	  <?CgProgram?>
	  </program>
	  <?InsertList ProgramMappings?>
	</cgvp>
      </vp>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?! Set the fragment program function ?>
  <?Template CgSetFragmentFunc FUNC ?>
    <?Template WriteFragmentProgram?>
      <fp plugin="glcg">
	<cgfp>
	  <entry>$FUNC$</entry>
	  <?TemplateWeak CgFragmentOptions?><?Endtemplate?>
	  <?CgFragmentOptions?>
	  <program>
	  <?InsertList _CgProgramPrepend?>
	  <?CgProgram?>
	  </program>
	  <?InsertList ProgramMappings?>
	</cgfp>
      </fp>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?! Add Cg code provided by a snippet ?>
  <?Template CgAddSnippet TEMPLATE?>
    <?AddToList _CgProgramPrepend $TEMPLATE$ ?>
  <?Endtemplate?>

  <?! Add a "named shared buffer" of name FEATURE. ?>
  <?Template CgAddSharedBufferSem FEATURE TYPE SVAR CGNAME SEM?>
    <?Template _CgUseShared_$FEATURE$ COND?>
      <?CgShareBufferSem "$TYPE$" "$SVAR$" $CGNAME$ $SEM$ ?>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?! Add a "named shared buffer" of name FEATURE. ?>
  <?Template CgAddSharedBuffer FEATURE TYPE SVAR CGNAME?>
    <?Template _CgUseShared_$FEATURE$ COND?>
      <?CgShareBuffer "$TYPE$" "$SVAR$" $CGNAME$ ?>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?! Add "named vertex-to-fragment data". ?>
  <?Template CgAddSharedV2F FEATURE TYPE CGNAME?>
    <?Template _CgUseShared_$FEATURE$ COND?>
      <?CgShareV2FCond $TYPE$ $CGNAME$ $$COND$$?>
    <?Endtemplate?>
  <?Endtemplate?>
  <?Template CgAddSharedV2FSem FEATURE TYPE CGNAME SEM?>
    <?Template _CgUseShared_$FEATURE$ COND?>
      <?CgShareV2FCondSem $TYPE$ $CGNAME$ $$COND$$ $SEM$?>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?! Some default "features" - buffers and vertex-to-fragment, named for 
      easier use ?>
  <?CgAddSharedBufferSem primaryColor "varying float4" "primary color" primaryColor COLOR?>
  <?CgAddSharedV2FSem primaryColorV2F float4 primaryColor COLOR?>
  
  <?CgAddSharedBuffer texCoord "varying float2" "texture coordinate" texCoord?>
  <?CgAddSharedV2F texCoordV2F float2 texCoord?>

 
  <?! Use a "feature" (see above) ?>
  <?Template CgUseShared FEATURE?>
    <?_CgUseShared_$FEATURE$ consts.true?>
  <?Endtemplate?>
  <?! Use a "feature" (see above) when condition COND is true ?>
  <?Template CgUseSharedCond FEATURE COND?>
    <?_CgUseShared_$FEATURE$ $"COND$?>
  <?Endtemplate?>
  
  <?SEndIf?>
</include>
