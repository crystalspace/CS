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

INC.CSGFX = include/csgfx/csimage.h \
  include/csgfx/quantize.h include/csgfx/inv_cmap.h \
  include/csgfx/memimage.h include/csgfx/bumpmap.h
SRC.CSGFX = libs/csgfx/csimage.cpp \
  libs/csgfx/quantize.cpp libs/csgfx/inv_cmap.cpp \
  libs/csgfx/memimage.cpp libs/csgfx/bumpmap.cpp

ifeq ($(DO_PNG),yes)
  INC.CSGFX += include/csgfx/pngsave.h
  LIBS.EXE += $(PNG_LIBS)
endif

CSGFX.LIB = $(OUT)$(LIB_PREFIX)csgfx$(LIB_SUFFIX)
OBJ.CSGFX = $(addprefix $(OUT),$(notdir $(SRC.CSGFX:.cpp=$O)))

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
