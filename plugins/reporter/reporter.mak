# This is a subinclude file used to define the rules needed
# to build the reporter plug-in.

# Driver description
DESCRIPTION.reporter = Crystal Space Reporter plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make reporter     Make the $(DESCRIPTION.reporter)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: reporter reporterclean
all plugins: reporter

reporter:
	$(MAKE_TARGET) MAKE_DLL=yes
reporterclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/reporter

ifeq ($(USE_PLUGINS),yes)
  REPORTER = $(OUTDLL)reporter$(DLL)
  LIB.REPORTER = $(foreach d,$(DEP.REPORTER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(REPORTER)
else
  REPORTER = $(OUT)$(LIB_PREFIX)reporter$(LIB)
  DEP.EXE += $(REPORTER)
  SCF.STATIC += reporter
  TO_INSTALL.STATIC_LIBS += $(REPORTER)
endif

INC.REPORTER = $(wildcard plugins/reporter/*.h)
SRC.REPORTER = $(wildcard plugins/reporter/*.cpp)
OBJ.REPORTER = $(addprefix $(OUT),$(notdir $(SRC.REPORTER:.cpp=$O)))
DEP.REPORTER = CSGEOM CSUTIL CSSYS CSUTIL
CFG.REPORTER = data/config/reporter.cfg

TO_INSTALL.CONFIG += $(CFG.REPORTER)

MSVC.DSP += REPORTER
DSP.REPORTER.NAME = reporter
DSP.REPORTER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: reporter reporterclean

reporter: $(OUTDIRS) $(REPORTER)

$(REPORTER): $(OBJ.REPORTER) $(LIB.REPORTER)
	$(DO.PLUGIN)

clean: reporterclean
reporterclean:
	$(RM) $(REPORTER) $(OBJ.REPORTER)

ifdef DO_DEPEND
dep: $(OUTOS)reporter.dep
$(OUTOS)reporter.dep: $(SRC.REPORTER)
	$(DO.DEP)
else
-include $(OUTOS)reporter.dep
endif

endif # ifeq ($(MAKESECTION),targets)
