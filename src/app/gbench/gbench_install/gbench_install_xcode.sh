#!/bin/sh

echo Updating VDB links

#PROJECT_DIR=/Users/katargir/Documents/gbench/compilers/xcode30_prj/dll
#CONFIGURATION=DebugDLL

project_tree_builder_ini=${PROJECT_DIR}/../../../src/build-system/project_tree_builder.ini

if [ $CONFIGURATION == 'DebugDLL' ] ; then
    d='debug'
else
    d='release'
fi

XCode_ThirdPartyVDBBasePath=`\
grep "XCode_ThirdPartyVDBBasePath[ \t]*=" $project_tree_builder_ini |\
cut -d '=' -f 2 - |\
sed -e 's/^[[:space:]]*//'`

ThirdParty_VDB=${XCode_ThirdPartyVDBBasePath}`\
grep 'ThirdParty_VDB[ \t]*=[ \t]*\$(XCode_ThirdPartyVDBBasePath)' $project_tree_builder_ini |\
cut -d '=' -f 2 - |\
sed -e 's/^[[:space:]]*$(XCode_ThirdPartyVDBBasePath)//'`/mac/$d/x86_64/lib

#echo $XCode_ThirdPartyVDBBasePath
#echo $ThirdParty_VDB

find ${PROJECT_DIR}/bin/${CONFIGURATION} -type f ! -name '*.*' -maxdepth 1 | while read f ; do
    for l in `otool -L $f | awk 'NR>1 {print $1}' | egrep '@executable_path\/\.\.\/lib\/libncbi-vdb.*\.dylib$'` ; do
        r=$(echo $l | sed 's/@executable_path\/\.\.\/lib/@rpath/')
        install_name_tool -change $l $r /$f
        install_name_tool -add_rpath $ThirdParty_VDB $f
    done
done 

find ${PROJECT_DIR}/lib/${CONFIGURATION} -type f -name '*.dylib' -maxdepth 1 | while read f ; do
    for l in `otool -L $f | awk 'NR>1 {print $1}' | egrep '@executable_path\/\.\.\/lib\/libncbi-vdb.*\.dylib$'` ; do
        r=$(echo $l | sed 's/@executable_path\/\.\.\/lib/@rpath/')
        install_name_tool -change $l $r $f
        install_name_tool -add_rpath $ThirdParty_VDB $f
    done
done 

abs_top_srcdir=${PROJECT_DIR}/../../..
makefile=${TEMP_FILES_DIR}/Makefile.gbench_install

sed -e '/^include/d' -e '/^requirements:/d' -e 's/@loadable_ext@/.dylib/' \
    <${abs_top_srcdir}/src/app/gbench/gbench_install/Makefile.gbench_install.in > ${makefile}

abs_top_srcdir=${abs_top_srcdir} bindir=${PROJECT_DIR}/bin/${CONFIGURATION} libdir=${PROJECT_DIR}/lib/${CONFIGURATION} make -f ${makefile}

