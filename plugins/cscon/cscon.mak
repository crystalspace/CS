# Makefile for Crystal Space Console Plugin
# This is an auto-generated file.  Do not edit.

ifneq (,$(findstring cscon,$(PLUGINS)))
DESCRIPTION.cscon = Crystal Space Console Plugin
DESCRIPTION.funcon = Crystal Space Funky Console Plugin

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP+=$(NEWLINE)echo $"  make cscon        Make the $(DESCRIPTION.cscon)$"
PLUGINHELP+=$(NEWLINE)echo $"  make funcon       Make the $(DESCRIPTION.funcon)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cscon csconclean csconclob funcon funconclean funconclob
csconclob: csconclean cscon
funconclob: funconclean funcon
plugins all: cscon funcon
funconclean csconclean:
	$(MAKE_CLEAN)
funcon cscon:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.CSCON+=
SRC.CSCON = $(wildcard plugins/cscon/con*.cpp plugins/cscon/cs*.cpp)
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

CFLAGS.FUNCON+=
SRC.FUNCON = $(wildcard plugins/cscon/*.cpp)
OBJ.FUNCON = $(addprefix $(OUT),$(notdir $(SRC.FUNCON:.cpp=$O)))
LIB.FUNCON = $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)  
LIB.EXTERNAL.FUNCON = 
DESCRIPTION.$(FUNCON.EXE) = $(DESCRIPTION.funcon)

ifeq ($(USE_SHARED_PLUGINS),yes)
FUNCON=$(OUTDLL)funcon$(DLL)
DEP.FUNCON=$(LIB.FUNCON)
else
FUNCON=$(OUT)$(LIB_PREFIX)funcon$(LIB)
DEP.EXE+=$(FUNCON)
CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_FUNCON
endif


endif # ifeq ($(MAKESECTION),postdefines)
#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cscon csconclean funcon funconclean
cscon: $(OUTDIRS) $(CSCON)
funcon: $(OUTDIRS) $(FUNCON)

#Begin User Defined
#End User Defined

$(OUT)%$O: plugins/cscon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSCON)

$(CSCON): $(OBJ.CSCON) $(DEP.CSCON)
	$(DO.PLUGIN)

$(FUNCON): $(OBJ.FUNCON) $(DEP.FUNCON)
	$(DO.PLUGIN)

clean: csconclean funconclean
csconclean:
	-$(RM) $(CSCON) $(OBJ.CSCON) $(OUTOS)cscon.dep
funconclean:
	-$(RM) $(FUNCON) $(OBJ.FUNCON) $(OUTOS)funcon.dep

ifdef DO_DEPEND
depend: $(OUTOS)cscon.dep $(OUTOS)funcon.dep
$(OUTOS)cscon.dep: $(SRC.CSCON)
	$(DO.DEP)
$(OUTOS)funcon.dep: $(SRC.FUNCON)
	$(DO.DEP)
else
-include $(OUTOS)cscon.dep
-include $(OUTOS)funcon.dep
endif

endif # ifeq ($(MAKESECTION),targets)

endif # ifneq (,$(findstring cscon,$(PLUGINS)))
