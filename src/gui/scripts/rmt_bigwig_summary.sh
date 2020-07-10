#!/bin/bash

# parameters are strictly ordered and are all required

# 1. source file URL
# 2. remote file id (chr1)
# 3. range start
# 4. range_stop
# 5. number of data points 
# 6. target file location (determined by caller, should be writable)
# 7. Summary type

BINDIR=.


##echo $0 $1 $2 $3 $4 $5 $6 $7 $8 $9

REMOTE_URL="$1"
OUT_FILE="$6"
ST="$7"

if [[ -z "${REMOTE_URL}" ]]
then
    exit
fi
CHR="$2"
START="$3"
END="$4"
DP="$5"

if [[ $ST == "data" ]]
then

    ${BINDIR}/bigWigToBedGraph -chrom=${CHR} -start=${START} -end=${END} ${REMOTE_URL} ${OUT_FILE}
else
    ${BINDIR}/bigWigSummaryEx "${REMOTE_URL}" "${CHR}" "${START}" "${END}" "${DP}" > $OUT_FILE
fi    

