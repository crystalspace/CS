# Application description
DESCRIPTION.walktest = Crystal Space WalkTest demo

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make walktest     Make the $(DESCRIPTION.walktest)$"

PSEUDOHELP += $(NEWLINE) \
  echo $"  make walkall      Make WalkTest and plugins it requires$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: walktest walktestclean walkall

walkall: walktest vfs soft3d softcanvas csfont csconin simpcon perfstat \
  rapid meshes cssynldr imgplex csgifimg csjpgimg cspngimg csbmpimg reporter \
  stdrep csparser frustvis bugplug sequence engseq
all apps: walktest
walktest:
	$(MAKE_APP)
walktestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

WALKTEST.EXE = walktest$(EXE)
DIR.WALKTEST = apps/walktest
OUT.WALKTEST = $(OUT)/$(DIR.WALKTEST)
INC.WALKTEST = $(wildcard $(DIR.WALKTEST)/*.h)
SRC.WALKTEST = $(wildcard $(DIR.WALKTEST)/*.cpp)
OBJ.WALKTEST = $(addprefix $(OUT.WALKTEST)/,$(notdir $(SRC.WALKTEST:.cpp=$O)))
DEP.WALKTEST = CSTOOL CSENGINE CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.WALKTEST = $(foreach d,$(DEP.WALKTEST),$($d.LIB))
CFG.WALKTEST = data/config/walktest.cfg data/config/autoexec.cfg

TO_INSTALL.EXE    += $(WALKTEST.EXE)
TO_INSTALL.CONFIG += $(CFG.WALKTEST)
TO_INSTALL.DATA   += \
  data/stdtex.zip data/standard.zip data/flarge/world data/partsys/world

MSVC.DSP += WALKTEST
DSP.WALKTEST.NAME = walktest
DSP.WALKTEST.TYPE = appcon

$(WALKTEST.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.walktest walktestclean walktestcleandep

all: $(WALKTEST.EXE)
build.walktest: $(OUT.WALKTEST) $(WALKTEST.EXE)
clean: walktestclean

$(OUT.WALKTEST)/%$O: $(DIR.WALKTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(WALKTEST.EXE): $(DEP.EXE) $(OBJ.WALKTEST) $(LIB.WALKTEST)
	$(DO.LINK.EXE)

$(OUT.WALKTEST):
	$(MKDIRS)

walktestclean:
	-$(RMDIR) $(WALKTEST.EXE) $(OBJ.WALKTEST)

cleandep: walktestcleandep
walktestcleandep:
	-$(RM) $(OUT.WALKTEST)/walktest.dep

ifdef DO_DEPEND
dep: $(OUT.WALKTEST) $(OUT.WALKTEST)/walktest.dep
$(OUT.WALKTEST)/walktest.dep: $(SRC.WALKTEST)
	$(DO.DEPEND)
else
-include $(OUT.WALKTEST)/walktest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
