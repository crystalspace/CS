DESCRIPTION.surf = Surface mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make surf         Make the $(DESCRIPTION.surf)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: surf surfclean
plugins meshes all: surf

surfclean:
	$(MAKE_CLEAN)
surf:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/surf

ifeq ($(USE_PLUGINS),yes)
  SURF = $(OUTDLL)surf$(DLL)
  LIB.SURF = $(foreach d,$(DEP.SURF),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SURF)
else
  SURF = $(OUT)$(LIB_PREFIX)surf$(LIB)
  DEP.EXE += $(SURF)
  SCF.STATIC += surf
  TO_INSTALL.STATIC_LIBS += $(SURF)
endif

INC.SURF = $(wildcard plugins/mesh/object/surf/*.h)
SRC.SURF = $(wildcard plugins/mesh/object/surf/*.cpp)
OBJ.SURF = $(addprefix $(OUT),$(notdir $(SRC.SURF:.cpp=$O)))
DEP.SURF = CSGEOM CSUTIL CSSYS

MSVC.DSP += SURF
DSP.SURF.NAME = surf
DSP.SURF.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: surf surfclean
surf: $(OUTDIRS) $(SURF)

$(SURF): $(OBJ.SURF) $(LIB.SURF)
	$(DO.PLUGIN)

clean: surfclean
surfclean:
	-$(RM) $(SURF) $(OBJ.SURF)

ifdef DO_DEPEND
dep: $(OUTOS)surf.dep
$(OUTOS)surf.dep: $(SRC.SURF)
	$(DO.DEP)
else
-include $(OUTOS)surf.dep
endif

endif # ifeq ($(MAKESECTION),targets)
