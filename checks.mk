#Copyright 2020, dettus@dettus.net
#
#Redistribution and use in source and binary forms, with or without modification,
#are permitted provided that the following conditions are met:
#
#1. Redistributions of source code must retain the above copyright notice, this 
#   list of conditions and the following disclaimer.
#
#2. Redistributions in binary form must reproduce the above copyright notice, 
#   this list of conditions and the following disclaimer in the documentation 
#   and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
#WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


### the following lines are the post-compilation test. This is a formality on some operating systems ###########################

ECHO_CMD?=echo
SHA256_CMD?=sha256
AWK_CMD?=awk

## the checks work by checking the sha256 sum of the output. 
## since this one relies on the input, as well as the .ini file,
## it might not work as a post-install check. it is more of a 
## post-compilation check, to see if it would support a new platform.


## check those graphic modes
CHECKS=        \
	check-none      \
	check-monochrome        \
	check-monochrome_inv    \
	check-low_ansi  \
	check-low_ansi2 \
	check-high_ansi \
	check-high_ansi2        \
	check-sixel

### this is the input. it does not really matter, but I thought I put in some easter egg in here.
### :::W WEYHE IST BUNT!!
INPUT_none=		"Weyhe-Leeste"
INPUT_monochrome=	"Sudweyhe"
INPUT_monochrome_inv=	"Melchiorshausen"
INPUT_low_ansi=		"Ahausen"
INPUT_low_ansi2=	"Kirchweyhe"
INPUT_high_ansi=	"Jebel"
INPUT_high_ansi2=	"Erichshof"
INPUT_sixel=		"Dreye"

### this is the output
CHECKSUM_none=		"3211640dc669f6b960a84a51559fc88a25bbc26966f01cdf44b9f4d9f4d71e1c"
CHECKSUM_monochrome=	"d8bc9ccf30b5cc53f545ac5458e1a864e27c18c3debdb9ac427886237997e415"
CHECKSUM_monochrome_inv="79ff5931e4b8b0812bfa15c49fce9dde985495e962dacd59c3e61d881bc91e01"
CHECKSUM_low_ansi=	"7ce4a85016794d60979e9b3543c036e30514e0cb3097a569cfb9b48506c567be"
CHECKSUM_low_ansi2=	"05e7fdd4d2ae3476db0baa38bf5dac6255a13cc46d71505c4d63a57cfa40e1fe"
CHECKSUM_high_ansi=	"48dada903b5196311f1f73aea3efd592a1120dc3731003ceac099b9be0df2cf4"
CHECKSUM_high_ansi2=	"e83e480afc67de933cc51dacf7a68d3c5c3fa4221fb299edf4bda495caf68bdc"
CHECKSUM_sixel=		"eb20c0de385696b55320ce15e703690e38be2e238b7dbddfb424b54023286180"



## so, here is my problem: I wanted to be able to run those checks in GNU make as well as BSD make.
## both of them have great features, like check-%, for-loops and check-{none,monochrome,sixel} as 
## targets. but none of those features worked in both makes.
## the only way it truely did what it was supposed to do was by having a little bit of spaghettified
## code. please enjoy.
##
## btw: if you want to run the checks on Linux, run 
## make SHA256_CMD="sha256sum" check


# the code for the 8 checks is IDENTICAL. the only difference is the target's name. 
check-none:		dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-monochrome:	dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-monochrome_inv:	dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-low_ansi:		dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-low_ansi2:	dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-high_ansi:	dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-high_ansi2:	dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-sixel:		dMagnetic
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi


		
############## invoke all the tests ############################################
check:	${CHECKS}
	@${ECHO_CMD} "***********************************************"
	@${ECHO_CMD} "Post-compilation tests for dMagnetic successful"
	@${ECHO_CMD} "***********************************************"

do-test:	check

