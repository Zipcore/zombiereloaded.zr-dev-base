#!/bin/sh

cd src/

# General and individual files.
LINES_MAIN=`wc *.sp -l | cut -d ' ' -f1`
LINES_BASE=`wc zr/base/*.* -l | tail -n1 | sed 's/^ *\(.*\) *$/\1/' | cut -d ' ' -f1`
LINES_LIBRARIES=`wc zr/libraries/*.* -l | tail -n1 | sed 's/^ *\(.*\) *$/\1/' | cut -d ' ' -f1`
LINES_MODULES=`wc zr/modules/*.* -l | tail -n1 | sed 's/^ *\(.*\) *$/\1/' | cut -d ' ' -f1`
LINES_OTHER=`wc zr/*.* -l | tail -n1 | sed 's/^ *\(.*\) *$/\1/' | cut -d ' ' -f1`

# Cores
LINES_ZRC_CORE=`wc zr/modules/zrc_core/*.* -l | tail -n1 | sed 's/^ *\(.*\) *$/\1/' | cut -d ' ' -f1`
LINES_ZRIOT_CORE=`wc zr/modules/zriot_core/*.* -l | tail -n1 | sed 's/^ *\(.*\) *$/\1/' | cut -d ' ' -f1`

# Modules
LINES_WEAPONS=`wc zr/modules/weapons/*.* -l | tail -n1 | sed 's/^ *\(.*\) *$/\1/' | cut -d ' ' -f1`



LINES_TOTAL="$(($LINES_MAIN + $LINES_BASE + $LINES_BASE + $LINES_LIBRARIES + $LINES_MODULES + $LINES_OTHER + $LINES_ZRC_CORE + $LINES_ZRIOT_CORE + $LINES_WEAPONS))"

echo "Number of lines:"
echo "$LINES_MAIN\tmain sp"
echo "$LINES_BASE\tbase"
echo "$LINES_LIBRARIES\tlibraries"
echo "$LINES_MODULES\tmodules"
echo "$LINES_OTHER\tother"
echo ""
echo "$LINES_ZRC_CORE\tzrc_core"
echo "$LINES_ZRIOT_CORE\tzriot_core"
echo ""
echo "$LINES_WEAPONS\tweapons"

echo "\nTotal:"
echo "$LINES_TOTAL"
