#!/bin/bash

# $Id: move-gbench.sh 15756 2008-01-22 16:53:29Z dicuccio $

install_dir=`dirname $0`
install_dir=(`cd $install_dir ; pwd`)

extra_sed=
if test "x$1" = "x--extra-sed"; then
    extra_sed=$2
    shift
    shift
fi

if [ ! -z "$1" ]
then
    dir=$1
    test -f "$dir/run-gbench.sh" || dir=`echo "$1" | sed "s,/bin$,,"`
else
    dir=`echo "$install_dir" | sed "s,/bin$,,"`
fi

sedscript="s%/netopt/ncbi_tools\.[^/ ]*%/netopt/ncbi_tools%g"
test -n "$extra_sed"  &&  sedscript="$sedscript; $extra_sed"
dir=`echo $dir | sed "$sedscript"`

echo "Preparing Genome Workbench installation for move to path $dir"

sed "s,^NCBI_GBENCH_INSTALL_DIR=.*,NCBI_GBENCH_INSTALL_DIR=$dir," $install_dir/run-gbench.sh > $install_dir/run-gbench.sh.$$
mv -f $install_dir/run-gbench.sh.$$ $install_dir/run-gbench.sh
chmod 755 $install_dir/run-gbench.sh

for ftype in auto asntext asnbin xml fasta newick textalign project workspace message; do
     sed "s,^NCBI_GBENCH_INSTALL_DIR=.*,NCBI_GBENCH_INSTALL_DIR=$dir," $install_dir/gbench_remote_template.sh | sed "s,^file_type=.*,file_type=$ftype," > $install_dir/gbench_remote_$ftype
     chmod 755 $install_dir/gbench_remote_$ftype
     sed "s,^NCBI_GBENCH_INSTALL_DIR=.*,NCBI_GBENCH_INSTALL_DIR=$dir," $install_dir/gbench_noremote_template.sh | sed "s,^file_type=.*,file_type=$ftype," > $install_dir/gbench_noremote_$ftype
     chmod 755 $install_dir/gbench_noremote_$ftype
done
