DESCRIPTION.ball = Ball mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ball         Make the $(DESCRIPTION.ball)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ball ballclean
plugins meshes all: ball

ballclean:
	$(MAKE_CLEAN)
ball:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/ball/object

ifeq ($(USE_PLUGINS),yes)
  BALL = $(OUTDLL)/ball$(DLL)
  LIB.BALL = $(foreach d,$(DEP.BALL),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BALL)
else
  BALL = $(OUT)/$(LIB_PREFIX)ball$(LIB)
  DEP.EXE += $(BALL)
  SCF.STATIC += ball
  TO_INSTALL.STATIC_LIBS += $(BALL)
endif

INF.BALL = $(SRCDIR)/plugins/mesh/ball/object/ball.csplugin
INC.BALL = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/ball/object/*.h))
SRC.BALL = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/ball/object/*.cpp))
OBJ.BALL = $(addprefix $(OUT)/,$(notdir $(SRC.BALL:.cpp=$O)))
DEP.BALL = CSGFX CSGEOM CSUTIL

MSVC.DSP += BALL
DSP.BALL.NAME = ball
DSP.BALL.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ball ballclean
ball: $(OUTDIRS) $(BALL)

$(BALL): $(OBJ.BALL) $(LIB.BALL)
	$(DO.PLUGIN)

clean: ballclean
ballclean:
	-$(RMDIR) $(BALL) $(OBJ.BALL) $(OUTDLL)/$(notdir $(INF.BALL))

ifdef DO_DEPEND
dep: $(OUTOS)/ball.dep
$(OUTOS)/ball.dep: $(SRC.BALL)
	$(DO.DEP)
else
-include $(OUTOS)/ball.dep
endif

endif # ifeq ($(MAKESECTION),targets)
