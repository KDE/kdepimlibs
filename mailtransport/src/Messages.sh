#! /bin/sh
$EXTRACTRC `find . -name \*.ui -o -iname \*.kcfg` >> rc.cpp || exit 11
$XGETTEXT *.cpp -o $podir/libmailtransport5.pot
