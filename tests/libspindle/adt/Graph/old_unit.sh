#! /bin/sh

## resolve $bindir
bindir=".";
for option 
do
    case "$option" in
    --*=*) optionarg=`echo "$option" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optionarg= ;;
    esac

    case "$option" in 
    --bindir=*) bindir="$optionarg" ;;
    esac
done

echo "NPARTS 6"
final_result='UNRESOLVED'

i='1';

for test_i in test01 test02 test03 test10 test11 test12; do
    echo "PART $i"
    if test -x ${bindir}/${test_i}.exe ; then 
	${bindir}/${test_i}.exe > ${bindir}/${test_i}.tmp 2>&1 ;
	if diff ${bindir}/${test_i}.tmp ${test_i}.out 2>&1 ; then
	    result='PASS';
	    if test "$final_result" != 'FAIL'; then 
		final_result='PASS';
	    fi
	else
	    echo "ERROR: Discrepancy found between actual and expected output."
	    result='FAIL';
	fi
	rm -f ${bindir}/${test_i}.tmp;
    else
	echo "ERROR: Test program $test_i not found."
	result='UNRESOLVED';
	final_result='FAIL';
    fi
    echo "RESULT $i $result"
    i=`expr $i + 1`
done

if test "$final_result" = 'UNRESOLVED'; then
    final_result='FAIL';
fi
echo "FINAL_RESULT $final_result"
