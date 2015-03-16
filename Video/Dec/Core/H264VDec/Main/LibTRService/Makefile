# vim: syntax=make
#
# Targets for building the component libraries.
#

SOURCES	:= \
	IVIScramble.cpp \
	tr.cpp

OBJECTS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))


ifdef MAKE_OPTIONS

	include $(MAKE_OPTIONS)

	# $(MAKE_OPTIONS) file has to define the following variables :
	# DBG
	# OPT
	# OLVL
	# WARN
	# PREPROC
	# INCDIRS
	# CXX
	# CC

else

	# OPT_DEBUG    := <false> | true
	OPT_DEBUG	   := true
	# OPT_TARGET_PLATFORM := <pentiumpro> | pentium | i386
	OPT_TARGET_PLATFORM := pentiumpro

	TOOLDIR := /home/local
	#COMPDIR := $(TOOLDIR)/gcc-2.95.3
	COMPDIR := /usr

	CXX		:= $(COMPDIR)/bin/g++
	CC		:= $(COMPDIR)/bin/gcc
	AR		:= ar
	RANLIB	:= ranlib
	LD		:= ld
	NM		:= nm

	ifeq ($(strip $(OPT_DEBUG)), true)
		DBG		:= -g -D_DEBUG
		# the following optimization options insure that inline asm works
		# correctly if no -Ox optimizations are enabled
		OLVL    :=
		OPT		:= -fasm -finline-functions -fno-exceptions -fno-rtti
	else
		DBG		:= -DNDEBUG
		OLVL	:= -O6
		OPT		:= -march=$(OPT_TARGET_PLATFORM) -mcpu=$(OPT_TARGET_PLATFORM) -fomit-frame-pointer -fno-exceptions -fno-rtti
	endif

	WARN	:= -Wall
	PREPROC	:= -D_REENTRANT -D__int64="long long" -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DRH_70 -DUNIX

	INCDIRS	:= \
		-I../../../DVDPLAYER/LIBLINUX -I../../../DVDPLAYER/LIBLINUX/include	\
		-I../../Include

	CPPFLAGS := $(DBG) $(OPT) $(OLVL) $(WARN) $(PREPROC) $(INCDIRS)
	CXXFLAGS :=

endif


ifndef LIBRARY
	LIBRARY := libtr
endif


%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $($@_XFLAGS) $< -o $@

all: $(LIBRARY).a

$(LIBRARY).a: $(OBJECTS)
	rm -f $@
	$(AR) cr $@ $^
	$(RANLIB) $@

.PHONY: depend clean

depend:
	@rm -f .depend
	@for i in $(SOURCES); do \
		$(CXX) -MM $(INCDIRS) $$i >>.depend; \
	done

clean: cleanlib cleanobj

cleanlib:
	-rm -f $(LIBRARY).a

cleanobj:
	-rm -f $(OBJECTS)

-include .depend
