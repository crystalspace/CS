# This is a subinclude file used to define the rules needed
# to build the frustvis plug-in.

# Driver description
DESCRIPTION.frustvis = Crystal Space Frustum Visibility System

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make frustvis     Make the $(DESCRIPTION.frustvis)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: frustvis frustvisclean
all plugins: frustvis

frustvis:
	$(MAKE_TARGET) MAKE_DLL=yes
frustvisclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/culling/frustvis

ifeq ($(USE_PLUGINS),yes)
  FRUSTVIS = $(OUTDLL)frustvis$(DLL)
  LIB.FRUSTVIS = $(foreach d,$(DEP.FRUSTVIS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FRUSTVIS)
else
  FRUSTVIS = $(OUT)$(LIB_PREFIX)frustvis$(LIB)
  DEP.EXE += $(FRUSTVIS)
  SCF.STATIC += frustvis
  TO_INSTALL.STATIC_LIBS += $(FRUSTVIS)
endif

INC.FRUSTVIS = $(wildcard plugins/culling/frustvis/*.h)
SRC.FRUSTVIS = $(wildcard plugins/culling/frustvis/*.cpp)
OBJ.FRUSTVIS = $(addprefix $(OUT),$(notdir $(SRC.FRUSTVIS:.cpp=$O)))
DEP.FRUSTVIS = CSUTIL CSGEOM CSUTIL CSSYS CSUTIL CSSYS

MSVC.DSP += FRUSTVIS
DSP.FRUSTVIS.NAME = frustvis
DSP.FRUSTVIS.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: frustvis frustvisclean

frustvis: $(OUTDIRS) $(FRUSTVIS)

$(FRUSTVIS): $(OBJ.FRUSTVIS) $(LIB.FRUSTVIS)
	$(DO.PLUGIN)

clean: frustvisclean
frustvisclean:
	$(RM) $(FRUSTVIS) $(OBJ.FRUSTVIS)

ifdef DO_DEPEND
dep: $(OUTOS)frustvis.dep
$(OUTOS)frustvis.dep: $(SRC.FRUSTVIS)
	$(DO.DEP)
else
-include $(OUTOS)frustvis.dep
endif

endif # ifeq ($(MAKESECTION),targets)
