# Application description
DESCRIPTION.mdltest = Model Importing Test Application

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make mdltest     Make the $(DESCRIPTION.mdltest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mdltest mdltestclean

all apps: mdltest
mdltest:
	$(MAKE_TARGET)
mdltestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/mdltest

MDLTEST.EXE = mdltest$(EXE)
INC.MDLTEST = $(wildcard apps/mdltest/*.h)
SRC.MDLTEST = $(wildcard apps/mdltest/*.cpp)
OBJ.MDLTEST = $(addprefix $(OUT),$(notdir $(SRC.MDLTEST:.cpp=$O)))
DEP.MDLTEST = CSPARSER CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.MDLTEST = $(foreach d,$(DEP.MDLTEST),$($d.LIB))

#TO_INSTALL.EXE += $(MDLTEST.EXE)

MSVC.DSP += MDLTEST
DSP.MDLTEST.NAME = mdltest
DSP.MDLTEST.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mdltest mdltestclean

all: $(MDLTEST.EXE)
mdltest: $(OUTDIRS) $(MDLTEST.EXE)
clean: mdltestclean

$(MDLTEST.EXE): $(DEP.EXE) $(OBJ.MDLTEST) $(LIB.MDLTEST)
	$(DO.LINK.EXE)

mdltest1clean:
        -$(RM) $(MDLTEST.EXE) $(OBJ.MDLTEST)

ifdef DO_DEPEND
dep: $(OUTOS)mdltest.dep
$(OUTOS)mdltest.dep: $(SRC.MDLTEST)
	$(DO.DEP)
else
-include $(OUTOS)mdltest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
