# Application description
DESCRIPTION.perftest = Crystal Space graphics performance tester

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make perftest     Make the $(DESCRIPTION.perftest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------ roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: perftest perftestclean

all apps: perftest
perftest:
	$(MAKE_APP)
perftestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------ postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/perftest apps/support

PERFTEST.EXE = perftest$(EXE)
INC.PERFTEST = $(wildcard apps/perftest/*.h)
SRC.PERFTEST = $(wildcard apps/perftest/*.cpp)
OBJ.PERFTEST = $(addprefix $(OUT)/,$(notdir $(SRC.PERFTEST:.cpp=$O)))
DEP.PERFTEST = CSUTIL CSTOOL CSSYS CSGEOM CSUTIL CSGFX
LIB.PERFTEST = $(foreach d,$(DEP.PERFTEST),$($d.LIB))

TO_INSTALL.EXE += $(PERFTEST.EXE)

MSVC.DSP += PERFTEST
DSP.PERFTEST.NAME = perftest
DSP.PERFTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#---------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.perftest perftestclean

all: $(PERFTEST.EXE)
build.perftest: $(OUTDIRS) $(PERFTEST.EXE)
clean: perftestclean

$(PERFTEST.EXE): $(DEP.EXE) $(OBJ.PERFTEST) $(LIB.PERFTEST)
	$(DO.LINK.EXE)

perftestclean:
	-$(RM) $(PERFTEST.EXE) $(OBJ.PERFTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/perftest.dep
$(OUTOS)/perftest.dep: $(SRC.PERFTEST)
	$(DO.DEP)
else
-include $(OUTOS)/perftest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
