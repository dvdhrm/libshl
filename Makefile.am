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
MEMTESTS =
check_PROGRAMS =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =

#
# GIT-HEAD helper
# The file ./src/shl_githead.c contains a constant "shl_githead" which is
# defined to the string returned by "git describe". We need to adjust this
# string for every build.
#
# We have a helper-script ./src/shl_githead.sh that takes as argument the source
# file and creates it if necessary. It updates it only if the new git-describe
# string is different to the old one. So the file is only modified on changes.
# Hence, we can use it as normal dependency in this Makefile.
# However, we need to run this script on _every_ "make" invocation before any
# recipy is executed. To achieve this, we use $(shell ...) and assign it to a
# "simply expanded" variable (:=) so the shell command is executed on
# variable-declaration and not during expansion.
#
# Note that we must not clean ./src/shl_githead.c ever! If we would do so, a
# distribution tarball might delete that file and have no way to recreate it.
#
# If the helper-script is executed in a directory that is not a git-repository
# (like a distribution tarball) and shl_githead.c exists, then it does nothing
# as it expects shl_githead.c to be correctly written by "make dist".
# However, if shl_githead.c does not exist, it will print a warning and write
# an unknown random git-revision. However, this requires a writable source-dir
# and might break VPATH builds. So don't remove shl_githead.c ever from tarballs
# to avoid this mess!
#
# The $(emptyvariable) expansion below is used for broken $(shell ...)
# syntax-highlighting algorithms in many existing editors.
#

EXTRA_DIST += src/shl_githead.sh
GITHEAD:=$(shell $(emptyvariable)"$(srcdir)/src/shl_githead.sh" "$(srcdir)/src/shl_githead.c")

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
	-I $(srcdir)/src \
	$(DEPS_CFLAGS)
AM_LDFLAGS = \
	-Wl,--as-needed \
	-Wl,--gc-sections \
	-Wl,-z,relro \
	-Wl,-z,now \
	$(DEPS_LIBS)

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
	src/shl_githead.h \
	src/shl_githead.c \
	src/shl_trie.h \
	src/shl_trie.c \
	src/shl_dlist.h \
	src/shl_htable.h \
	src/shl_htable.c \
	src/shl_edbus.h \
	src/shl_edbus.c \
	src/shl_pty.h \
	src/shl_pty.c \
	src/shl_log.h \
	src/shl_log.c \
	src/shl_llog.h \
	src/shl_ring.h \
	src/shl_ring.c \
	src/shl_buf.h \
	src/shl_macro.h \
	src/shl_util.h \
	src/shl_util.c
libshl_la_CPPFLAGS = $(AM_CPPFLAGS)
libshl_la_LDFLAGS = $(AM_LDFLAGS)
libshl_la_LIBADD = $(AM_LIBADD)

#
# Tests
#

tests = \
	test_buf \
	test_dlist \
	test_edbus \
	test_githead \
	test_htable \
	test_llog \
	test_log \
	test_macro \
	test_pty \
	test_ring \
	test_trie \
	test_util

if BUILD_HAVE_CHECK
check_PROGRAMS += $(tests) test_valgrind
TESTS += $(tests) test_valgrind
MEMTESTS += $(tests)
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

test_buf_SOURCES = test/test_buf.c $(test_sources)
test_buf_CPPFLAGS = $(test_cflags)
test_buf_LDADD = $(test_libs)
test_buf_LDFLAGS = $(test_lflags)

test_dlist_SOURCES = test/test_dlist.c $(test_sources)
test_dlist_CPPFLAGS = $(test_cflags)
test_dlist_LDADD = $(test_libs)
test_dlist_LDFLAGS = $(test_lflags)

test_edbus_SOURCES = test/test_edbus.c $(test_sources)
test_edbus_CPPFLAGS = $(test_cflags)
test_edbus_LDADD = $(test_libs)
test_edbus_LDFLAGS = $(test_lflags)

test_githead_SOURCES = test/test_githead.c $(test_sources)
test_githead_CPPFLAGS = $(test_cflags)
test_githead_LDADD = $(test_libs)
test_githead_LDFLAGS = $(test_lflags)

test_htable_SOURCES = test/test_htable.c $(test_sources)
test_htable_CPPFLAGS = $(test_cflags)
test_htable_LDADD = $(test_libs)
test_htable_LDFLAGS = $(test_lflags)

test_llog_SOURCES = test/test_llog.c $(test_sources)
test_llog_CPPFLAGS = $(test_cflags)
test_llog_LDADD = $(test_libs)
test_llog_LDFLAGS = $(test_lflags)

test_log_SOURCES = test/test_log.c $(test_sources)
test_log_CPPFLAGS = $(test_cflags)
test_log_LDADD = $(test_libs)
test_log_LDFLAGS = $(test_lflags)

test_macro_SOURCES = test/test_macro.c $(test_sources)
test_macro_CPPFLAGS = $(test_cflags)
test_macro_LDADD = $(test_libs)
test_macro_LDFLAGS = $(test_lflags)

test_pty_SOURCES = test/test_pty.c $(test_sources)
test_pty_CPPFLAGS = $(test_cflags)
test_pty_LDADD = $(test_libs)
test_pty_LDFLAGS = $(test_lflags)

test_ring_SOURCES = test/test_ring.c $(test_sources)
test_ring_CPPFLAGS = $(test_cflags)
test_ring_LDADD = $(test_libs)
test_ring_LDFLAGS = $(test_lflags)

test_trie_SOURCES = test/test_trie.c $(test_sources)
test_trie_CPPFLAGS = $(test_cflags)
test_trie_LDADD = $(test_libs)
test_trie_LDFLAGS = $(test_lflags)

test_util_SOURCES = test/test_util.c $(test_sources)
test_util_CPPFLAGS = $(test_cflags)
test_util_LDADD = $(test_libs)
test_util_LDFLAGS = $(test_lflags)

test_valgrind_SOURCES = test/test_valgrind.c $(test_sources)
test_valgrind_CPPFLAGS = $(test_cflags)
test_valgrind_LDADD = $(test_libs)
test_valgrind_LDFLAGS = $(test_lflags)

VALGRIND = CK_FORK=no valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --leak-resolution=high --error-exitcode=1 --suppressions=$(top_builddir)/test.supp

# verify that test_valgrind actually leaks data
memcheck-verify: check
	$(AM_V_GEN)$(VALGRIND) --log-file=/dev/null ./test_valgrind >/dev/null ; test 1 = $$?

TPHONY += memcheck-verify

# run memcheck tests via valgrind
memcheck: memcheck-verify
	$(AM_V_GEN)for i in $(MEMTESTS) ; do \
		$(VALGRIND) --log-file=$(top_builddir)/$$i.memlog \
			$(top_builddir)/$$i >/dev/null || (echo "memcheck failed on: $$i" ; exit 1) ; \
	done

TPHONY += memcheck memcheck-verify

distcheck-hook: memcheck

#
# Phony targets
#

.PHONY: $(TPHONY)

#
# Empty .SECONDARY target causes alle intermediate files to be treated as
# secondary files. That is, they don't get deleted after make finished.
#

.SECONDARY:
