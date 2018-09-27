#include "OmpFunctions.h"

double distance2Points(point_t p1, point_t p2)
{
	return sqrt(pow(p1.x + p2.x, 2) + pow(p1.y + p2.y, 2) + pow(p1.z + p2.z, 2));
}

void setClosestCluster(point_t* points, int numberOfPoints, point_t* cluster, int numberOfCluster)
{
	int tid, numberOfThreads = omp_get_max_threads();
	point_t **setOfK = (point_t**)malloc(numberOfThreads * sizeof(point_t*));
	int *result = (int*)calloc(numberOfPoints, sizeof(int));
#pragma omp parallel private(tid)
	{
		tid = omp_get_thread_num();//Thread id
		setOfK[tid] = (point_t*)calloc(numberOfCluster, sizeof(point_t));
		memcpy(setOfK[tid], cluster, numberOfCluster * sizeof(point_t));
#pragma omp for
		for (int i = 0; i < numberOfPoints; i++)
		{
			double distance, temp;
			for (int j = 0; j < numberOfCluster; j++)
			{
				temp = distance2Points(points[i], setOfK[tid][j]);
				if (j = 0 || temp < distance)
				{
					result[i] = j;
					distance = temp;
				}
			}
		}
		free(setOfK[tid]);
	}
	free(setOfK);
}

point_t* averageCluster(point_t* points, int numberOfPoints, point_t* cluster, int numberOfCluster)
{
	int tid, numberOfThreads = omp_get_max_threads();
	point_t **setOfK = (point_t**)malloc(numberOfThreads * sizeof(point_t*));
	point_t * result = (point_t*)malloc(numberOfCluster * sizeof(point_t));
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
#pragma omp for
		for (int i = 0; i < numberOfCluster; i++)
		{
			
			for (int j = 0; j < numberOfThreads; j++)
			{
				result[i].x += setOfK[j][i].x;
				result[i].y += setOfK[j][i].y;
				result[i].z += setOfK[j][i].z;
				result[i].cluster += setOfK[j][i].cluster;
			}
		}
		free(setOfK[tid]);
	}
	free(setOfK);
	return result;
}