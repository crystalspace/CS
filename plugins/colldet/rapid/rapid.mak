DESCRIPTION.rapid = Crystal Space RAPID CD System

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make rapid        Make the $(DESCRIPTION.rapid)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: rapid rapidclean
plugins all: rapid
rapidclean:
	$(MAKE_CLEAN)
rapid:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SRC.RAPID = $(wildcard plugins/colldet/rapid/*.cpp)
OBJ.RAPID = $(addprefix $(OUT),$(notdir $(SRC.RAPID:.cpp=$O)))
LIB.RAPID = $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)

ifeq ($(USE_SHARED_PLUGINS),yes)
RAPID=$(OUTDLL)rapid$(DLL)
DEP.RAPID=$(LIB.RAPID)
TO_INSTALL.DYNAMIC_LIBS += $(RAPID)
else
RAPID=$(OUT)$(LIB_PREFIX)rapid$(LIB)
DEP.EXE+=$(RAPID)
CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_RAPID
TO_INSTALL.STATIC_LIBS += $(RAPID)
endif

vpath %.cpp $(sort $(dir $(SRC.RAPID)))

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: rapid rapidclean
rapid: $(OUTDIRS) $(RAPID)

$(RAPID): $(OBJ.RAPID) $(DEP.RAPID)
	$(DO.PLUGIN)

clean: rapidclean
rapidclean:
	-$(RM) $(RAPID) $(OBJ.RAPID) $(OUTOS)rapid.dep

ifdef DO_DEPEND
dep: $(OUTOS)rapid.dep
$(OUTOS)rapid.dep: $(SRC.RAPID)
	$(DO.DEP)
else
-include $(OUTOS)rapid.dep
endif

endif # ifeq ($(MAKESECTION),targets)
