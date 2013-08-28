#!/bin/sh
#
# SHL - git-head constant
#
# Copyright (c) 2010-2013 David Herrmann <dh.herrmann@gmail.com>
# Dedicated to the Public Domain
#

#
# Generate $1 with:
#   const char shl_githead[] = "<git-head-revision>";
# But do not touch $1 if the git-revision is already up-to-date.
#

if test "x$1" = "x" ; then
	echo "usage: ./shl_githead.sh <file>"
	exit 1
fi

#
# Check whether this is a valid git repository.
# Set ISGIT to 1=true or 0=false.
#

ISGIT=0
REV=`git rev-parse --git-dir 2>/dev/null`
if test "x$?" = "x0" ; then
	ISGIT=1
fi

#
# Check the old revision from $1.
# If no old revision exists and this is no valid git-repository, we are quite
# likely being run in a corrupted tarball. Print a warning and try creating the
# required file with a random revision.
#

if test -f "$1" ; then
	OLDREV=`cat "$1"`
else
	if test $ISGIT = 0 ; then
		RNDREV="random-rev-$((RANDOM + 32000))"
		echo "WARNING: version file $1 is missing, using: $RNDREV"
		echo "const char shl_githead[] = \"$RNDREV\";" >"$1"
		exit 0
	fi

	OLDREV=""
fi

#
# Check new revision from "git describe". However, if this is no valid
# git-repository, return success and do nothing.
#

if test $ISGIT = 0 ; then
	exit 0
fi

NEWREV=`git describe`
NEWREV="const char shl_githead[] = \"$NEWREV\";"

#
# Exit if the file is already up to date.
# Otherwise, write the new revision into the file.
#

if test "x$OLDREV" = "x$NEWREV" ; then
	exit 0
fi

echo "$NEWREV" >"$1"
