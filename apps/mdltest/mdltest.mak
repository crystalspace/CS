# Application description
DESCRIPTION.mdltest = Model Importing Test Application

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make mdltest      Make the $(DESCRIPTION.mdltest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mdltest mdltestclean

all apps: mdltest
mdltest:
	$(MAKE_APP)
mdltestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

MDLTEST.EXE = mdltest$(EXE)
DIR.MDLTEST = apps/mdltest
OUT.MDLTEST = $(OUT)/$(DIR.MDLTEST)
INC.MDLTEST = $(wildcard $(DIR.MDLTEST)/*.h)
SRC.MDLTEST = $(wildcard $(DIR.MDLTEST)/*.cpp)
OBJ.MDLTEST = $(addprefix $(OUT.MDLTEST)/,$(notdir $(SRC.MDLTEST:.cpp=$O)))
DEP.MDLTEST = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL
LIB.MDLTEST = $(foreach d,$(DEP.MDLTEST),$($d.LIB))

#TO_INSTALL.EXE += $(MDLTEST.EXE)

MSVC.DSP += MDLTEST
DSP.MDLTEST.NAME = mdltest
DSP.MDLTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.mdltest mdltestclean mdltestcleandep

all: $(MDLTEST.EXE)
build.mdltest: $(OUT.MDLTEST) $(MDLTEST.EXE)
clean: mdltestclean

$(OUT.MDLTEST)/%$O: $(DIR.MDLTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(MDLTEST.EXE): $(DEP.EXE) $(OBJ.MDLTEST) $(LIB.MDLTEST)
	$(DO.LINK.EXE)

$(OUT.MDLTEST):
	$(MKDIRS)

mdltestclean:
	-$(RMDIR) $(MDLTEST.EXE) $(OBJ.MDLTEST)

cleandep: mdltestcleandep
mdltestcleandep:
	-$(RM) $(OUT.MDLTEST)/mdltest.dep

ifdef DO_DEPEND
dep: $(OUT.MDLTEST) $(OUT.MDLTEST)/mdltest.dep
$(OUT.MDLTEST)/mdltest.dep: $(SRC.MDLTEST)
	$(DO.DEPEND)
else
-include $(OUT.MDLTEST)/mdltest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
