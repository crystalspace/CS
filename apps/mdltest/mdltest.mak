# Application description
DESCRIPTION.mdltst = Model Importing Test Application

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make mdltst       Make the $(DESCRIPTION.mdltst)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mdltst mdltstclean

all apps: mdltst
mdltst:
	$(MAKE_TARGET)
mdltstclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/mdltest

MDLTEST.EXE = mdltest$(EXE)
INC.MDLTEST = $(wildcard apps/mdltest/*.h)
SRC.MDLTEST = $(wildcard apps/mdltest/*.cpp)
OBJ.MDLTEST = $(addprefix $(OUT),$(notdir $(SRC.MDLTEST:.cpp=$O)))
DEP.MDLTEST = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.MDLTEST = $(foreach d,$(DEP.MDLTEST),$($d.LIB))

#TO_INSTALL.EXE += $(MDLTEST.EXE)

MSVC.DSP += MDLTEST
DSP.MDLTEST.NAME = mdltest
DSP.MDLTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mdltst mdltstclean

all: $(MDLTEST.EXE)
mdltst: $(OUTDIRS) $(MDLTEST.EXE)
clean: mdltstclean

$(MDLTEST.EXE): $(DEP.EXE) $(OBJ.MDLTEST) $(LIB.MDLTEST)
	$(DO.LINK.EXE)

mdltstclean:
	-$(RM) $(MDLTEST.EXE) $(OBJ.MDLTEST)

ifdef DO_DEPEND
dep: $(OUTOS)mdltest.dep
$(OUTOS)mdltest.dep: $(SRC.MDLTEST)
	$(DO.DEP)
else
-include $(OUTOS)mdltest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
