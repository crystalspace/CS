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

PERFTEST.EXE = perftest$(EXE)
DIR.PERFTEST = apps/perftest
OUT.PERFTEST = $(OUT)/$(DIR.PERFTEST)
INC.PERFTEST = $(wildcard $(DIR.PERFTEST)/*.h)
SRC.PERFTEST = $(wildcard $(DIR.PERFTEST)/*.cpp)
OBJ.PERFTEST = $(addprefix $(OUT.PERFTEST)/,$(notdir $(SRC.PERFTEST:.cpp=$O)))
DEP.PERFTEST = CSUTIL CSTOOL CSSYS CSGEOM CSUTIL CSGFX
LIB.PERFTEST = $(foreach d,$(DEP.PERFTEST),$($d.LIB))

TO_INSTALL.EXE += $(PERFTEST.EXE)

MSVC.DSP += PERFTEST
DSP.PERFTEST.NAME = perftest
DSP.PERFTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#---------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.perftest perftestclean perftestcleandep

all: $(PERFTEST.EXE)
build.perftest: $(OUT.PERFTEST) $(PERFTEST.EXE)
clean: perftestclean

$(OUT.PERFTEST)/%$O: $(DIR.PERFTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(PERFTEST.EXE): $(DEP.EXE) $(OBJ.PERFTEST) $(LIB.PERFTEST)
	$(DO.LINK.EXE)

$(OUT.PERFTEST):
	$(MKDIRS)

perftestclean:
	-$(RMDIR) $(PERFTEST.EXE) $(OBJ.PERFTEST)

cleandep: perftestcleandep
perftestcleandep:
	-$(RM) $(OUT.PERFTEST)/perftest.dep

ifdef DO_DEPEND
dep: $(OUT.PERFTEST) $(OUT.PERFTEST)/perftest.dep
$(OUT.PERFTEST)/perftest.dep: $(SRC.PERFTEST)
	$(DO.DEPEND)
else
-include $(OUT.PERFTEST)/perftest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
