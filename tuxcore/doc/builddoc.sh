#!/bin/bash
#
# This script builds the Doxygen documentation
#
# Written by David Bourgeois <david@jaguarondi.com>
#
# $Id: builddoc.sh 490 2007-09-06 12:07:50Z jaguarondi $

# This script should be started from the base folder, not the doc folder.
if [ $(basename $PWD) == "doc" ]
then
    cd ..
fi

# Get version number from version.h.
if [ -f svnrev.h ]
then
    VERSION_MAJ=$(sed -n "s/\#define VER_MAJOR *//p" version.h)
    VERSION_MIN=$(sed -n "s/\#define VER_MINOR *//p" version.h)
    VERSION_UP=$(sed -n "s/\#define VER_UPDATE *//p" version.h)
else
    echo "Error: version.h doesn't exist, aborting."
    exit 1
fi

# Get revision number and whether we generate the documentation of a tag.
if [ -f svnrev.h ]
then
    REVISION=$(sed -n "s/\#define SVN_REV\> *//p" svnrev.h)
    URL=$(sed -n "s/\#define SVN_URL\> *//p" svnrev.h)
    # if we're in a tag folder, we don't show 'UNRELEASED'
    if [ !$(sed -n "s/\#define SVN_URL\> *//p" svnrev.h | grep tags) ]
    then
        UNRELEASED="UNRELEASED "
    fi
else
    echo "Warning: SVN information can't be found, is svnwcrev installed?"
fi

export VERSION="Version $VERSION_MAJ.$VERSION_MIN.$VERSION_UP\
        ${UNRELEASED}(Revision $REVISION)"

echo "Generating documentation for tuxcore"
echo $VERSION
doxygen doc/Doxyfile
