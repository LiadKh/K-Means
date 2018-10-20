#include "OmpFunctions.h"

float distancePoints(point_t p1, point_t p2)
{//Find distance between two points
	return (float)sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

void setCloseClusterOMP(point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters)
{//Find the close cluster
	int tid = NULL, numberOfThreads = omp_get_max_threads();
#pragma omp parallel for
	for (int i = 0; i < numberOfPoints; i++)
	{
		float distance, temp;
		for (int j = 0; j < numberOfClusters; j++)
		{
			temp = distancePoints(points[i], clusters[j]);
			if (j == 0 || temp < distance)
			{//Set close cluster id
				points[i].cluster = j;
				distance = temp;
			}
		}
	}
}

point_t* sumClusters(point_t* points, int numberOfPoints, int numberOfClusters, int *pointInCluster)
{//Sum the points (x,y,z) in the same cluster
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	point_t *result, *setOfK = (point_t*)calloc(numberOfThreads * numberOfClusters, sizeof(point_t));
	if (setOfK == NULL)
	{//Allocation problem
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			int cluster = points[i].cluster;
			setOfK[tid*numberOfClusters + cluster].x += points[i].x;
			setOfK[tid*numberOfClusters + cluster].y += points[i].y;
			setOfK[tid*numberOfClusters + cluster].z += points[i].z;
			setOfK[tid*numberOfClusters + cluster].cluster++;
		}
	}
	result = combainPointsArrays(setOfK, numberOfThreads, numberOfClusters, pointInCluster);
	free(setOfK);
	return result;
}

point_t* combainPointsArrays(point_t* points, int numberOfArrays, int pointsInArray, int *pointInCluster)
{//Combain arrays of points (x,y,z) to one array - each array have the same number of points
	point_t * result = (point_t*)calloc(pointsInArray, sizeof(point_t));
	if (result == NULL)
	{//Allocation problem
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
#pragma omp parallel for
	for (int i = 0; i < pointsInArray; i++)
	{
		for (int j = 0; j < numberOfArrays; j++)
		{
			result[i].x += points[j*pointsInArray + i].x;
			result[i].y += points[j*pointsInArray + i].y;
			result[i].z += points[j*pointsInArray + i].z;
			result[i].cluster += points[j*pointsInArray + i].cluster;
		}
		if (pointInCluster != NULL)
			pointInCluster[i] = result[i].cluster;
	}
	return result;
}

void setAverageToClusters(point_t* clusters, point_t* previousClusters, int numberOfPoints)
{//Divide point coordinates to number of point connect to this point
#pragma omp parallel for
	for (int i = 0; i < numberOfPoints; i++)
	{
		int cluster = clusters[i].cluster;
		if (cluster != 0)
		{
			clusters[i].x = clusters[i].x / clusters[i].cluster;
			clusters[i].y = clusters[i].y / clusters[i].cluster;
			clusters[i].z = clusters[i].z / clusters[i].cluster;
		}
		else
		{//No point in this cluster - set the previous cluster
			clusters[i].x = previousClusters[i].x;
			clusters[i].y = previousClusters[i].y;
			clusters[i].z = previousClusters[i].z;
		}
	}
}

bool isMovedPoint(point_t* p1, point_t* p2, int size)
{//Check if point in p1 move to another cluster in p2
	if (p1 == NULL || p2 == NULL)
		return true;
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	bool answer, *setAnswer = (bool*)malloc(numberOfThreads * sizeof(bool));
	if (setAnswer == NULL)
	{//Allocation problem
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	memset(setAnswer, false, sizeof(setAnswer));
#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
#pragma omp for
		for (int i = 0; i < size; i++)
		{
			if (p1[i].cluster != p2[i].cluster)
				setAnswer[tid] = true;
		}
	}
	answer = checkArray(setAnswer, numberOfThreads);
	free(setAnswer);
	return answer;
}

bool checkArray(bool *arr, int size)
{//Check if is there true in the array
	for (int i = 0; i < size; i++)
		if (arr[i] == true)
			return true;
	return false;
}
//
float biggestDistance(point_t* points, int numberOfPoints)
{
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	float dis = NULL, maxDistance;
	float* distanceArray = (float*)calloc(numberOfThreads, sizeof(float));
	if (distanceArray == NULL)
	{//Allocation problem
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}

#pragma omp parallel private(tid,dis)
	{
		tid = omp_get_thread_num();//Thread id
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{//Each thread find the max distance
			for (int j = i + 1; j < numberOfPoints; j++)
			{
				dis = distancePoints(points[i], points[j]);
				if (dis > distanceArray[tid])
					distanceArray[tid] = dis;
			}
		}
	}
	for (int i = 0; i < numberOfThreads; i++)
	{//Find the max distance
		if (i == 0 || distanceArray[i] > maxDistance)
			maxDistance = distanceArray[i];
	}
	free(distanceArray);
	return maxDistance;
}

float findQ(float* maxDistance, point_t* clusters, int numberOfClusters)
{
	float q = 0, dis = NULL;
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	float* distanceArray = (float*)calloc(numberOfThreads, sizeof(float));
	if (distanceArray == NULL)
	{//Allocation problem
		printf("Not enough memory. Exiting!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}

#pragma omp parallel private(tid,dis)
	{
		tid = omp_get_thread_num();//Thread id
#pragma omp for
		for (int i = 0; i < numberOfClusters; i++)
		{
			for (int j = 0; j < numberOfClusters; j++)
			{
				if (i != j)
				{
					dis = distancePoints(clusters[i], clusters[j]);
					if (dis != 0)
						distanceArray[tid] = distanceArray[tid] + maxDistance[i] / dis;
				}
			}
		}
	}
	for (int i = 0; i < numberOfThreads; i++)
		q += distanceArray[i];
	free(distanceArray);
	q = q / (numberOfClusters*(numberOfClusters - 1));
	return q;
}