DESCRIPTION.pgtest = Crystal Space PicoGUI Test

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application
APPHELP += \
  $(NEWLINE)@echo $"  make pgtest       Make the $(DESCRIPTION.pgtest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pgtest pgtestclean

all apps: pgtest

pgtest:
	$(MAKE_APP)

pgtestclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/apps/tests/picogui

PGTEST.EXE = pgtest$(EXE)
LIB.PGTEST = $(foreach d,$(DEP.PGTEST),$($d.LIB))

INC.PGTEST = $(wildcard $(addprefix $(SRCDIR)/,apps/tests/picogui/*.h))
SRC.PGTEST = $(wildcard $(addprefix $(SRCDIR)/,apps/tests/picogui/*.cpp))
OBJ.PGTEST = $(addprefix $(OUT)/,$(notdir $(SRC.PGTEST:.cpp=$O)))
DEP.PGTEST = CSTOOL CSGFX CSUTIL CSSYS CSUTIL

MSVC.DSP += PGTEST
DSP.PGTEST.NAME = pgtest
DSP.PGTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: pgtest pgtestclean

all: $(PGTEST.EXE)
build.pgtest: $(OUTDIRS) $(PGTEST.EXE)
$(PGTEST.EXE): $(DEP.EXE) $(OBJ.PGTEST) $(LIB.PGTEST)
	$(DO.LINK.EXE)

clean: pgtestclean
pgtestclean:
	-$(RMDIR) $(PGTEST.EXE) $(OBJ.PGTEST)

ifdef DO_DEPEND
dep: $(OUTOS)/pgtest.dep
$(OUTOS)/pgtest.dep: $(SRC.PGTEST)
	$(DO.DEP)
else
-include $(OUTOS)/pgtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
