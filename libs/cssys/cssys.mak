#
# This submakefile requires the following variables defined in
# system-dependent makefile:
#
# SRC.SYS_CSSYS
#   - all system-dependent source files that should be included in cssys
#     library
#

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/libs/cssys $(filter-out libs/cssys/general/, $(sort $(dir $(SRC.SYS_CSSYS)))) libs/cssys/general
vpath %.c   $(SRCDIR)/libs/cssys $(filter-out libs/cssys/general/, $(sort $(dir $(SRC.SYS_CSSYS)))) libs/cssys/general

INC.CSSYS = $(wildcard $(addprefix $(SRCDIR)/,include/cssys/*.h)) $(INC.SYS_CSSYS)
SRC.CSSYS = $(wildcard $(addprefix $(SRCDIR)/,libs/cssys/*.cpp)) $(SRC.SYS_CSSYS)

# Platform-specific makefiles may want to provide their own value for
# OBJ.CSSYS (for instance, they might recognize other file types in addition
# to .s, .c, and .cpp), so we set it only if not already set by the
# platform-specific makefile.
ifeq (,$(strip $(OBJ.CSSYS)))
OBJ.CSSYS = $(addprefix $(OUT)/,$(notdir \
  $(subst .s,$O,$(subst .c,$O,$(SRC.CSSYS:.cpp=$O)))))
endif

endif # ifeq ($(MAKESECTION),postdefines)
