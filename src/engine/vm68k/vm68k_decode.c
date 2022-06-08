/*

Copyright 2019, dettus@dettus.net

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this 
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*/

#include <stdio.h>
#include <stdlib.h>
#include "vm68k_datatypes.h"
#include "vm68k_decode.h"


// the purpose of this function is to perform a pattern matching to the instruction, and return the enumeration value.
// the more bits are constant, the higher should be the matche's priority.
tVM68k_instruction vm68k_decode(tVM68k_uword opcode)
{
	tVM68k_instruction retval=VM68K_INST_UNKNOWN;

	// instructions with 16 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x023c) retval=VM68K_INST_ANDItoCCR;	//ANDItoCCR: 0000 0010 0011 1100 00000000dddddddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x027c) retval=VM68K_INST_ANDItoSR;	//ANDItoSR:  0000 0010 0111 1100 dddddddddddddddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x0A3C) retval=VM68K_INST_EORItoCCR;	//EORItoCCR: 0000 1010 0011 1100 00000000dddddddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x0A7C) retval=VM68K_INST_EORItoSR;	//EORItoSR:  0000 1010 0111 1100 dddddddddddddddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4AFC) retval=VM68K_INST_ILLEGAL;		//ILLEGAL:   0100 1010 1111 1100
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4E71) retval=VM68K_INST_NOP;	
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x003C) retval=VM68K_INST_ORItoCCR;	//ORItoCCR:  0000 0000 0011 1100 00000000dddddddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x007C) retval=VM68K_INST_ORItoSR;		//ORItoSR:   0000 0000 0111 1100 dddddddddddddddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4E70) retval=VM68K_INST_RESET;		//RESET:     0100 1110 0111 0000
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4E73) retval=VM68K_INST_RTE;		//RTE:       0100 1110 0111 0011
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4E77) retval=VM68K_INST_RTR;		//RTR:       0100 1110 0111 0111
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4E75) retval=VM68K_INST_RTS;		//RTS:       0100 1110 0111 0101
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4E72) retval=VM68K_INST_STOP;		//STOP:      0100 1110 0111 0010 iiiiiiiiiiiiiiii
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffff)==0x4E76) retval=VM68K_INST_TRAPV;		//TRAPV:     0100 1110 0111 0110



	// instructions with 13 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xfff8)==0x4E50) retval=VM68K_INST_LINK;		//LINK:      0100 1110 0101 0yyy dddddddddddddddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xfff8)==0x4840) retval=VM68K_INST_SWAP;		//SWAP:      0100 1000 0100 0yyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xfff8)==0x4E58) retval=VM68K_INST_UNLK;		//UNLK:      0100 1110 0101 1yyy

	// instructions with 12 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xfff0)==0x4E40) retval=VM68K_INST_TRAP;		//TRAP:      0100 1110 0100 vvvv
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xfff0)==0x4E60) retval=VM68K_INST_MOVEUSP;		//MOVE USP:  0100 1110 0110 dyyy

	// instructions with 10 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x0800) retval=VM68K_INST_BTSTB;		//BTST.B:    0000 1000 00mm myyy 0000 0000 bbbb bbbb
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x0840) retval=VM68K_INST_BCHGB;		//BCHG.B:    0000 1000 01mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x0880) retval=VM68K_INST_BCLRI;		//BCLRI:     0000 1000 10mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x08C0) retval=VM68K_INST_BSETB;		//BSET.B:    0000 1000 11mm myyy

	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x44C0) retval=VM68K_INST_MOVEtoCCR;	//MOVEtoCCR: 0100 0100 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x40C0) retval=VM68K_INST_MOVEfromSR;	//MOVEfromSR:0100 0000 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x46C0) retval=VM68K_INST_MOVEtoSR;	//MOVEtoSR:  0100 0110 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x4840) retval=VM68K_INST_PEA;		//PEA:       0100 1000 01mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffc0)==0x4AC0) retval=VM68K_INST_TAS;		//TAS:       0100 1010 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffC0)==0x4EC0) retval=VM68K_INST_JMP;		//JMP:       0100 1110 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xffC0)==0x4E80) retval=VM68K_INST_JSR;		//JSR:       0100 1110 10mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xfe38)==0x4800) retval=VM68K_INST_EXT;		//EXT:       0100 100o oo00 0yyy

	// instructions with 9 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf0f8)==0x50C8) retval=VM68K_INST_DBcc;		//DBcc:      0101 CCCC 1100 1yyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1f0)==0xC100) retval=VM68K_INST_ABCD;		//ABCD:      1100 xxx1 0000 myyy 
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1f0)==0x8100) retval=VM68K_INST_SBCD;		//SBCD:      1000 xxx1 0000 ryyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff80)==0x4880) retval=VM68K_INST_MOVEMregtomem;	//MOVEM:     0100 1000 1smm myyy	// reg to mem
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff80)==0x4C80) retval=VM68K_INST_MOVEMmemtoreg;	//MOVEM:     0100 1100 1smm myyy	// mem to reg
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1C0)==0x01C0) retval=VM68K_INST_BSET;		//BSET:      0000 xxx1 11mm myyy

	// instructions with 8 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x4200) retval=VM68K_INST_CLR;		//CLR:       0100 0010 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x0C00) retval=VM68K_INST_CMPI;		//CMPI:      0000 1100 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x0A00) retval=VM68K_INST_EORI;		//EORI:      0000 1010 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x0600) retval=VM68K_INST_ADDI;		//ADDI:      0000 0110 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x0200) retval=VM68K_INST_ANDI;		//ANDI:      0000 0010 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x0000) retval=VM68K_INST_ORI;		//ORI:       0000 0000 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x4400) retval=VM68K_INST_NEG;		//NEG:       0100 0100 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x4000) retval=VM68K_INST_NEGX;		//NEGX:      0100 0000 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x4600) retval=VM68K_INST_NOT;		//NOT:       0100 0110 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x0400) retval=VM68K_INST_SUBI;		//SUBI:      0000 0100 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xff00)==0x4A00) retval=VM68K_INST_TST; 		//TST:       0100 1010 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf0C0)==0xD0C0) retval=VM68K_INST_ADDA;		//ADDA:      1101 rrrs 11mm myyy	// IMPORTANT! THIS HAS TO COME BEFORE ADDX!
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf130)==0xD100) retval=VM68K_INST_ADDX;		//ADDX:      1101 xxx1 ss00 myyy	// s=00,01,10=ADDX. 11=ADDA!!
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf130)==0xC100) retval=VM68K_INST_EXG;		//EXG:       1100 xxx1 oo00 oyyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf0C0)==0x90C0) retval=VM68K_INST_SUBA;		//SUBA:      1001 xxxo 11mm myyy	// probably the same problem as ADDA/ADDX.
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf130)==0x9100) retval=VM68K_INST_SUBX;		//SUBX:      1001 yyy1 ss00 ryyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf0C0)==0xB0C0) retval=VM68K_INST_CMPA;		//CMPA:      1011 xxxo 11mm myyy	/// IMPORANT! THIS HAS TO COME BEFORE CMPM!
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf138)==0xb108) retval=VM68K_INST_CMPM;		//CMPM:      1011 xxx1 ss00 1yyy

	// instructions with 7 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1c0)==0x0140) retval=VM68K_INST_BCHG;		//BCHG:      0000 rrr1 01mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1c0)==0x0180) retval=VM68K_INST_BCLR;		//BCLR:      0000 xxx1 10mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1C0)==0x0100) retval=VM68K_INST_BTST;		//BTST:      0000 xxx1 00mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1C0)==0x81C0) retval=VM68K_INST_DIVS;		//DIVS:      1000 xxx1 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1C0)==0x80C0) retval=VM68K_INST_DIVU;		//DIVU:      1000 xxx0 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1C0)==0xC1C0) retval=VM68K_INST_MULS;		//MULS:      1100 xxx1 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1C0)==0xC0C0) retval=VM68K_INST_MULU;		//MULU:      1100 xxx0 11mm myyy 
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf1C0)==0x41C0) retval=VM68K_INST_LEA;		//LEA:       0100 xxx1 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf038)==0x0008) retval=VM68K_INST_MOVEP;		//MOVEP:     0000 xxxo oo00 1yyy dddddddddddddddd

	// instructions with 6 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf018)==0xe018) retval=VM68K_INST_ROL_ROR;		//ROL/ROR:   1110 cccd ssl1 1yyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf018)==0xe010) retval=VM68K_INST_ROXL_ROXR;	//ROXL/ROXR: 1110 cccd ssl1 0yyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf0c0)==0x50C0) retval=VM68K_INST_SCC;		//SCC:       0101 CCCC 11mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf140)==0x4100) retval=VM68K_INST_CHK;		//CHK:       0100 xxx1 s0mm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf018)==0xE008) retval=VM68K_INST_LSL_LSR; 	//LSL/LSR:   1110 cccd ssl0 1yyy 
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf018)==0xE000) retval=VM68K_INST_ASL_ASR; 	//ASL/ASR:   1110 cccd ssl0 0yyy 

	// instructions with 5 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf100)==0x5000) retval=VM68K_INST_ADDQ;		//ADDQ:      0101 ddd0 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf100)==0xb100) retval=VM68K_INST_EOR;		//EOR:       1011 xxx1 oomm myyy 
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf100)==0x7000) retval=VM68K_INST_MOVEQ;		//MOVEQ:     0111 xxx0 dddd dddd
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf000)==0x9000) retval=VM68K_INST_SUB;		//SUB:       1001 xxx0 oomm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf100)==0x5100) retval=VM68K_INST_SUBQ;		//SUBQ:      0101 ddd1 ssmm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xC1C0)==0x0040) retval=VM68K_INST_MOVEA;		//MOVEA:     00ss xxx0 01mm myyy 

	// instructions with 4 constant bits
	//
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf000)==0xD000) retval=VM68K_INST_ADD;		//ADD:	     1101 rrro oomm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf000)==0xC000) retval=VM68K_INST_AND;		//AND:       1100 xxxo oomm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf000)==0x6000) retval=VM68K_INST_BCC;		//BCC:       0110 CCCC dddd dddd	
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf000)==0xB000) retval=VM68K_INST_CMP;		//CMP:       1011 xxx0 oomm myyy
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xf000)==0x8000) retval=VM68K_INST_OR;		//OR:        1000 xxxo oomm myyy

	// instructions with 2 constant bits
	if (retval==VM68K_INST_UNKNOWN) if ((opcode&0xc000)==0x0000) retval=VM68K_INST_MOVE;		//MOVE:      00ss xxxm mmMM Myyy




	return retval;
}

void vm68k_get_instructionname(tVM68k_instruction instruction,char* name)
{
	switch(instruction)
	{	
		case VM68K_INST_UNKNOWN :	snprintf(name,16,"UNKNOWN");break;
		case VM68K_INST_ABCD :		snprintf(name,16,"ABCD");break;
		case VM68K_INST_ADD :		snprintf(name,16,"ADD");break;
		case VM68K_INST_ADDA :		snprintf(name,16,"ADDA");break;
		case VM68K_INST_ADDI :		snprintf(name,16,"ADDI");break;
		case VM68K_INST_ADDQ :		snprintf(name,16,"ADDQ");break;
		case VM68K_INST_ADDX :		snprintf(name,16,"ADDX");break;
		case VM68K_INST_AND :		snprintf(name,16,"AND");break;
		case VM68K_INST_ANDI :		snprintf(name,16,"ANDI");break;
		case VM68K_INST_ANDItoCCR :	snprintf(name,16,"ANDItoCCR");break;
		case VM68K_INST_ANDItoSR :	snprintf(name,16,"ANDItoSR");break;
		case VM68K_INST_ASL_ASR :	snprintf(name,16,"ASL_ASR");break;
		case VM68K_INST_BCC :		snprintf(name,16,"BCC");break;
		case VM68K_INST_BCHG :		snprintf(name,16,"BCHG");break;
		case VM68K_INST_BCHGB :		snprintf(name,16,"BCHGB");break;
		case VM68K_INST_BCLR :		snprintf(name,16,"BCLR");break;
		case VM68K_INST_BCLRI :		snprintf(name,16,"BCLRI");break;
		case VM68K_INST_BRA :		snprintf(name,16,"BRA");break;
		case VM68K_INST_BSET :		snprintf(name,16,"BSET");break;
		case VM68K_INST_BSETB :		snprintf(name,16,"BSETB");break;
		//case VM68K_INST_BSR :		snprintf(name,16,"BSR");break;
		case VM68K_INST_BTST :		snprintf(name,16,"BTST");break;
		case VM68K_INST_BTSTB :		snprintf(name,16,"BTSTB");break;
		case VM68K_INST_CHK :		snprintf(name,16,"CHK");break;
		case VM68K_INST_CLR :		snprintf(name,16,"CLR");break;
		case VM68K_INST_CMP :		snprintf(name,16,"CMP");break;
		case VM68K_INST_CMPA :		snprintf(name,16,"CMPA");break;
		case VM68K_INST_CMPI :		snprintf(name,16,"CMPI");break;
		case VM68K_INST_CMPM :		snprintf(name,16,"CMPM");break;
		case VM68K_INST_DBcc :		snprintf(name,16,"DBcc");break;
		case VM68K_INST_DIVS :		snprintf(name,16,"DIVS");break;
		case VM68K_INST_DIVU :		snprintf(name,16,"DIVU");break;
		case VM68K_INST_EOR :		snprintf(name,16,"EOR");break;
		case VM68K_INST_EORI :		snprintf(name,16,"EORI");break;
		case VM68K_INST_EORItoCCR :	snprintf(name,16,"EORItoCCR");break;
		case VM68K_INST_EORItoSR :	snprintf(name,16,"EORItoSR");break;
		case VM68K_INST_EXG :		snprintf(name,16,"EXG");break;
		case VM68K_INST_EXT :		snprintf(name,16,"EXT");break;
		case VM68K_INST_ILLEGAL :	snprintf(name,16,"ILLEGAL");break;
		case VM68K_INST_JMP :		snprintf(name,16,"JMP");break;
		case VM68K_INST_JSR :		snprintf(name,16,"JSR");break;
		case VM68K_INST_LEA :		snprintf(name,16,"LEA");break;
		case VM68K_INST_LINK :		snprintf(name,16,"LINK");break;
		case VM68K_INST_LSL_LSR :	snprintf(name,16,"LSL_LSR");break;
		case VM68K_INST_MOVE :		snprintf(name,16,"MOVE");break;
		case VM68K_INST_MOVEA :		snprintf(name,16,"MOVEA");break;
		case VM68K_INST_MOVEtoCCR :	snprintf(name,16,"MOVEtoCCR");break;
		case VM68K_INST_MOVEfromSR :	snprintf(name,16,"MOVEfromSR");break;
		case VM68K_INST_MOVEtoSR :	snprintf(name,16,"MOVEtoSR");break;
		case VM68K_INST_MOVEUSP :	snprintf(name,16,"MOVEUSP");break;
		case VM68K_INST_MOVEMregtomem :	snprintf(name,16,"MOVEMregtomem");break;
		case VM68K_INST_MOVEMmemtoreg :	snprintf(name,16,"MOVEMmemtoreg");break;
		case VM68K_INST_MOVEP :		snprintf(name,16,"MOVEP");break;
		case VM68K_INST_MOVEQ :		snprintf(name,16,"MOVEQ");break;
		case VM68K_INST_MULS :		snprintf(name,16,"MULS");break;
		case VM68K_INST_MULU :		snprintf(name,16,"MULU");break;
		case VM68K_INST_NBCD :		snprintf(name,16,"NBCD");break;
		case VM68K_INST_NEG :		snprintf(name,16,"NEG");break;
		case VM68K_INST_NEGX :		snprintf(name,16,"NEGX");break;
		case VM68K_INST_NOP :		snprintf(name,16,"NOP");break;
		case VM68K_INST_NOT :		snprintf(name,16,"NOT");break;
		case VM68K_INST_OR :		snprintf(name,16,"OR");break;
		case VM68K_INST_ORI :		snprintf(name,16,"ORI");break;
		case VM68K_INST_ORItoCCR :	snprintf(name,16,"ORItoCCR");break;
		case VM68K_INST_ORItoSR :	snprintf(name,16,"ORItoSR");break;
		case VM68K_INST_PEA :		snprintf(name,16,"PEA");break;
		case VM68K_INST_RESET :		snprintf(name,16,"RESET");break;
		case VM68K_INST_ROL_ROR :	snprintf(name,16,"ROL_ROR");break;
		case VM68K_INST_ROXL_ROXR :	snprintf(name,16,"ROXL_ROXR");break;
		case VM68K_INST_RTE :		snprintf(name,16,"RTE");break;
		case VM68K_INST_RTR :		snprintf(name,16,"RTR");break;
		case VM68K_INST_RTS :		snprintf(name,16,"RTS");break;
		case VM68K_INST_SBCD :		snprintf(name,16,"SBCD");break;
		case VM68K_INST_SCC :		snprintf(name,16,"SCC");break;
		case VM68K_INST_STOP :		snprintf(name,16,"STOP");break;
		case VM68K_INST_SUB :		snprintf(name,16,"SUB");break;
		case VM68K_INST_SUBA :		snprintf(name,16,"SUBA");break;
		case VM68K_INST_SUBI :		snprintf(name,16,"SUBI");break;
		case VM68K_INST_SUBQ :		snprintf(name,16,"SUBQ");break;
		case VM68K_INST_SUBX :		snprintf(name,16,"SUBX");break;
		case VM68K_INST_SWAP :		snprintf(name,16,"SWAP");break;
		case VM68K_INST_TAS :		snprintf(name,16,"TAS");break;
		case VM68K_INST_TRAP :		snprintf(name,16,"TRAP");break;
		case VM68K_INST_TRAPV :		snprintf(name,16,"TRAPV");break;
		case VM68K_INST_TST :		snprintf(name,16,"TST");break;
		case VM68K_INST_UNLK :		snprintf(name,16,"UNLK");break;
	}

}
