# Makefile for Crystal Space Console Plugin
# This is an auto-generated file.  Do not edit.

ifneq (,$(findstring cscon,$(PLUGINS)))
DESCRIPTION.cscon = Crystal Space Console Plugin

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP+=$(NEWLINE)echo $"  make cscon        Make the $(DESCRIPTION.cscon)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cscon csconclean csconclob
csconclob: csconclean cscon
plugins all: cscon
csconclean:
	$(MAKE_CLEAN)
cscon:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.CSCON+=
SRC.CSCON = $(wildcard plugins/cscon/*.cpp)
OBJ.CSCON = $(addprefix $(OUT),$(notdir $(SRC.CSCON:.cpp=$O)))
LIB.CSCON =  $(CSUTIL.LIB) $(CSSYS.LIB)
LIB.EXTERNAL.CSCON = 
DESCRIPTION.$(CSCON.EXE) = $(DESCRIPTION.cscon)

ifeq ($(USE_SHARED_PLUGINS),yes)
CSCON=$(OUTDLL)cscon$(DLL)
DEP.CSCON=$(LIB.CSCON)
else
CSCON=$(OUT)$(LIB_PREFIX)cscon$(LIB)
DEP.EXE+=$(CSCON)
CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_CSCON
endif


endif # ifeq ($(MAKESECTION),postdefines)
#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cscon csconclean
cscon: $(OUTDIRS) $(CSCON)

#Begin User Defined
#End User Defined

$(OUT)%$O: plugins/cscon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSCON)
$(CSCON): $(OBJ.CSCON) $(DEP.CSCON)
	$(DO.PLUGIN)

clean: csconclean
csconclean:
	-$(RM) $(CSCON) $(OBJ.CSCON) $(OUTOS)cscon.dep
ifdef DO_DEPEND
depend: $(OUTOS)cscon.dep
$(OUTOS)cscon.dep: $(SRC.CSCON)
	$(DO.DEP)
else
-include $(OUTOS)cscon.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring cscon,$(PLUGINS)))
