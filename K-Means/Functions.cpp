#include "Functions.h"

char* createFileName(char* path, int pathSize, char* fileName, int nameSize)
{//Create full path with the file name
	char* fullName = (char*)calloc(pathSize + nameSize + 2, sizeof(char));
	checkAllocation(fullName);
	char* back = "\\";
	int size;
	strncpy(fullName, path, pathSize);
	size = pathSize;
	strncpy(&(fullName[size++]), back, strlen(back));
	strncpy(&(fullName[size]), fileName, nameSize);
	return fullName;
}

void initProcesses(int *argc, char** argv[], int *rank, int *numberOfProcesses, char* path, int *pathSize)
{//Init
	omp_set_nested(true);
	mpiInit(argc, argv, rank, numberOfProcesses);
	commitMpiPointType();//Commit to MPI point type
	if (*rank == MASTER)
	{
		if (*argc != 2) {
			printf("Please run the program with path of the folder of input file\n"); fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
		*pathSize = int(strlen((*argv)[1]));
		memcpy(path, (*argv)[1], *pathSize);
		createFileName(path, *pathSize, INPUT_FILE, int(strlen(INPUT_FILE)));
	}
}

point_velocity_t* readDataFile(char* path, int pathSize, int *N, int* K, double* T, double* dT, int* LIMIT, double* QM)
{//Read data from file
	point_velocity_t* points;
	char* input = createFileName(path, pathSize, INPUT_FILE, int(strlen(INPUT_FILE)));
	FILE* f = fopen(input, "r");
	if (f == NULL)//File problem
	{
		printf("Failed opening the file. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MOOD
	printf("Read points from: %s\n%s\n", input, newLine); fflush(stdout);
#endif
	fscanf(f, "%d %d %lf %lf %d %lf", N, K, T, dT, LIMIT, QM);//Read N, K, T, dT, LIMIT, QM
	points = (point_velocity_t*)malloc((*N) * sizeof(point_velocity_t));
	checkAllocation(points);
	if ((*K) > (*N))// Check if number of points is more than clusters
	{
		printf("Number of clusters bigger than number of points. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < *N; i++)// Read N points
		fscanf(f, "%lf %lf %lf %lf %lf %lf", &(points[i].x), &(points[i].y), &(points[i].z), &(points[i].vx), &(points[i].vy), &(points[i].vz));
	fclose(f);
	free(input);
	return points;
}

void initWork(int rank, int numberOfProcesses, point_velocity_t *allPoints, int N, point_velocity_t **myPoints, int *myNumberOfPoints, point_t **clusters, int *k)
{//Start every process with points and allocate memory for clusters
	MPI_Bcast(k, 1, MPI_INT, MASTER, MPI_COMM_WORLD);//Broadcast number of clusters to send
	*clusters = (point_t*)malloc((*k) * sizeof(point_t));//Allocate number of clusters points
	checkAllocation(*clusters);
	scatterPoints(rank, numberOfProcesses, allPoints, N, myPoints, myNumberOfPoints);//Init process with The points that belong to him
}

void incPointsDT(point_velocity_t* points, int numberOfPoints, double dt, point_t *incPoints)
{//Calculate increased points
	int workSize = int(numberOfPoints * CUDA_PERCENT_OF_WORK);
#pragma omp parallel sections // Divides the work into sections
	{
#pragma omp section
		{
			incPointsCUDA(points, workSize, dt, incPoints);
		}
#pragma omp section
		{
			incPointsOMP(points, numberOfPoints, dt, incPoints);
		}
	}
}

void setCluster(point_t *points, int numberOfPoints, point_t* clusters, int numberOfCluster)
{//Set the close cluster - CUDA and Cluster
	int workSize = int(numberOfPoints * CUDA_PERCENT_OF_WORK);
#pragma omp parallel sections // Divides the work into sections
	{
#pragma omp section
		{
			setClusterCUDA(points, workSize, clusters, numberOfCluster);//Set closet cluster to each point - CUDA
		}
#pragma omp section
		{
			setCloseClusterOMP(&(points[workSize]), numberOfPoints - workSize, clusters, numberOfCluster);//Set closet cluster to each point - OMP
		}
	}
}

point_t* getNewClusters(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* oldClusters, int numberOfClusters, int *numberOfPointInCluster)
{//Find new clusters
	point_t* newClusters = NULL;
	memset(numberOfPointInCluster, 0, numberOfClusters * sizeof(int));
	point_t* sumClustersArray = sumClusters(points, numberOfPoints, numberOfClusters, numberOfPointInCluster);//Sum the points x,y,z in each cluster
	point_t* allSumClustersArrays = gatherPoints(rank, numberOfProcesses, sumClustersArray, numberOfClusters);
	if (rank == MASTER)
	{
		newClusters = combainPointsArrays(allSumClustersArrays, numberOfProcesses, numberOfClusters, NULL);//Combain clusters arrays to one array
		setAverageToClusters(newClusters, oldClusters, numberOfClusters);
	}
	free(allSumClustersArrays);
	free(sumClustersArray);
	return newClusters;
}

bool checkMovedPoint(int rank, int numberOfProcesses, point_t* points, int* pointInCluster, int numberOfPoints)
{//Check if there is point that moved to another cluster
	bool isMoved = isMovedPoint(points, pointInCluster, numberOfPoints);
	bool* isMovedArray = gatherBool(rank, numberOfProcesses, isMoved);
	if (rank == MASTER)
		isMoved = checkArray(isMovedArray, numberOfProcesses);
	free(isMovedArray);
	return isMoved;
}

point_t** setPointsInCluster(point_t* points, int numberOfPoints, int numberOfClusters, int *pointsInClusters)
{//Set of cluster - each cluster array contains the point in the cluster
	int cluster;
	int *counterPointInArray = (int*)calloc(numberOfClusters, sizeof(int));//Count point in the cluster
	checkAllocation(counterPointInArray);
	point_t **setArray = (point_t**)malloc(numberOfClusters * sizeof(point_t*));
	checkAllocation(setArray);

	for (int i = 0; i < numberOfClusters; i++)
	{
		setArray[i] = (point_t*)malloc(pointsInClusters[i] * sizeof(point_t));
		checkAllocation(setArray[i]);
	}
	for (int i = 0; i < numberOfPoints; i++)
	{
		cluster = points[i].cluster;
		memcpy(&(setArray[cluster][counterPointInArray[cluster]]), &(points[i]), sizeof(point_t));
		counterPointInArray[cluster]++;
	}
	free(counterPointInArray);
	return setArray;
}

bool checkTerminationCondition(int iterations, int LIMIT, bool movedPoint)
{//Check termination condition of K-Means algo.
	if (iterations == 0)//Check first iteration
		return true;
	if (iterations >= LIMIT - 1)//Check maximum number of iterations
	{
#ifdef DEBUG_MOOD
		printf("\tMax iteration\n"); fflush(stdout);
#endif
		return false;
	}
	if (!movedPoint)//Check no point move to another cluster
	{
#ifdef DEBUG_MOOD
		printf("\tNo point moved\n"); fflush(stdout);
#endif
		return false;
	}
	return true;
}

double kmeansIterations(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int k, int LIMIT)
{
	point_t* newClusters = NULL;
	int iteration = 0, *pointCloseCluster, *numberOfPointInCluster = NULL;
	bool anotherIteration, isMovedPoint;
	numberOfPointInCluster = (int*)malloc(k * sizeof(int));
	checkAllocation(numberOfPointInCluster);
	pointCloseCluster = (int*)calloc(numberOfPoints, sizeof(int));
	checkAllocation(pointCloseCluster);
	if (rank == MASTER)
		memcpy(clusters, points, k * sizeof(point_t)); //Copy first k to be clusters
	do
	{
		MPI_Bcast(clusters, k, PointMPIType, MASTER, MPI_COMM_WORLD);//Broadcast k clusters
		setCluster(points, numberOfPoints, clusters, k);
		newClusters = getNewClusters(rank, numberOfProcesses, points, numberOfPoints, clusters, k, numberOfPointInCluster);//Get the new clusters
		isMovedPoint = checkMovedPoint(rank, numberOfProcesses, points, pointCloseCluster, numberOfPoints);//If there is point that moved to another cluster
		if (rank == MASTER)
		{
#ifdef DEBUG_MOOD
			printf("\tK-Means iteration: %d\n", iteration + 1); fflush(stdout);
#endif
			anotherIteration = checkTerminationCondition(iteration, LIMIT, isMovedPoint);//Check the termination condition
			iteration++;
			memcpy(clusters, newClusters, k * sizeof(point_t));
		}
		MPI_Bcast(&anotherIteration, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);//MASTER send if there is more iteration
		free(newClusters);
	} while (anotherIteration);
	double q = calucQ(rank, numberOfProcesses, points, numberOfPoints, clusters, k, numberOfPointInCluster);//Calculate quality measure
	freeAllocations(2, pointCloseCluster, numberOfPointInCluster);
	return q;
}

double calucQ(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters, int *numberOfPointsInCluster)
{//Calculate quality measure
	double q = NULL, *biggestDistanceArray;
	point_t** pointsInCluster = setPointsInCluster(points, numberOfPoints, numberOfClusters, numberOfPointsInCluster);//Set points in there cluster array

	collectPointsInClusters(rank, numberOfProcesses, numberOfClusters, pointsInCluster, numberOfPointsInCluster);//Sent the point to MASTER - each point in the custom array (cluster array)
	if (rank == MASTER)
	{//MASTER send points
		biggestDistanceArray = masterWorkCalcQ(numberOfProcesses, pointsInCluster, numberOfPointsInCluster, numberOfClusters);
		q = findQ(biggestDistanceArray, clusters, numberOfClusters);
		free(biggestDistanceArray);
	}
	else
	{//Slave get points to find the biggest distance
		int workSize, clusterId;
		bool found = false;
		point_t* work;
		do {
			work = slavesWorkCalcQ(&workSize, &clusterId, q, found);
			if (work != NULL)
			{
				found = true;
				q = biggestDistance(work, workSize);
				free(work);
			}
			else
				break;
		} while (true);
	}
	for (int i = 0; i < numberOfClusters; i++)
		free(pointsInCluster[i]);
	free(pointsInCluster);
	return q;
}

bool checkConditions(double T, double time, double QM, double q)
{//Check stop program conditions
	if (time >= T)//Check end of time interval 
	{
#ifdef DEBUG_MOOD
		printf("End of time interval\n"); fflush(stdout);
#endif
		return false;
	}
	if (q <= QM)//Check quality measure to stop
	{
#ifdef DEBUG_MOOD
		printf("Quality measure to stop\n"); fflush(stdout);
#endif
		return false;
	}
	return true;
}

void work(int rank, int numberOfProcesses, double *time, double *q, double dt, point_velocity_t *points, int numberOfPoints, point_t* clusters, int k, int LIMIT, double T, double QM)
{
	int iterationNumber = 0;
	bool stopWork;
	point_t* incPoints = (point_t*)malloc(numberOfPoints * sizeof(point_t));//Allocate number of points
	checkAllocation(incPoints);
	do
	{
		if (rank == MASTER)
		{
			*time = iterationNumber*dt;
#ifdef DEBUG_MOOD
			printf("Time: %f\n", *time); fflush(stdout);
#endif
		}
		MPI_Bcast(time, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
		incPointsDT(points, numberOfPoints, *time, incPoints);
		*q = kmeansIterations(rank, numberOfProcesses, incPoints, numberOfPoints, clusters, k, LIMIT);
		if (rank == MASTER)
		{
			stopWork = checkConditions(T, *time, QM, *q);//Check the condition to stop
			iterationNumber++;
#ifdef DEBUG_MOOD
			printf("Quality found: %f\n%s\n", *q, newLine); fflush(stdout);
#endif
		}
		MPI_Bcast(&stopWork, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);
	} while (stopWork);
	free(incPoints);
}

void writeToFile(char* path, int pathSize, double t, double q, point_t *clusters, int k)
{//Write project output
	char* output = createFileName(path, pathSize, OUTPUT_FILE, int(strlen(OUTPUT_FILE)));
	FILE* f = fopen(output, "w");
	if (f == NULL)//File problem
	{
		printf("Failed opening the file. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	printf("Write result to: %s\n", output); fflush(stdout);
	fprintf(f, "First occurrence t = %f  with q = %f\n", t, q);
	fprintf(f, "Centers of the clusters :\n");
	for (int i = 0; i < k; i++)// Write K clusters
		fprintf(f, "%f \t %f \t %f \t \n", clusters[i].x, clusters[i].y, clusters[i].z);
	printf("Thanks for your use!!! Exiting...\n"); fflush(stdout);
	free(output);
}

void freeAllocations(int count, ...)
{//Free allocations
	va_list list;
	va_start(list, count);
	for (int i = 0; i < count; i++)
		free(va_arg(list, point_t*));
	va_end(list);
}