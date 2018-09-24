#include "MpiFunctions.h"

void mpiInit(int *argc, char** argv[], int *rank, int *numprocs)
{//Init MPI
	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
	MPI_Comm_size(MPI_COMM_WORLD, numprocs);
}

void sendPoints(float* allPoints, float** myPoints, int *numberOfPoints)
{//Sent points
	MPI_Bcast(numberOfPoints, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	*myPoints = (float*)malloc((*numberOfPoints) * sizeof(float) * DIMENSION * 2);
	if (myPoints == NULL)
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	MPI_Scatter(allPoints, (*numberOfPoints) * DIMENSION * 2, MPI_FLOAT, *myPoints, (*numberOfPoints) * DIMENSION * 2, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
}

void getPoints(float* allPoints, float* myPoints, int numberOfPoints)
{//Get points
	MPI_Gather(myPoints, numberOfPoints * DIMENSION, MPI_FLOAT, allPoints, numberOfPoints * DIMENSION * 2, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
}


void mpiFinish()
{//MPI finalize
	MPI_Finalize();
}