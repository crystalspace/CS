# Application description
DESCRIPTION.md32spr = Quake MD3 conversion tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make md32spr      Make the $(DESCRIPTION.md32spr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: md32spr md32sprclean

all apps: md32spr
md32spr:
	$(MAKE_APP)
md32sprclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/import/md32spr

MD32SPR.EXE = md32spr$(EXE.CONSOLE)
INC.MD32SPR = $(wildcard apps/import/md32spr/*.h)
SRC.MD32SPR = $(wildcard apps/import/md32spr/*.cpp)
OBJ.MD32SPR = $(addprefix $(OUT)/,$(notdir $(SRC.MD32SPR:.cpp=$O)))
DEP.MD32SPR = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSSYS CSUTIL
LIB.MD32SPR = $(foreach d,$(DEP.MD32SPR),$($d.LIB))

TO_INSTALL.EXE += $(MD32SPR.EXE)

MSVC.DSP += MD32SPR
DSP.MD32SPR.NAME = md32spr
DSP.MD32SPR.TYPE = appcon
DSP.MD32SPR.LIBS = zlib

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.md32spr md32sprclean

all: $(MD32SPR.EXE)
build.md32spr: $(OUTDIRS) $(MD32SPR.EXE)
clean: md32sprclean

$(MD32SPR.EXE): $(OBJ.MD32SPR) $(LIB.MD32SPR)
	$(DO.LINK.CONSOLE.EXE) $(ZLIB.LFLAGS)

md32sprclean:
	-$(RMDIR) $(MD32SPR.EXE) $(OBJ.MD32SPR)

ifdef DO_DEPEND
dep: $(OUTOS)/md32spr.dep
$(OUTOS)/md32spr.dep: $(SRC.MD32SPR)
	$(DO.DEP1) $(ZLIB.CFLAGS) $(DO.DEP2)
else
-include $(OUTOS)/md32spr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
