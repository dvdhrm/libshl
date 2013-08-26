#
# libshl - Global Makefile
# Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
#

#
# Global Configurations and Initializations
#

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory
AUTOMAKE_OPTIONS = color-tests
AM_DISTCHECK_CONFIGURE_FLAGS = \
	--enable-debug \
	--enable-optimizations

SUBDIRS = .

.DELETE_ON_ERROR:

include_HEADERS =
EXTRA_DIST = \
	README \
	COPYING \
	NEWS
CLEANFILES =
TPHONY =

TESTS =
check_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =

#
# Default CFlags
# Make all files include "config.h" by default. This shouldn't cause any
# problems and we cannot forget to include it anymore.
#
# Also make the linker discard all unused symbols.
#
# When compiling in debug mode, we enable debug symbols so debugging with gdb
# is easier. If optimizations are disabled, we pass -O0 to the compiler.
# Otherwise, we use standard optimizations -O2.
#

AM_CFLAGS = \
	-Wall \
	-pipe \
	-fno-common \
	-ffast-math \
	-fdiagnostics-show-option \
	-fno-strict-aliasing \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections \
	-fstack-protector
AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-I $(srcdir)/src
AM_LDFLAGS = \
	-Wl,--as-needed \
	-Wl,--gc-sections \
	-Wl,-z,relro \
	-Wl,-z,now

if BUILD_ENABLE_DEBUG
AM_CFLAGS += -g
endif

if BUILD_ENABLE_OPTIMIZATIONS
AM_CFLAGS += -O2
else
AM_CFLAGS += -O0
endif

#
# SHL - Static Helper Library
# The SHL subsystem contains several small code pieces.
#

noinst_LTLIBRARIES += libshl.la

libshl_la_SOURCES = \
	src/shl_trie.h \
	src/shl_trie.c \
	src/shl_dlist.h \
	src/shl_htable.h \
	src/shl_htable.c
libshl_la_CPPFLAGS = $(AM_CPPFLAGS)
libshl_la_LDFLAGS = $(AM_LDFLAGS)
libshl_la_LIBADD = $(AM_LIBADD)

#
# Tests
#

tests = \
	test_trie \
	test_dlist \
	test_htable

if BUILD_HAVE_CHECK
check_PROGRAMS += $(tests)
TESTS += $(tests)
endif

test_sources = \
	test/test_common.h
test_libs = \
	libshl.la \
	$(CHECK_LIBS)
test_cflags = \
	$(AM_CPPFLAGS) \
	$(CHECK_CFLAGS)
test_lflags = \
	$(AM_LDFLAGS)

test_trie_SOURCES = test/test_trie.c $(test_sources)
test_trie_CPPFLAGS = $(test_cflags)
test_trie_LDADD = $(test_libs)
test_trie_LDFLAGS = $(test_lflags)

test_dlist_SOURCES = test/test_dlist.c $(test_sources)
test_dlist_CPPFLAGS = $(test_cflags)
test_dlist_LDADD = $(test_libs)
test_dlist_LDFLAGS = $(test_lflags)

test_htable_SOURCES = test/test_htable.c $(test_sources)
test_htable_CPPFLAGS = $(test_cflags)
test_htable_LDADD = $(test_libs)
test_htable_LDFLAGS = $(test_lflags)

#
# Phony targets
#

.PHONY: $(TPHONY)

#
# Empty .SECONDARY target causes alle intermediate files to be treated as
# secondary files. That is, they don't get deleted after make finished.
#

.SECONDARY:
