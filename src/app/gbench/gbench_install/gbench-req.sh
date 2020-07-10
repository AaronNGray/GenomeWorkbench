#!/bin/sh

filelist=$(cat)
#REQUIRES=$(echo $filelist | %{__find_requires})
#PROVIDES=$(echo $filelist | %{__find_provides})

solist=$(echo $filelist | grep "\\.so" | grep -v "^/lib/ld.so" | \
	xargs file -L 2>/dev/null | grep "ELF.*shared object" | cut -d: -f1)

#
# --- Alpha does not mark 64bit dependencies
case `uname -m` in
  alpha*)	mark64="" ;;
  *)		mark64="()(64bit)" ;;
esac

#
# --- Library sonames and weak symbol versions (from glibc).
PROVIDES=$(
for f in $solist; do
    soname=$(objdump -p $f | awk '/SONAME/ {print $2}')

    lib64=`if file -L $f 2>/dev/null | \
	grep "ELF 64-bit" >/dev/null; then echo "$mark64"; fi`
    if [ "$soname" != "" ]; then
	if [ ! -L $f ]; then
	    echo $soname$lib64
	    objdump -p $f | awk '
		BEGIN { START=0 ; }
		/Version definitions:/ { START=1; }
		/^[0-9]/ && (START==1) { print $4; }
		/^$/ { START=0; }
	    ' | \
		grep -v $soname | \
		while read symbol ; do
		    echo "$soname($symbol)`echo $lib64 | sed 's/()//'`"
		done
	fi
    else
	echo ${f##*/}$lib64
    fi
done)

exelist=`echo $filelist | xargs -r file | egrep -v ":.* (commands|script) " | \
	grep ":.*executable" | cut -d: -f1`

REQUIRES=$(
  # --- Executable dependency sonames.
  for f in `echo "$exelist $solist"`; do
    [ -r $f ] || continue
    lib64=`if file -L $f 2>/dev/null | \
	grep "ELF 64-bit" >/dev/null; then echo "$mark64"; fi`
    ldd $f 2>/dev/null | awk '/=>/ {
	if ($1 !~ /linux-vdso.so/ && $1 !~ /libNoVersion.so/ && $1 !~ /4[um]lib.so/ && $1 !~ /libredhat-kernel.so/) {
	    gsub(/'\''"/,"\\&",$1);
	    printf "%s'$lib64'\n", $1
	}
    }'
  done | xargs -r -n 1 basename | sort -u)

# Filter out dups from both lists
REQUIRES=$(echo "$REQUIRES" | sort | uniq)
PROVIDES=$(echo "$PROVIDES" | sort | uniq)

#
# Get a list of elements that exist in exactly one of PROVIDES or REQUIRES
#
UNIQ=$(echo "$PROVIDES
$REQUIRES" | sort | uniq -u)

#
# Of those, only chose the ones that are in REQUIRES
#
echo "$UNIQ
$REQUIRES" | sort | uniq -d

