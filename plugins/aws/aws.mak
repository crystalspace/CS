#------------------------------------------------------------------------------
# Alternate Window Manager plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.aws = Alternate Windowing System plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make aws          Make the $(DESCRIPTION.aws)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: aws awsclean
all plugins: aws

aws:
	$(MAKE_TARGET) MAKE_DLL=yes
awsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/aws

ifeq ($(USE_PLUGINS),yes)
  AWS = $(OUTDLL)/aws$(DLL)
  LIB.AWS = $(foreach d,$(DEP.AWS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(AWS)
else
  AWS = $(OUT)/$(LIB_PREFIX)aws$(LIB)
  DEP.EXE += $(AWS)
  SCF.STATIC += aws
  TO_INSTALL.STATIC_LIBS += $(AWS)
endif

INF.AWS = $(SRCDIR)/plugins/aws/aws.csplugin
INC.AWS = $(wildcard $(addprefix $(SRCDIR)/,plugins/aws/*.h include/iaws/*.h))
# We add skinlex.cpp and skinpars.cpp explicitly since they might not be
# present (thus need to be regenerated automatically).  We use $(sort) for its
# side-effect of folding out duplicate entries to ensure that skinlex.cpp and
# skinpars.cpp only appear in the list once each.
SRC.AWS = $(sort $(wildcard $(SRCDIR)/plugins/aws/*.cpp) \
  $(addprefix $(SRCDIR)/,plugins/aws/skinlex.cpp plugins/aws/skinpars.cpp))
OBJ.AWS = $(addprefix $(OUT)/,$(notdir $(SRC.AWS:.cpp=$O)))
DEP.AWS = CSUTIL CSUTIL CSGEOM CSTOOL CSGFX

TO_INSTALL.DATA += $(SRCDIR)/data/awsdef.zip

MSVC.DSP += AWS
DSP.AWS.NAME = aws
DSP.AWS.TYPE = plugin
DSP.AWS.CFLAGS = /D "YY_NEVER_INTERACTIVE"

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: aws awsclean
aws: $(OUTDIRS) $(AWS)

$(AWS): $(OBJ.AWS) $(LIB.AWS)
	$(DO.PLUGIN)

ifneq (,$(FLEXBIN))
ifneq (,$(BISONBIN))

$(SRCDIR)/plugins/aws/skinlex.cpp: \
  $(SRCDIR)/plugins/aws/skinpars.hpp \
  $(SRCDIR)/plugins/aws/skinlex.ll \
  $(SRCDIR)/plugins/aws/skinpars.cpp
	flex -L -S$(SRCDIR)/plugins/aws/flex.skl \
	-t $(SRCDIR)/plugins/aws/skinlex.ll > $(SRCDIR)/plugins/aws/skinlex.cpp

$(SRCDIR)/plugins/aws/skinpars.cpp: $(SRCDIR)/plugins/aws/skinpars.yy
	bison --no-lines -d -p aws -o $@ $(<)

$(SRCDIR)/plugins/aws/skinpars.hpp: $(SRCDIR)/plugins/aws/skinpars.cpp
	@if [ -f "$(SRCDIR)/plugins/aws/skinpars.cpp.hpp" ]; then \
	$(MV) $(SRCDIR)/plugins/aws/skinpars.cpp.hpp \
	$(SRCDIR)/plugins/aws/skinpars.hpp; \
	fi
endif
endif
clean: awsclean
awsclean:
	-$(RMDIR) $(AWS) $(OBJ.AWS) $(OUTDLL)/$(notdir $(INF.AWS))

ifdef DO_DEPEND
dep: $(OUTOS)/aws.dep
$(OUTOS)/aws.dep: $(SRC.AWS)
	$(DO.DEP)
else
-include $(OUTOS)/aws.dep
endif

endif # ifeq ($(MAKESECTION),targets)
