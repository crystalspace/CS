DESCRIPTION.terrbig = Large terrain plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make terrbig     Make the $(DESCRIPTION.terrbig)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: terrbig terrbigclean
plugins meshes all: terrbig

terrbigclean:
	$(MAKE_CLEAN)
terrbig:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/terrbig

ifeq ($(USE_PLUGINS),yes)
  TERRBIG = $(OUTDLL)terrbig$(DLL)
  LIB.TERRBIG = $(foreach d,$(DEP.TERRBIG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(TERRBIG)
else
  TERRBIG = $(OUT)$(LIB_PREFIX)terrbig$(LIB)
  DEP.EXE += $(TERRBIG)
  SCF.STATIC += terrbig
  TO_INSTALL.STATIC_LIBS += $(TERRBIG)
endif

INC.TERRBIG = $(wildcard plugins/mesh/terrbig/*.h)
SRC.TERRBIG = $(wildcard plugins/mesh/terrbig/*.cpp)
OBJ.TERRBIG = $(addprefix $(OUT),$(notdir $(SRC.TERRBIG:.cpp=$O)))
DEP.TERRBIG = CSGEOM CSUTIL CSSYS CSUTIL
CFLAGS.TERRBIG = $(CFLAGS.I)plugins/mesh/terrbig

MSVC.DSP += TERRBIG
DSP.TERRBIG.NAME = terrbig
DSP.TERRBIG.TYPE = plugin
DSP.TERRBIG.CFLAGS = /I "..\..\plugins\mesh\terrbig"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: terrbig terrbigclean
terrbig: $(OUTDIRS) $(TERRBIG)

clean: terrbigclean
terrbigclean:
	-$(RM) $(TERRBIG) $(OBJ.TERRBIG)

$(OUT)%$O: plugins/mesh/terrbig/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.TERRBIG)

# Some (broken) versions of GNU make appear to be sensitive to the order in
# which implicit rules are seen.  Without the following rule (which is just
# a reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)%$O: %.cpp
	$(DO.COMPILE.CPP)

$(TERRBIG): $(OBJ.TERRBIG) $(LIB.TERRBIG)
	$(DO.PLUGIN)

ifdef DO_DEPEND
dep: $(OUTOS)terrbig.dep
$(OUTOS)terrbig.dep: $(SRC.TERRBIG)
	$(DO.DEP1) $(CFLAGS.TERRBIG) $(DO.DEP2)
else
-include $(OUTOS)terrbig.dep
endif

endif # ifeq ($(MAKESECTION),targets)
