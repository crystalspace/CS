# Application description
DESCRIPTION.eventtest = Crystal Space eventtest tester

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make eventtest     Make the $(DESCRIPTION.eventtest)$"

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

vpath %.cpp apps/tests/eventtest

EVENTTEST.EXE = eventtest$(EXE.CONSOLE)
INC.EVENTTEST = $(wildcard apps/tests/eventtest/*.h)
SRC.EVENTTEST = $(wildcard apps/tests/eventtest/*.cpp)
OBJ.EVENTTEST = $(addprefix $(OUT)/,$(notdir $(SRC.EVENTTEST:.cpp=$O)))
DEP.EVENTTEST = CSTOOL CSUTIL CSSYS
LIB.EVENTTEST = $(foreach d,$(DEP.EVENTTEST),$($d.LIB))

#TO_INSTALL.EXE += $(EVENTTEST.EXE)

MSVC.DSP += EVENTTEST
DSP.EVENTTEST.NAME = eventtest
DSP.EVENTTEST.TYPE = appcon

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.eventtest eventtestclean

all: $(EVENTTEST.EXE)
build.eventtest: $(OUTDIRS) $(EVENTTEST.EXE)
clean: eventtestclean
check: eventtestcheck

$(EVENTTEST.EXE): $(DEP.EXE) $(OBJ.EVENTTEST) $(LIB.EVENTTEST)
	$(DO.LINK.CONSOLE.EXE)

eventtestclean:
	-$(RMDIR) $(EVENTTEST.EXE) $(OBJ.EVENTTEST)

eventtestcheck: $(EVENTTEST.EXE)
	$(RUN_TEST)$(EVENTTEST.EXE)

ifdef DO_DEPEND
dep: $(OUTOS)/eventtest.dep
$(OUTOS)/eventtest.dep: $(SRC.EVENTTEST)
	$(DO.DEP)
else
-include $(OUTOS)/eventtest.dep
endif

endif # ifeq ($(MAKESECTION),targets)
