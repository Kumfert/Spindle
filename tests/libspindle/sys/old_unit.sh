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

echo "NPARTS 4"
final_result='UNRESOLVED'

i='3';

# test01 test02
echo "PART 1"
echo "RESULT 1 XFAIL"
echo "PART 2"
echo "RESULT 2 XFAIL"
final_result='XFAIL';

for test_i in  test04 test05; do
    echo "PART $i"
    if test -x ${bindir}/${test_i}.exe ; then 
	${bindir}/${test_i}.exe > ${bindir}/${test_i}.tmp 2>&1 ;
	perl -p -i -e 's/WARNING: Instance.*\n//' ${bindir}/${test_i}.tmp;
	# remove Warnings about Instance counting disabled.
	if diff ${bindir}/${test_i}.tmp ${test_i}.out 2>&1 ; then
	    result='PASS';
	    if test "$final_result" != 'FAIL'; then 
		final_result='PASS';
	    fi
	else
	    echo "ERROR: Discrepancy found between actual and expected output."
	    result='FAIL';
	    final_result='FAIL';
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
