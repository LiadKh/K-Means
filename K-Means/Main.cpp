#include "Functions.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int rank, numprocs;
	int N, K, T;
	float dT, LIMIT, QM;
	float* points = readDataFile(INPUT_FILE, &N, &K, &T, &dT, &LIMIT, &QM);
	free(points);
	//mpiInit(&argc, &argv, &rank, &numprocs);
	return EXIT_SUCCESS;
}