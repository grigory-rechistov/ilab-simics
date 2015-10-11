#!/usr/bin/env bash
#
# check_c_style.sh - check input C file to be conformant against code style
# Usage: check_c_style.sh <file.c>
# Return codes:
# - 0 if style is OK,
# - 1 if style violations are detected,
# - 2 if input file is not recognized as a C source/header.
#
# Dependencies:
#  - astyle http://astyle.sourceforge.net/
#  - diff https://www.gnu.org/software/diffutils/
#
# Copyright 2015 Grigory Rechistov <grigory.rechistov@phystech.edu>
#
# Released under the BSD license: http://opensource.org/licenses/BSD-2-Clause
#

set -e
trap cleanup SIGINT SIGTERM EXIT

TEMP="$(mktemp)"

# It was found that GNU indent results are unstable, i.e., formatting
# changes subtly between consequent runs. You may never
# INDENT=indent
# INDENTOPTS="-l80 -i8 -ts8 -nhnl -nut -br -brf -brs -ce -bbb -nprs -ncs -npcs"

# Use Artistic Style formatter. Older versions (2.01 shipped with Ubuntu 12.04)
# do not have an option to limit line length

INDENT=astyle
# Java style with eight spaces indentation
INDENTOPTS="--mode=c --style=java -s8"


function usage {
    echo "Usage: $0 <file.c>"
    exit 127
}

function cleanup {
    if [ -f $TEMP ]; then
        rm -rf $TEMP
    fi
}

if  ( [[ -z $1 ]] || [[ ! -f $1 ]] ) ; then
    usage
fi

# Check input file to have an extension either .c or .h
# It is rather naïve approach but it should be enough.
# An alternative would be to run `file` to attempt to sense for file contents,
# yet `file` is prone to give false negatives/positives.

FILE=$1
EXT="${FILE##*.}"

if ( [[ ! $EXT = "c" ]] && [[ ! $EXT = "h" ]] ) ; then
    echo "Not a C source/header"
    exit 2
fi

# Process input file with indent, and compare the result against the same file

$INDENT $INDENTOPTS < $FILE > $TEMP

( diff -q $FILE $TEMP > /dev/null ) || {
    echo "Indentation violations for $FILE detected:"
    diff -u $FILE $TEMP || false
    exit 1
}

exit 0
