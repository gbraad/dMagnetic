#Copyright 2022, dettus@dettus.net
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
	check-sixel	\
	check-utf

### this is the input. it does not really matter, but I thought I put in some easter egg in here.
### :::W WEYHE IST BUNT!!
INPUT_none=		"Weyhe-Leeste"
INPUT_monochrome=	"Sudweyhe"
INPUT_monochrome_inv=	"Melchiorshausen"
INPUT_low_ansi=		"Ahausen"
INPUT_low_ansi2=	"Kirchweyhe"
INPUT_high_ansi=	"Jeebel"
INPUT_high_ansi2=	"Erichshof"
INPUT_sixel=		"Dreye"
INPUT_utf=		"Lahausen"

### this is the output
CHECKSUM_none=		"3211640dc669f6b960a84a51559fc88a25bbc26966f01cdf44b9f4d9f4d71e1c"
CHECKSUM_monochrome=	"aa0614428ccdb7e7806a4f829f9ababa8c109e7487437fab1bf449fb2534c98b"
CHECKSUM_monochrome_inv="56946544673dfbfa0d3769bbbab82bf942e8dcfc2dabf463cc734ac0133ea53f"
CHECKSUM_low_ansi=	"ab7f5805f56e453a75d93910321e27d99d6ebd23c36c5af12f4017bbffa650ef"
CHECKSUM_low_ansi2=	"c7b8c38ecb041fc71e0e29dbd93bb8687d15205893c0edf90c02ae8fc4ddf45d"
CHECKSUM_high_ansi=	"bcc24e07c6e207bd4988e11d7e3cbaa0594820670bf1f3df530722700934e970"
CHECKSUM_high_ansi2=	"e83e480afc67de933cc51dacf7a68d3c5c3fa4221fb299edf4bda495caf68bdc"
CHECKSUM_sixel=		"f6dafb92b7da82cedb14f1620064dc1da93b6f36ddf8b7989c95831a625b0a00"
CHECKSUM_utf=		"f484470b10a415b6a6aea37778055c7a2649e285078a32f33ef558de0228de85"



## so, here is my problem: I wanted to be able to run those checks in GNU make as well as BSD make.
## both of them have great features, like check-%, for-loops and check-{none,monochrome,sixel} as 
## targets. but none of those features worked in both makes.
## the only way it truely did what it was supposed to do was by having a little bit of spaghettified
## code. please enjoy.
##
## btw: if you want to run the checks on Linux, run 
## make SHA256_CMD="sha256sum" check


# the code for the 8 checks is IDENTICAL. the only difference is the target's name. 
check-none:		dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-monochrome:	dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-monochrome_inv:	dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-low_ansi:		dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-low_ansi2:	dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-high_ansi:	dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-high_ansi2:	dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-sixel:		dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi

check-utf:		dMagnetic dMagnetic.ini
	if [ "`${ECHO_CMD} ${INPUT_${@:check-%=%}}    | ./dMagnetic -ini dMagnetic.ini -vmode "${@:check-%=%}" -vcols 300 -vrows 300 -vecho -sres 1024x768 -mag testcode/minitest.mag | ${SHA256_CMD} | ${AWK_CMD} -F' ' '{ print $$1; }' - `" = ${CHECKSUM_${@:check-%=%}} ]       ; then ${ECHO_CMD} "$@ OK" ; else ${ECHO_CMD} "$@ failed" ; exit 1 ; fi


		
############## invoke all the tests ############################################
check:	${CHECKS}
	@${ECHO_CMD} "***********************************************"
	@${ECHO_CMD} "Post-compilation tests for dMagnetic successful"
	@${ECHO_CMD} "***********************************************"

do-test:	check

