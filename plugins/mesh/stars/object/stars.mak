DESCRIPTION.stars = Stars mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make stars        Make the $(DESCRIPTION.stars)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stars starsclean
plugins meshes all: stars

starsclean:
	$(MAKE_CLEAN)
stars:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/stars/object

ifeq ($(USE_PLUGINS),yes)
  STARS = $(OUTDLL)stars$(DLL)
  LIB.STARS = $(foreach d,$(DEP.STARS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(STARS)
else
  STARS = $(OUT)$(LIB_PREFIX)stars$(LIB)
  DEP.EXE += $(STARS)
  SCF.STATIC += stars
  TO_INSTALL.STATIC_LIBS += $(STARS)
endif

INC.STARS = $(wildcard plugins/mesh/stars/object/*.h)
SRC.STARS = $(wildcard plugins/mesh/stars/object/*.cpp)
OBJ.STARS = $(addprefix $(OUT),$(notdir $(SRC.STARS:.cpp=$O)))
DEP.STARS = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += STARS
DSP.STARS.NAME = stars
DSP.STARS.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: stars starsclean
stars: $(OUTDIRS) $(STARS)

$(STARS): $(OBJ.STARS) $(LIB.STARS)
	$(DO.PLUGIN)

clean: starsclean
starsclean:
	-$(RM) $(STARS) $(OBJ.STARS)

ifdef DO_DEPEND
dep: $(OUTOS)stars.dep
$(OUTOS)stars.dep: $(SRC.STARS)
	$(DO.DEP)
else
-include $(OUTOS)stars.dep
endif

endif # ifeq ($(MAKESECTION),targets)
