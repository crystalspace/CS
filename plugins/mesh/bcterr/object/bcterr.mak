DESCRIPTION.bcterr = Bezier Terrain plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make bcterr       Make the $(DESCRIPTION.bcterr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bcterr bcterrclean
plugins meshes all: bcterr

bcterrclean:
	$(MAKE_CLEAN)
bcterr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/bcterr/object

ifeq ($(USE_PLUGINS),yes)
  BCTERR = $(OUTDLL)/bcterr$(DLL)
  LIB.BCTERR = $(foreach d,$(DEP.BCTERR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BCTERR)
else
  BCTERR = $(OUT)/$(LIB_PREFIX)bcterr$(LIB)
  DEP.EXE += $(BCTERR)
  SCF.STATIC += bcterr
  TO_INSTALL.STATIC_LIBS += $(BCTERR)
endif

INC.BCTERR = $(wildcard plugins/mesh/bcterr/object/*.h)
SRC.BCTERR = $(wildcard plugins/mesh/bcterr/object/*.cpp)
OBJ.BCTERR = $(addprefix $(OUT)/,$(notdir $(SRC.BCTERR:.cpp=$O)))
DEP.BCTERR = CSGEOM CSUTIL CSSYS CSUTIL
CFLAGS.BCTERR = $(CFLAGS.I)plugins/mesh/bcterr/object

MSVC.DSP += BCTERR
DSP.BCTERR.NAME = bcterr
DSP.BCTERR.TYPE = plugin
DSP.BCTERR.CFLAGS = /I "..\..\plugins\mesh\bcterr\object"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bcterr bcterrclean
bcterr: $(OUTDIRS) $(BCTERR)

clean: bcterrclean
bcterrclean:
	-$(RM) $(BCTERR) $(OBJ.BCTERR)

$(OUT)/%$O: plugins/mesh/bcterr/object/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.BCTERR)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)/%$O: %.cpp
	$(DO.COMPILE.CPP)

$(BCTERR): $(OBJ.BCTERR) $(LIB.BCTERR)
	$(DO.PLUGIN)

ifdef DO_DEPEND
dep: $(OUTOS)/bcterr.dep
$(OUTOS)/bcterr.dep: $(SRC.BCTERR)
	$(DO.DEP1) $(CFLAGS.BCTERR) $(DO.DEP2)
else
-include $(OUTOS)/bcterr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
