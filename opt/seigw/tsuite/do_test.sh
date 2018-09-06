#!/bin/sh


TCASE_DATE=`date +%Y%m%d%H%m%S`
TCASE_CMD="${WORKHOME}/src/app/hurl/opt/seigw/client/seigwd"
TCASE_DIR="${WORKHOME}/src/app/hurl/opt/seigw/tsuite"
TCASE_LIST=`ls ${TCASE_DIR}/tcase/*.in`

TCASE_OUT_DIR="${TCASE_DIR}/tout"
TCASE_OUT_SUFFIX="out"
TCASE_RESULT_DIR="${TCASE_DIR}/tresult"
TCASE_RESULT_SUFFIX="result"


function do_test()
{
    local tcase=$1

    full_fname="${TCASE_DIR}/tcase/${tcase}.in"
    ${TCASE_CMD} -i ${full_fname}
}
do_test $1
