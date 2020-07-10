#!/bin/bash


src=""
dst=""

do_link=n

while [ ! -z "$1" ]
do
    case "$1" in
        --link)
        do_link=y
        ;;

        *)
        if [ -z "$src" ]
        then
            src="$1"
        elif [ -z "$dst" ]
        then
            dst="$1"
        else
            echo "unknown parameter: $1"
            exit 1
        fi
        ;;
    esac
    shift
done


if [ -e "$dst" ]
then
    rm -f "$dst"
fi

if [ "$do_link" = "y" ]
then
    mkdir -p "`dirname "$dst"`"
    ln "$src" "$dst"
else
    params="-c -D"
    if [ -x "$src" ]
    then
        params="$params -m 755"
    fi
    install $params "$src" "$dst"
fi

