# Application description
DESCRIPTION.demsky2 = Crystal Space sky demo 2

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make demsky2      Make the $(DESCRIPTION.demsky2)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: demsky2 demsky2clean

all apps: demsky2
demsky2:
	$(MAKE_TARGET)
demsky2clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/demosky2 apps/support

DEMOSKY2.EXE=demosky2$(EXE)
INC.DEMOSKY2 = $(wildcard apps/demosky2/*.h)
SRC.DEMOSKY2 = $(wildcard apps/demosky2/*.cpp)
OBJ.DEMOSKY2 = $(addprefix $(OUT),$(notdir $(SRC.DEMOSKY2:.cpp=$O)))
DEP.DEMOSKY2 = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.DEMOSKY2 = $(foreach d,$(DEP.DEMOSKY2),$($d.LIB))

#TO_INSTALL.EXE += $(DEMOSKY2.EXE)

MSVC.DSP += DEMOSKY2
DSP.DEMOSKY2.NAME = demosky2
DSP.DEMOSKY2.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: demsky2 demsky2clean

all: $(DEMOSKY2.EXE)
demsky2: $(OUTDIRS) $(DEMOSKY2.EXE)
clean: demsky2clean

$(DEMOSKY2.EXE): $(DEP.EXE) $(OBJ.DEMOSKY2) $(LIB.DEMOSKY2)
	$(DO.LINK.EXE)

demsky2clean:
	-$(RM) $(DEMOSKY2.EXE) $(OBJ.DEMOSKY2)

ifdef DO_DEPEND
dep: $(OUTOS)demosky2.dep
$(OUTOS)demosky2.dep: $(SRC.DEMOSKY2)
	$(DO.DEP)
else
-include $(OUTOS)demosky2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
