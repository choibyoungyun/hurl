#!/bin/sh

TCASE_DATE=`date +%Y%m%d%H%m%S`
TCASE_CMD="/home/bychoi/workspace/KT114/src/app/hurl/opt/seigw/client/seigwd"
TCASE_DIR="/home/bychoi/workspace/KT114/src/app/hurl/opt/seigw/tsuite"
TCASE_LIST=`ls ${TCASE_DIR}/tcase/*.in`
#TCASE_OUT="${TCASE_DIR}/tcase_result_${TCASE_DATE}.result"

for tcase_suite in ${TCASE_LIST}
do
    TCASE_FNAME="`basename ${tcase_suite} | awk -F. '{print $1}'`".out
    TCASE_OUT="${TCASE_DIR}/tout/${TCASE_FNAME}"

    echo "-----------------------------------------------" >> ${TCASE_OUT}
    grep "DESC" ${tcase_suite} >> ${TCASE_OUT}
    echo "-----------------------------------------------" >> ${TCASE_OUT}
    ${TCASE_CMD} -i ${tcase_suite} >> ${TCASE_OUT}
done
