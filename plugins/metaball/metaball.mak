# This is a subinclude file used to define the rules needed
# to build the metaball plug-in.

# Driver description
DESCRIPTION.metaball = Crystal Space MetaBall Renderer Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make metaball     Make the $(DESCRIPTION.metaball)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: metaball metaballclean

all plugins: metaball

metaball:
	$(MAKE_TARGET) MAKE_DLL=yes
metaballclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/metaball

ifeq ($(USE_SHARED_PLUGINS),yes)
  METABALL=$(OUTDLL)metaball$(DLL)
  DEP.METABALL=$(CSGEOM.LIB) $(CSSYS.LIB) $(CSUTIL.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(METABALL)
else
  METABALL=$(OUT)$(LIB_PREFIX)meta$(LIB)
  DEP.EXE+=$(METABALL)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_METABALL
  TO_INSTALL.STATIC_LIBS+=$(METABALL)
endif
TO_INSTALL.CONFIG += data/config/metaball.cfg
DESCRIPTION.$(METABALL) = $(DESCRIPTION.metaball)
SRC.METABALL = $(wildcard plugins/metaball/*.cpp)
OBJ.METABALL = $(addprefix $(OUT),$(notdir $(SRC.METABALL:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: metaball metaballclean

# Chain rules
net: metaball
clean: metaballclean

metaball: $(OUTDIRS) $(METABALL)

$(METABALL): $(OBJ.METABALL) $(DEP.METABALL)
	$(DO.PLUGIN)

metaballclean:
	$(RM) $(METABALL) $(OBJ.METABALL) $(OUTOS)metaball.dep

ifdef DO_DEPEND
dep: $(OUTOS)metaball.dep
$(OUTOS)metaball.dep: $(SRC.METABALL)
	$(DO.DEP)
else
-include $(OUTOS)metaball.dep
endif

endif # ifeq ($(MAKESECTION),targets)
