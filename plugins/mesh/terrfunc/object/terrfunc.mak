DESCRIPTION.terrfunc = Function Terrain plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make terrfunc     Make the $(DESCRIPTION.terrfunc)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: terrfunc terrfuncclean
plugins meshes all: terrfunc

terrfuncclean:
	$(MAKE_CLEAN)
terrfunc:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/terrfunc/object

ifeq ($(USE_PLUGINS),yes)
  TERRFUNC = $(OUTDLL)terrfunc$(DLL)
  LIB.TERRFUNC = $(foreach d,$(DEP.TERRFUNC),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(TERRFUNC)
else
  TERRFUNC = $(OUT)$(LIB_PREFIX)terrfunc$(LIB)
  DEP.EXE += $(TERRFUNC)
  SCF.STATIC += terrfunc
  TO_INSTALL.STATIC_LIBS += $(TERRFUNC)
endif

INC.TERRFUNC = $(wildcard plugins/mesh/terrfunc/object/*.h)
SRC.TERRFUNC = $(wildcard plugins/mesh/terrfunc/object/*.cpp)
OBJ.TERRFUNC = $(addprefix $(OUT),$(notdir $(SRC.TERRFUNC:.cpp=$O)))
DEP.TERRFUNC = CSGEOM CSUTIL CSSYS CSUTIL
CFLAGS.TERRFUNC = $(CFLAGS.I)plugins/mesh/terrfunc/object

MSVC.DSP += TERRFUNC
DSP.TERRFUNC.NAME = terrfunc
DSP.TERRFUNC.TYPE = plugin
DSP.TERRFUNC.CFLAGS = /I "..\..\plugins\mesh\terrfunc\object"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: terrfunc terrfuncclean
terrfunc: $(OUTDIRS) $(TERRFUNC)

clean: terrfuncclean
terrfuncclean:
	-$(RM) $(TERRFUNC) $(OBJ.TERRFUNC)

$(OUT)%$O: plugins/mesh/terrfunc/object/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.TERRFUNC)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)%$O: %.cpp
	$(DO.COMPILE.CPP)

$(TERRFUNC): $(OBJ.TERRFUNC) $(LIB.TERRFUNC)
	$(DO.PLUGIN)

ifdef DO_DEPEND
dep: $(OUTOS)terrfunc.dep
$(OUTOS)terrfunc.dep: $(SRC.TERRFUNC)
	$(DO.DEP1) $(CFLAGS.TERRFUNC) $(DO.DEP2)
else
-include $(OUTOS)terrfunc.dep
endif

endif # ifeq ($(MAKESECTION),targets)
