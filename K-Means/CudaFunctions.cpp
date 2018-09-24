#include "CudaFunctions.h"

bool cudaInicDT(float* points, int numberOfPoints, float dt, float** inicedPoints)
{//Inic the points with dt time
	bool status = true;
	*inicedPoints = (float*)malloc(numberOfPoints*DIMENSION * sizeof(float));
	cudaError_t cudaStatus = inicDTWithCuda(points, numberOfPoints, DIMENSION, dt, inicedPoints);// Inic points arr with dt time in parallel.
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