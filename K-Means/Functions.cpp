#include "Functions.h"

void MpiInit(int *argc, char** argv[], int *rank, int *numprocs)
{//Init MPI
	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
	MPI_Comm_size(MPI_COMM_WORLD, numprocs);
}

bool ReadDataFile(char* fname, int *N, int* K, int *  T, double*   dT, double*  LIMIT, double*  QM)
{//Read data from file
	FILE* f = fopen(fname, "r");
	if (f == NULL)
	{
		printf("File problem - doesn't open");
		return false;
	}
	fclose(f);
}