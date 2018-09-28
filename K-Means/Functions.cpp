#include "Functions.h"

point_t* readDataFile(char* fname, int *N, int* K, int* T, float* dT, int* LIMIT, float* QM)
{//Read data from file
	point_t* points;
	FILE* f = fopen(fname, "r");
	if (f == NULL)
	{
		printf("Failed opening the file. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}

	fscanf(f, "%d %d %d %f %d %f", N, K, T, dT, LIMIT, QM);
	points = (point_t*)malloc((*N) * sizeof(point_t));
	if (points == NULL)
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < *N; i++)
		fscanf(f, "%f %f %f %f %f %f", &(points[i].x), &(points[i].y), &(points[i].z), &(points[i].vx), &(points[i].vy), &(points[i].vz));
	fclose(f);
	return points;
}

void initProcesses(int *argc, char** argv[], int *rank, int *numberOfProcesses)
{
	mpiInit(argc, argv, rank, numberOfProcesses);
	commitMpiPointType();
}

void initProject(int rank, int numberOfPoints, point_t *allPoints, point_t **myPoints, int *myNumberOfPoints, point_t *clusters, int *k)
{
	if (rank == MASTER)
		clusters = chooseK(allPoints, numberOfPoints, *k);
	broadcastClusters(rank, &clusters, k);
	scatterPoints(allPoints, myPoints, myNumberOfPoints);
}

point_t* chooseK(point_t* points, int numberOfPoints, int k)
{
	if (k < numberOfPoints)
	{
		printf("More clusters than points");
		exit(EXIT_FAILURE);
	}
	point_t* clusters = (point_t*)malloc(k * sizeof(point_t));
	memcpy(clusters, points, k * sizeof(point_t));
}

void interaction(point_t* points, int numberOfPoints, point_t* clusters, int rank, int numberOfProcess, int k, float *dt)
{
	broadcastDT(dt);
	point_t* inicedMyPoints = incPoints(points, numberOfPoints, *dt);
	setClosestCluster(points, numberOfPoints, clusters, k);
	point_t* newClusters = averageClusters(points, numberOfPoints, k);
	point_t* newClusters = gatherPoints(rank, points, numberOfProcess, numberOfPoints);


	if (rank == MASTER)
	{

	}
	//need to calc the new centers
}