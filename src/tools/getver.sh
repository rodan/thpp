#!/bin/sh

set -e

usage()
{
    echo "$(basename "${0}") commands:"
    echo " -t TYPE"
    echo "     versioning type"
    echo " -h"
    echo "     short usage message"
    echo " supported versioning types: MAJ.MINbBUILD "
}

err_unknown()
{
    echo "unknown version"
    exit 1
}

get_from_header()
{
    ITEM=''
    ITEM=$(grep "${1}" "${input}" | gsed "s|.*${1}\s*\([0-9]\{1,9\}\).*|\1|")
    [ -z "${ITEM}" ] && {
        echo 'unknown version'
        exit 1
    }
    echo "${ITEM}"
}

majminbuild()
{
    maj=$(get_from_header 'VER_MAJOR')
    min=$(get_from_header 'VER_MINOR')
    build=$(get_from_header 'BUILD')

    echo "${maj}.${min}b${build}"
}

while [ $(( "$#" )) -gt 0 ]; do
    if [ "$1" = "-t" ]; then
        type="${2}"
        shift; shift;
    elif [ "$1" = "-i" ]; then
        input="${2}"
        shift; shift;
    else
        shift;
        usage
    fi
done

if [ "$type" = 'MAJ.MINbBUILD' ]; then
    majminbuild
else
    echo "unknown type, exiting"
    exit 1;
fi

