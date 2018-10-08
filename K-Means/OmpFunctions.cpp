#include "OmpFunctions.h"

float distancePoints(point_t p1, point_t p2)
{//Find distance between two points
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

void setClosestCluster(point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters)
{//Find the closest cluster
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	point_t **setOfK = (point_t**)malloc(numberOfThreads * sizeof(point_t*));

#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
		setOfK[tid] = (point_t*)calloc(numberOfClusters, sizeof(point_t));//Each thread have array of k points - clusters
		memcpy(setOfK[tid], clusters, numberOfClusters * sizeof(point_t));//Copy k clusters

#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			float distance, temp;
			for (int j = 0; j < numberOfClusters; j++)
			{
				temp = distancePoints(points[i], setOfK[tid][j]);
				if (j == 0 || temp < distance)
				{//Set closest cluster id
					points[i].cluster = j;
					distance = temp;
				}
			}
		}
		free(setOfK[tid]);
	}
	free(setOfK);
}

point_t* sumClusters(point_t* points, int numberOfPoints, int numberOfClusters)
{//Sum the points (x,y,z) in the same cluster
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	point_t *result, *setOfK = (point_t*)calloc(numberOfThreads * numberOfClusters, sizeof(point_t));

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
	result = combainPointsArrays(setOfK, numberOfThreads, numberOfClusters);
	free(setOfK);
	return result;
}

point_t* combainPointsArrays(point_t* points, int numberOfArrays, int pointsInArray)
{//Combain arrays of points (x,y,z) to one array - each array have the same number of points
	point_t * result = (point_t*)calloc(pointsInArray, sizeof(point_t));

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
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	bool answer, *setAnswer = (bool*)malloc(numberOfThreads * sizeof(bool));
	memset(setAnswer, false, sizeof(setAnswer));
#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
#pragma omp parallel for
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