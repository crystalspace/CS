<include>

  <?! Some basic utility stuff for shaders. ?>

  <?! "Lists" - uses templates to realize "semi-dynamic" lists to which 
      templates can be added. Later on, everything that was added to the list 
      can be inserted at one or more places in the source. ?>
  <?! Create a list with name LIST. ?>
  <?template CreateList LIST?>
    <?template _List_$LIST$ ?><?endtemplate?>
  <?endtemplate?>
  <?! Add an item to LIST. ITEM must be the name of a template that contains
      the code to be added. ?>
  <?template AddToList LIST ITEM?>
    <?template _List_$LIST$ ?>
      <?_List_$LIST$?>
      <?$ITEM$?>
    <?endtemplate?>
  <?endtemplate?>
  <?! Insert the contents of LIST into the code. ?>
  <?template InsertList LIST?>
    <?_List_$LIST$?>
  <?endtemplate?>
  
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
  <?template _SetSnippetHelper CLASS TYPE MODULE?>
    #ifdef $CLASS$_$TYPE$
    #undef $CLASS$_$TYPE$
    #endif
    #define $CLASS$_$TYPE$ $CLASS$_$MODULE$
  <?endtemplate?>
  <?template SetSnippet TYPE MODULE?>
    <?_SetSnippetHelper AppToVert $TYPE$ $MODULE$?>
    <?_SetSnippetHelper VertToFrag $TYPE$ $MODULE$?>

    <?_SetSnippetHelper Frag $TYPE$ $MODULE$?>
  <?endtemplate?>
  
</include>