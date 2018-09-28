#include "MpiFunctions.h"

void mpiInit(int *argc, char** argv[], int *rank, int *numberOfProcesses)
{//Init MPI
	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
	MPI_Comm_size(MPI_COMM_WORLD, numberOfProcesses);
}

void commitMpiPointType()
{// Create MPI user data type for point
	point_t pointWithVelocity;
	MPI_Datatype type[7] = { MPI_FLOAT, MPI_FLOAT, MPI_FLOAT ,MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_INT };
	int blocklen[7] = { 1, 1, 1, 1, 1, 1, 1 };
	MPI_Aint disp[7];

	disp[0] = (char *)&pointWithVelocity.x - (char *)&pointWithVelocity;
	disp[1] = (char *)&pointWithVelocity.y - (char *)&pointWithVelocity;
	disp[2] = (char *)&pointWithVelocity.z - (char *)&pointWithVelocity;
	disp[3] = (char *)&pointWithVelocity.vx - (char *)&pointWithVelocity;
	disp[4] = (char *)&pointWithVelocity.vy - (char *)&pointWithVelocity;
	disp[5] = (char *)&pointWithVelocity.vz - (char *)&pointWithVelocity;
	disp[6] = (char *)&pointWithVelocity.cluster - (char *)&pointWithVelocity;
	MPI_Type_create_struct(7, blocklen, disp, type, &PointMPIType);
	MPI_Type_commit(&PointMPIType);

}

void broadcastClusters(int rank, point_t** clusters, int *k)
{
	MPI_Bcast(k, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	if (rank != MASTER)
	{
		*clusters = (point_t*)malloc((*k) * sizeof(point_t));
		if (*clusters == NULL)
		{
			printf("Not enough memory. Exiting!\n"); fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
	}
	MPI_Bcast(*clusters, *k, PointMPIType, MASTER, MPI_COMM_WORLD);
}

void scatterPoints(point_t* allPoints, point_t** myPoints, int *numberOfPoints)
{//Sent points from master process
	MPI_Bcast(numberOfPoints, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	*myPoints = (point_t*)malloc((*numberOfPoints) * sizeof(point_t));
	if (myPoints == NULL)
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	MPI_Scatter(allPoints, *numberOfPoints, PointMPIType, *myPoints, *numberOfPoints, PointMPIType, MASTER, MPI_COMM_WORLD);
}

void broadcastDT(float *dt)
{
	MPI_Bcast(dt, 1, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
}

point_t* gatherPoints(int rank, point_t* myPoints, int numberOfProcesses, int numberOfPointsToSend)
{//Get points to master process
	point_t *allPoints = NULL;
	if (rank == MASTER)
		allPoints = (point_t*)malloc(numberOfProcesses * numberOfPointsToSend * sizeof(point_t));
	MPI_Gather(myPoints, numberOfPointsToSend, PointMPIType, allPoints, numberOfPointsToSend, PointMPIType, MASTER, MPI_COMM_WORLD);
	return allPoints;
}

void mpiFinish()
{//MPI finalize
	MPI_Finalize();
}