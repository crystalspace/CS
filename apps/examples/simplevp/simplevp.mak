# Application description
DESCRIPTION.simplevp = Effects and vertex programs demonstration

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

vpath %.cpp apps/examples/simplevp

simplevp.EXE = simplevp$(EXE)
INC.simplevp = $(wildcard apps/examples/simplevp/*.h)
SRC.simplevp = $(wildcard apps/examples/simplevp/*.cpp)
OBJ.simplevp = $(addprefix $(OUT)/,$(notdir $(SRC.simplevp:.cpp=$O)))
DEP.simplevp = CSTOOL CSGFX CSUTIL CSSYS CSGEOM CSUTIL CSSYS
LIB.simplevp = $(foreach d,$(DEP.simplevp),$($d.LIB))

#TO_INSTALL.EXE += $(simplevp.EXE)

MSVC.DSP += simplevp
DSP.simplevp.NAME = simplevp
DSP.simplevp.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simplevp simplevpclean

all: $(simplevp.EXE)
build.simplevp: $(OUTDIRS) $(simplevp.EXE)
clean: simplevpclean

$(simplevp.EXE): $(DEP.EXE) $(OBJ.simplevp) $(LIB.simplevp)
	$(DO.LINK.EXE)

simplevpclean:
	-$(RMDIR) $(simplevp.EXE) $(OBJ.simplevp)

ifdef DO_DEPEND
dep: $(OUTOS)/simplevp.dep
$(OUTOS)/simplevp.dep: $(SRC.simplevp)
	$(DO.DEP)
else
-include $(OUTOS)/simplevp.dep
endif

endif # ifeq ($(MAKESECTION),targets)
