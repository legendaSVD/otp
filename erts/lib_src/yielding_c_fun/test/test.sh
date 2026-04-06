#!/bin/bash
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
GC=$1
RR=-rr
RR=""
PATH=$PATH:$DIR/../bin
set -e
set -x
TMP_DIR=$DIR/tmp_dir
mkdir -p $TMP_DIR
TMP_FILE=$TMP_DIR/tmp
TMP_C_FILE=$TMP_DIR/tmp.c
TMP_INC_FILE=$TMP_DIR/tmp.ycf.h
TMP_C_FILE2=$TMP_DIR/tmp2.c
TMP_O_FILE1=$TMP_DIR/tmp1.o
TMP_O_FILE2=$TMP_DIR/tmp2.o
TMP_H_FILE=$TMP_DIR/tmp.h
TMP_CC_OUT=$TMP_DIR/a.out
CC_ARGS="-std=c99 -pedantic -Wall -g"
CC=clang
SIMPLE_TEST_FILES=("$DIR/examples/simple_yield.c"
                   "$DIR/examples/multi_scope_yield.c"
                   "$DIR/examples/nested_loop_yield.c"
                   "$DIR/examples/void_ret_fun.c"
                   "$DIR/examples/declarations.c"
                   "$DIR/examples/void_param.c"
                   "$DIR/examples/simple_fun_call.c"
                   "$DIR/examples/control_statements.c"
                   "$DIR/examples/simple_yielding_fun_call.c"
                   "$DIR/examples/yielding_mutual_recursion.c"
                   "$DIR/examples/stack_array.c"
                   "$DIR/examples/custom_code_save_restore_yield_state.c"
                   "$DIR/examples/custom_code_save_restore_yield_state_alt_syntax.c"
                   "$DIR/examples/destroy_while_yielded.c"
                   "$DIR/examples/consume_reds.c"
                   "$DIR/examples/consume_reds_variable.c"
                   "$DIR/examples/nested_call_consume_reds.c"
                   "$DIR/examples/auto_yield.c"
                   "$DIR/examples/yield_with_struct.c"
                   "$DIR/examples/const_defenition.c"
                   "$DIR/examples/ycf_stack_alloc.c"
		   "$DIR/examples/declarations_in_for_loops.c"
                   "$DIR/examples/in_code_var_declaration.c"
                   "$DIR/examples/static_inline_function.c"
                   "$DIR/examples/yielding_fun_in_control.c"
                   "$DIR/examples/debug_ptr_to_stack.c"
                   "$DIR/examples/simple_yield_no_reds.c"
                   "$DIR/examples/thread_example.c")
SIMPLE_TEST_FILES_YIELD_FUNS=("-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun -fnoauto sub_fun_1 -fnoauto sub_fun_2 -fnoauto sub_fun_3"
                              "-fnoauto A -fnoauto B"
                              "-fnoauto fun"
                              "-fnoauto fun -fnoauto fun2"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun -fnoauto sub_fun"
                              "-frec fun -frec rec_inc"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun -fnoauto sub_fun"
			      "-f fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-fnoauto fun -fnoauto sub_fun"
                              "-fnoauto fun"
                              "-fnoauto fun"
                              "-f f_1 -f f_2")
for C_FILE in $SIMPLE_TEST_FILES
do
    yielding_c_fun $GC $RR -repeat $C_FILE > $TMP_C_FILE
    cmp $C_FILE $TMP_C_FILE
done
yielding_c_fun $GC -yield $DIR/examples/simple_yield.c > $TMP_C_FILE
yielding_c_fun $GC $DIR/examples/simple_yield.c > $TMP_C_FILE2
cmp $TMP_C_FILE $TMP_C_FILE2
for ((i = 0; i < ${
do
    yielding_c_fun $GC $RR -yield ${SIMPLE_TEST_FILES_YIELD_FUNS[$i]} "${SIMPLE_TEST_FILES[$i]}" > $TMP_C_FILE
    $CC $CC_ARGS -g $TMP_C_FILE -o $TMP_CC_OUT
    $TMP_CC_OUT > $TMP_FILE
    cmp $TMP_FILE "${SIMPLE_TEST_FILES[$i]}.out"
done
yielding_c_fun $GC $RR -yield -fnoauto A -fnoauto B -header_file_name $TMP_H_FILE -output_file_name $TMP_C_FILE "$DIR/examples/test_generated_header_file_code.c"
$CC $CC_ARGS -c $TMP_C_FILE  -o $TMP_O_FILE1
$CC $CC_ARGS -I$DIR -DYCF_YIELD_CODE_GENERATED=1 -c "$DIR/examples/test_generated_header_file_main.c"  -o $TMP_O_FILE2
$CC $CC_ARGS $TMP_O_FILE1 $TMP_O_FILE2 -o $TMP_CC_OUT
$TMP_CC_OUT > $TMP_FILE
cmp $TMP_FILE "$DIR/examples/test_generated_header_file_main.c.out"
yielding_c_fun.bin $GC $RR -yield -static_aux_funs -only_yielding_funs -fnoauto fun -output_file_name $TMP_INC_FILE "$DIR/examples/test_only_output_yielding_funs.c"
$CC $CC_ARGS -I "$TMP_DIR" "$DIR/examples/test_only_output_yielding_funs.c"  -o $TMP_CC_OUT
$TMP_CC_OUT > $TMP_FILE
cmp $TMP_FILE "$DIR/examples/test_only_output_yielding_funs.c.out"
yielding_c_fun $GC $RR -yield -debug -fnoauto fun "$DIR/examples/debug_ptr_to_stack.c" > $TMP_C_FILE
$CC $CC_ARGS $TMP_C_FILE -o $TMP_CC_OUT
(set +e ; $TMP_CC_OUT ; [ $? -ne 0 ])
yielding_c_fun.bin $GC $RR -yield \
                           -static_aux_funs \
                           -debug \
                           -only_yielding_funs \
                           -fnoauto fun \
                           -fnoauto fun2 \
                           -output_file_name $TMP_INC_FILE \
                           "$DIR/examples/test_only_output_yielding_funs_ptr_to_stack.c"
$CC $CC_ARGS -I "$TMP_DIR" "$DIR/examples/test_only_output_yielding_funs_ptr_to_stack.c"  -o $TMP_CC_OUT
(set +e ; $TMP_CC_OUT ; [ $? -ne 0 ])
(set +e ; yielding_c_fun.bin $GC $RR -yield -f fun -f sub_fun2 -f sub_fun "$DIR/examples/ycf_cannot_transform_fun_call.c" > $TMP_C_FILE ; [ $? -ne 0 ])
MEM_LOG_FILE="$TMP_DIR/my_mem_log_file.txt"
yielding_c_fun $GC $RR -log_max_mem_usage "$MEM_LOG_FILE" -yield -debug -fnoauto fun "$DIR/examples/multi_scope_yield.c" > $TMP_C_FILE
test -f "$MEM_LOG_FILE"