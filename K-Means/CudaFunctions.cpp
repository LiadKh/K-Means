#include "CudaFunctions.h"

point_t* incPoints(point_t* points, int numberOfPoints, float dt)
{//Inic the points with dt time
	point_t *incPoints = (point_t*)malloc(numberOfPoints * sizeof(point_t));
	cudaError_t cudaStatus = incPointsWithCuda(points, numberOfPoints, dt, incPoints);// Inic points arr with dt time in parallel.
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "inicDTWithCuda failed!");
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
	}
	return incPoints;
}