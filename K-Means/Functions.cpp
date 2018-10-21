#include "Functions.h"

char* createFileName(char* path, int pathSize, char* fileName, int nameSize)
{//Create full path with the file name
	char* fullName = (char*)calloc(pathSize + nameSize + 2, sizeof(char));
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

point_t* readDataFile(char* path, int pathSize, int *N, int* K, int* T, float* dT, int* LIMIT, float* QM)
{//Read data from file
	point_t* points;
	char* input = createFileName(path, pathSize, INPUT_FILE, int(strlen(INPUT_FILE)));
	FILE* f = fopen(input, "r");
	printf("Read points from: %s\n%s\n", input, newLine); fflush(stdout);
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
	free(input);
	return points;
}

void initWork(int rank, int numberOfProcesses, point_t *allPoints, int N, point_t **myPoints, int *myNumberOfPoints, point_t **clusters, int *k)
{//Start every process with points and allocate memory for clusters
	MPI_Bcast(k, 1, MPI_INT, MASTER, MPI_COMM_WORLD);//Broadcast number of clusters to send
	*clusters = (point_t*)malloc((*k) * sizeof(point_t));//Allocate number of clusters points
	if (*clusters == NULL)//Allocation problem
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	if (rank == MASTER)
		memcpy(*clusters, allPoints, (*k) * sizeof(point_t)); //Copy first k to be clusters
	scatterPoints(rank, numberOfProcesses, allPoints, N, myPoints, myNumberOfPoints);//Init process with The points that belong to him
}

void incPoints(point_t* points, int numberOfPoints, float dt, point_t **incPoints)
{//Calculate increased points
	*incPoints = (point_t*)malloc(numberOfPoints * sizeof(point_t));
	int workSize = int(numberOfPoints * CUDA_PERCENT_OF_WORK);
	if (*incPoints == NULL)//Allocation problem
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
#pragma omp parallel sections // Divides the work into sections
	{
#pragma omp section
		{
			incPointsCUDA(points, workSize, dt, *incPoints);
		}
#pragma omp section
		{
			incPointsOMP(points, numberOfPoints, dt, *incPoints);
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

point_t* getNewClusters(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* oldClusters, int numberOfClusters, int **pointInCluster)
{//Find new clusters
	point_t* newClusters = NULL;
	*pointInCluster = (int*)calloc(numberOfClusters, sizeof(int));
	point_t* sumClustersArray = sumClusters(points, numberOfPoints, numberOfClusters, *pointInCluster);//Sum the points x,y,z in each cluster
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

bool checkMovedPoint(int rank, int numberOfProcesses, point_t* p1, point_t* p2, int numberOfPoints)
{//Check if there is point that moved to another cluster
	bool isMoved = isMovedPoint(p1, p2, numberOfPoints);
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
	if (counterPointInArray == NULL)//Allocation problem
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	point_t **setArray = (point_t**)malloc(numberOfClusters * sizeof(point_t*));
	if (setArray == NULL)//Allocation problem
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	for (int i = 0; i < numberOfClusters; i++)
	{
		setArray[i] = (point_t*)malloc(pointsInClusters[i] * sizeof(point_t));
		if (setArray[i] == NULL)//Allocation problem
		{
			printf("Not enough memory. Exiting!\n"); fflush(stdout);
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		}
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

float calucQ(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters, int *numberOfPointsInCluster)
{//Calculate quality measure
	float q = NULL, *biggestDistanceArray;
	point_t** pointsInCluster = setPointsInCluster(points, numberOfPoints, numberOfClusters, numberOfPointsInCluster);//Set points in there cluster array

	if (pointsInCluster == NULL)//Allocation problem
	{
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
	collectPointsInClusters(rank, numberOfProcesses, numberOfClusters, pointsInCluster, numberOfPointsInCluster);//Sent the point to MASTER - each point in the custom array (cluster array)
	if (rank == MASTER)
	{//MASTER send points
		for (int i = 0; i < numberOfClusters; i++)//Add new cluster as a point in the array
		{
			pointsInCluster[i] = (point_t*)realloc(pointsInCluster[i], (numberOfPointsInCluster[i] + 1) * sizeof(point_t));
			if (pointsInCluster[i] == NULL)//Allocation problem
			{
				printf("Not enough memory. Exiting!\n"); fflush(stdout);
				MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
			}
			memcpy(&(pointsInCluster[i][numberOfPointsInCluster[i]]), &(clusters[i]), sizeof(point_t));
			numberOfPointsInCluster[i]++;
		}
		biggestDistanceArray = sendArrayOfPointInCluster(numberOfProcesses, pointsInCluster, numberOfPointsInCluster, numberOfClusters);
		q = findQ(biggestDistanceArray, clusters, numberOfClusters);
		free(biggestDistanceArray);
	}
	else
	{//Slave get points to find the biggest distance
		int workSize, clusterId;
		bool found = false;
		point_t* work;
		do {
			work = receiveArrayOfPointInCluster(&workSize, &clusterId, q, found);
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

void iteration(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t** clusters, int k, float dt, point_t* oldPoints, point_t** incedPoints, bool *isMovedPoint, float *q)
{
	point_t* newClusters = NULL;
	int *pointInCluster;
	broadcastIterationData(clusters, k, &dt);
#ifdef DEBUG
	if (rank == MASTER)
		printf("Inc points\n"); fflush(stdout);
#endif
	incPoints(points, numberOfPoints, dt, incedPoints);
#ifdef DEBUG
	if (rank == MASTER)
		printf("Set close cluster\n"); fflush(stdout);
#endif
	setCluster(*incedPoints, numberOfPoints, *clusters, k);
#ifdef DEBUG
	if (rank == MASTER)
		printf("Calculate new clusters\n"); fflush(stdout);
#endif
	newClusters = getNewClusters(rank, numberOfProcesses, *incedPoints, numberOfPoints, *clusters, k, &pointInCluster);//Get the new clusters
#ifdef DEBUG
	if (rank == MASTER)
		printf("Check moved points\n"); fflush(stdout);
#endif
	*isMovedPoint = checkMovedPoint(rank, numberOfProcesses, *incedPoints, oldPoints, numberOfPoints);//If there is point that moved to another cluster
#ifdef DEBUG
	if (rank == MASTER)
		printf("Calculate quality\n"); fflush(stdout);
#endif
	*q = calucQ(rank, numberOfProcesses, *incedPoints, numberOfPoints, newClusters, k, pointInCluster);//Calculate quality measure
	if (rank == MASTER)
		memcpy(*clusters, newClusters, k * sizeof(point_t));
	freeAllocations(2, newClusters, pointInCluster);
}

bool checkConditions(int iterations, int LIMIT, int T, float time, bool movedPoint, float QM, float q)
{//Check termination condition
	if (iterations == 0)//Check first iteration
		return true;
	if (iterations >= LIMIT - 1)//Check maximum number of iterations
	{
#ifdef DEBUG
		printf("Max iteration\n"); fflush(stdout);
#endif
		return false;
	}
	if (time >= T)//Check end of time interval 
	{
#ifdef DEBUG
		printf("Max time - dt\n"); fflush(stdout);
#endif
		return false;
	}
	if (!movedPoint)//Check no point move to another cluster
	{
#ifdef DEBUG
		printf("No point moved\n"); fflush(stdout);
#endif
		return false;
	}
	if (q <= QM)//Check quality measure to stop
	{
#ifdef DEBUG
		printf("Quality measure to stop\n"); fflush(stdout);
#endif
		return false;
	}
	return true;
}

void writeToFile(char* path, int pathSize, float t, float q, point_t *clusters, int k)
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