DESCRIPTION.cstool = Crystal Space tool Library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP+=$(NEWLINE)echo $"  make cstool       Make the $(DESCRIPTION.cstool)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cstool cstoolclean
all libs: cstool

cstool:
	$(MAKE_TARGET)
cstoolclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSTOOL.LIB = $(OUT)/$(LIB_PREFIX)cstool$(LIB_SUFFIX)

DIR.CSTOOL = libs/cstool
OUT.CSTOOL = $(OUT)/$(DIR.CSTOOL)
INC.CSTOOL = \
  $(wildcard $(SRCDIR)/$(DIR.CSTOOL)/*.h $(SRCDIR)/include/cstool/*.h)
SRC.CSTOOL = $(wildcard $(SRCDIR)/$(DIR.CSTOOL)/*.cpp)
OBJ.CSTOOL = $(addprefix $(OUT.CSTOOL)/,$(notdir $(SRC.CSTOOL:.cpp=$O)))
CFG.CSTOOL = $(SRCDIR)/data/config/system.cfg

OUTDIRS += $(OUT.CSTOOL)

TO_INSTALL.CONFIG += $(CFG.CSTOOL)
TO_INSTALL.DATA += $(SRCDIR)/data/varia/vidprefs.def
TO_INSTALL.STATIC_LIBS += $(CSTOOL.LIB)

MSVC.DSP += CSTOOL
DSP.CSTOOL.NAME = cstool
DSP.CSTOOL.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cstool cstoolclean cstoolcleandep

cstool: $(OUTDIRS) $(CSTOOL.LIB)

$(OUT.CSTOOL)/%$O: $(SRCDIR)/$(DIR.CSTOOL)/%.cpp
	$(DO.COMPILE.CPP)

$(CSTOOL.LIB): $(OBJ.CSTOOL)
	$(DO.LIBRARY)

clean: cstoolclean
cstoolclean:
	-$(RMDIR) $(CSTOOL.LIB) $(OBJ.CSTOOL)

cleandep: cstoolcleandep
cstoolcleandep:
	-$(RM) $(OUT.CSTOOL)/cstool.dep

ifdef DO_DEPEND
dep: $(OUT.CSTOOL) $(OUT.CSTOOL)/cstool.dep
$(OUT.CSTOOL)/cstool.dep: $(SRC.CSTOOL)
	$(DO.DEPEND)
else
-include $(OUT.CSTOOL)/cstool.dep
endif

endif # ifeq ($(MAKESECTION),targets)
