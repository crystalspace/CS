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

vpath %.cpp apps/demo

CSDEMO.EXE=csdemo$(EXE)
INC.CSDEMO = $(wildcard apps/demo/*.h)
SRC.CSDEMO = $(wildcard apps/demo/*.cpp)
OBJ.CSDEMO = $(addprefix $(OUT)/,$(notdir $(SRC.CSDEMO:.cpp=$O)))
DEP.CSDEMO = CSGFX CSUTIL CSTOOL CSSYS CSGEOM CSUTIL CSSYS
LIB.CSDEMO = $(foreach d,$(DEP.CSDEMO),$($d.LIB))
CFG.CSDEMO = data/config/csdemo.cfg

#TO_INSTALL.EXE += $(DEMO.EXE)

MSVC.DSP += CSDEMO
DSP.CSDEMO.NAME = csdemo
DSP.CSDEMO.TYPE = appcon

$(CSDEMO.EXE).WINRSRC = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.csdemo csdemoclean

all: $(CSDEMO.EXE)
build.csdemo: $(OUTDIRS) $(CSDEMO.EXE)
clean: csdemoclean

$(CSDEMO.EXE): $(DEP.EXE) $(OBJ.CSDEMO) $(LIB.CSDEMO)
	$(DO.LINK.EXE)

csdemoclean:
	-$(RMDIR) $(CSDEMO.EXE) $(OBJ.CSDEMO)

ifdef DO_DEPEND
dep: $(OUTOS)/csdemo.dep
$(OUTOS)/csdemo.dep: $(SRC.CSDEMO)
	$(DO.DEP)
else
-include $(OUTOS)/csdemo.dep
endif

endif # ifeq ($(MAKESECTION),targets)
