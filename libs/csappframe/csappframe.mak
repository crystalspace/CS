DESCRIPTION.csappframe = Crystal Space application framework library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csappframe   Make the $(DESCRIPTION.csappframe)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csappframe csappframeclean
all libs: csappframe

csappframe:
	$(MAKE_TARGET)
csappframeclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSAPPFRAME.LIB = $(OUT)/$(LIB_PREFIX)csappframe$(LIB_SUFFIX)

DIR.CSAPPFRAME = libs/csappframe
OUT.CSAPPFRAME = $(OUT)/$(DIR.CSAPPFRAME)
INC.CSAPPFRAME = \
  $(wildcard $(SRCDIR)/$(DIR.CSAPPFRAME)/*.h $(SRCDIR)/include/csappframe/*.h)
SRC.CSAPPFRAME = $(wildcard $(SRCDIR)/$(DIR.CSAPPFRAME)/*.cpp)
OBJ.CSAPPFRAME = \
  $(addprefix $(OUT.CSAPPFRAME)/,$(notdir $(SRC.CSAPPFRAME:.cpp=$O)))

OUTDIRS += $(OUT.CSAPPFRAME)

TO_INSTALL.STATIC_LIBS += $(CSAPPFRAME.LIB)

MSVC.DSP += CSAPPFRAME
DSP.CSAPPFRAME.NAME = csappframe
DSP.CSAPPFRAME.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csappframe csappframeclean csappframecleandep

csappframe: $(OUTDIRS) $(CSAPPFRAME.LIB)

$(OUT.CSAPPFRAME)/%$O: $(SRCDIR)/$(DIR.CSAPPFRAME)/%.cpp
	$(DO.COMPILE.CPP)

$(CSAPPFRAME.LIB): $(OBJ.CSAPPFRAME)
	$(DO.LIBRARY)

clean: csappframeclean
csappframeclean:
	-$(RMDIR) $(CSAPPFRAME.LIB) $(OBJ.CSAPPFRAME)

cleandep: csappframecleandep
csappframecleandep:
	-$(RM) $(OUT.CSAPPFRAME)/csappframe.dep

ifdef DO_DEPEND
dep: $(OUT.CSAPPFRAME) $(OUT.CSAPPFRAME)/csappframe.dep
$(OUT.CSAPPFRAME)/csappframe.dep: $(SRC.CSAPPFRAME)
	$(DO.DEPEND)
else
-include $(OUT.CSAPPFRAME)/csappframe.dep
endif

endif # ifeq ($(MAKESECTION),targets)
