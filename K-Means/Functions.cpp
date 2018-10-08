#include "Functions.h"

point_t* readDataFile(char* fname, int *N, int* K, int* T, float* dT, int* LIMIT, float* QM)
{//Read data from file
	point_t* points;
	FILE* f = fopen(fname, "r");
	if (f == NULL)//File problem
	{
		printf("Failed opening the file. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fscanf(f, "%d %d %d %f %d %f", N, K, T, dT, LIMIT, QM);//Read N, K, T, dT, LIMIT, QM
	points = (point_t*)malloc((*N) * sizeof(point_t));
	if (points == NULL)//Allocation problem
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	if ((*K) > (*N))// Check if number of points is more than clusters
	{
		printf("Number of clusters bigger than number of points. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < *N; i++)// Read N points
		fscanf(f, "%f %f %f %f %f %f", &(points[i].x), &(points[i].y), &(points[i].z), &(points[i].vx), &(points[i].vy), &(points[i].vz));
	fclose(f);
	return points;
}

void writeToFile(float t, float q, point_t *clusters, int k)
{//Write project output
	FILE* f = fopen(OUTPUT_FILE, "w");
	if (f == NULL)//File problem
	{
		printf("Failed opening the file. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fprintf(f, "First occurrence t = %f  with q = %f\n", t, q);
	fprintf(f, "Centers of the clusters :\n");
	for (int i = 0; i < k; i++)// Write K clusters
		fprintf(f, "%f \t %f \t %f \t \n", clusters[i].x, clusters[i].y, clusters[i].z);
	printf("Thanks for your use!!! Exiting.\n"); fflush(stdout);
}

void initProcesses(int *argc, char** argv[], int *rank, int *numberOfProcesses)
{//Init MPI
	mpiInit(argc, argv, rank, numberOfProcesses);
	commitMpiPointType();//Commit to MPI point type
}

void initWork(int rank, int numberOfProcesses, point_t *allPoints, int N, point_t **myPoints, int *myNumberOfPoints, point_t **clusters, int *k)
{//Start every process with all clusters and scatter points to work
	if (rank == MASTER)
		*clusters = chooseFirstClusters(allPoints, *k);// Choose first k point to be clusters
	broadcastClusters(rank, clusters, k);
	scatterPoints(rank, numberOfProcesses, allPoints, N, myPoints, myNumberOfPoints);
}

point_t* chooseFirstClusters(point_t* points, int numberOfClusters)
{//Return copy of first k to be clusters
	point_t* clusters = (point_t*)malloc(numberOfClusters * sizeof(point_t));
	memcpy(clusters, points, numberOfClusters * sizeof(point_t));
	return clusters;
}

point_t* getNewClusters(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, int numberOfClusters)
{//Find new clusters
	point_t* newClusters = NULL;
	point_t* sumClustersArray = sumClusters(points, numberOfPoints, numberOfClusters);//Sum the points x,y,z in each cluster
	point_t* allSumClustersArrays = gatherPoints(rank, numberOfProcesses, sumClustersArray, numberOfClusters);
	if (rank == MASTER)
	{
		newClusters = combainPointsArrays(allSumClustersArrays, numberOfProcesses, numberOfClusters);//Combain clusters arrays to one array
		setAverageToClusters(newClusters, numberOfClusters);
		free(allSumClustersArrays);
	}
	free(sumClustersArray);
	return newClusters;
}

bool checkMovedPoint(int rank, int numberOfProcesses, point_t* p1, point_t* p2, int numberOfPoints)
{//Check if there is point that moved to another cluster
	bool isMoved = isMovedPoint(p1, p2, numberOfPoints);
	bool* isMovedArray = gatherBool(rank, numberOfProcesses, isMoved);
	if (rank == MASTER)
	{
		isMoved = checkArray(isMovedArray, numberOfProcesses);
		free(isMovedArray);
	}
	return isMoved;
}

point_t* iteration(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int k, float *dt, point_t* oldPoints, point_t** incedPoints, float *q)
{
	point_t* newClusters = NULL;
	broadcastDT(dt);//Broadcast dt to this iteration
	*incedPoints = incPoints(points, numberOfPoints, *dt);//Calculate increased points
	setClosestCluster(*incedPoints, numberOfPoints, clusters, k);//Set closet cluster to each point
	newClusters = getNewClusters(rank, numberOfProcesses, *incedPoints, numberOfPoints, k);//Get the new clusters
	bool isMoved = checkMovedPoint(rank, numberOfProcesses, *incedPoints, oldPoints, numberOfPoints);//If there is point that moved to another cluster
	return newClusters;
}

bool checkConditions(int iterations, int LIMIT, int T, float dt, bool movedPoint, float QM, float q)
{
	if (iterations == 0)//Cheack first iteration
		return true;
	if (iterations >= LIMIT)//Cheack maximum number of iterations
		return false;
	if (iterations*dt >= T)//Cheack end of time interval 
		return false;
	if (!movedPoint)//Cheack no point move to another cluster
		return false;
	if (q <= QM)//Cheack quality measure to stop
		return false;
	return true;
}

void freeAllocations(int count, ...) 
{//Free allocations
	va_list list;
	va_start(list, count);
	for (int i = 0; i < count; i++)
		free(*(va_arg(list, int**)));
	va_end(list);
}


void printIt(point_t* points, int size)
{
	printf("%d\n", size); fflush(stdout);
	for (int i = 0; i < size; i++)
	{
		printf("%3f \t", points[i].x);
		printf("%3f \t", points[i].y);
		printf("%3f \t", points[i].z);
		printf("%3f \t", points[i].vx);
		printf("%3f \t", points[i].vy);
		printf("%3f \t", points[i].vz);
		printf("%d \t", points[i].cluster);
		printf("\n"); fflush(stdout);
	}
}