DESCRIPTION.ball = Ball mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ball         Make the $(DESCRIPTION.ball)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ball ballclean
plugins all: ball

ballclean:
	$(MAKE_CLEAN)
ball:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/ball

ifeq ($(USE_PLUGINS),yes)
  BALL = $(OUTDLL)ball$(DLL)
  LIB.BALL = $(foreach d,$(DEP.BALL),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BALL)
else
  BALL = $(OUT)$(LIB_PREFIX)ball$(LIB)
  DEP.EXE += $(BALL)
  SCF.STATIC += ball
  TO_INSTALL.STATIC_LIBS += $(BALL)
endif

INC.BALL = $(wildcard plugins/mesh/object/ball/*.h)
SRC.BALL = $(wildcard plugins/mesh/object/ball/*.cpp)
OBJ.BALL = $(addprefix $(OUT),$(notdir $(SRC.BALL:.cpp=$O)))
DEP.BALL = CSGEOM CSUTIL CSSYS

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
	-$(RM) $(BALL) $(OBJ.BALL) $(OUTOS)ball.dep

ifdef DO_DEPEND
dep: $(OUTOS)ball.dep
$(OUTOS)ball.dep: $(SRC.BALL)
	$(DO.DEP)
else
-include $(OUTOS)ball.dep
endif

endif # ifeq ($(MAKESECTION),targets)
