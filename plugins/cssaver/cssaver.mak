#------------------------------------------------------------------------------
# Map File Saver plugin makefile
#------------------------------------------------------------------------------
DESCRIPTION.cssaver = Crystal Space map file saver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make cssaver      Make the $(DESCRIPTION.cssaver)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cssaver cssaverclean
all plugins: cssaver

cssaver:
	$(MAKE_TARGET) MAKE_DLL=yes
cssaverclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/cssaver

ifeq ($(USE_PLUGINS),yes)
  CSSAVER = $(OUTDLL)/cssaver$(DLL)
  LIB.CSSAVER = $(foreach d,$(DEP.CSSAVER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSSAVER)
else
  CSSAVER = $(OUT)/$(LIB_PREFIX)cssaver$(LIB)
  DEP.EXE += $(CSSAVER)
  SCF.STATIC += cssaver
  TO_INSTALL.STATIC_LIBS += $(CSSAVER)
endif

INC.CSSAVER = $(wildcard plugins/cssaver/*.h)
SRC.CSSAVER = $(wildcard plugins/cssaver/*.cpp)
OBJ.CSSAVER = $(addprefix $(OUT)/,$(notdir $(SRC.CSSAVER:.cpp=$O)))
DEP.CSSAVER = CSUTIL CSTOOL CSSYS CSUTIL

MSVC.DSP += CSSAVER
DSP.CSSAVER.NAME = cssaver
DSP.CSSAVER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cssaver cssaverclean
cssaver: $(OUTDIRS) $(CSSAVER)

$(CSSAVER): $(OBJ.CSSAVER) $(LIB.CSSAVER)
	$(DO.PLUGIN)

clean: cssaverclean
cssaverclean:
	-$(RM) $(CSSAVER) $(OBJ.CSSAVER)

ifdef DO_DEPEND
dep: $(OUTOS)/cssaver.dep
$(OUTOS)/cssaver.dep: $(SRC.CSSAVER)
	$(DO.DEP)
else
-include $(OUTOS)/cssaver.dep
endif

endif # ifeq ($(MAKESECTION),targets)
