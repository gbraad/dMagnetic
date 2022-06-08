#include <stdio.h>
#include <stdlib.h>
#include "vm68k_datatypes.h"
#include "vm68k_decode.h"

int main(int argc,char** argv)
{
	int i;
	tVM68k_instruction inst;
	char name[16];
	
	for (i=0;i<0xffff;i++)
	{
		vm68k_get_instructionname(vm68k_decode(i),name);
		printf("%04X: %s\n",i,name);
	}

	i=0x0880;
	vm68k_get_instructionname(vm68k_decode(i),name);
	printf("%04X: %s\n",i,name);
}

