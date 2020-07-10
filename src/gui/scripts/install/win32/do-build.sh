#!/bin/bash


srcdir='/cygdrive/f/users/dicuccio'
srctag='gbench-current'
installtag=''
dstdir='//snowman/win-coremake/App/Ncbi/gbench/previous/test'
build_log="build.log"

devenv="/cygdrive/e/Program Files/Microsoft Visual Studio .net 2003/Common7/IDE/devenv"


do_rebuild=n
do_build=n
do_configure=n
do_install=n
do_touch=y
do_debug=n
do_release=n
do_msvc7=y
do_msvc8=n
do_internal=n



ShowUsage()
{
    echo "usage: do_build.sh <opts>"
    echo "where options include:"
    echo ""
    echo "  --srcdir=<path>      --  source tree"
    echo "  --srctag=<tag>       --  tag name for source tree"
    echo "  --installdir=<path>  --  installation tree"
    echo "  --dstdir=<path>      --  destination tree"
    echo "  --msvc7|--msvc8      --  build using MSVC7 / MSVC8"
    echo "  --build              --  perform a build"
    echo "  --rebuild            --  rebuild the solution"
    echo "  --configure          --  run 'configure'"
    echo "  --no-touch           --  don't touch the dated components"
    echo "  --no-build           --  don't build"
    echo "  --no-install         --  don't perform the installation step"
    echo "  --no-internal        --  don't build the intyernal projects"
    echo "  --debug              --  build debug versions"
    echo "  --release            --  build release versions"
    exit 0
}


while [ ! -z "$1" ]
do
    case "$1" in
        --srcdir=*)
            srcdir=`echo "$1" | sed 's/^--srcdir=//'`
            ;;

        --srctag=*)
            srctag=`echo "$1" | sed 's/^--srctag=//'`
            ;;

        --dstdir=*)
            dstdir=`echo "$1" | sed 's/^--dstdir=//'`
            ;;

        --installdir=*)
            installdir=`echo "$1" | sed 's/^--installdir=//'`
            ;;

        --configure)
            do_configure=y
            ;;

        --no-configure)
            do_configure=n
            ;;

        --build)
            do_build=y
            ;;

        --debug)
            do_debug=y
            build_log="build.debug.log"
            ;;

        --release)
            do_release=y
            build_log="build.release.log"
            ;;

        --rebuild)
            do_build=y
            do_rebuild=y
            ;;

        --install)
            do_install=y
            ;;

        --install=*)
            do_install=y
            installtag=`echo $1 | sed 's/--install=//'`
            ;;

        --installer)
            do_installer=y
            ;;

        --no-touch)
            do_touch=n
            ;;

        --msvc7)
            do_msvc7=y
            do_msvc8=n
            ;;

        --msvc8)
            do_msvc7=n
            do_msvc8=y
            ;;


        --help)
            ShowUsage
            ;;
    esac

    shift

done


#
# Final path clean-ups
#
if [ -z "$installtag" ]
then
    installtag="$srctag.`date --date="next Monday" +"%Y%m%d"`"
    num=1
    while [ -d "$srcdir/$installtag.rc$num" ]
    do
        num=`expr $num + 1`
    done
    installtag="$installtag.rc$num"
fi

installdir="$srcdir/$installtag"
tmp=$srcdir/tmp/gbench-build
srcdir="$srcdir/$srctag"


build_script="./build.msvc7.bat"
if [ "$do_msvc8" = "y" ]
then
    build_script="./build.msvc8.bat"
fi


ObtainLock()
{
    if [ -f "$tmp/BUILD_LOCK" ]
    then
        echo "The build is already being run"
        exit 1
    fi

    mkdir -p "$tmp"
    date > "$tmp/BUILD_LOCK"
}

CleanTmp()
{
    rm -rf "$tmp"
}

ReleaseLock()
{
    rm "$tmp/BUILD_LOCK"
    #CleanTmp()
}

#trap ReleaseLock 0



DoConfigure()
{
    echo "Configuring..."

    # force a reconfigure
    $build_script "$srctag" "-CONFIGURE-" "ReleaseDLL" "$rebuild" "$build_log"
}

DoBuild()
{
    echo "Building..."

    # insure that GUI components that report build dates are updated
    touch "$srcdir"/src/app/gbench/gbench/gbench_version.cpp

    if [ "$do_debug" = "y" ]
    then
        $build_script "$srctag" "gbench-install" "DebugDLL" "$build_log"
        $build_script "$srctag" "gbench-install-internal" "DebugDLL" "$build_log"
    fi

    if [ "$do_release" = "y" ]
    then
        $build_script "$srctag" "gbench-install" "ReleaseDLL" "$build_log"
        $build_script "$srctag" "gbench-install-internal" "ReleaseDLL" "$build_log"
    fi
}


DoInstaller()
{
    if [ ! -d "$installdir" ]
    then
        echo "error: install dir $installdir doesn't exist"
    fi

    ver_file=$srcdir/include/gui/utils/version.hpp
    if [ ! -f "$ver_file" ]
    then
        ver_file=$srcdir/include/gui/core/version.hpp
    fi

    ver_major=`grep 'eMajor  *= [0-9]*' $ver_file | sed 's/.*= \([0-9]*\).*/\1/'`
    ver_minor=`grep 'eMinor  *= [0-9]*' $ver_file | sed 's/.*= \([0-9]*\).*/\1/'`
    ver_revision=`grep 'ePatchLevel  *= [0-9]*' $ver_file | sed 's/.*= \([0-9]*\).*/\1/'`
    ver_build_date=`date +"%Y%m%d"`
    input_path=`echo $installdir | sed 's,/cygdrive/\(.\),\1:,'`

    install_files="license.txt gbench-res1.ini gbench-res2.ini gbench-res3.ini"

    for f in $install_files
    do
        cp $srcdir/src/gui/scripts/install/win32/$f $installdir
    done

    for nsi_file in gbench.nsi
    do
        nsi_file_in="$nsi_file".in
        sed \
            -e "s/@VERSION_MAJOR@/$ver_major/" \
            -e "s/@VERSION_MINOR@/$ver_minor/" \
            -e "s/@VERSION_REVISION@/$ver_revision/" \
            -e "s/@VERSION_BUILD_DATE@/$ver_build_date/" \
            -e "s,@INPUT_PATH@,$input_path," \
            $srcdir/src/gui/scripts/install/win32/$nsi_file_in > \
            $installdir/$nsi_file

        //snowman/win-coremake/App/ThirdParty/NSIS/makensis.exe $input_path/$nsi_file
    done

    # clean up
    rm $installdir/*.nsi
    for f in $install_files
    do
        rm $installdir/$f
    done

}


DoInstall()
{
    echo "Installing from $srcdir to $installdir"

    if [ -d "$installdir" ]
    then
        ext="1"
        while [ -d "$installdir.$ext" ]
        do
            ext=`expr $ext + 1`
        done
        mv "$installdir" "$installdir.$ext"
    fi

    mkdir -p "$installdir"
    mkdir -p "$installdir/SDK/ReleaseDLL"
    mkdir -p "$installdir/SDK/DebugDLL"

    # install the binaries first
    if [ "$do_msvc7" = "y" ]
    then
        cp -aR "$srcdir"/compilers/msvc710_prj/dll/bin/ReleaseDLL/gbench/* "$installdir"
    elif [ "$do_msvc8" = "y" ]
    then
        cp -aR "$srcdir"/compilers/msvc800_prj/dll/bin/ReleaseDLL/gbench/* "$installdir"
    else
        echo "no build defined!!!"
    fi

    # install the etc/ extra information
    cd "$installdir"/etc
    tar xzvf ~/gbench-etc-public.tgz

    # build the installer package
    DoInstaller

    # unpack the proprietary stuff
    cd "$installdir"/etc
    tar xzvf ~/gbench-etc-private.tgz

    # install the SDK components
    if [ "$do_msvc7" = "y" ]
    then
        bash "$srcdir"/compilers/msvc710_prj/install.sh "$srcdir" "$installdir"/SDK
        cp -aR "$srcdir"/compilers/msvc710_prj/dll/bin/ReleaseDLL/gbench/* "$installdir"/SDK/ReleaseDLL
        cp -aR "$srcdir"/compilers/msvc710_prj/dll/bin/DebugDLL/gbench/* "$installdir"/SDK/DebugDLL
    elif [ "$do_msvc8" = "y" ]
    then
        bash "$srcdir"/compilers/msvc800_prj/install.sh "$srcdir" "$installdir"/SDK
        cp -aR "$srcdir"/compilers/msvc800_prj/dll/bin/ReleaseDLL/gbench/* "$installdir"/SDK/ReleaseDLL
        cp -aR "$srcdir"/compilers/msvc800_prj/dll/bin/DebugDLL/gbench/* "$installdir"/SDK/DebugDLL
    else
        echo "no build defined!!!"
    fi

    # purge some useless crap out of the installation tree
    cd "$installdir"/SDK
    rm -rf include/internal
    rm -rf src/internal

    # restore some things list from above
    cd "$installdir"/SDK/ReleaseDLL/etc
    tar xzvf ~/gbench-etc-public.tgz
    tar xzvf ~/gbench-etc-private.tgz

    cd "$installdir"/SDK/DebugDLL/etc
    tar xzvf ~/gbench-etc-public.tgz
    tar xzvf ~/gbench-etc-private.tgz

}



DoCheckErrors()
{
    errs=`tr -cd '[:print:]\n' < $build_log | grep 'error(s)' | grep -v '0 error(s)'`
    echo ""
    echo "------------------------------------------------------------------"
    if [ -z "$errs" ]
    then
        echo "No errors reported in build."
    else
        echo "Errors in build:"
        echo ""
        echo "$errs"
        echo ""
    fi
    echo "------------------------------------------------------------------"
}


#ObtainLock

rm $build_log
touch $build_log

if [ "$do_configure" = "y" ]
then
    DoConfigure
fi

if [ "$do_build" = "y" ]
then
    DoBuild
fi

if [ "$do_install" = "y" ]
then
    echo "-------------------------------------------------------------------" >> $build_log
    echo "Installing..." >> $build_log
    echo "-------------------------------------------------------------------" >> $build_log
    DoInstall 2>&1 | tee -a $build_log
fi

if [ "$do_installer" = "y" ]
then
    DoInstaller
fi

DoCheckErrors

