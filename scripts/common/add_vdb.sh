#!/bin/sh
# $Id: add_vdb.sh 515345 2016-09-30 13:43:54Z ucko $
. "`dirname $0`/common.sh"

set -e
exec 3>&1 >&2
root=`dirname $0`/../../local
mkdir -p $root
cd $root
root=`pwd`
ptb_ini=../src/build-system/project_tree_builder.ini
tag=`sed -ne 's,.*/vdb-versions/,,p' $ptb_ini`
name=ncbi-vdb${tag:+"-$tag"}
platform=`COMMON_DetectPlatform`

mkdir -p src
cd src
if [ -d ncbi-vdb/.git ]; then
    (cd ncbi-vdb  &&  git fetch --tags origin master)
else
    git clone https://github.com/ncbi/ncbi-vdb.git
fi
if [ -d ngs/.git ]; then
    (cd ngs  &&  git pull)
else
    git clone https://github.com/ncbi/ngs.git
fi
cd ncbi-vdb
git checkout "${tag:-master}" || git checkout master
if [ ! -d interfaces ]; then
    echo "WARNING: tag $tag unusable; trying master."
    name=ncbi-vdb
    git checkout master
fi
if [ "$platform" = IntelMAC ]; then
    # archflag=--arch=fat86
    if [ ! -d $root/tmp/force-clang ]; then
        mkdir -p $root/tmp/force-clang
        ln -s /usr/bin/clang $root/tmp/force-clang/gcc
    fi
    PATH=$root/tmp/force-clang:$PATH
else
    archflag=
fi
./configure --prefix=$root/$name --build-prefix=$root/build/$name $archflag \
    ${LIBXML_LIBPATH:+"LDFLAGS=$LIBXML_LIBPATH"}
make

for x in lib lib64; do
    if [ -L $root/$name/$x ]; then
        rm $root/$name/$x
    fi
done
make install
if [ -f $root/$name/include/klib/rc.h ]; then
    :
else
    (cd interfaces  &&  tar cf - *)  |  (cd $root/$name/include  &&  tar xf -)
fi
if [ -d $root/$name/lib32_64 ]; then
    for x in lib lib64; do
        [ -d $root/$name/$x ] || ln -s lib32_64 $root/$name/$x
    done
fi
cat <<EOF

----------------------------------------------------------------------
NCBI VDB libraries successfully built and installed to
$root/$name

To take advantage of them, please configure the C++ Toolkit
--with-vdb=$root/$name
----------------------------------------------------------------------
EOF

echo $root/$name >&3
