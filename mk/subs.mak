# This submakefile dynamically compute the name for all
# driver, libs and apps submakefiles and includes them.

ifneq ($(TARGET),)
  include mk/system/$(TARGET).mak
endif

ifeq ($(LIBRARY_SUBMAKEFILES),)
  LIBRARY_SUBMAKEFILES=$(wildcard libs/*/*.mak)
endif
ifneq ($(LIBRARY_SUBMAKEFILES),)
  include $(LIBRARY_SUBMAKEFILES)
endif

ifeq ($(DRIVER_SUBMAKEFILES),)
  DRIVER_SUBMAKEFILES=$(wildcard $(addsuffix /*.mak,$(addprefix libs/,$(DRIVERS) $(DRIVERS.SYSTEM))))
endif
ifneq ($(DRIVER_SUBMAKEFILES),)
  include $(DRIVER_SUBMAKEFILES)
endif

ifeq ($(APPLICATION_SUBMAKEFILES),)
  APPLICATION_SUBMAKEFILES=$(wildcard apps/*/*.mak)
endif
ifneq ($(APPLICATION_SUBMAKEFILES),)
  include $(APPLICATION_SUBMAKEFILES)
endif

ifeq ($(TESTS_SUBMAKEFILES),)
  TESTS_SUBMAKEFILES=$(wildcard apps/tests/*/*.mak)
endif
ifneq ($(TESTS_SUBMAKEFILES),)
  include $(TESTS_SUBMAKEFILES)
endif
