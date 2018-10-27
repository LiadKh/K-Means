#include "Const.h"

void checkAllocation(void* ptr)
{
	if (!ptr)
	{//Allocation problem
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
}