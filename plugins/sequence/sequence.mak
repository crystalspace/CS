# This is a subinclude file used to define the rules needed
# to build the sequence plug-in.

# Driver description
DESCRIPTION.sequence = Crystal Space Sequence Manager Plug-In

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sequence     Make the $(DESCRIPTION.sequence)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sequence sequenceclean
all plugins: sequence

sequence:
	$(MAKE_TARGET) MAKE_DLL=yes
sequenceclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sequence

ifeq ($(USE_PLUGINS),yes)
  SEQUENCE = $(OUTDLL)sequence$(DLL)
  LIB.SEQUENCE = $(foreach d,$(DEP.SEQUENCE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SEQUENCE)
else
  SEQUENCE = $(OUT)$(LIB_PREFIX)sequence$(LIB)
  DEP.EXE += $(SEQUENCE)
  SCF.STATIC += sequence
  TO_INSTALL.STATIC_LIBS += $(SEQUENCE)
endif

INC.SEQUENCE = $(wildcard plugins/sequence/*.h)
SRC.SEQUENCE = $(wildcard plugins/sequence/*.cpp)
OBJ.SEQUENCE = $(addprefix $(OUT),$(notdir $(SRC.SEQUENCE:.cpp=$O)))
DEP.SEQUENCE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SEQUENCE
DSP.SEQUENCE.NAME = sequence
DSP.SEQUENCE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sequence sequenceclean

sequence: $(OUTDIRS) $(SEQUENCE)

$(SEQUENCE): $(OBJ.SEQUENCE) $(LIB.SEQUENCE)
	$(DO.PLUGIN)

clean: sequenceclean
sequenceclean:
	$(RM) $(SEQUENCE) $(OBJ.SEQUENCE)

ifdef DO_DEPEND
dep: $(OUTOS)sequence.dep
$(OUTOS)sequence.dep: $(SRC.SEQUENCE)
	$(DO.DEP)
else
-include $(OUTOS)sequence.dep
endif

endif # ifeq ($(MAKESECTION),targets)
