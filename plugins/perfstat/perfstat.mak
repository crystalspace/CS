# This is a subinclude file used to define the rules needed
# to build the perfstat plug-in.

# Driver description
DESCRIPTION.perfstat = Crystal Space performance statistics plug-in

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

ifeq ($(USE_PLUGINS),yes)
  PERFSTAT = $(OUTDLL)perfstat$(DLL)
  LIB.PERFSTAT = $(foreach d,$(DEP.PERFSTAT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PERFSTAT)
else
  PERFSTAT = $(OUT)$(LIB_PREFIX)perfstat$(LIB)
  DEP.EXE += $(PERFSTAT)
  SCF.STATIC += perfstat
  TO_INSTALL.STATIC_LIBS += $(PERFSTAT)
endif

INC.PERFSTAT = $(wildcard plugins/perfstat/*.h)
SRC.PERFSTAT = $(wildcard plugins/perfstat/*.cpp)
OBJ.PERFSTAT = $(addprefix $(OUT),$(notdir $(SRC.PERFSTAT:.cpp=$O)))
DEP.PERFSTAT = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += PERFSTAT
DSP.PERFSTAT.NAME = perfstat
DSP.PERFSTAT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: perfstat perfstatclean

perfstat: $(OUTDIRS) $(PERFSTAT)

$(PERFSTAT): $(OBJ.PERFSTAT) $(LIB.PERFSTAT)
	$(DO.PLUGIN)

clean: perfstatclean
perfstatclean:
	$(RM) $(PERFSTAT) $(OBJ.PERFSTAT)

ifdef DO_DEPEND
dep: $(OUTOS)perfstat.dep
$(OUTOS)perfstat.dep: $(SRC.PERFSTAT)
	$(DO.DEP)
else
-include $(OUTOS)perfstat.dep
endif

endif # ifeq ($(MAKESECTION),targets)
