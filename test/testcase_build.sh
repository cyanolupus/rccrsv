set -e

function rccrsv_build() {
    make rccrsv >/dev/null || {
        echo "\e[31mFailed to build rccrsv\e[m"
        exit 1
    }
}

function testcase_build() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_tmp_dir="tmp/$testcase_name"
    local testcase_build_dir="$testcase_tmp_dir/build"
    local test_dir="../../../test"
    local testcases_dir="../../../test/testcases"

    mkdir -p $testcase_build_dir
    touch $testcase_build_dir/error_log.txt

    ln -sf $testcases_dir/$testcase_name.c $testcase_build_dir/main.c
    ln -sf $test_dir/utils.c $testcase_build_dir/utils.c
    if [ -z $NO_RCC ]; then
        ./rccrsv $testcase_build_dir/main.c -o $testcase_tmp_dir/main.s 2>$testcase_build_dir/error_log.txt || {
            echo -ne "\033[31mx\033[m"
            echo -e "\033[31mFailed to compile testcase $testcase_name\033[m" >>$testcase_tmp_dir/error.txt
            cat $testcase_build_dir/error_log.txt >>$testcase_tmp_dir/error.txt
            exit 1
        }
    fi
    cc -S -o $testcase_tmp_dir/main_cc.s $testcase_build_dir/main.c
    cc -c -o $testcase_build_dir/main.o $testcase_tmp_dir/main.s 2>>$testcase_build_dir/error_log.txt || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mFailed to assemble testcase $testcase_name\033[m" >>$testcase_tmp_dir/error.txt
        cat $testcase_build_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        exit 1
    }
    cc -c -O0 -o $testcase_build_dir/main_cc.o $testcase_build_dir/main.c 2>>$testcase_build_dir/error_log.txt || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mFailed to compile testcase $testcase_name with cc\033[m" >>$testcase_tmp_dir/error.txt
        cat $testcase_build_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        exit 1
    }
    cc -c -O0 -o $testcase_build_dir/utils.o $testcase_build_dir/utils.c
    cc -o $testcase_tmp_dir/main_cc $testcase_build_dir/main_cc.o $testcase_build_dir/utils.o
    cc -o $testcase_tmp_dir/main $testcase_build_dir/main.o $testcase_build_dir/utils.o
}

function testcase_run() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_tmp_dir="tmp/$testcase_name"
    local testcase_build_dir="$testcase_tmp_dir/build"
    local testcase_run_dir="$testcase_tmp_dir/run"

    mkdir -p $testcase_run_dir
    touch $testcase_run_dir/error_log.txt

    $testcase_tmp_dir/main >$testcase_tmp_dir/output.txt 2>$testcase_run_dir/error_log.txt || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mFailed to run testcase $testcase_name\033[m" >>$testcase_tmp_dir/error.txt
        cat $testcase_build_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        cat $testcase_run_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        exit 1
    }
    $testcase_tmp_dir/main_cc >$testcase_tmp_dir/output_cc.txt 2>>$testcase_run_dir/error_log.txt || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mFailed to run testcase $testcase_name with cc\033[m" >>$testcase_tmp_dir/error.txt
        cat $testcase_build_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        cat $testcase_run_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        exit 1
    }
}

function testcase_assert() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_tmp_dir="tmp/$testcase_name"
    local testcase_build_dir="$testcase_tmp_dir/build"
    local testcase_run_dir="$testcase_tmp_dir/run"

    diff -u $testcase_tmp_dir/output.txt $testcase_tmp_dir/output_cc.txt >$testcase_tmp_dir/diff.txt && {
        echo -ne "\033[32mo\033[m"
    } || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mTestcase $testcase_name failed\033[m" >>$testcase_tmp_dir/error.txt
        cat $testcase_build_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        cat $testcase_run_dir/error_log.txt >>$testcase_tmp_dir/error.txt
        cat $testcase_tmp_dir/diff.txt >>$testcase_tmp_dir/error.txt
        exit 1
    }
}

function testcase_clean() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_tmp_dir="tmp/$testcase_name"

    if [ -z $NOCLEAN ]; then
        rm -rf $testcase_tmp_dir
    fi
}

function testcase() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_tmp_dir="tmp/$testcase_name"

    mkdir -p $testcase_tmp_dir
    echo -n "" >$testcase_tmp_dir/error.txt

    testcase_build $testcase_file
    testcase_run $testcase_file
    testcase_assert $testcase_file
    testcase_clean $testcase_file
}

function testcases() {
    rccrsv_build

    for testcase_file in $@; do
        testcase $testcase_file &
    done

    wait

    echo ""

    for testcase_file in $@; do
        local testcase_name=$(basename $testcase_file .c)
        local testcase_tmp_dir="tmp/$testcase_name"

        if [ -s $testcase_tmp_dir/error.txt ]; then
            cat $testcase_tmp_dir/error.txt
        fi
    done
}

testcases $@
