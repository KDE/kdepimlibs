#! /bin/sh
$XGETTEXT `find . src -name "*.cpp" -o -name "*.h" | grep -v "/autotests"` -o $podir/akonadinotes5.pot
