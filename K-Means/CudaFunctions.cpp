#include "CudaFunctions.h"

void incPointsCUDA(point_velocity_t* points, int numberOfPoints, double dt, point_t *incPoints)
{//Inic the points with dt time
	cudaError_t cudaStatus = incPointsWithCuda(points, numberOfPoints, dt, incPoints);// Inic points arr with dt time in parallel.
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "inicDTWithCuda failed!");
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
	}
}

void setClusterCUDA(point_t* points, int numberOfPoints, point_t* clusters, int numberOfCluster)
{//Set the close cluster with CUDA

	cudaError_t cudaStatus = setCloseClusterWithCuda(points, numberOfPoints, clusters, numberOfCluster);// Inic points arr with dt time in parallel.
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "inicDTWithCuda failed!");
	}

	// cudaDeviceReset must be called before exiting in order for profiling and
	cudaStatus = cudaDeviceReset();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceReset failed!");
	}
}