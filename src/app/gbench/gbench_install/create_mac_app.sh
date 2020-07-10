#!/bin/sh

#Shell script to create a double-clickable Mac application package from parts.

if [ $# -ne 4 ] ; then
    echo Usage: create_mac_app.sh top_source_directory bin_directory lib_directory package_name
    exit 1
fi

abs_top_srcdir=$1
bindir=$2
libdir=$3
package_name=$4
mac_res_dir=${abs_top_srcdir}/src/gui/res/share/gbench
install_dir=${bindir}/gbench
mac_app_dir=${install_dir}/${package_name}.app
locallibs="(^/(netopt).*\.dylib$)|(^$HOME.*\.dylib$)|(^/(sw/lib).*\.dylib$)"
windowmasker_ini='window-masker.ini' 

#sysver=`sw_vers -productVersion | cut -c 1-4`
sysver=10.12
echo $sysver

###############################################################################
#  Functions Definitions
###############################################################################

function read_version()
{
    local re='[[:blank:]]*=[[:blank:]]*([[:digit:]]{1,2}).*'

    local eMajor=""
    local eMinor=""
    local ePatchLevel=""
    local versionFile=${abs_top_srcdir}"/include/gui/objects/gbench_version.hpp"

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

InstallLibrary()
{
    if [ -h "$1" ]; then
        # Mac does not have readlink -f so get dir from filename and append it. This requires 
        # links and their targets to be in the same directory (which for wxwidgets they are)
        export LINK_NAME=`readlink $1`
        export DIR=`dirname $1`
        export FULL_LINK_NAME=$DIR/$LINK_NAME
        echo $1 Full Link Name: $FULL_LINK_NAME

        if [ -h "$FULL_LINK_NAME" ]; then
            export FULL_SUB_LINK_NAME=$DIR/`readlink $FULL_LINK_NAME`
            export SUB_LINK_NAME=`basename $FULL_SUB_LINK_NAME`

            echo $1 is a double symbolic link to $FULL_SUB_LINK_NAME
        
            echo install  "$FULL_SUB_LINK_NAME"  "${mac_app_dir}/Contents/Frameworks/$SUB_LINK_NAME"
            install  "$FULL_SUB_LINK_NAME"  "${mac_app_dir}/Contents/Frameworks/$SUB_LINK_NAME"
           
            echo ln -s  "$SUB_LINK_NAME" "${mac_app_dir}/Contents/Frameworks/$LINK_NAME"
            ln -s  "$SUB_LINK_NAME" "${mac_app_dir}/Contents/Frameworks/$LINK_NAME"

            echo ln -s  "$LINK_NAME" "${mac_app_dir}/Contents/Frameworks/`basename $1`"
            ln -s  "$LINK_NAME" "${mac_app_dir}/Contents/Frameworks/`basename $1`"
        else 
            echo $1 is a symbolic link to $FULL_LINK_NAME
            echo install  "$FULL_LINK_NAME"  "${mac_app_dir}/Contents/Frameworks/$LINK_NAME"
            install "$FULL_LINK_NAME"  "${mac_app_dir}/Contents/Frameworks/$LINK_NAME"

            echo ln -s  "$LINK_NAME" "${mac_app_dir}/Contents/Frameworks/`basename $1`"
            ln -s  "$LINK_NAME" "${mac_app_dir}/Contents/Frameworks/`basename $1`"
        fi
    else
        install "$1" "${mac_app_dir}/Contents/Frameworks/`basename $1`"
    fi
}

rpathList=()
# Gets the list of run-time libraries
GetRunTimePaths()
{
    local IFS_save=$IFS
    IFS=$'\n'

    unset rpathList

    _next_path_is_rpath=

    for line in `otool -l "$1"` ; do
        case "${line}" in
            *LC_RPATH)
                _next_path_is_rpath=yes
                ;;
            *path*\(offset*\))
                if [ -z "${_next_path_is_rpath}" ]
                then
                    continue
                fi
                line="${line#* path }"
                line="${line% (offset *}"
                if [ ${#} -gt 1 ]
                then
                    line=$'\t'"${line}"
                fi
                _next_path_is_rpath=
                rpathList+=(${line})
                ;;
        esac
    done

    IFS=$IFS_save
}

# Copies libraries to the package
InstallToolkitLibraries()
{
	for ff in `otool -L "$1" | \
	  awk 'NR>1 {print $1}' | egrep $locallibs` ; do
		echo Installing Mac App Package: `basename $ff`
		InstallLibrary "$ff"
	done
}

# Copies run-time libraries (i.e. not part of the C++ toolkit)
InstallRuntimeLibraries()
{
	GetRunTimePaths "$1"
	for ff in `otool -L "$1" | \
	  awk 'NR>1 {print $1}' | egrep "^(@rpath).*\.dylib$"` ; do
	  for i in ${rpathList[@]}; do
		export FPATH="$i"/`basename $ff`
		if [ -f "$FPATH" ] && [ ! -e "${mac_app_dir}/Contents/Frameworks/`basename "$ff"`" ]; then
            echo Installing LC_RPATH Mac App Package: `basename $ff`
            InstallLibrary "$FPATH"
			break
		fi
	  done
	done
}

# Copies package dependencies
InstallPackageToolkitDependencies()
{
	for ff in `find "${mac_app_dir}/Contents/MacOS/packages" -name "$1" -exec otool -L {} \\; \
	  | awk 'NR>1 {print $1}' | egrep $locallibs` ; do
		if [ ! -f "${mac_app_dir}"/Contents/Frameworks/`basename $ff` ] ; then
			InstallLibrary "$ff"
			echo Installed package dylib: `basename $ff`
		fi
	done
}

# Copies run-time package dependencies  (i.e. not part of the C++ toolkit)
InstallRuntimeDependenciesForFolder()
{
	find "$1" -name "$2" | while read ff; do
		InstallRuntimeLibraries "$ff"
	done
}

UpdatePaths()
{
    for ff in `otool -L "$1" | awk 'NR>1 {print $1}'` ; do
        if test -f "${mac_app_dir}/Contents/Frameworks/`basename "$ff"`"; then
            install_name_tool -change $ff @executable_path/../Frameworks/`basename $ff` "$1"
        fi
    done
}

###############################################################################
#  End of Functions Definitions
###############################################################################

#First, create the app package directory structure, copy the binary, plugins, packages, icons, etc.
echo "Installing Mac App Package: Genome Workbench"
read_version gbenchVersion
echo "Genome Workbench version: $gbenchVersion"
install -d "${mac_app_dir}/Contents/MacOS"
install -d "${mac_app_dir}/Contents/Resources"
install -d "${mac_app_dir}/Contents/Frameworks"

echo Installing ${bindir}/gbench-bin as "${mac_app_dir}/Contents/MacOS/Genome Workbench"
install ${bindir}/gbench-bin "${mac_app_dir}/Contents/MacOS/Genome Workbench"

utils="gbench_monitor gbench_feedback_agent windowmasker blastp blastn blastx tblastn tblastx"
(cd ${bindir}; install ${utils} "${mac_app_dir}/Contents/MacOS/")

default_metallib=${install_dir}/bin/default.metallib

if [ -e $default_metallib ]; then
    echo "Installing $default_metallib"
    install $default_metallib "${mac_app_dir}/Contents/Resources/"
fi 

util_dirs="etc share packages"
(cd ${bindir}/gbench; cp -R ${util_dirs} "${mac_app_dir}/Contents/MacOS/")

ln -f ${mac_res_dir}/gbench.icns  "${mac_app_dir}/Contents/Resources/Genome Workbench.icns"
ln -f ${mac_res_dir}/gbench_project.icns  "${mac_app_dir}/Contents/Resources/gbench_project.icns"
ln -f ${mac_res_dir}/gbench_workspace.icns  "${mac_app_dir}/Contents/Resources/gbench_workspace.icns"
sed -e 's/XX.X/'${sysver}'/g' -e 's/placeholder_for_gbench_version/'${gbenchVersion}'/g' ${mac_res_dir}/Info.plist > "${mac_app_dir}/Contents/Info.plist"

#create widowmasker ini if absent
if [ -e $windowmasker_ini ]; then
	echo "Windowmasker file exists and will not be generated."
else 
	echo "WindowMasker does not exists - generating..."
	echo "[WindowMasker]" >> "${mac_app_dir}/Contents/MacOS/etc/${windowmasker_ini}"	
	echo "DataPath=/Volumes/gbench/data/window-masker" >> "${mac_app_dir}/Contents/MacOS/etc/${windowmasker_ini}"	
fi 


#We explicitly copy the dylibs that are not explicitly linked.
#ncbi_xloader_*, ncbi_xreader_*, ncbi_xcache_*, and ncbi_xdbapi_*
install "${libdir}"/libncbi_xloader_*.dylib "${mac_app_dir}/Contents/Frameworks"
install "${libdir}"/libncbi_xreader_*.dylib "${mac_app_dir}/Contents/Frameworks"
install "${libdir}"/libncbi_xcache_*.dylib "${mac_app_dir}/Contents/Frameworks"
install "${libdir}"/libncbi_xcache_sqlite3.so "${mac_app_dir}/Contents/Frameworks"
install "${libdir}"/libncbi_xdbapi_*.dylib "${mac_app_dir}/Contents/Frameworks"
install "${libdir}"/libblast*.dylib "${mac_app_dir}/Contents/Frameworks"
install "${libdir}"/libxblastformat*.dylib "${mac_app_dir}/Contents/Frameworks"
install "${libdir}"/libblastdb_format*.dylib "${mac_app_dir}/Contents/Frameworks"


###############################################################################
#  Temporary fix since X11 is no longer included with mac 10.10. This
#  can go away after these libraries are gotten from netopt
###############################################################################
install /opt/X11/lib/libpng*.dylib "${mac_app_dir}/Contents/Frameworks"
install /opt/X11/lib/libfreetype*.dylib "${mac_app_dir}/Contents/Frameworks"

#Now copy all the dylibs that we built and are needed by the binary.
echo Copying Mac App Package Libraries
InstallToolkitLibraries "${mac_app_dir}/Contents/MacOS/Genome Workbench"

#Now copy all the dylibs that are not part of the C++ toolkit
echo Copying Mac App Package Libraries from LC_RPATH
InstallRuntimeLibraries "${mac_app_dir}/Contents/MacOS/Genome Workbench"

#We copy the dylibs needed by the .so plugins.
InstallPackageToolkitDependencies *.so

#We copy the dylibs needed by the .dylib plugins.
InstallPackageToolkitDependencies *.dylib

#We copy the run-time dylibs needed by the .dylib plugins.
echo Copying Package Libraries Run-time Dependencies
InstallRuntimeDependenciesForFolder "${mac_app_dir}/Contents/MacOS/packages" *.dylib

#Next we copy all the dylibs that the dylibs we copied in the previous step may require.  We
#repeat this step in case the new dylibs require other dylibs.
echo Finding missing Mac App Package Libraries
for i in 1 2 3; do
    for ff in "${mac_app_dir}"/Contents/Frameworks/*.dylib ; do
        for ll in `otool -L "$ff" | awk 'NR>1 {print $1}' | \
          egrep $locallibs` ; do
            if [ ! -f "${mac_app_dir}"/Contents/Frameworks/`basename $ll` ] ; then
                install "$ll" "${mac_app_dir}/Contents/Frameworks/"
                echo Pass ${i}: Installed missing dylib: `basename $ll`
            fi
        done
    done
done

#Install run-time dependencies for all libraries in the Frameworks folder
echo Copying Package Libraries Run-time Dependencies
InstallRuntimeDependenciesForFolder "${mac_app_dir}/Contents/Frameworks" *.dylib

#Once all the dylibs are copied into the package, we change the path for the binary dylibs
#to the package itself where we just copied everything.
echo Adjusting Mac App Package Binary Paths

UpdatePaths "${mac_app_dir}/Contents/MacOS/Genome Workbench" 

for f in $utils; do
    UpdatePaths "${mac_app_dir}/Contents/MacOS/$f"
done

#We update the dylib path for the .so packages.
echo Adjusting .so package library paths
find "${mac_app_dir}/Contents/MacOS/packages" -name '*.so' | while read ff ; do
    install_name_tool -id \@executable_path/../Frameworks/`basename "$ff"` "$ff"
    UpdatePaths "$ff"
done

#We update the dylib path for the .dlylib packages.
echo Adjusting .dylib package library paths
find "${mac_app_dir}/Contents/MacOS/packages" -name '*.dylib' | while read ff ; do
    install_name_tool -id \@executable_path/../Frameworks/`basename "$ff"` "$ff"
    UpdatePaths "$ff"
done

#Finally we update the path of the dylibs themselves so they match their physical location in the
#package.
echo Adjusting Mac App Package Library Paths ; \
find "${mac_app_dir}/Contents/Frameworks" -name *.dylib | while read ff; do
    install_name_tool -id \@executable_path/../Frameworks/`basename "$ff"` "$ff"
    UpdatePaths "$ff"
done
find "${mac_app_dir}/Contents/Frameworks" -name *.so | while read ff; do
    install_name_tool -id \@executable_path/../Frameworks/`basename "$ff"` "$ff"
    UpdatePaths "$ff"
done

#Now that we have the app package built, we put it into a dmg
echo Installing disk image: "${package_name}.dmg"
hdiutil create -srcfolder "${mac_app_dir}" -ov -fs HFS+ -volname 'NCBI Genome Workbench' \
    "${install_dir}/${package_name}.dmg"

