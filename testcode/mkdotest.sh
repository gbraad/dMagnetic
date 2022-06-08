#!/bin/sh

export SHA256_CMD=sha256sum
export ECHO_CMD=echo
export AWK_CMD=awk

echo "export SHA256_CMD="$SHA256_CMD
echo "export ECHO_CMD="$ECHO_CMD
echo "export AWK_CMD="$AWK_CMD

echo "\tif [ true \\"
for I in none monochrome monochrome_inv low_ansi low_ansi2 high_ansi high_ansi2 sixel
do
	(
		echo -n "\${ECHO_CMD} Hello | ./dMagnetic -ini dMagnetic.ini -vmode "$I" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | \${SHA256_CMD} | \${AWK_CMD} -F' ' '{ print \$1; }' - "	
	)	>tmp1.sh
	export TEST_OUT=`sh tmp1.sh`
	echo "\t\t-a \"\``cat tmp1.sh`\`\" = \""$TEST_OUT"\" \\"
done
echo "\t] ; then echo OK ; else echo \"expected output not seen\"; exit 1; fi"

