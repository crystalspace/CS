# Application description
DESCRIPTION.perf = Crystal Space Graphics Performance Tester

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make perf         Make the $(DESCRIPTION.perf)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: perf perfclean

all apps: perf
perf:
	$(MAKE_TARGET)
perfclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/perftest apps/support

PERF.EXE=perftest$(EXE)
SRC.PERF = $(wildcard apps/perftest/*.cpp) apps/support/static.cpp
OBJ.PERF = $(addprefix $(OUT),$(notdir $(SRC.PERF:.cpp=$O)))
DESCRIPTION.$(PERF.EXE) = $(DESCRIPTION.perf)
TO_INSTALL.EXE+=$(PERF.EXE)
TO_INSTALL.DATA+=data/perf.zip

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: perf perfclean

all: $(PERF.EXE)
perf: $(OUTDIRS) $(PERF.EXE)
clean: perfclean

$(PERF.EXE): $(DEP.EXE) $(OBJ.PERF) \
  $(CSUTIL.LIB) $(CSSYS.LIB) $(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB)
	$(DO.LINK.EXE)

perfclean:
	-$(RM) $(PERF.EXE) $(OBJ.PERF) $(OUTOS)perf.dep

ifdef DO_DEPEND
dep: $(OUTOS)perf.dep
$(OUTOS)perf.dep: $(SRC.PERF)
	$(DO.DEP)
else
-include $(OUTOS)perf.dep
endif

endif # ifeq ($(MAKESECTION),targets)
