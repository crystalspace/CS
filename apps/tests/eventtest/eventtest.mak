# Application description
DESCRIPTION.eventtest = Crystal Space event tester

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make eventtest    Make the $(DESCRIPTION.eventtest)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: eventtest eventtestclean

all apps: eventtest
check: eventtestcheck
eventtest:
	$(MAKE_APP)
eventtestclean:
	$(MAKE_CLEAN)
eventtestcheck:
	$(MAKE_CHECK)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

EVENTTEST.EXE = eventtest$(EXE.CONSOLE)
DIR.EVENTTEST = apps/tests/eventtest
OUT.EVENTTEST = $(OUT)/$(DIR.EVENTTEST)
INC.EVENTTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.EVENTTEST)/*.h))
SRC.EVENTTEST = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.EVENTTEST)/*.cpp))
OBJ.EVENTTEST = \
  $(addprefix $(OUT.EVENTTEST)/,$(notdir $(SRC.EVENTTEST:.cpp=$O)))
DEP.EVENTTEST = CSTOOL CSUTIL CSUTIL
LIB.EVENTTEST = $(foreach d,$(DEP.EVENTTEST),$($d.LIB))

OUTDIRS += $(OUT.EVENTTEST)

#TO_INSTALL.EXE += $(EVENTTEST.EXE)

MSVC.DSP += EVENTTEST
DSP.EVENTTEST.NAME = eventtest
DSP.EVENTTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.eventtest eventtestclean eventtestcleandep

build.eventtest: $(OUTDIRS) $(EVENTTEST.EXE)
clean: eventtestclean
check: eventtestcheck

$(OUT.EVENTTEST)/%$O: $(SRCDIR)/$(DIR.EVENTTEST)/%.cpp
	$(DO.COMPILE.CPP)

$(EVENTTEST.EXE): $(DEP.EXE) $(OBJ.EVENTTEST) $(LIB.EVENTTEST)
	$(DO.LINK.CONSOLE.EXE)

eventtestclean:
	-$(RMDIR) $(EVENTTEST.EXE) $(OBJ.EVENTTEST)

eventtestcheck: $(EVENTTEST.EXE)
	$(RUN_TEST)$(EVENTTEST.EXE)

cleandep: eventtestcleandep
eventtestcleandep:
	-$(RM) $(OUT.EVENTTEST)/eventtest.dep

ifdef DO_DEPEND
dep: $(OUT.EVENTTEST) $(OUT.EVENTTEST)/eventtest.dep
$(OUT.EVENTTEST)/eventtest.dep: $(SRC.EVENTTEST)
	$(DO.DEPEND)
else
-include $(OUT.EVENTTEST)/eventtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
