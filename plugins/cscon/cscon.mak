DESCRIPTION.cscon = Crystal Space Console Plugin

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

CFLAGS.CSCON +=
SRC.CSCON = $(wildcard plugins/cscon/*.cpp)
OBJ.CSCON = $(addprefix $(OUT),$(notdir $(SRC.CSCON:.cpp=$O)))
LIB.CSCON = $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
LIB.EXTERNAL.CSCON = 
DESCRIPTION.$(CSCON.EXE) = $(DESCRIPTION.cscon)

ifeq ($(USE_SHARED_PLUGINS),yes)
  CSCON=$(OUTDLL)cscon$(DLL)
  DEP.CSCON=$(LIB.CSCON)
  TO_INSTALL.DYNAMIC_LIBS+=$(CSCON)
else
  CSCON=$(OUT)$(LIB_PREFIX)cscon$(LIB)
  DEP.EXE+=$(CSCON)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_CSCON
  TO_INSTALL.STATIC_LIBS+=$(CSCON)
endif
TO_INSTALL.CONFIG += data/config/funcon.cfg
TO_INSTALL.DATA += data/funcon.zip

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cscon csconclean
cscon: $(OUTDIRS) $(CSCON)

$(OUT)%$O: plugins/cscon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSCON)

$(CSCON): $(OBJ.CSCON) $(DEP.CSCON)
	$(DO.PLUGIN)

clean: csconclean
csconclean:
	-$(RM) $(CSCON) $(OBJ.CSCON) $(OUTOS)cscon.dep

ifdef DO_DEPEND
dep: $(OUTOS)cscon.dep
$(OUTOS)cscon.dep: $(SRC.CSCON)
	$(DO.DEP)
else
-include $(OUTOS)cscon.dep
endif

endif # ifeq ($(MAKESECTION),targets)
