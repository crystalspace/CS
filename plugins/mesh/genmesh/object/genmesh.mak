DESCRIPTION.genmesh = General mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make genmesh      Make the $(DESCRIPTION.genmesh)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: genmesh genmeshclean
plugins meshes all: genmesh

genmeshclean:
	$(MAKE_CLEAN)
genmesh:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/genmesh/object

ifeq ($(USE_PLUGINS),yes)
  GENMESH = $(OUTDLL)genmesh$(DLL)
  LIB.GENMESH = $(foreach d,$(DEP.GENMESH),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(GENMESH)
else
  GENMESH = $(OUT)$(LIB_PREFIX)genmesh$(LIB)
  DEP.EXE += $(GENMESH)
  SCF.STATIC += genmesh
  TO_INSTALL.STATIC_LIBS += $(GENMESH)
endif

INC.GENMESH = $(wildcard plugins/mesh/genmesh/object/*.h)
SRC.GENMESH = $(wildcard plugins/mesh/genmesh/object/*.cpp)
OBJ.GENMESH = $(addprefix $(OUT),$(notdir $(SRC.GENMESH:.cpp=$O)))
DEP.GENMESH = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += GENMESH
DSP.GENMESH.NAME = genmesh
DSP.GENMESH.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: genmesh genmeshclean
genmesh: $(OUTDIRS) $(GENMESH)

$(GENMESH): $(OBJ.GENMESH) $(LIB.GENMESH)
	$(DO.PLUGIN)

clean: genmeshclean
genmeshclean:
	-$(RM) $(GENMESH) $(OBJ.GENMESH)

ifdef DO_DEPEND
dep: $(OUTOS)genmesh.dep
$(OUTOS)genmesh.dep: $(SRC.GENMESH)
	$(DO.DEP)
else
-include $(OUTOS)genmesh.dep
endif

endif # ifeq ($(MAKESECTION),targets)
