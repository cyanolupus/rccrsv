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
    local testcase_dir=$(dirname $testcase_file)
    local testcase_build_dir="tmp/$testcase_name/build"

    mkdir -p $testcase_build_dir

    cp $testcase_dir/$testcase_name.expected $testcase_build_dir/expected.txt
    ./rccrsv $testcase_file -o $testcase_build_dir/main.s || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mFailed to compile testcase $testcase_name\033[m" >$testcase_build_dir/error.txt
        exit 1
    }
    cc -c -o $testcase_build_dir/main.o $testcase_build_dir/main.s
    cc -c -O0 -o $testcase_build_dir/utils.o test/utils.c
    cc -o $testcase_build_dir/main $testcase_build_dir/main.o $testcase_build_dir/utils.o
}

function testcase_run() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_build_dir="tmp/$testcase_name/build"

    $testcase_build_dir/main >$testcase_build_dir/output.txt 2>>$testcase_build_dir/error.txt || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mFailed to run testcase $testcase_name\033[m" >>$testcase_build_dir/error.txt
        exit 1
    }
}

function testcase_assert() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_build_dir="tmp/$testcase_name/build"

    diff -u $testcase_build_dir/output.txt $testcase_build_dir/expected.txt >$testcase_build_dir/diff.txt && {
        echo -ne "\033[32mo\033[m"
    } || {
        echo -ne "\033[31mx\033[m"
        echo -e "\033[31mTestcase $testcase_name failed\033[m" >>$testcase_build_dir/error.txt
        cat $testcase_build_dir/diff.txt >>$testcase_build_dir/error.txt
        exit 1
    }
}

function testcase_clean() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)
    local testcase_build_dir="tmp/$testcase_name/build"

    rm -rf tmp/$testcase_name
}

function testcase() {
    local testcase_file=$1
    local testcase_name=$(basename $testcase_file .c)

    testcase_build $testcase_file
    testcase_run $testcase_file
    testcase_assert $testcase_file
    testcase_clean $testcase_file
}

function testcases() {
    rccrsv_build

    for testcase_file in $@; do
        testcase $testcase_file
    done

    wait

    echo ""

    for testcase_file in $@; do
        local testcase_name=$(basename $testcase_file .c)
        local testcase_build_dir="tmp/$testcase_name/build"

        if [ -s $testcase_build_dir/error.txt ]; then
            cat $testcase_build_dir/error.txt
        fi
    done
}

testcases $@
