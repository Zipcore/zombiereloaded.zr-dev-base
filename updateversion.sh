#!/bin/sh

# Note: Copy this script to the source code repository and execute it
#       from that location.

# Program for printing date.
DATEPATH='date'

UNOFFICIAL=false

if [ "$1" ]
then
    if [ "$1" = "--unofficial" ]
    then
        UNOFFICIAL=true
    else
        DATEPATH=$1
    fi
fi

PROJECT_VERSION_FILE="src/zr/base/hgversion.inc"

PROJECT_REVISION=$(hg id -n):$(hg id -i)

if [ $UNOFFICIAL = "true" ]
then
    PROJECT_REVISION="Unofficial build - based on $PROJECT_REVISION"
fi

DATE=$($DATEPATH -R)

echo "#define PROJECT_MERCURIAL_REVISION         \"$PROJECT_REVISION\"" > $PROJECT_VERSION_FILE
echo "#define PROJECT_MERCURIAL_DATE             \"$PROJECT_DATE\"" >> $PROJECT_VERSION_FILE

echo "Updated $PROJECT_VERSION_FILE"
