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