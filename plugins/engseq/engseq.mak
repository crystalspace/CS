# This is a subinclude file used to define the rules needed
# to build the engseq plug-in.

# Driver description
DESCRIPTION.engseq = Crystal Space Engine Sequence Manager Plug-In

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make engseq       Make the $(DESCRIPTION.engseq)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: engseq engseqclean
all plugins: engseq

engseq:
	$(MAKE_TARGET) MAKE_DLL=yes
engseqclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/engseq

ifeq ($(USE_PLUGINS),yes)
  ENGSEQ = $(OUTDLL)/engseq$(DLL)
  LIB.ENGSEQ = $(foreach d,$(DEP.ENGSEQ),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ENGSEQ)
else
  ENGSEQ = $(OUT)/$(LIB_PREFIX)engseq$(LIB)
  DEP.EXE += $(ENGSEQ)
  SCF.STATIC += engseq
  TO_INSTALL.STATIC_LIBS += $(ENGSEQ)
endif

INC.ENGSEQ = $(wildcard plugins/engseq/*.h)
SRC.ENGSEQ = $(wildcard plugins/engseq/*.cpp)
OBJ.ENGSEQ = $(addprefix $(OUT)/,$(notdir $(SRC.ENGSEQ:.cpp=$O)))
DEP.ENGSEQ = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += ENGSEQ
DSP.ENGSEQ.NAME = engseq
DSP.ENGSEQ.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: engseq engseqclean

engseq: $(OUTDIRS) $(ENGSEQ)

$(ENGSEQ): $(OBJ.ENGSEQ) $(LIB.ENGSEQ)
	$(DO.PLUGIN)

clean: engseqclean
engseqclean:
	$(RM) $(ENGSEQ) $(OBJ.ENGSEQ)

ifdef DO_DEPEND
dep: $(OUTOS)/engseq.dep
$(OUTOS)/engseq.dep: $(SRC.ENGSEQ)
	$(DO.DEP)
else
-include $(OUTOS)/engseq.dep
endif

endif # ifeq ($(MAKESECTION),targets)

