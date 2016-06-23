#include <stdio.h>

// assembly instructions : output operands : input operands : list of clobbered registers

int main(int argc,char **argv)
{
	int a=10,b=0,fSupported=0;
	int features_edx=0,features_ecx=0;

	asm("movl %0, %%eax;\n\t"
		"addl $1, %%eax;\n\t"
		"movl %%eax, %1;\n\t"
		"movl $1, %%eax;\n\t"
		"cpuid;\n\t"
		"movl %%ecx, %2;\n\t"
		"movl %%edx, %1;\n\t"
		: "=r" (b), "=r" (features_edx), "=r" (features_ecx)
		: "r" (a)
		: "%eax", "%ecx"
		);

	__asm {
		mov eax, 1
		cpuid
		and edx, 0x800000
		mov fSupported, edx
	}

	puts("Feature string: ");
	if (features_edx & (1<<18))
		puts ("PN");
	if (features_edx & (1<<19))
		puts ("CLFlush");
	if (features_edx & (1<<23))
		puts ("MMX");
	if (features_edx & (1<<25))
		puts ("SSE");
	if (features_edx & (1<<26))
		puts ("SSE2");
	if (features_edx & (1<<28))
		puts ("HTT");
	if (features_ecx & 1)
		puts ("SSE3");
	
	
	
	
	

	printf("Value before was %d, now is %d\n",a,b);
	
}
