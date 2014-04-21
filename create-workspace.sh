#!/bin/sh

if [ -z $SIMICSPATH ]
then
    SIMICSPATH=/opt/simics/simics-4.6/simics-4.6.100
fi

$SIMICSPATH/bin/workspace-setup --force

