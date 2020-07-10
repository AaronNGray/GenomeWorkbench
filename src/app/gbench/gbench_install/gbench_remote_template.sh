#!/bin/sh

# $Id: gbench_remote_template.sh 15756 2008-01-22 16:53:29Z dicuccio $

NCBI_GBENCH_INSTALL_DIR=@install_dir@
export NCBI_GBENCH_INSTALL_DIR

file_type=@file_type@

exec $NCBI_GBENCH_INSTALL_DIR/bin/run-gbench.sh -t $file_type $@
