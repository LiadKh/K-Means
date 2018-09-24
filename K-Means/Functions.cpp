#include "Functions.h"

float* readDataFile(char* fname, int *N, int* K, int* T, float* dT, float* LIMIT, float* QM)
{//Read data from file
	float* points;
	FILE* f = fopen(fname, "r");
	if (f == NULL)
	{
		printf("Failed opening the file. Exiting!\n");
		exit(EXIT_FAILURE);
	}
	fscanf(f, "%d %d %d %f %f %f", N, K, T, dT, LIMIT, QM);
	printf("%d %d %d %f %f %f\n", *N, *K, *T, *dT, *LIMIT, *QM);
	points = (float*)malloc((*N) * sizeof(float) * DIMENSION * 2);
	if (points == NULL)
	{
		printf("Not enough memory. Exiting!\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < *N; i++)
	{
		for (int j = 0; j < 2 * DIMENSION; j++)
			fscanf(f, "%f", &(points[i*(2 * DIMENSION) + j]));
	}
	fclose(f);
	return points;
}