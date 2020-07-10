#!/bin/sh

#Shell script to sign the files in the application bundle (all must be signed) and then
#optionally (re)create the dmg file 

if [ $# -eq 0 ] ; then
    echo "Usage: sign_mac_app.sh --src=app-pkg-dir --name=package-name --create-dmg"
    echo "Files in application package src/name.app will be signed."
    echo "--create-dmg: signed files will be compressed into DMG src/name.dmg"
    exit 1
fi


src_path="./"
package_name=""
create_dmg=""

while [ ! -z "$1" ]
do
    case "$1" in
        --src=*)
            src_path=`echo $1 | sed 's/--src=//'`
	    src_path="${src_path/#\~/$HOME}"
        ;;

        --name=*)
            package_name=`echo $1 | sed 's/--name=//'`
        ;;

        --create-dmg)
            create_dmg="y"
        ;;

        *)
        echo "unrecognized option: $1"
        exit 1
    esac
    shift
done

mac_app_dir="${src_path}/${package_name}.app"

#Verify the package name is a directory:
if [ ! -d "${mac_app_dir}" ]
then
    echo "Error: ${mac_app_dir} application bundle is not a directory"
    exit 1
fi

# check system version - older versions don't support our signing
sysver=`sw_vers -productVersion | cut -c 1-4`
echo $sysver
if [ $sysver = 10.6 ]; then
    echo "This is a Snow Leopard System - 10.9 and above are required for code signing"
    exit 1
elif [ $sysver = 10.7 ]; then
    echo "This is a Lion System - 10.9 and above are required for code signing"
    exit 1
elif [ $sysver = 10.8 ]; then
    echo "This is a Mountain Lion System - 10.9 and above are required for code signing"
    exit 1
elif [ $sysver = 10.9 ]; then
    echo "This is a Leopard System"
elif [ $sysver = 10.10 ]; then
    echo "This is a Yosemite System"
fi

#
# sign code with developer id signature (for distribution outside of the mac store)
# script currently requires that signature to be added to your login keychain
#

if ! [[ `security find-certificate -c "Developer ID Application: National Library of Medicine (NMBUW4N66P)" 2>/dev/null` ]]; then 
    echo "Certificate for Developer ID Application: National Library of Medicine (NMBUW4N66P) not found in login keychain"
    echo "Install needed developer certificates to login keychain before signing."
    exit 1 
fi

# Function to sign a file. This only requires 1 call to codesign, but we add in logic
# for error handling and retries.  Also, we do not sign symlinks. That is not an error,
# but it produces un-needed messages in the output.
sign () {
    if [ -h "$1" ]
      then
          # This is a symlink so we do not need to sign it. 
	  return 0
      fi 
   
    # Suppress normal output so that any errors standout.
    # Note that this replaces existing signatures which may also be valid on system
    # libraries we redistribute, such as libpng (messages will be shown for that)
    CS=$(codesign -f -s "Developer ID Application: National Library of Medicine (NMBUW4N66P)" "$1")
    VERIFIED=$(codesign --verify "$1" 2>&1 > /dev/null)

    # Usual reason for not signing successfully first time is unable to reach timestamp server
    # over internet.  Retrying pretty much always fixes this, usually just 1 time.  But we
    # allow up to 5 

    if [ -n "$VERIFIED" ]
      then
          echo "codesign failed - retrying"

	  for i in `seq 1 5`;
          do
              codesign -f -s "Developer ID Application: National Library of Medicine (NMBUW4N66P)" "$1" 
              VERIFIED=$(codesign --verify "$1" 2>&1 > /dev/null)
              if [ -z "$VERIFIED" ]
                then
                    echo "$1 signed successfully"
                    break;
                fi
          done

	  # If we were not able to sign, exit (maybe internet connection is down)
          if [ -n "$VERIFIED" ]
            then
                codesign --verify --verbose "$1"
                echo "second try to sign: $1 failed - giving up"
                exit 1 
            fi
      fi
}

# sign all files in packages
find "${mac_app_dir}/Contents/MacOS/packages" -type f -name '*' | while read ff ; do
    sign "$ff"
done

# sign anything in Resources directory
find "${mac_app_dir}/Contents/Resources" -type f -name '*' | while read ff ; do
    sign "$ff";
done

# sign data files (but these should really be in Genome Workbench.app/Resources, where
# they would not need to be signed)
find "${mac_app_dir}/Contents/MacOS/etc" -type f -name '*' | while read ff ; do
    sign "$ff";
done
find "${mac_app_dir}/Contents/MacOS/share" -type f -name '*' | while read ff ; do
    sign "$ff";
done

# sign dylib and so libraries (in Frameworks) 
echo "Signing dylib and so libraries in Frameworks"
for ff in "${mac_app_dir}"/Contents/Frameworks/*.dylib ; do
    sign "$ff";
done

#sign Info.plist
echo "Signing plist"
sign "${mac_app_dir}/Contents/Info.plist"

sign_execs () {
    if [ -h "$1" ]
      then
          # This is a symlink so we do not need to sign it. 
      return 0
      fi 
   
    # Suppress normal output so that any errors standout.
    # Note that this replaces existing signatures which may also be valid on system
    # libraries we redistribute, such as libpng (messages will be shown for that)
    CS=$(codesign -f --verbose --options=runtime -s "Developer ID Application: National Library of Medicine (NMBUW4N66P)" "$1" --entitlements workbench.entitlements)
    VERIFIED=$(codesign --verify "$1" 2>&1 > /dev/null)

    # Usual reason for not signing successfully first time is unable to reach timestamp server
    # over internet.  Retrying pretty much always fixes this, usually just 1 time.  But we
    # allow up to 5 

    if [ -n "$VERIFIED" ]
      then
          echo "codesign failed - retrying"

      for i in `seq 1 5`;
          do
              codesign -f --verbose --options=runtime -s "Developer ID Application: National Library of Medicine (NMBUW4N66P)" "$1" --entitlements workbench.entitlements
              VERIFIED=$(codesign --verify "$1" 2>&1 > /dev/null)
              if [ -z "$VERIFIED" ]
                then
                    echo "$1 signed successfully"
                    break;
                fi
          done

      # If we were not able to sign, exit (maybe internet connection is down)
          if [ -n "$VERIFIED" ]
            then
                codesign --verify --verbose "$1"
                echo "second try to sign: $1 failed - giving up"
                exit 1 
            fi
      fi
}


# sign the executables
echo "Sign executables"
sign_execs "${mac_app_dir}/Contents/MacOS/gbench_monitor" -v
sign_execs "${mac_app_dir}/Contents/MacOS/gbench_feedback_agent" -v
sign_execs "${mac_app_dir}/Contents/MacOS/windowmasker" -v
sign_execs "${mac_app_dir}/Contents/MacOS/blastn" -v 
sign_execs "${mac_app_dir}/Contents/MacOS/blastp" -v
sign_execs "${mac_app_dir}/Contents/MacOS/blastx" -v 
sign_execs "${mac_app_dir}/Contents/MacOS/tblastn" -v 
sign_execs "${mac_app_dir}/Contents/MacOS/tblastx" -v
sign_execs "${mac_app_dir}/Contents/MacOS/Genome Workbench" -v

echo "Application signed"

#Now that we have the app package signed we replace the dmg
if [ ! -z "$create_dmg" ]; 
then
    if [ -e "${src_path}/${package_name}.dmg" ]
    then
        echo "Removing existing dmg file"
        rm "${src_path}/${package_name}.dmg"
    fi

    echo Installing disk image: "${package_name}.dmg"
    hdiutil create -srcfolder "${mac_app_dir}" -ov -fs HFS+ -volname 'NCBI Genome Workbench' \
        "${src_path}/${package_name}.dmg"
fi

