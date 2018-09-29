#include "OmpFunctions.h"

float distance2Points(point_t p1, point_t p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

void setClosestCluster(point_t* points, int numberOfPoints, point_t* clusters, int numberOfCluster)
{
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	point_t **setOfK = (point_t**)malloc(numberOfThreads * sizeof(point_t*));

#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
		setOfK[tid] = (point_t*)calloc(numberOfCluster, sizeof(point_t));
		memcpy(setOfK[tid], clusters, numberOfCluster * sizeof(point_t));

#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			float distance, temp;
			for (int j = 0; j < numberOfCluster; j++)
			{
				temp = distance2Points(points[i], setOfK[tid][j]);
				if (j == 0 || temp < distance)
				{
					points[i].cluster = j;
					distance = temp;
				}
			}
		}
		free(setOfK[tid]);
	}
	free(setOfK);
}

point_t* sumClusters(point_t* points, int numberOfPoints, int numberOfCluster)
{
	int tid = NULL, numberOfThreads = omp_get_max_threads();
	point_t *result, *setOfK = (point_t*)calloc(numberOfThreads * numberOfCluster, sizeof(point_t));
#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			int cluster = points[i].cluster;
			setOfK[tid*numberOfCluster + cluster].x += points[i].x;
			setOfK[tid*numberOfCluster + cluster].y += points[i].y;
			setOfK[tid*numberOfCluster + cluster].z += points[i].z;
			setOfK[tid*numberOfCluster + cluster].cluster++;
		}
	}
	result = combainPointsArrays(setOfK, numberOfThreads, numberOfCluster);
	free(setOfK);
	return result;
}

point_t* combainPointsArrays(point_t* points, int numberOfArrays, int pointsInArray)
{
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

void averageClusters(point_t* points, int numberOfPoints)
{
#pragma omp parallel for
	{
		for (int i = 0; i < numberOfPoints; i++)
		{
			int cluster=points[i].cluster;
			if (cluster != 0)
			{
				points[i].x = points[i].x / points[i].cluster;
				points[i].y = points[i].y / points[i].cluster;
				points[i].z = points[i].z / points[i].cluster;
			}
		}
	}
}