DESCRIPTION.cscon = Crystal Space console plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make cscon        Make the $(DESCRIPTION.cscon)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cscon csconclean
plugins all: cscon

cscon:
	$(MAKE_TARGET) MAKE_DLL=yes
csconclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/cscon

ifeq ($(USE_PLUGINS),yes)
  CSCON = $(OUTDLL)cscon$(DLL)
  LIB.CSCON = $(foreach d,$(DEP.CSCON),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSCON)
else
  CSCON = $(OUT)$(LIB_PREFIX)cscon$(LIB)
  DEP.EXE += $(CSCON)
  SCF.STATIC += cscon
  TO_INSTALL.STATIC_LIBS += $(CSCON)
endif

INC.CSCON = $(wildcard plugins/cscon/*.h)
SRC.CSCON = $(wildcard plugins/cscon/*.cpp)
OBJ.CSCON = $(addprefix $(OUT),$(notdir $(SRC.CSCON:.cpp=$O)))
DEP.CSCON = CSGFXLDR CSUTIL CSSYS
CFG.CSCON = data/config/funcon.cfg

TO_INSTALL.CONFIG += $(CFG.CSCON)
TO_INSTALL.DATA += data/funcon.zip

MSVC.DSP += CSCON
DSP.CSCON.NAME = cscon
DSP.CSCON.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cscon csconclean
cscon: $(OUTDIRS) $(CSCON)

$(CSCON): $(OBJ.CSCON) $(LIB.CSCON)
	$(DO.PLUGIN)

clean: csconclean
csconclean:
	-$(RM) $(CSCON) $(OBJ.CSCON)

ifdef DO_DEPEND
dep: $(OUTOS)cscon.dep
$(OUTOS)cscon.dep: $(SRC.CSCON)
	$(DO.DEP)
else
-include $(OUTOS)cscon.dep
endif

endif # ifeq ($(MAKESECTION),targets)
