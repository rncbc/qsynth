#!/bin/bash
# create-dmg is available from Homebrew and https://github.com/create-dmg/create-dmg

NAME="qsynth"
FULLNAME="${NAME}-0.9.91-1.1.mac-x64"
CURDIR=$(pushd `dirname $0`>/dev/null; pwd; popd>/dev/null)
BINDIR=`pwd`

if [ ! -d "${BINDIR}/${NAME}.app" ]; then
    echo "ERROR. Missing bundle:  ${BINDIR}/${NAME}.app"
    exit
fi

create-dmg \
--volname ${FULLNAME} \
--background "${CURDIR}/../images/dmg_background.png" \
--eula "${CURDIR}/../win32/gpl-2.0.rtf" \
--window-pos 200 120 \
--window-size 600 450 \
--icon ${NAME} 150 218 \
--icon-size 100 \
--app-drop-link 450 218 \
${BINDIR}/${FULLNAME}.dmg \
${BINDIR}/${NAME}.app
