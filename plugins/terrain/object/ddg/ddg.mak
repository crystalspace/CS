DESCRIPTION.ddg = DDG Terrain plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ddg          Make the $(DESCRIPTION.ddg)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddg ddgclean
plugins all: ddg

ddgclean:
	$(MAKE_CLEAN)
ddg:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/terrain/object/ddg plugins/terrain/object/ddg/math plugins/terrain/object/ddg/struct plugins/terrain/object/ddg/util

ifeq ($(USE_PLUGINS),yes)
  DDG = $(OUTDLL)ddg$(DLL)
  LIB.DDG = $(foreach d,$(DEP.DDG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(DDG)
else
  DDG = $(OUT)$(LIB_PREFIX)ddg$(LIB)
  DEP.EXE += $(DDG)
  SCF.STATIC += ddg
  TO_INSTALL.STATIC_LIBS += $(DDG)
endif

INC.DDG = $(wildcard plugins/terrain/object/ddg/*.h plugins/terrain/object/ddg/*/*.h)
SRC.DDG = $(wildcard plugins/terrain/object/ddg/*.cpp plugins/terrain/object/ddg/*/*.cpp)
OBJ.DDG = $(addprefix $(OUT),$(notdir $(SRC.DDG:.cpp=$O)))
CFLAGS.DDG = $(CFLAGS.D)__CRYSTAL_SPACE__ $(CFLAGS.I)plugins/terrain/object/ddg
# @@@@@ The following line is temporary until this makefile is fixed
CFLAGS += $(CFLAGS.D)__CRYSTAL_SPACE__ $(CFLAGS.I)plugins/terrain/object/ddg

DEP.DDG = CSGEOM CSUTIL

MSVC.DSP += DDG
DSP.DDG.NAME = ddg
DSP.DDG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ddg ddgclean
ddg: $(OUTDIRS) $(DDG)

clean: ddgclean
ddgclean:
	-$(RM) $(DDG) $(OBJ.DDG)

$(OUT)%$O: plugins/terrain/object/ddg/math/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.DDG)

$(OUT)%$O: plugins/terrain/object/ddg/struct/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.DDG)

$(OUT)%$O: plugins/terrain/object/ddg/util/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.DDG)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)%$O: %.cpp
	$(DO.COMPILE.CPP)

$(DDG): $(OBJ.DDG) $(LIB.DDG)
	$(DO.PLUGIN)

ifdef DO_DEPEND
dep: $(OUTOS)ddg.dep
$(OUTOS)ddg.dep: $(SRC.DDG)
	$(DO.DEP)
else
-include $(OUTOS)ddg.dep
endif

endif # ifeq ($(MAKESECTION),targets)
