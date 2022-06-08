#!/bin/sh

echo "-------------[ netbsd ]------------"
echo	"\t cd \${WRKSRC}/testcode;  if [ true \\"
for I in none monochrome low_ansi high_ansi sixel
do
	echo "\t\t-a \`echo Hello | ../dMagnetic -ini ../dMagnetic.ini \\"
	echo "\t\t\t-mag minitest.mag -gfx minitest.gfx \\"
	echo "\t\t\t-vmode "$I" -vcols 300 -vrows 300 \\"
	echo -n "\t\t| md5 \`== "
	echo -n `echo Hello | ../dMagnetic -ini ../dMagnetic.ini -mag minitest.mag -gfx minitest.gfx -vmode $I -vcols 300 -vrows 300 | md5sum | awk -F" " '{ print $1; }' -`
	echo " \\"
done
echo "\t\t]; \\"
echo "\t\tthen echo ok; else echo expected output not seen; exit 1; fi"
echo

echo "-------------[ openbsd ]------------"
echo	"\t cd \${WRKSRC}/testcode;  if [ true \\"
for I in none monochrome low_ansi high_ansi sixel
do
	echo "\t\t-a \`echo Hello | ../dMagnetic -ini ../dMagnetic.ini \\"
	echo "\t\t\t-mag minitest.mag -gfx minitest.gfx \\"
	echo "\t\t\t-vmode "$I" -vcols 300 -vrows 300 \\"
	echo -n "\t\t| sha256 -b \`== "
	echo -n `echo Hello | ../dMagnetic -ini ../dMagnetic.ini -mag minitest.mag -gfx minitest.gfx -vmode $I -vcols 300 -vrows 300 | sha256 -b | awk -F" " '{ print $1; }' -`
	echo " \\"
done
echo "\t\t]; \\"
echo "\t\tthen echo ok; else echo expected output not seen; exit 1; fi"
echo

echo "-------------[ freebsd ]------------"
echo	"\t cd \${WRKSRC}/testcode;  if [ true \\"
for I in none monochrome low_ansi high_ansi sixel
do
	echo "\t\t-a \`echo Hello | ../dMagnetic -ini ../dMagnetic.ini \\"
	echo "\t\t\t-mag minitest.mag -gfx minitest.gfx \\"
	echo "\t\t\t-vmode "$I" -vcols 300 -vrows 300 \\"
	echo -n "\t\t| sha256 \`== "
	echo -n `echo Hello | ../dMagnetic -ini ../dMagnetic.ini -mag minitest.mag -gfx minitest.gfx -vmode $I -vcols 300 -vrows 300 | sha256sum | awk -F" " '{ print $1; }' -`
	echo " \\"
done
echo "\t\t]; \\"
echo "\t\tthen echo ok; else echo expected output not seen; exit 1; fi"
echo

