#include "Functions.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int rank, numprocs;
	MpiInit(&argc, &argv, &rank, &numprocs);
	return EXIT_SUCCESS;
}