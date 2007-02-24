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
  
  <?CreateList CgAddV2F ?>
  <?CreateList CgInitV2F_V ?>
  <?CreateList CgInitV2F_F ?>
  <?CreateList CgProgramPrepend?>

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
    <?AddToList CgProgramPrepend _prepend_buf_$CGNAME$ ?>
  <?Endtemplate?>
  
  <?Template CgShareBuffer TYPE SVAR CGNAME ?>
    <?_CgShareBuffer $TYPE$ $SVAR$ $CGNAME$ "" ?>
  <?Endtemplate?>
  <?Template CgShareBufferSem TYPE SVAR CGNAME SEM ?>
    <?_CgShareBuffer $TYPE$ $SVAR$ $CGNAME$ ": $SEM$" ?>
  <?Endtemplate?>
  
  <?Template _CgShareV2F TYPE CGNAME COND?>
    <?Template _v2f_$CGNAME$?>
      <?if $COND$?>
      #ifndef _resshared_cgv2f_$CGNAME$
      #define _resshared_cgv2f_$CGNAME$
      $TYPE$ local_$CGNAME$;
      #endif
      <?endif?>
    <?Endtemplate?>
    <?AddToList CgAddV2F _v2f_$CGNAME$ ?>
    
    <?Template _v2f_$CGNAME$_init_v?>
      <?if $COND$?>
      #ifndef _resshared_cgv2f_v_$CGNAME$
      #define _resshared_cgv2f_v_$CGNAME$
      local_$CGNAME$ = $CGNAME$;
      #endif
      <?endif?>
    <?Endtemplate?>
    <?AddToList CgInitV2F_V _v2f_$CGNAME$_init_v ?>
    
    <?Template _v2f_$CGNAME$_init_f?>
      <?if $COND$?>
      #ifndef _resshared_cgv2f_f_$CGNAME$
      #define _resshared_cgv2f_f_$CGNAME$
      $CGNAME$ = local_$CGNAME$;
      #endif
      <?endif?>
    <?Endtemplate?>
    <?AddToList CgInitV2F_F _v2f_$CGNAME$_init_f ?>
  <?Endtemplate?>
  <?Template CgShareV2F TYPE CGNAME ?>
    <?_CgShareV2F "$TYPE$" "$CGNAME$" consts.true?>
  <?Endtemplate?>
  <?Template CgShareV2FCond TYPE CGNAME COND ?>
    <?_CgShareV2F "$TYPE$" "$CGNAME$" "$COND$"?>
  <?Endtemplate?>

  <?Template CgSetVertexFunc FUNC ?>
    <?Template WriteVertexProgram?>
      <?Template CgProgram?>
	struct VertToFrag_Framework 
	{
	  <?InsertList CgAddV2F?>
	  
	  void SetupVert ()
	  {
	    <?InsertList CgInitV2F_V?>
	  }
	  
	  void SetupFrag ()
	  {
	    <?InsertList CgInitV2F_F?>
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
	  <?InsertList CgProgramPrepend?>
	  <?CgProgram?>
	  </program>
	  <?InsertList ProgramMappings?>
	</cgvp>
      </vp>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?Template CgSetFragmentFunc FUNC ?>
    <?Template WriteFragmentProgram?>
      <fp plugin="glcg">
	<cgfp>
	  <entry>$FUNC$</entry>
	  <?TemplateWeak CgFragmentOptions?><?Endtemplate?>
	  <?CgFragmentOptions?>
	  <program>
	  <?InsertList CgProgramPrepend?>
	  <?CgProgram?>
	  </program>
	  <?InsertList ProgramMappings?>
	</cgfp>
      </fp>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?Template CgAddSnippet TEMPLATE?>
    <?AddToList CgProgramPrepend $TEMPLATE$ ?>
  <?Endtemplate?>
  
  <?Template CgAddSharedBufferSem FEATURE TYPE SVAR CGNAME SEM?>
    <?Template _CgUseShared_$FEATURE$ COND?>
      <?CgShareBufferSem "$TYPE$" "$SVAR$" $CGNAME$ $SEM$ ?>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?Template CgAddSharedBuffer FEATURE TYPE SVAR CGNAME?>
    <?Template _CgUseShared_$FEATURE$ COND?>
      <?CgShareBuffer "$TYPE$" "$SVAR$" $CGNAME$ ?>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?Template CgAddSharedV2F FEATURE TYPE CGNAME?>
    <?Template _CgUseShared_$FEATURE$ COND?>
      <?CgShareV2F $TYPE$ $CGNAME$ "$COND$"?>
    <?Endtemplate?>
  <?Endtemplate?>
  
  <?CgAddSharedBuffer texCoord "varying float2" "texture coordinate" texCoord?>
  <?CgAddSharedV2F texCoordV2F float2 texCoord?>

  <?CgAddSharedBufferSem primaryColor "varying float4" "primary color" primaryColor COLOR?>
  <?CgAddSharedV2F primaryColorV2F float4 primaryColor ?>
 
  <?Template CgUseShared FEATURE?>
    <?_CgUseShared_$FEATURE$ consts.true?>
  <?Endtemplate?>
  <?Template CgUseSharedCond FEATURE COND?>
    <?_CgUseShared_$FEATURE$ "$COND$"?>
  <?Endtemplate?>
  
  <?SEndIf?>
</include>
