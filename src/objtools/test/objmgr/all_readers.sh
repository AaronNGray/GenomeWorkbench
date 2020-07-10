#! /bin/sh
#$Id: all_readers.sh 508558 2016-07-28 19:27:51Z vasilche $

status_dir="$CFG_LIB/../status"
if test ! -d "$status_dir"; then
    status_dir="../../../../status"
fi

disabled() {
    if test -f "$status_dir/$1.enabled"; then
        return 1
    fi
    case "$FEATURES" in
        *" $1 "*) return 1;;
    esac
    return 0;
}

if test "$1" = "-id2"; then
    shift
    methods="ID2"
elif disabled PubSeqOS; then
    echo "Skipping PUBSEQOS loader test (loader unavailable)"
    methods="ID1 ID2"
elif disabled in-house-resources; then
    echo "Skipping PUBSEQOS loader test (in-house resources unavailable)"
    methods="ID1 ID2"
else
    methods="PUBSEQOS ID1 ID2"
fi
if disabled DLL_BUILD; then
    # enable dll plugins for ftds and bdb
    NCBI_LOAD_PLUGINS_FROM_DLLS=1
    export NCBI_LOAD_PLUGINS_FROM_DLLS
fi

# netcache port on localhost
case "$NCBI_CONFIG_OVERRIDES" in
    *.netcache)
    f=~/"$NCBI_CONFIG_OVERRIDES"
    nc="localhost:9135"
    ncs="ids blobs"
    if test -r "$f"; then
        h=`grep '^host *=' $f | sed 's/.*= *//'`
        p=`grep '^port *=' $f | sed 's/.*= *//'`
        if  test -n "$h" && test -n "$p"; then
            nc="$h:$p"
        fi
        s=`grep '^service *=' $f | sed 's/.*= *//'`
        if  test -n "$s"; then
            nc="$s"
        fi
        a=`grep '^name *= *ids' $f | sed 's/.*= *//'`
        b=`grep '^name *= *blobs' $f | sed 's/.*= *//'`
        if  test -z "$a"; then
            a=ids
        fi
        if  test -z "$b"; then
            b=blobs
        fi
        ncs="$a $b"
    fi
    ;;
    *)
    nc="";;
esac

init_cache() {
    if test -n "$nc"; then
        echo "Init netcache $nc/$ncs"
        for c in $ncs; do
            if grid_cli --admin purge --nc "$nc" --auth netcache_admin $c; then
                :
            else
                nc=""
                break
            fi
        done
        if test -n "$nc"; then
            return 0
        fi
        unset NCBI_CONFIG_OVERRIDES
    fi
    if disabled BerkeleyDB; then
        echo BerkeleyDB is not enabled
        return 1
    fi
    if disabled MT; then
        echo MT is not enabled
        return 1
    fi
    if disabled DLL; then
        echo No DLL plugins
        return 1
    fi
    echo "Init BDB cache"
    rm -rf .genbank_cache
    return 0
}

exitcode=0
failed=''
for method in $methods; do
    GENBANK_LOADER_METHOD_BASE="$method"
    export GENBANK_LOADER_METHOD_BASE
    for cache in 1 2 3; do
        GENBANK_ALLOW_INCOMPLETE_COMMANDS=1
        export GENBANK_ALLOW_INCOMPLETE_COMMANDS
        if test "$cache" = 1; then
            m="$method"
            unset GENBANK_ALLOW_INCOMPLETE_COMMANDS
        elif test "$cache" = 2; then
            if init_cache; then
                :
            else
                echo "Skipping cache test"
                break
            fi
            m="cache;$method"
        else
            m="cache"
        fi
        echo "Checking GenBank loader $m (base: $method):"
        GENBANK_LOADER_METHOD="$m"
        export GENBANK_LOADER_METHOD
        $CHECK_EXEC "$@"
        error=$?
        if test $error -ne 0; then
            echo "Test of GenBank loader $m failed (base: $method): $error"
            exitcode=$error
            failed="$failed $m (base: $method)"
            case $error in
            # signal 1 (HUP), 2 (INTR), 9 (KILL), or 15 (TERM).
                129|130|137|143) echo "Apparently killed"; break ;;
            esac
        fi
    done
done

if test $exitcode -ne 0; then
    echo "Failed tests:$failed"
fi
exit $exitcode
