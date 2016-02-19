#!/bin/sh
# Use this script to populate current directory with Simics workspace contents.

# default local installation path
if [ -z $SIMICSPATH ]
then
    SIMICSPATH=/opt/simics/simics-4.8/simics-4.8.128
fi

WRKSPCSTP=$SIMICSPATH/bin/workspace-setup

if [ ! -f $WRKSPCSTP  ]
then
    echo "workspace-setup script is not found! Please set SIMICSPATH to point to the Simics Base package directory"
    exit 1
fi

$WRKSPCSTP --force
