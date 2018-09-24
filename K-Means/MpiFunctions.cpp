#include "MpiFunctions.h"

void mpiInit(int *argc, char** argv[], int *rank, int *numprocs)
{//Init MPI
	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
	MPI_Comm_size(MPI_COMM_WORLD, numprocs);
}