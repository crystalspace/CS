# Application description
DESCRIPTION.simplevp = effects and vertex programs demonstration

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simplevp     Make the $(DESCRIPTION.simplevp)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simplevp simplevpclean

all apps: simplevp
simplevp:
	$(MAKE_APP)
simplevpclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SIMPLEVP.EXE = simplevp$(EXE)
DIR.SIMPLEVP = apps/examples/simplevp
OUT.SIMPLEVP = $(OUT)/$(DIR.SIMPLEVP)
INC.SIMPLEVP = $(wildcard $(DIR.SIMPLEVP)/*.h)
SRC.SIMPLEVP = $(wildcard $(DIR.SIMPLEVP)/*.cpp)
OBJ.SIMPLEVP = $(addprefix $(OUT.SIMPLEVP)/,$(notdir $(SRC.SIMPLEVP:.cpp=$O)))
DEP.SIMPLEVP = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.SIMPLEVP = $(foreach d,$(DEP.SIMPLEVP),$($d.LIB))

#TO_INSTALL.EXE += $(SIMPLEVP.EXE)

MSVC.DSP += SIMPLEVP
DSP.SIMPLEVP.NAME = simplevp
DSP.SIMPLEVP.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simplevp simplevpclean simplevpcleandep

all: $(SIMPLEVP.EXE)
build.simplevp: $(OUT.SIMPLEVP) $(SIMPLEVP.EXE)
clean: simplevpclean

$(OUT.SIMPLEVP)/%$O: $(DIR.SIMPLEVP)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPLEVP.EXE): $(DEP.EXE) $(OBJ.SIMPLEVP) $(LIB.SIMPLEVP)
	$(DO.LINK.EXE)

$(OUT.SIMPLEVP):
	$(MKDIRS)

simplevpclean:
	-$(RMDIR) $(SIMPLEVP.EXE) $(OBJ.SIMPLEVP)

cleandep: simplevpcleandep
simplevpcleandep:
	-$(RM) $(OUT.SIMPLEVP)/simplevp.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPLEVP) $(OUT.SIMPLEVP)/simplevp.dep
$(OUT.SIMPLEVP)/simplevp.dep: $(SRC.SIMPLEVP)
	$(DO.DEPEND)
else
-include $(OUT.SIMPLEVP)/simplevp.dep
endif

endif # ifeq ($(MAKESECTION),targets)
