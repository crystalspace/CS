# Application description
DESCRIPTION.walktest = Crystal Space WalkTest demo executable

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make walktest     Make the $(DESCRIPTION.walktest)$"

PSEUDOHELP += $(NEWLINE) \
  echo $"  make walkall      Make WalkTest and all plug-ins it requires$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: walktest walkclean

walkall: walktest vfs soft3d softcanvas csfont csconin simpcon perfstat \
  rapid meshes cssynldr imgplex csgifimg csjpgimg cspngimg csbmpimg reporter \
  stdrep csparser frustvis csjngimg
all apps: walktest
walktest:
	$(MAKE_APP)
walkclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/walktest apps/support

WALKTEST.EXE = walktest$(EXE)
INC.WALKTEST = $(wildcard apps/walktest/*.h)
SRC.WALKTEST = $(wildcard apps/walktest/*.cpp)
OBJ.WALKTEST = $(addprefix $(OUT),$(notdir $(SRC.WALKTEST:.cpp=$O)))
DEP.WALKTEST = CSTOOL CSENGINE CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.WALKTEST = $(foreach d,$(DEP.WALKTEST),$($d.LIB))
CFG.WALKTEST = data/config/walktest.cfg data/config/autoexec.cfg

TO_INSTALL.EXE    += $(WALKTEST.EXE)
TO_INSTALL.CONFIG += $(CFG.WALKTEST)
TO_INSTALL.DATA   += data/stdtex.zip

MSVC.DSP += WALKTEST
DSP.WALKTEST.NAME = walktest
DSP.WALKTEST.TYPE = appcon

$(WALKTEST.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.walktest walkclean

all: $(WALKTEST.EXE)
build.walktest: $(OUTDIRS) $(WALKTEST.EXE)
clean: walkclean

$(WALKTEST.EXE): $(DEP.EXE) $(OBJ.WALKTEST) $(LIB.WALKTEST)
	$(DO.LINK.EXE)

walkclean:
	-$(RM) $(WALKTEST.EXE) $(OBJ.WALKTEST)

ifdef DO_DEPEND
dep: $(OUTOS)walktest.dep
$(OUTOS)walktest.dep: $(SRC.WALKTEST)
	$(DO.DEP)
else
-include $(OUTOS)walktest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
