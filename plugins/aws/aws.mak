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
  AWS = $(OUTDLL)aws$(DLL)
  LIB.AWS = $(foreach d,$(DEP.AWS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(AWS)
else
  AWS = $(OUT)$(LIB_PREFIX)aws$(LIB)
  DEP.EXE += $(AWS)
  SCF.STATIC += aws
  TO_INSTALL.STATIC_LIBS += $(AWS)
endif

INC.AWS = $(wildcard plugins/aws/*.h)
SRC.AWS = $(wildcard plugins/aws/*.cpp) plugins/video/canvas/common/graph2d.cpp plugins/video/renderer/null/null_txt.cpp plugins/video/renderer/common/polybuf.cpp  plugins/video/canvas/common/protex2d.cpp plugins/video/canvas/common/scrshot.cpp plugins/video/renderer/common/txtmgr.cpp plugins/video/renderer/common/vbufmgr.cpp

OBJ.AWS = $(addprefix $(OUT),$(notdir $(SRC.AWS:.cpp=$O)))
DEP.AWS = CSUTIL CSSYS CSUTIL CSGEOM CSTOOL CSGFX

MSVC.DSP += AWS
DSP.AWS.NAME = aws
DSP.AWS.TYPE = plugin
# The following is additional info need by BisonFlex for VC
DSP.AWS.RESOURCES = $(wildcard plugins/aws/*.bsn) $(wildcard plugins/aws/*.flx)
DSP.AWS.CFLAGS = /D "YY_NEVER_INTERACTIVE"
DSP.AWS.LIBS = libflex

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: aws awsclean
aws: $(OUTDIRS) $(AWS)

$(AWS): $(OBJ.AWS) $(LIB.AWS)
	$(DO.PLUGIN)

clean: awsclean
awsclean:
	-$(RM) $(AWS) $(OBJ.AWS)

ifdef DO_DEPEND
dep: $(OUTOS)aws.dep
$(OUTOS)aws.dep: $(SRC.AWS)
	$(DO.DEP)
else
-include $(OUTOS)aws.dep
endif

endif # ifeq ($(MAKESECTION),targets)
