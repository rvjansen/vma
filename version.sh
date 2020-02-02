#!/bin/bash
#
# A simple versioning system
#
    YEAR=$(date '+%y')
    DAY=$(date '+%j')
    DATE=$YEAR.$DAY

    if [ ! -f .version -o ! -f .revision ]
    then
        echo >.version  "$DATE"
        echo >.revision "1"
    fi
        
    VER=$(cat .version)
    REV=$(cat .revision)

    case "$1" in
        -u)
            if [ $(expr "$DATE" \> "$VER") -eq 1 ]
            then
                VER="$DATE"
                echo >.version  "$VER"
                echo >.revision "1"
            else
                REV=$(expr $REV \+ 1)
                echo >.revision "$REV"
            fi

            shift
        ;;
    esac

    REVS=" abcdefghijklmnopqrstuvwxyz"
    VERSION=$(echo $VER${REVS:$REV:1})
    echo $VERSION

    if [ -n "$1" ]
    then
        cat <<EOF >$1/version.h
#if !defined( _VERSION_H )
#define _VERSION_H
#define LIBVER  "$VERSION"
#define VERSION wxT( "$VERSION" )
#define FILEVER $YEAR,$DAY,0,$REV
#define PRODVER $YEAR,$DAY,0,$REV
#define STRFILEVER "$VERSION\0"
#define STRPRODVER "$VERSION\0"
#endif
EOF
    fi

exit 0
