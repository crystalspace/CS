DESCRIPTION.csgfx = Crystal Space graphics utility library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csgfx        Make the $(DESCRIPTION.csgfx)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csgfx csgfxclean
all libs: csgfx

csgfx:
	$(MAKE_TARGET)
csgfxclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSGFX.LIB = $(OUT)/$(LIB_PREFIX)csgfx$(LIB_SUFFIX)

DIR.CSGFX = libs/csgfx
OUT.CSGFX = $(OUT)/$(DIR.CSGFX)
INC.CSGFX = $(wildcard $(SRCDIR)/$(DIR.CSGFX)/*.h $(SRCDIR)/include/csgfx/*.h)
SRC.CSGFX = $(wildcard $(SRCDIR)/$(DIR.CSGFX)/*.cpp)
OBJ.CSGFX = $(addprefix $(OUT.CSGFX)/,$(notdir $(SRC.CSGFX:.cpp=$O)))

ifneq ($(USE_PLUGINS),yes)
  # When not using plugins, applications usually require this library since
  # it is needed by several plugin modules which are linked into the app.
  DEP.EXE += $(CSGFX.LIB)
endif

OUTDIRS += $(OUT.CSGFX)

TO_INSTALL.STATIC_LIBS += $(CSGFX.LIB)

MSVC.DSP += CSGFX
DSP.CSGFX.NAME = csgfx
DSP.CSGFX.TYPE = library
DSP.CSGFX.RESOURCES = $(wildcard $(SRCDIR)/$(DIR.CSGFX)/*.inc)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csgfx csgfxclean csgfxcleandep

csgfx: $(OUTDIRS) $(CSGFX.LIB)

$(OUT.CSGFX)/%$O: $(SRCDIR)/$(DIR.CSGFX)/%.cpp
	$(DO.COMPILE.CPP)

$(CSGFX.LIB): $(OBJ.CSGFX)
	$(DO.LIBRARY)

clean: csgfxclean
csgfxclean:
	-$(RMDIR) $(CSGFX.LIB) $(OBJ.CSGFX)

cleandep: csgfxcleandep
csgfxcleandep:
	-$(RM) $(OUT.CSGFX)/csgfx.dep

ifdef DO_DEPEND
dep: $(OUT.CSGFX) $(OUT.CSGFX)/csgfx.dep
$(OUT.CSGFX)/csgfx.dep: $(SRC.CSGFX)
	$(DO.DEPEND)
else
-include $(OUT.CSGFX)/csgfx.dep
endif

endif # ifeq ($(MAKESECTION),targets)
