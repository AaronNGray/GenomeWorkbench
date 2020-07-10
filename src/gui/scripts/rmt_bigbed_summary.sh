#!/bin/bash

# parameters are strictly ordered and are all required

# 1. source file URL
# 2. remote file id (chr1)
# 3. range start
# 4. range_stop
# 5. number of data points 
# 6. target file location (determined by caller, should be writable)
# 7  mode (feats |summary)


BINDIR=.

GetContentType()
{
    local filetype=$1
    case "${filetype}" in
    bigBed) echo "bed" ;;
    bigWig) echo "wig" ;;
    esac
}


RetrieveRemoteData()
{
    case "${MODE}" in
    feats)
        retriever=${BINDIR}/bigBedToBed 
        ${BINDIR}/bigBedToBed "${REMOTE_URL}" /dev/stdout "-chrom=${CHR}" "-start=${START}" "-end=${END}" | awk 'BEGIN{OFS="\t"}{print $2,$3}' > $OUT_FILE
        ;;
    summary)
        ${BINDIR}/bigBedSummary "${REMOTE_URL}" "${CHR}" "${START}" "${END}" "${DP}" -type=max > $OUT_FILE
        ;;
    esac

}

##echo $0 $1 $2 $3 $4 $5 $6 $7 $8 $9

REMOTE_URL="$1"
OUT_FILE="$6"
MODE="$7"

if [[ -z "${REMOTE_URL}" ]]
then
    exit
fi
CHR="$2"
START="$3"
END="$4"
DP="$5"
RetrieveRemoteData 

