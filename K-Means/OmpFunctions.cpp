#include "OmpFunctions.h"

float distance2Points(point_t p1, point_t p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}
#include <stdio.h>
void setClosestCluster(point_t* points, int numberOfPoints, point_t* clusters, int numberOfCluster)
{
	int tid, numberOfThreads = omp_get_max_threads();
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

point_t* averageClusters(point_t* points, int numberOfPoints, int numberOfCluster)
{
	int tid, numberOfThreads = omp_get_max_threads();
	point_t *result, **setOfK = (point_t**)malloc(numberOfThreads * sizeof(point_t*));
#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
		setOfK[tid] = (point_t*)calloc(numberOfCluster, sizeof(point_t));
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			for (int j = 0; j < numberOfCluster; j++)
			{
				setOfK[tid][points[i].cluster].x += points[i].x;
				setOfK[tid][points[i].cluster].y += points[i].y;
				setOfK[tid][points[i].cluster].z += points[i].z;
				setOfK[tid][points[i].cluster].cluster++;
			}
		}
#pragma omp single
		{
			result = combainPointsArrays(*setOfK, numberOfThreads, numberOfCluster);
		}

		free(setOfK[tid]);
	}
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
			result[i].x += points[j * numberOfArrays *pointsInArray + i].x;
			result[i].y += points[j * numberOfArrays *pointsInArray + i].y;
			result[i].z += points[j * numberOfArrays *pointsInArray + i].z;
			result[i].cluster += points[j * numberOfArrays *pointsInArray + i].cluster;
		}
	}
}