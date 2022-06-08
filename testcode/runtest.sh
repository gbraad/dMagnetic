#!/bin/sh


echo "If all goes well, this script should say nothing."
bzcat expected.log.bz2 >/tmp/expected.log
( echo "HELLO world" | ../dMagnetic -vcols 300 -vrows 300 -vmode high_ansi -vecho -mag minitest.mag )>/tmp/is.log

diff /tmp/expected.log /tmp/is.log
