#include "CudaFunctions.h"

bool incDTpoint(point_t* points, int numberOfPoints, float dt, point_t** inicedPoints)
{//Inic the points with dt time
	bool status = true;
	*inicedPoints = (point_t*)malloc(numberOfPoints * sizeof(point_t));
	cudaError_t cudaStatus = incDTWithCuda(points, numberOfPoints, dt, *inicedPoints);// Inic points arr with dt time in parallel.
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "inicDTWithCuda failed!");
		status = false;
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
		status = false;
	}

	return status;
}