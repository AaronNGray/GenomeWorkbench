#!/bin/bash

config_sviewer=n
config_gbench=n

while [ ! -z "$1" ]
do
    case "$1" in
        --sviewer)
            config_sviewer=y
            ./configure \
                --without-debug \
                --without-dll \
                --with-mt \
                --with-internal \
                --with-projects=scripts/projects/ncbi_gbench.lst \
                --with-distcc --without-ccache
                ;;

        --gbench)
            config_gbench=y
            ./configure \
                --without-debug \
                --with-gbench \
                --with-dll \
                --with-mt \
                --with-internal \
                --with-projects=scripts/projects/ncbi_gbench.lst \
                --with-distcc --without-ccache
            ;;

    esac
    shift
done

if [ "$config_sviewer" = "n" -a "$config_gbench" = "n" ]
then
    echo "one of --sviewer or --gbench must be provided"
    exit 1
fi

