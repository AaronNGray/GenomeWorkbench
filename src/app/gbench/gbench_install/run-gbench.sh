#!/bin/sh

# $Id: run-gbench.sh 21848 2010-07-28 17:18:38Z voronov $

NCBI_GBENCH_INSTALL_DIR=@install_dir@
export NCBI_GBENCH_INSTALL_DIR

NCBI_GBENCH_HOME=${NCBI_GBENCH_HOME:=$NCBI_GBENCH_INSTALL_DIR}
export NCBI_GBENCH_HOME

LD_LIBRARY_PATH=$NCBI_GBENCH_INSTALL_DIR/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH

case `uname` in
    Darwin*)
        DYLD_LIBRARY_PATH="$LD_LIBRARY_PATH"
        export DYLD_LIBRARY_PATH

        # fix around dynamic libs bug (Apple radar 3012645)
        DYLD_BIND_AT_LAUNCH=1
        export DYLD_BIND_AT_LAUNCH
        ;;
esac

# now, just exec the app
LC_NUMERIC=en_US.UTF-8 exec $NCBI_GBENCH_INSTALL_DIR/bin/gbench-bin $@
