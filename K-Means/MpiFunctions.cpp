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
	MPI_Bcast(k, 1, MPI_INT, MASTER, MPI_COMM_WORLD);//Broadcast number of clusters to send
	if (rank != MASTER)
	{
		*clusters = (point_t*)malloc((*k) * sizeof(point_t));//Allocate number of clusters points
		if (*clusters == NULL)//Allocation problem
		{
			printf("Not enough memory. Exiting!\n"); fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
	}
	MPI_Bcast(*clusters, *k, PointMPIType, MASTER, MPI_COMM_WORLD);//Broadcast k clusters
}

void scatterPoints(int rank, int numberOfProcesses, point_t* allPoints, int N, point_t** myPoints, int *numberOfPoints)
{//Scatter points from master
	if (rank == MASTER)
		*numberOfPoints = int(N / numberOfProcesses);
	MPI_Bcast(numberOfPoints, 1, MPI_INT, MASTER, MPI_COMM_WORLD);//Broadcast number of points to each process
	*myPoints = (point_t*)malloc((*numberOfPoints) * sizeof(point_t));//Allocate number of points
	if (myPoints == NULL)//Allocation problem
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	MPI_Scatter(allPoints, *numberOfPoints, PointMPIType, *myPoints, *numberOfPoints, PointMPIType, MASTER, MPI_COMM_WORLD);//Scatter points
	if (rank == MASTER || rank == numberOfProcesses - 1)//Check if there are points that has not sent
	{
		if (rank == MASTER)
		{
			int morePoint = N % numberOfProcesses;
			MPI_Send(&morePoint, 1, MPI_INT, numberOfProcesses - 1, 0, MPI_COMM_WORLD);
			if (morePoint != 0)
				MPI_Send(&(allPoints[(*numberOfPoints)*numberOfProcesses]), morePoint, PointMPIType, numberOfProcesses - 1, 0, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Status mpiS;
			int morePoint;
			MPI_Recv(&morePoint, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &mpiS);
			if (morePoint != 0)
			{
				*myPoints = (point_t*)realloc(*myPoints, morePoint);
				if (myPoints == NULL)//Allocation problem
				{
					printf("Not enough memory. Exiting!\n"); fflush(stdout);
					MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
				}
				*numberOfPoints = (*numberOfPoints) + morePoint;
				MPI_Recv(&(myPoints[(*numberOfPoints)]), morePoint, PointMPIType, MASTER, 0, MPI_COMM_WORLD, &mpiS);
			}
		}
	}
}

void broadcastDT(float *dt)
{// Broadcast the dt of the iteration
	MPI_Bcast(dt, 1, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
}

point_t* gatherPoints(int rank, int numberOfProcesses, point_t* myPoints, int numberOfPointsToSend)
{//Gather points from all processes
	point_t *allPoints = NULL;
	if (rank == MASTER)
		allPoints = (point_t*)malloc(numberOfProcesses * numberOfPointsToSend * sizeof(point_t));
	MPI_Gather(myPoints, numberOfPointsToSend, PointMPIType, allPoints, numberOfPointsToSend, PointMPIType, MASTER, MPI_COMM_WORLD);
	return allPoints;
}

bool* gatherBool(int rank, int numberOfProcesses, bool check)
{
	bool *allPoints = NULL;
	if (rank == MASTER)
		allPoints = (bool*)malloc(numberOfProcesses * sizeof(bool));
	MPI_Gather(&check, 1, MPI_C_BOOL, allPoints, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);
	return allPoints;
}

void checkForWork(bool *work)
{
	MPI_Bcast(work, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);
}

void mpiFinish()
{//MPI finalize
	MPI_Finalize();
}