# Application description
DESCRIPTION.csdemo = Crystal Space Demo

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make csdemo       Make the $(DESCRIPTION.csdemo)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csdemo csdemoclean

all apps: csdemo
csdemo:
	$(MAKE_APP)
csdemoclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSDEMO.EXE=csdemo$(EXE)
DIR.CSDEMO = apps/demo
OUT.CSDEMO = $(OUT)/$(DIR.CSDEMO)
INC.CSDEMO = $(wildcard $(DIR.CSDEMO)/*.h)
SRC.CSDEMO = $(wildcard $(DIR.CSDEMO)/*.cpp)
OBJ.CSDEMO = $(addprefix $(OUT.CSDEMO)/,$(notdir $(SRC.CSDEMO:.cpp=$O)))
DEP.CSDEMO = CSGFX CSUTIL CSTOOL CSSYS CSGEOM CSUTIL CSSYS
LIB.CSDEMO = $(foreach d,$(DEP.CSDEMO),$($d.LIB))
CFG.CSDEMO = data/config/csdemo.cfg

TO_INSTALL.EXE += $(DEMO.EXE)

MSVC.DSP += CSDEMO
DSP.CSDEMO.NAME = csdemo
DSP.CSDEMO.TYPE = appcon

$(CSDEMO.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.csdemo csdemoclean csdemocleandep

all: $(CSDEMO.EXE)
build.csdemo: $(OUT.CSDEMO) $(CSDEMO.EXE)
clean: csdemoclean

$(OUT.CSDEMO)/%$O: $(DIR.CSDEMO)/%.cpp
	$(DO.COMPILE.CPP)

$(CSDEMO.EXE): $(DEP.EXE) $(OBJ.CSDEMO) $(LIB.CSDEMO)
	$(DO.LINK.EXE)

$(OUT.CSDEMO):
	$(MKDIRS)

csdemoclean:
	-$(RMDIR) $(CSDEMO.EXE) $(OBJ.CSDEMO)

cleandep: csdemocleandep
csdemocleandep:
	-$(RM) $(OUT.CSDEMO)/csdemo.dep

ifdef DO_DEPEND
dep: $(OUT.CSDEMO) $(OUT.CSDEMO)/csdemo.dep
$(OUT.CSDEMO)/csdemo.dep: $(SRC.CSDEMO)
	$(DO.DEPEND)
else
-include $(OUT.CSDEMO)/csdemo.dep
endif

endif # ifeq ($(MAKESECTION),targets)
