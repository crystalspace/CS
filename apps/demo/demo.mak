# Application description
DESCRIPTION.demo = Crystal Space Demo

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make demo         Make the $(DESCRIPTION.demo)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: demo democlean

all apps: demo
demo:
	$(MAKE_TARGET)
democlean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/demo

DEMO.EXE=csdemo$(EXE)
INC.DEMO = $(wildcard apps/demo/*.h)
SRC.DEMO = $(wildcard apps/demo/*.cpp)
OBJ.DEMO = $(addprefix $(OUT),$(notdir $(SRC.DEMO:.cpp=$O)))
DEP.DEMO = CSGFX CSUTIL CSTOOL CSSYS CSGEOM CSUTIL CSSYS 
LIB.DEMO = $(foreach d,$(DEP.DEMO),$($d.LIB))
CFG.DEMO = data/config/csdemo.cfg

#TO_INSTALL.EXE += $(DEMO.EXE)

MSVC.DSP += DEMO
DSP.DEMO.NAME = csdemo
DSP.DEMO.TYPE = appgui
DSP.DEMO.RESOURCES = apps\win32\cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: demo democlean

all: $(DEMO.EXE)
demo: $(OUTDIRS) $(DEMO.EXE)
clean: democlean

$(DEMO.EXE): $(DEP.EXE) $(OBJ.DEMO) $(LIB.DEMO)
	$(DO.LINK.EXE)

democlean:
	-$(RM) $(DEMO.EXE) $(OBJ.DEMO)

ifdef DO_DEPEND
dep: $(OUTOS)csdemo.dep
$(OUTOS)csdemo.dep: $(SRC.DEMO)
	$(DO.DEP)
else
-include $(OUTOS)csdemo.dep
endif

endif # ifeq ($(MAKESECTION),targets)
