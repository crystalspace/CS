# This is a subinclude file used to define the rules needed
# to build the perfstat plug-in.

# Driver description
DESCRIPTION.perfstat = Crystal Space Performance Stats Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make perfstat     Make the $(DESCRIPTION.perfstat)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: perfstat perfstatclean

all plugins: perfstat

perfstat:
	$(MAKE_TARGET) MAKE_DLL=yes
perfstatclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/perfstat

ifeq ($(USE_SHARED_PLUGINS),yes)
  PERFSTAT=$(OUTDLL)perfstat$(DLL)
  DEP.PERFSTAT=$(CSGEOM.LIB) $(CSSYS.LIB) $(CSUTIL.LIB)
  TO_INSTALL.DYNAMIC_LIBS+=$(PERFSTAT)
else
  PERFSTAT=$(OUT)$(LIB_PREFIX)perfstat$(LIB)
  DEP.EXE+=$(PERFSTAT)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_PERFSTAT
  TO_INSTALL.STATIC_LIBS+=$(PERFSTAT)
endif
DESCRIPTION.$(PERFSTAT) = $(DESCRIPTION.perfstat)
TO_INSTALL.DATA += data/perf.zip
SRC.PERFSTAT = $(wildcard plugins/perfstat/*.cpp)
OBJ.PERFSTAT = $(addprefix $(OUT),$(notdir $(SRC.PERFSTAT:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: perfstat perfstatclean

# Chain rules
net: perfstat
clean: perfstatclean

perfstat: $(OUTDIRS) $(PERFSTAT)

$(PERFSTAT): $(OBJ.PERFSTAT) $(DEP.PERFSTAT)
	$(DO.PLUGIN)

perfstatclean:
	$(RM) $(PERFSTAT) $(OBJ.PERFSTAT) $(OUTOS)perfstat.dep

ifdef DO_DEPEND
dep: $(OUTOS)perfstat.dep
$(OUTOS)perfstat.dep: $(SRC.PERFSTAT)
	$(DO.DEP)
else
-include $(OUTOS)perfstat.dep
endif

endif # ifeq ($(MAKESECTION),targets)
