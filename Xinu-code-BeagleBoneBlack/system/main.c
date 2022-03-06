/*  main.c  - main */

#include <xinu.h>

void runShell();

process	main(void)
{

	runShell();

	return OK;
}
