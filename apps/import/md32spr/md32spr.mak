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

MD32SPR.EXE = md32spr$(EXE.CONSOLE)
DIR.MD32SPR = apps/import/md32spr
OUT.MD32SPR = $(OUT)/$(DIR.MD32SPR)
INC.MD32SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MD32SPR)/*.h ))
SRC.MD32SPR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.MD32SPR)/*.cpp ))
OBJ.MD32SPR = $(addprefix $(OUT.MD32SPR)/,$(notdir $(SRC.MD32SPR:.cpp=$O)))
DEP.MD32SPR = CSTOOL CSGEOM CSTOOL CSGFX CSUTIL CSUTIL
LIB.MD32SPR = $(foreach d,$(DEP.MD32SPR),$($d.LIB))

OUTDIRS += $(OUT.MD32SPR)

TO_INSTALL.EXE += $(MD32SPR.EXE)

MSVC.DSP += MD32SPR
DSP.MD32SPR.NAME = md32spr
DSP.MD32SPR.TYPE = appcon
DSP.MD32SPR.LIBS = zlib

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.md32spr md32sprclean md32sprcleandep

all: $(MD32SPR.EXE)
build.md32spr: $(OUTDIRS) $(MD32SPR.EXE)
clean: md32sprclean

$(OUT.MD32SPR)/%$O: $(SRCDIR)/$(DIR.MD32SPR)/%.cpp
	$(DO.COMPILE.CPP)

$(MD32SPR.EXE): $(OBJ.MD32SPR) $(LIB.MD32SPR)
	$(DO.LINK.CONSOLE.EXE) $(ZLIB.LFLAGS)

md32sprclean:
	-$(RM) md32spr.txt
	-$(RMDIR) $(MD32SPR.EXE) $(OBJ.MD32SPR)

cleandep: md32sprcleandep
md32sprcleandep:
	-$(RM) $(OUT.MD32SPR)/md32spr.dep

ifdef DO_DEPEND
dep: $(OUT.MD32SPR) $(OUT.MD32SPR)/md32spr.dep
$(OUT.MD32SPR)/md32spr.dep: $(SRC.MD32SPR)
	$(DO.DEPEND)
else
-include $(OUT.MD32SPR)/md32spr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
