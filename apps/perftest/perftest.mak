# Application description
DESCRIPTION.perf = Crystal Space graphics performance tester

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make perf         Make the $(DESCRIPTION.perf)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------ roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: perf perfclean

all apps: perf
perf:
	$(MAKE_TARGET)
perfclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------ postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/perftest apps/support

PERF.EXE = perftest$(EXE)
INC.PERF = $(wildcard apps/perftest/*.h)
SRC.PERF = $(wildcard apps/perftest/*.cpp)
OBJ.PERF = $(addprefix $(OUT),$(notdir $(SRC.PERF:.cpp=$O)))
DEP.PERF = CSUTIL CSSYS CSGEOM CSGFXLDR CSUTIL
LIB.PERF = $(foreach d,$(DEP.PERF),$($d.LIB))

TO_INSTALL.EXE += $(PERF.EXE)

MSVC.DSP += PERF
DSP.PERF.NAME = perftest
DSP.PERF.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#---------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: perf perfclean

all: $(PERF.EXE)
perf: $(OUTDIRS) $(PERF.EXE)
clean: perfclean

$(PERF.EXE): $(DEP.EXE) $(OBJ.PERF) $(LIB.PERF)
	$(DO.LINK.EXE)

perfclean:
	-$(RM) $(PERF.EXE) $(OBJ.PERF)

ifdef DO_DEPEND
dep: $(OUTOS)perf.dep
$(OUTOS)perf.dep: $(SRC.PERF)
	$(DO.DEP)
else
-include $(OUTOS)perf.dep
endif

endif # ifeq ($(MAKESECTION),targets)
