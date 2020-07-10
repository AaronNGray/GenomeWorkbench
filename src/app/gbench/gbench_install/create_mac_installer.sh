#!/bin/sh

# Shell script to create DMG this requires a DMG template with finder
# parameters such as background, icon size and locations and finder
# window size already set. Instructions are below:

# Steps to creating a DMG template:
# 1. Create the background image (no icons - just an arrow and maybe text)
#    Current version is src/gui/share/gbench/mac_install_background.png
# 2. Make a directory to save template DMG contents (name unimportant)
# 3. Copy shortcut (link) to Applications folder to this directory
# 4. Copy a version of Genome Workbench application bundle (.app) to this directory.
#    This will be replaced so the bundles contents should be deleted 
# 5. Create subdirectory to hold background image, e.g. install_background
# 6. Copy Image created in 1. to this subdirectory
# 7. Run mac Disk Utility
# 8. File -> New -> Disk Image From Folder
# 9. File->New->Blank Disk Image
#      Save As: NCBI Genome Workbench Installer
#      Name: NCBI Genome Workbench Installer
#      Size: 250 MB (Use Custom Size option)
#      Format: Mac OS Extended (Journaled) (this is default)
#      Encryption: None
#      Partition: Single Partition (default)
#      Image Format: read/write disk image
# 10. Double click on new, empty dmg and then copy into it the contents
#     of other folder: image_background directory, .app and application shortcut
# 11. Click background of dmg window (now with your files in it) to give it focus.
#     Then right-click and select "Show View Options" On this menu:
# 12. Check "Always Open in Icon View" and "Browse in Icon View"
# 13. Set icon size to something larger (108x108)
# 14. Set "Arrange By" and "Sort By" to "None"
# 15. For background select "picture"
# 16. Open (browse) dmg in finder and navigate to the background image.
# 17. Drag the background image to the "drag image here" icon on the Show View Options dialog
# 18. Move the icons around (using mouse) to get them positioned correctly around the arrow. 
#     You may want to update icon size and grid spacing here.
# 19. Resize window to fit everything nicely.
# 20. Hide install_background folder in the mounted dmg using terminal command:
#     SetFile -a V /Volumes/NCBI\ Genome\ Workbench\ Installer/install_background
# 21. Unmount/eject disk.  template is ready.


if [ $# -eq 0 ] ; then
    #echo Usage: gb_installer.sh build_dir install_dir dmg_template package_name
    echo "Usage: create_mac_installer.sh --src=app-pkg-dir --dst=output-dir --dmg=empty-dmg-template --name=pkg-name --sign"
    echo "Creates a DMG with application bundle, background image and link to applications folder"
    echo "--sign: invokes sign_mac_app.sh to codesing files in bundle"
    exit 1
fi

build_dir=$1
install_dir=$2
template=$3
package_name=$4
sign_app=""

while [ ! -z "$1" ]
do
    case "$1" in
        --src=*)
            build_dir=`echo $1 | sed 's/--src=//'`
	    build_dir="${build_dir/#\~/$HOME}"
        ;;

        --dst=*)
            install_dir=`echo $1 | sed 's/--dst=//'`
	    install_dir="${install_dir/#\~/$HOME}"
        ;;
        
        --dmg=*)
            template=`echo $1 | sed 's/--dmg=//'`
	    template="${template/#\~/$HOME}"
        ;;

        --name=*)
            package_name=`echo $1 | sed 's/--name=//'`
        ;;

        --sign)
            sign_app="y"
        ;;

        *)
        echo "unrecognized option: $1"
        exit 1
    esac
    shift
done

mac_app_dir="${build_dir}/${package_name}.app"

#Verify the package name is a directory:
if [ ! -d "${mac_app_dir}" ]
then
    echo "Error: ${build_dir} application bundle is not a directory."
    exit 1
fi

#Verify the destination is a directory:
if [ ! -d "${install_dir}" ]
then
    echo "Error: ${install_dir} destination is not a directory."
    exit 1
fi

#Verify the dmg template is a file 
if [ ! -f "${template}" ]
then
    echo "Error: ${template} is not a file."
    exit 1
fi

# Copy template to package name. It should already have an icon
DMG_TEMPLATE="${install_dir}/${dmg_template}"
cp "${template}" "${DMG_TEMPLATE}"

# Attempts to enlarge template DMG if needed did not work - seems it's not
# as easy as checking DMG total size and and updating it with "hdiutil resize -size ..."


# Mount copied template (package name).  Note that this may already be mounted. Not sure this would
# cause problems.  But in any case if I don't know the name of the volume in advance, that is not
# so easy to check (and unmount/detach). So leave that up to the user for now.
VOLNAME=$(hdiutil attach -readwrite -noverify -noautoopen "${template}" | egrep '/Volumes/' | sed 's/.*\(\/Volumes\/.*$\)/\1/') 
echo "Volume is: " $VOLNAME

if [ ! -d "${VOLNAME}" ]
then
    echo "Error: unable to mount DMG template: ${template}" 
    exit 1
fi

# Copy application bundle to mounted template. Use -a to preserve symlinks
# "rsync -rtuvl src dst" also works
cp -a "${mac_app_dir}" "${VOLNAME}/."

# sign application if user specifed --sign (requires developer IDs be installed in your keychain)
if [ ! -z "${sign_app}" ];
then
    echo "Signing app:"
    ./sign_mac_app.sh --src="${VOLNAME}" --name="${package_name}"

    # check exit code and exit if non-zero
    if [ ! $? -eq 0 ]
    then
        echo "sign_mac_app.sh returned error. Exiting"
	exit 1
    fi
fi

#copy current version of background image to DMG (note that it has been signed) 
cp "${VOLNAME}/${package_name}.app/Contents/MacOS/share/gbench/mac_install_background.png" \
   "${VOLNAME}/install_background/install_background.png"

# re-build mounted template with updated contents.  Remove existing dmg
# if any first
hdiutil detach "$VOLNAME"

if [ -e "${install_dir}/${package_name}.dmg" ]
then
    echo "Removing existing dmg file"
    rm "${install_dir}/${package_name}.dmg"
fi

echo "Compressing disk image..."
hdiutil convert ${template} -format UDZO -imagekey zlib-level=9 -o "${install_dir}/${package_name}.dmg"

# Add Genome Workbench icon to the installer dmg:
cp  "${build_dir}/${package_name}.app/Contents/Resources/Genome Workbench.icns" "${install_dir}/gbench.icns"
sips -i "${install_dir}/gbench.icns"
DeRez -only icns "${install_dir}/gbench.icns" > "${install_dir}/tmpicns.rsrc"
Rez -append "${install_dir}/tmpicns.rsrc" -o "${install_dir}/${package_name}.dmg" 
# Use the resource to set the icon.
SetFile -a C "${install_dir}/${package_name}.dmg" 

# clean up.
rm "${install_dir}/tmpicns.rsrc"
rm "${install_dir}/gbench.icns"
rm "${install_dir}/${template}"
