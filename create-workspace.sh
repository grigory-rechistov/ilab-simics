#!/bin/sh
# Use this script to populate current directory with Simics workspace contents.

if [ -z $SIMICSPATH ]
then
    SIMICSPATH=/home/karfly/Documents/simics/simics-4.6/simics-4.6.100
fi

WRKSPCSTP=$SIMICSPATH/bin/workspace-setup
if [ ! -f $WRKSPCSTP  ]
then
    echo "workspace-setup script is not found! Please set SIMICSPATH to point to the Simics Base package directory"
    exit 1
fi

$WRKSPCSTP --force
