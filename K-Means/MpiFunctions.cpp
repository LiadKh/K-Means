#include "MpiFunctions.h"

void mpiInit(int *argc, char** argv[], int *rank, int *numprocs)
{//Init MPI
	MPI_Init(argc, argv);
	MPI_Comm_rank(MPI_COMM_WORLD, rank);
	MPI_Comm_size(MPI_COMM_WORLD, numprocs);
}

void commitMpiPointType()
{
	point_t pointWithVelocity;
	MPI_Datatype type[6] = { MPI_FLOAT, MPI_FLOAT, MPI_FLOAT ,MPI_FLOAT, MPI_FLOAT, MPI_FLOAT };
	int blocklen[6] = { 1, 1, 1,1, 1, 1 };
	MPI_Aint disp[6];

	// Create MPI user data type for point
	disp[0] = (char *)&pointWithVelocity.x - (char *)&pointWithVelocity;
	disp[1] = (char *)&pointWithVelocity.y - (char *)&pointWithVelocity;
	disp[2] = (char *)&pointWithVelocity.z - (char *)&pointWithVelocity;
	disp[3] = (char *)&pointWithVelocity.vx - (char *)&pointWithVelocity;
	disp[4] = (char *)&pointWithVelocity.vy - (char *)&pointWithVelocity;
	disp[5] = (char *)&pointWithVelocity.vz - (char *)&pointWithVelocity;
	MPI_Type_create_struct(6, blocklen, disp, type, &PointMPIType);
	MPI_Type_commit(&PointMPIType);

}

void scatterPoints(point_t* allPoints, point_t** myPoints, int *numberOfPoints, float *dt)
{//Sent points
	MPI_Bcast(numberOfPoints, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(dt, 1, MPI_FLOAT, MASTER, MPI_COMM_WORLD);
	*myPoints = (point_t*)malloc((*numberOfPoints) * sizeof(point_t));
	if (myPoints == NULL)
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	MPI_Scatter(allPoints, *numberOfPoints, PointMPIType, *myPoints, *numberOfPoints, PointMPIType, MASTER, MPI_COMM_WORLD);
}

void gatherPoints(point_t* allPoints, point_t* myPoints, int numberOfPoints)
{//Get points
	MPI_Gather(myPoints, numberOfPoints, PointMPIType, allPoints, numberOfPoints, PointMPIType, MASTER, MPI_COMM_WORLD);
}

void mpiFinish()
{//MPI finalize
	MPI_Finalize();
}