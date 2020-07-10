#!/bin/bash -e

function read_version()
{
    local re='[[:blank:]]*=[[:blank:]]*([[:digit:]]{1,2}).*'

    local eMajor=""
    local eMinor=""
    local ePatchLevel=""
    local versionFile="include/gui/objects/gbench_version.hpp"

    if [ -e $versionFile ] ; then
        while read line
        do
            if [[ $line =~ eMajor$re ]] ; then
                eMajor=${BASH_REMATCH[1]}
            fi
            if [[ $line =~ eMinor$re ]] ; then
                eMinor=${BASH_REMATCH[1]}
            fi
            if [[ $line =~ ePatchLevel$re ]] ; then
                ePatchLevel=${BASH_REMATCH[1]}
            fi
        done < "$versionFile"
    fi

    if [ -z "eMajor" -o  -z "$eMinor" -o -z "$ePatchLevel" ] ; then
        echo "Failed to read version from \"$versionFile\"."
        exit 1
    fi

    local __resultvar=$1
    local myresult="$eMajor.$eMinor.$ePatchLevel"
    eval $__resultvar="'$myresult'"
}

read_version version
release=1

prefix=/opt/ncbi
debug=false
build_deb=false
build_rpm=false
build_snap=false

threads=2
if [ -e "/proc/cpuinfo" ] ; then
	threads=`grep -c ^processor /proc/cpuinfo`
	echo "Found $threads cpu cores"
fi

os="linux"
if [ -e /etc/os-release ] ; then
	source /etc/os-release
	os=$ID
	ver=$VERSION_ID
	echo "Detected $os $ver"
fi

case "$os" in
	debian)
		build_deb=true
		sudo apt-get update
		sudo apt-get -y install git build-essential libgtk2.0-dev libgl1-mesa-dev \
		libglu1-mesa-dev libbz2-dev libxml2-dev libxslt1-dev libdb5.3-dev libftgl-dev \
		libglew-dev libboost-dev liblzo2-dev libsqlite3-dev libgnutls28-dev \
		flex bison gnupg2	        		
		;;
	centos)
		build_rpm=true
		sudo yum -y install epel-release
		sudo yum -y install git make automake gcc gcc-c++ kernel-devel wget rpm-build rpm-sign pinentry \
		    	gtk2-devel gtk+-devel mesa-libGL-devel mesa-libGLU-devel \
		        libdb-devel libxml2-devel libxslt-devel libsq3-devel ftgl-devel glew-devel \
		        boost-devel lzo-devel gnutls-devel libidn2-devel libunistring-devel trousers-devel \
		        flex bison centos-release-scl gnupg2 rpm-sign expect
		sudo yum -y install devtoolset-7
	        source scl_source enable devtoolset-7 || :
	        ;;
	ubuntu)
	        build_deb=true
	        sudo apt-get update
	        sudo apt-get -y install git build-essential libgtk2.0-dev libgl1-mesa-dev \
	             libglu1-mesa-dev libbz2-dev libxml2-dev libxslt1-dev libdb5.3-dev libftgl-dev \
	             libglew-dev libboost-dev liblzo2-dev libsqlite3-dev libgnutls28-dev \
		     flex bison gnupg2
	        ;;
	fedora)
	        build_rpm=true
	        sudo dnf -y install git make automake gcc gcc-c++ kernel-devel wget rpm-build rpm-sign pinentry \
	                            gtk2-devel gtk+-devel mesa-libGL-devel mesa-libGLU-devel \
	                            libdb-devel libxml2-devel libxslt-devel libsq3-devel ftgl-devel glew-devel \
	                            boost-devel lzo-devel gnutls-devel libidn2-devel libunistring-devel trousers-devel \
	                            flex bison libstdc++-static gnupg2 rpm-sign expect
	       ;;
	opensuse-leap)
	       build_rpm=true
	       sudo zypper  --gpg-auto-import-keys refresh
	       sudo zypper  --gpg-auto-import-keys install -y make automake gcc gcc-c++ kernel-devel wget \
	            gtk2-devel Mesa-libGL-devel \
	            libdb-4_8-devel libxml2-devel libxslt-devel sqlite3-devel ftgl-devel glew-devel \
	            boost-devel lzo-devel git-core libgnutls-devel flex bison gpg2 bzip2 expect
               sudo zypper  --gpg-auto-import-keys install -y rpm-build || :
	       ;;
esac

while [ ! -z "$1" ]
do
    case "$1" in
        --version=*)
            echo "Warning: --version parameter ignored."
            ;;

        --build=*)
            release=`echo $1 | sed 's/--build=//'`
            ;;

        --no-svn)
            ;;

        --with-debug)
            debug=true
            ;;

        --without-debug)
            debug=false
            ;;

        --build-deb)
            build_deb=true
            ;;

        --build-rpm)
            build_rpm=true
            ;;

        --build-snap)
            build_snap=true
            ;;
            
        --threads=*)
            threads=`echo $1 | sed 's/--threads=//'`
            ;;
            
        --prefix=*)
            prefix=`echo $1 | sed 's/--prefix=//' | sed 's/\/*$//'`
            ;;

        *)
            echo "option not understood: $1"
            exit 1
            ;;
    esac
    shift
done

if [[ -z "$release" || ! $release =~ ^[0-9]+$ ]]; then
  echo "Invalid build value: $release"
  exit 1
fi

case $prefix in
    /*)
    ;;

    *)
    echo "prefix should be an absolute path: \"$prefix\""
    exit 1
    ;;
esac

GBENCH_DIR=gbench-$version
BUILD_PATH=`pwd`
ARCH=`uname -m`

if [[ $ARCH =~ i[3-6]86 ]]; then
    ARCH=i386
fi

GBENCH_TAR=gbench-linux-$ARCH-$version-$release.tgz

if [ $debug == 'false' ] ; then
    ARCH_DIR=${ARCH}Release
else
    ARCH_DIR=${ARCH}Debug
fi

RPM_ROOT=rpm-build

SPEC_FILE=ncbi-gbench.spec
DEB_CONTROL_FILE=ncbi-gbench.control

THIRD_PARTY_DIR=ThirdParty
THIRD_PARTY_PATH=$BUILD_PATH/$THIRD_PARTY_DIR

WXWIDGETS_VER=3.1.3
WXWIDGETS_TAR=wxWidgets-$WXWIDGETS_VER.tar.bz2
WXWIDGETS_DIR=wxWidgets-$WXWIDGETS_VER
#WXWIDGETS_LINK=ftp://ftp.ncbi.nlm.nih.gov/toolbox/gbench/wxWidgets/$WXWIDGETS_TAR
WXWIDGETS_LINK=https://github.com/wxWidgets/wxWidgets/releases/download/v$WXWIDGETS_VER/$WXWIDGETS_TAR

clear_install()
{
    cd $BUILD_PATH
    rm -rf $THIRD_PARTY_DIR
}

vdb_install()
{
    cd $BUILD_PATH

    if [ ! -d $THIRD_PARTY_DIR ]; then
        mkdir $THIRD_PARTY_DIR
    fi

    cd $THIRD_PARTY_DIR

    echo "Downloading and installing NCBI VDB..."

    local ptb_ini=$BUILD_PATH/src/build-system/project_tree_builder.ini
    local tag=`sed -ne 's,.*/vdb-versions/,,p' $ptb_ini`

    rm -rf ncbi-vdb
    rm -rf ngs

    git clone https://github.com/ncbi/ncbi-vdb.git
    git clone https://github.com/ncbi/ngs.git

    cd ncbi-vdb
    git checkout "${tag:-master}"

    local vdb_install_path=$THIRD_PARTY_PATH/$ARCH_DIR

    #Fix perl issue (Debian 9)
    export PERL5LIB=.
    ./configure --prefix=$vdb_install_path --build-prefix=$THIRD_PARTY_PATH/build/ncbi-vdb
    make
    make install

    if [ -f $vdb_install_path/klib/rc.h ]; then
        :
    else
        (cd interfaces  &&  tar cf - *)  |  (cd $vdb_install_path/include  &&  tar xf -)
    fi
}

wxwidgets_install()
{
    if [ $debug == 'false' ] ; then
        cfg_dir=gb-release
    else
        cfg_dir=gb-debug
    fi

    if [ -e $THIRD_PARTY_PATH/$ARCH_DIR/lib/libwx_baseu-2.9.so ]; then
        return
    fi

    cd $BUILD_PATH

    if [ ! -d $THIRD_PARTY_DIR ]; then
        mkdir $THIRD_PARTY_DIR
    fi

    cd $THIRD_PARTY_DIR

    echo "Downloading and installing wxWidgets $WXWIDGETS_VER..."

    rm -rf $WXWIDGETS_DIR

    if [ ! -f $WXWIDGETS_TAR ]; then
        wget --no-check-certificate $WXWIDGETS_LINK
    fi

    bzip2 -tv $WXWIDGETS_TAR

    tar xjf $WXWIDGETS_TAR && cd $WXWIDGETS_DIR && mkdir $cfg_dir && cd $cfg_dir

    if [ $debug == 'false' ] ; then
        local d='--disable-debug'
    else
        local d='--enable-debug --enable-debug_gdb'
    fi

    local configure="../configure \
--prefix=$THIRD_PARTY_PATH/$ARCH_DIR --libdir=$THIRD_PARTY_PATH/$ARCH_DIR/lib \
--with-opengl --enable-shared --disable-monolithic --enable-sockets --enable-unicode --enable-filesystem --enable-fs_inet $d"
    echo -e "\nConfiguring wxWidgets at `pwd`\n"
    echo -e "$configure\n"

    $configure && make -j $threads && make install
}

gbench_build()
{
    cd $BUILD_PATH

    if [ $debug == 'false' ] ; then
        local d='--without-debug'
    else
        local d='--with-debug'
    fi

    local configure="./configure \
--with-dll --with-mt --with-gbench --with-vdb=$THIRD_PARTY_PATH/$ARCH_DIR $d \
--with-boost --with-libxml --with-libxslt --with-sqlite3 --with-lzo --with-ftgl \
--with-wxwidgets=$THIRD_PARTY_PATH/$ARCH_DIR --with-wxwidgets-ucs \
--with-projects=scripts/projects/ncbi_gbench.lst --without-glew-mx --with-bin-release \
--with-runpath=$prefix/gbench-$version/lib --without-sybase --with-flat-makefile --without-caution --without-internal"

    echo -e "\nConfiguring Genopme Workbench at `pwd`\n"
    echo -e "$configure\n"
    $configure

    if [ $? -ne 0 ]; then
      echo "Error: Failed to configure GBench."
      exit 1
    fi

    if [ $debug == 'false' ] ; then
        BUILD_DIR=`ls | grep ReleaseMTDLL`
    else
        BUILD_DIR=`ls | grep DebugMTDLL`
    fi

    cd $BUILD_DIR/build && make -j $threads all_p

    if [ $? -ne 0 ]; then
      echo "Error: Failed to build GBench."
      exit 1
    fi

    if [ $THIRD_PARTY_PATH/$ARCH_DIR/lib ]; then
        cp -d $THIRD_PARTY_PATH/$ARCH_DIR/lib/lib* $BUILD_PATH/$BUILD_DIR/bin/gbench/lib
    fi

    if [ $THIRD_PARTY_PATH/$ARCH_DIR/lib64 ]; then
        cp -d $THIRD_PARTY_PATH/$ARCH_DIR/lib64/lib* $BUILD_PATH/$BUILD_DIR/bin/gbench/lib
    fi
}

build_rpm_tree()
{
    cd $BUILD_PATH

    rm -rf $RPM_ROOT
    mkdir $RPM_ROOT && cd $RPM_ROOT
    mkdir RPMS
    mkdir SOURCES
    mkdir SPECS
    mkdir SRPMS
    mkdir -p BUILD/$GBENCH_DIR$prefix/$GBENCH_DIR
    mkdir -p BUILD/$GBENCH_DIR/usr/share/applications
    
}

run_rpm()
{
    cd $BUILD_PATH
    sed -e "s!^/opt/ncbi/gbench-%{version}\$!$prefix/gbench-%{version}!" \
        -e "s!^Version: *\$version!Version:    $version!" \
        -e "s!^Release: *\$release!Release:    $release!" \
      <src/app/gbench/gbench_install/$SPEC_FILE >$RPM_ROOT/SPECS/$SPEC_FILE

    cat <<EOF >$HOME/.rpmmacros
%_topdir $BUILD_PATH/$RPM_ROOT
%_tmpdir %{_topdir}/tmp
%packager Roman Katargin
%_signature gpg
%_gpg_name Roman Katargin
EOF

    cd $RPM_ROOT    
    rpmbuild -bb SPECS/$SPEC_FILE

    cd $BUILD_PATH
    ln -f $RPM_ROOT/RPMS/$ARCH/*.rpm gbench-${os}${ver}-$ARCH-$version-$release.rpm

    if [ -e ../gbsign.tgz ] ; then
        tar -xzf ../gbsign.tgz 
        gpg --list-keys >/dev/null
        rm -rf ~/.gnupg/*
        cp gbsign/gnupg/* ~/.gnupg/
        sudo rpm --import gbsign/RPM-GPG-KEY-katargir
        echo "To sign the rpm package and verify the signature please run the following commands:"
        echo "rpm --addsign gbench-${os}${ver}-$ARCH-$version-$release.rpm"
        echo "rpm -qpi gbench-${os}${ver}-$ARCH-$version-$release.rpm | grep Signature"
    fi
}

copy_fils_to_rpm()
{
    cd $BUILD_PATH

    if [ $debug == 'false' ] ; then
        BUILD_DIR=`ls | grep ReleaseMTDLL`
    else
        BUILD_DIR=`ls | grep DebugMTDLL`
    fi

    cd $BUILD_DIR/bin/gbench
     
    echo copy_files_to_rpm $BUILD_DIR `pwd`
   
    cp -r * $BUILD_PATH/$RPM_ROOT/BUILD/$GBENCH_DIR$prefix/$GBENCH_DIR

    sed -e "s!\$PATH!$prefix/gbench-$version!" \
         <$BUILD_PATH/src/app/gbench/gbench_install/ncbi-gbench.desktop >$BUILD_PATH/$RPM_ROOT/BUILD/$GBENCH_DIR/usr/share/applications/ncbi-gbench.desktop

    cp $BUILD_PATH/src/app/gbench/gbench_install/gbench-req.sh $BUILD_PATH/$RPM_ROOT/BUILD/gbench-req
    chmod +x $BUILD_PATH/$RPM_ROOT/BUILD/gbench-req

    find $BUILD_PATH/$RPM_ROOT/BUILD/$GBENCH_DIR$prefix/$GBENCH_DIR/lib/* -type f | xargs chmod 0644
    
    if [ $build_snap == 'true' ] ; then
        local snap_prefix='$SNAP'
    else
        local snap_prefix=''
    fi

    cd bin
    sed "s:^\(NCBI_GBENCH_INSTALL_DIR=\).*$:\1$snap_prefix$prefix/gbench-$version:" <gbench >$BUILD_PATH/$RPM_ROOT/BUILD/$GBENCH_DIR$prefix/$GBENCH_DIR/bin/gbench
}

build_rpm_sources()
{
    echo "Creating tar file $GBENCH_TAR..."
    
    if [ -e $BUILD_PATH/$GBENCH_TAR ]; then
        rm $BUILD_PATH/$GBENCH_TAR
    fi

    tar --directory=$BUILD_PATH/$RPM_ROOT/BUILD/$GBENCH_DIR -czf $BUILD_PATH/$GBENCH_TAR .
    ln -f $BUILD_PATH/$GBENCH_TAR $BUILD_PATH/$RPM_ROOT/SOURCES/
}

build_deb()
{
    DEB_HOST_ARCH=`dpkg-architecture -qDEB_HOST_ARCH`

    cd $BUILD_PATH
    rm -rf $RPM_ROOT/BUILD/$GBENCH_DIR/DEBIAN

    mkdir $RPM_ROOT/BUILD/$GBENCH_DIR/DEBIAN

    local libs=$(find $RPM_ROOT/BUILD/$GBENCH_DIR/opt/ncbi -type f | $BUILD_PATH/$RPM_ROOT/BUILD/gbench-req | sed 's/().*$//')

    local packages=$(
    for f in $libs; do
        dpkg -S $f 2>/dev/null | cut -d: -f1
    done | sort -u)

    local depends=$(
    for p in $packages; do
        echo "$p (>=`dpkg -s $p | grep Version | sed 's/Version: //'`),"
    done)
    depends=$(echo $depends | sed 's/,$//')

    sed -e "s!^Architecture: *\$ARCH!Architecture: $DEB_HOST_ARCH!" \
        -e "s!^Version: *\$version!Version: $version-$release!" \
        -e "s!^Depends: *\$depends!Depends: $depends!" \
         <src/app/gbench/gbench_install/$DEB_CONTROL_FILE >$BUILD_PATH/$RPM_ROOT/BUILD/$GBENCH_DIR/DEBIAN/control

    fakeroot dpkg-deb --build $RPM_ROOT/BUILD/$GBENCH_DIR gbench-${os}${ver}-$DEB_HOST_ARCH-$version-$release.deb

# cleanup
   rm -rf $BUILD_PATH/$RPM_ROOT/BUILD/$GBENCH_DIR/DEBIAN
}

build_snap()
{
    echo "Building snap package..."
    cd $BUILD_PATH
    local snap_path='snap'
    rm -rf $snap_path; mkdir $snap_path; cd $snap_path
    ln $BUILD_PATH/$GBENCH_TAR
    sed -e "s!^version: *\$version!version: $version!" <$BUILD_PATH/src/app/gbench/gbench_install/snapcraft.yaml >snapcraft.yaml
    ln $BUILD_PATH/src/gui/res/share/gbench/ncbilogo.svg
    snapcraft
    cd $BUILD_PATH
    ln -f $snap_path/*.snap
}

vdb_install
#clear_install
wxwidgets_install
gbench_build
build_rpm_tree
copy_fils_to_rpm
build_rpm_sources

if [ $build_rpm == 'true' ] ; then
    run_rpm
fi

if [ $build_deb == 'true' ] ; then
    build_deb
fi

if [ $build_snap == 'true' ] ; then
    build_snap
fi


