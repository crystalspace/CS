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

vpath %.cpp plugins/aws

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

INC.AWS = $(wildcard plugins/aws/*.h) $(wildcard include/iaws/*.h)
SRC.AWS = $(filter-out plugins/aws/sllex.cpp plugins/aws/slp.cpp, \
  $(wildcard plugins/aws/*.cpp))				  \
  plugins/aws/skinlex.cpp plugins/aws/skinpars.cpp
OBJ.AWS = $(addprefix $(OUT)/,$(notdir $(SRC.AWS:.cpp=$O)))
DEP.AWS = CSUTIL CSSYS CSUTIL CSGEOM CSTOOL CSGFX

TO_INSTALL.DATA += data/awsdef.zip

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

plugins/aws/skinlex.cpp: plugins/aws/skinpars.hpp plugins/aws/skinlex.ll \
			 plugins/aws/skinpars.cpp
	flex -L -Splugins/aws/flex.skl -t plugins/aws/skinlex.ll > plugins/aws/skinlex.cpp

plugins/aws/skinpars.cpp: plugins/aws/skinpars.yy
	bison --no-lines -d -p aws -o $@ $(<)

plugins/aws/skinpars.hpp: plugins/aws/skinpars.cpp
	@if [ -f "plugins/aws/skinpars.cpp.hpp" ]; then \
	$(MV) plugins/aws/skinpars.cpp.hpp plugins/aws/skinpars.hpp; \
	fi
endif
endif
clean: awsclean
awsclean:
	-$(RM) $(AWS) $(OBJ.AWS)

ifdef DO_DEPEND
dep: $(OUTOS)/aws.dep
$(OUTOS)/aws.dep: $(SRC.AWS)
	$(DO.DEP)
else
-include $(OUTOS)/aws.dep
endif

endif # ifeq ($(MAKESECTION),targets)
