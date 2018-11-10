#include "MpiFunctions.h"

MPI_Datatype PointMPIType;
MPI_Datatype PointVelocityMPIType;

void mpiInit(int *argc, char** argv[], int *rank, int *numberOfProcesses)
{//Init MPI
	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
	MPI_Comm_size(MPI_COMM_WORLD, numberOfProcesses);
	if (*numberOfProcesses < 2)
	{
		printf("Run with 2 processes or more. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
}

void commitMpiPointType()
{// Create MPI user data type for point
	{
		point_t point;
		MPI_Datatype type[4] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE , MPI_INT };
		int blocklen[4] = { 1, 1, 1, 1 };
		MPI_Aint disp[4];

		disp[0] = (char *)&point.x - (char *)&point;
		disp[1] = (char *)&point.y - (char *)&point;
		disp[2] = (char *)&point.z - (char *)&point;
		disp[3] = (char *)&point.cluster - (char *)&point;
		MPI_Type_create_struct(4, blocklen, disp, type, &PointMPIType);
		MPI_Type_commit(&PointMPIType);
	}
	{
		point_velocity_t pointWithVelocity;
		MPI_Datatype type[7] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE ,MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT };
		int blocklen[7] = { 1, 1, 1, 1, 1, 1, 1 };
		MPI_Aint disp[7];

		disp[0] = (char *)&pointWithVelocity.x - (char *)&pointWithVelocity;
		disp[1] = (char *)&pointWithVelocity.y - (char *)&pointWithVelocity;
		disp[2] = (char *)&pointWithVelocity.z - (char *)&pointWithVelocity;
		disp[3] = (char *)&pointWithVelocity.vx - (char *)&pointWithVelocity;
		disp[4] = (char *)&pointWithVelocity.vy - (char *)&pointWithVelocity;
		disp[5] = (char *)&pointWithVelocity.vz - (char *)&pointWithVelocity;
		disp[6] = (char *)&pointWithVelocity.cluster - (char *)&pointWithVelocity;
		MPI_Type_create_struct(7, blocklen, disp, type, &PointVelocityMPIType);
		MPI_Type_commit(&PointVelocityMPIType);
	}
}

void scatterPoints(int rank, int numberOfProcesses, point_velocity_t* allPoints, int N, point_velocity_t** myPoints, int *numberOfPoints)
{//Scatter points from master
	if (rank == MASTER)
		*numberOfPoints = int(N / numberOfProcesses);
	MPI_Bcast(numberOfPoints, 1, MPI_INT, MASTER, MPI_COMM_WORLD);//Broadcast number of points to each process
	*myPoints = (point_velocity_t*)malloc((*numberOfPoints) * sizeof(point_velocity_t));//Allocate number of points
	checkAllocation(*myPoints);
	MPI_Scatter(allPoints, *numberOfPoints, PointVelocityMPIType, *myPoints, *numberOfPoints, PointVelocityMPIType, MASTER, MPI_COMM_WORLD);//Scatter points
	if (rank == MASTER || rank == numberOfProcesses - 1)//Check if there are points that has not sent
	{
		int morePoint;
		if (rank == MASTER)
		{
			morePoint = N % numberOfProcesses;
			MPI_Send(&morePoint, 1, MPI_INT, numberOfProcesses - 1, 0, MPI_COMM_WORLD);
			if (morePoint != 0)
				MPI_Send(&(allPoints[(*numberOfPoints)*numberOfProcesses]), morePoint, PointVelocityMPIType, numberOfProcesses - 1, 0, MPI_COMM_WORLD);
		}
		else
		{
			MPI_Status status;
			MPI_Recv(&morePoint, 1, MPI_INT, MASTER, 0, MPI_COMM_WORLD, &status);
			if (morePoint != 0)
			{
				*myPoints = (point_velocity_t*)realloc(*myPoints, (morePoint + (*numberOfPoints)) * sizeof(point_velocity_t));
				checkAllocation(*myPoints);
				MPI_Recv(&((*myPoints)[(*numberOfPoints)]), morePoint, PointVelocityMPIType, MASTER, 0, MPI_COMM_WORLD, &status);
				*numberOfPoints = (*numberOfPoints) + morePoint;
			}
		}
	}
}

point_t* gatherPoints(int rank, int numberOfProcesses, point_t* myPoints, int numberOfPointsToSend)
{//Gather points from all processes
	point_t *allPoints = NULL;
	if (rank == MASTER)
	{
		allPoints = (point_t*)malloc(numberOfProcesses * numberOfPointsToSend * sizeof(point_t));
		checkAllocation(allPoints);
	}
	MPI_Gather(myPoints, numberOfPointsToSend, PointMPIType, allPoints, numberOfPointsToSend, PointMPIType, MASTER, MPI_COMM_WORLD);
	return allPoints;
}

bool* gatherBool(int rank, int numberOfProcesses, bool check)
{
	bool *allPoints = NULL;
	if (rank == MASTER)
	{
		allPoints = (bool*)malloc(numberOfProcesses * sizeof(bool));
		checkAllocation(allPoints);
	}
	MPI_Gather(&check, 1, MPI_C_BOOL, allPoints, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);
	return allPoints;
}

void collectPointsInClusters(int rank, int numberOfProcesses, int numberOfClusters, point_t** points, int *pointsInClusters)
{//Send the points from all the processes to the MASTER
	MPI_Status status;
	int numberOfPointToSent;

	for (int i = 0; i < numberOfClusters; i++)
	{
		if (rank != numberOfProcesses - 1)
		{//Get data
			MPI_Recv(&numberOfPointToSent, 1, MPI_INT, rank + 1, i, MPI_COMM_WORLD, &status);//Number of point to send
			if (numberOfPointToSent != 0)
			{
				points[i] = (point_t*)realloc(points[i], (pointsInClusters[i] + numberOfPointToSent) * sizeof(point_t));
				checkAllocation(points[i]);
				MPI_Recv(&(points[i][pointsInClusters[i]]), numberOfPointToSent, PointMPIType, rank + 1, i, MPI_COMM_WORLD, &status);//Send point
				pointsInClusters[i] = pointsInClusters[i] + numberOfPointToSent;
			}
		}
		if (rank != MASTER)
		{//Send data
			MPI_Send(&(pointsInClusters[i]), 1, MPI_INT, rank - 1, i, MPI_COMM_WORLD);//Number of point to send
			if (pointsInClusters[i] != 0)
				MPI_Send(points[i], pointsInClusters[i], PointMPIType, rank - 1, i, MPI_COMM_WORLD);//Send point
		}
	}
}

double* masterWorkCalcQ(int numberOfProcesses, point_t** pointsInClusters, int* numberOfPointInClusters, int numberOfClusters)
{//MASTER send point in clusters
	MPI_Status status;
	int i = 0, workProcesses = 0;
	double temp;
	double *calculatedDistance = (double*)calloc(numberOfClusters, sizeof(double));
	checkAllocation(calculatedDistance);

	for (; i < numberOfClusters && workProcesses < numberOfProcesses - 1; i++)
	{//Send first job
		if (numberOfPointInClusters[i] > 1)
		{
			MPI_Send(&(numberOfPointInClusters[i]), 1, MPI_INT, workProcesses + 1, i, MPI_COMM_WORLD);//Number of point to send
			MPI_Send(pointsInClusters[i], numberOfPointInClusters[i], PointMPIType, workProcesses + 1, i, MPI_COMM_WORLD);//Send point
			workProcesses++;
		}
	}
	for (int j = workProcesses; j < numberOfProcesses - 1; j++)//more procsses than clusters
		MPI_Send(&j, 1, MPI_INT, j + 1, FINISH_WORK_TAG, MPI_COMM_WORLD);//Stop process - no more work
	for (; i < numberOfClusters; i++)
	{
		if (numberOfPointInClusters[i] > 1)
		{
			MPI_Recv(&temp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//Receive result 
			calculatedDistance[status.MPI_TAG] = temp;
			MPI_Send(&(numberOfPointInClusters[i]), 1, MPI_INT, status.MPI_SOURCE, i, MPI_COMM_WORLD);//Number of point to send
			MPI_Send(pointsInClusters[i], numberOfPointInClusters[i], PointMPIType, status.MPI_SOURCE, i, MPI_COMM_WORLD);//Send point
		}
	}
	for (i = 0; i < workProcesses; i++)
	{
		MPI_Recv(&temp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//Receive result 
		calculatedDistance[status.MPI_TAG] = temp;
		MPI_Send(&i, 1, MPI_INT, status.MPI_SOURCE, FINISH_WORK_TAG, MPI_COMM_WORLD);//Stop process - no more work
	}
	return calculatedDistance;
}

point_t* slavesWorkCalcQ(int *workSize, int *clusterId, double distance, bool found)
{//Slave get points to work from MASTER
	point_t* result = NULL;
	MPI_Status status;

	if (found == true)// Not first iteration
		MPI_Send(&distance, 1, MPI_DOUBLE, MASTER, *clusterId, MPI_COMM_WORLD);//Send result
	MPI_Recv(workSize, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//Number of point to receive
	if (status.MPI_TAG != FINISH_WORK_TAG)
	{
		*clusterId = status.MPI_TAG;
		result = (point_t*)malloc((*workSize) * sizeof(point_t));
		checkAllocation(result);
		MPI_Recv(result, *workSize, PointMPIType, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);//Receive point
	}
	return result;
}
