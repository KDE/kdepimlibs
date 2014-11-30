#! /bin/sh
$EXTRACTRC `find . -name \*.rc` >> rc.cpp || exit 11
$XGETTEXT `find . -name "*.cpp" -o -name "*.h"` -o $podir/libakonadi-kmime5.pot
