# Application description
DESCRIPTION.isotest = Crystal Space isotest demo executable

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make isotest      Make the $(DESCRIPTION.isotest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: isotest isotestclean

all apps: isotest
isotest:
	$(MAKE_APP)
isotestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/isotest

ISOTEST.EXE = isotest$(EXE)
INC.ISOTEST = $(wildcard apps/isotest/*.h)
SRC.ISOTEST = $(wildcard apps/isotest/*.cpp)
OBJ.ISOTEST = $(addprefix $(OUT)/,$(notdir $(SRC.ISOTEST:.cpp=$O)))
DEP.ISOTEST = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL
LIB.ISOTEST = $(foreach d,$(DEP.ISOTEST),$($d.LIB))
#CFG.ISOTEST = data/config/isotest.cfg

TO_INSTALL.EXE    += $(ISOTEST.EXE)
#TO_INSTALL.CONFIG += $(CFG.ISOTEST)

MSVC.DSP += ISOTEST
DSP.ISOTEST.NAME = isotest
DSP.ISOTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.isotest isotestclean

all: $(ISOTEST.EXE)
build.isotest: $(OUTDIRS) $(ISOTEST.EXE)
clean: isotestclean

$(ISOTEST.EXE): $(DEP.EXE) $(OBJ.ISOTEST) $(LIB.ISOTEST)
	$(DO.LINK.EXE)

isotestclean:
	-$(RM) $(ISOTEST.EXE) $(OBJ.ISOTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/isotest.dep
$(OUTOS)/isotest.dep: $(SRC.ISOTEST)
	$(DO.DEP)
else
-include $(OUTOS)/isotest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
