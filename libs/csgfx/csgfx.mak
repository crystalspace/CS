# Library description
DESCRIPTION.csgfx = Crystal Space graphics utility library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csgfx        Make the $(DESCRIPTION.csgfx)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csgfx

all libs: csgfx
csgfx:
	$(MAKE_TARGET)
csgfxclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csgfx

CSGFX.LIB = $(OUT)$(LIB_PREFIX)csgfx$(LIB_SUFFIX)
INC.CSGFX = $(wildcard include/csgfx/*.h)
SRC.CSGFX = $(wildcard libs/csgfx/*.cpp)
OBJ.CSGFX = $(addprefix $(OUT),$(notdir $(SRC.CSGFX:.cpp=$O)))

ifneq ($(USE_PLUGINS),yes)
  # When not using plugins, applications usually require this library since
  # it is needed by several plugin modules which are linked into the app.
  DEP.EXE += $(CSGFX.LIB)
endif

TO_INSTALL.STATIC_LIBS += $(CSGFX.LIB)

MSVC.DSP += CSGFX
DSP.CSGFX.NAME = csgfx
DSP.CSGFX.TYPE = library
DSP.CSGFX.RESOURCES = libs/csgfx/mipmap.inc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csgfx csgfxclean

all: $(CSGFX.LIB)
csgfx: $(OUTDIRS) $(CSGFX.LIB)
clean: csgfxclean

$(CSGFX.LIB): $(OBJ.CSGFX)
	$(DO.LIBRARY)

csgfxclean:
	-$(RM) $(CSGFX.LIB) $(OBJ.CSGFX)

ifdef DO_DEPEND
dep: $(OUTOS)csgfx.dep
$(OUTOS)csgfx.dep: $(SRC.CSGFX)
	$(DO.DEP)
else
-include $(OUTOS)csgfx.dep
endif

endif # ifeq ($(MAKESECTION),targets)
