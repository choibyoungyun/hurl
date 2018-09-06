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
    local file_suffix=$1
    local fdir=""
    local base_fname=""
    local full_fanme=""

    if [ ${file_suffix} = ${TCASE_RESULT_SUFFIX} ]
    then
        fdir="${TCASE_RESULT_DIR}"
    elif [ ${file_suffix} = ${TCASE_OUT_SUFFIX} ]
    then
        fdir="${TCASE_OUT_DIR}"
    else
        echo "unknown suffix: $file_suffix"
        exit;
    fi


    echo "DO TEST : $file_suffix"
    for tcase_suite in ${TCASE_LIST}
    do
        base_fname="`basename ${tcase_suite} | awk -F. '{print $1}'`".${file_suffix}
        full_fname="$fdir/${base_fname}"

        echo "-----------------------------------------------" > ${full_fname}
        grep "DESC" ${tcase_suite} >> ${full_fname}
        echo "-----------------------------------------------" >> ${full_fname}
        ${TCASE_CMD} -i ${tcase_suite} >> ${full_fname}
    done
}



function do_verify()
{
    local out_suffix=${TCASE_OUT_SUFFIX}
    local out_dir=${TCASE_OUT_DIR}
    local result_suffix=${TCASE_RESULT_SUFFIX}
    local result_dir=${TCASE_RESULT_DIR}
    local out_base_fname=""
    local result_base_fname=""


    echo "DO VERIFY : $file_suffix"
    for tcase_suite in ${TCASE_LIST}
    do
        out_base_fname="`basename ${tcase_suite} | awk -F. '{print $1}'`".${out_suffix}
        result_base_fname="`basename ${tcase_suite} | awk -F. '{print $1}'`".${result_suffix}

        out_full_fname="$out_dir/${out_base_fname}"
        result_full_fname="$result_dir/${result_base_fname}"

        diff $out_full_fname $result_full_fname
        if [ $? != 0 ]
        then
            echo --------------------------------------------------
            echo "tcase error : ${out_base_fname}"
            echo --------------------------------------------------
        fi
    done
}


if [ $1 = "verify" ]
then
    do_test result
else
    # -----------------------------------------------------------------------
    #  execute TEST
    # -----------------------------------------------------------------------
    do_test out

    # -----------------------------------------------------------------------
    #  execute VERIFY
    # -----------------------------------------------------------------------
    do_verify
fi
