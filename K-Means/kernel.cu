
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include "Const.h"

#define THREAD_IN_BLOCK 1000
#define MAX_CLUSTERS 200
#define ONE_THREAD_WORK 5

__global__ void incKernel(point_t *incPoints, const point_t *points, double dT, int numberOfPoints)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < numberOfPoints)
	{
		incPoints[index].x = points[index].x + dT*points[index].vx;
		incPoints[index].y = points[index].y + dT*points[index].vy;
		incPoints[index].z = points[index].z + dT*points[index].vz;
	}
}

__device__ double distance2Points(point_t *p1, point_t *p2)
{//Find distance between two points
	return sqrt(pow(p1->x - p2->x, 2) + pow(p1->y - p2->y, 2) + pow(p1->z - p2->z, 2));
}

__global__ void setCloseClusterKernel(point_t *points, int numberOfPoints, point_t *clusters, int numberOfClusters)
{
	int index = (blockIdx.x * blockDim.x + threadIdx.x)*ONE_THREAD_WORK;
	double temp, distance;
	__shared__ point_t sharedClusters[MAX_CLUSTERS];
	if (threadIdx.x < numberOfClusters)
		sharedClusters[threadIdx.x] = clusters[threadIdx.x];
	__syncthreads();
	for (int i = 0; i < ONE_THREAD_WORK && index < numberOfPoints; i++, index++)
	{
		for (int j = 0; j < numberOfClusters; j++)
		{
			temp = distance2Points(&(points[index]), &(sharedClusters[j]));
			if (j == 0 || temp < distance)
			{//Set close cluster id
				points[index].cluster = j;
				distance = temp;
			}
		}
	}
}

// Helper function for using CUDA to inc point with dt speed in parallel.
cudaError_t incPointsWithCuda(point_t* points, int numberOfPoints, double dT, point_t* incPoints)
{
	point_t *dev_points = 0;
	point_t *dev_iniced_points = 0;
	cudaError_t cudaStatus;

	// Choose which GPU to run on, change this on a multi-GPU system.
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		goto Error;
	}

	// Allocate GPU buffers for three vectors (two input, one output)    .
	cudaStatus = cudaMalloc((void**)&dev_points, numberOfPoints * sizeof(point_t));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_iniced_points, numberOfPoints * sizeof(point_t));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	// Copy input vectors from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(dev_points, points, numberOfPoints * sizeof(point_t), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

	// Launch a kernel on the GPU with one thread for each element.
	incKernel << <numberOfPoints / THREAD_IN_BLOCK + 1, THREAD_IN_BLOCK >> > (dev_iniced_points, dev_points, dT, numberOfPoints);

	// Check for any errors launching the kernel
	cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "incKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
		goto Error;
	}

	// cudaDeviceSynchronize waits for the kernel to finish, and returns
	// any errors encountered during the launch.
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
		goto Error;
	}

	// Copy output vector from GPU buffer to host memory.
	cudaStatus = cudaMemcpy(incPoints, dev_iniced_points, numberOfPoints * sizeof(point_t), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

Error:
	cudaFree(dev_points);
	cudaFree(dev_iniced_points);

	return cudaStatus;
}

// Helper function for using CUDA to set the close cluster to each point in parallel..
cudaError_t setCloseClusterWithCuda(point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters)
{
	point_t *dev_points = 0;
	point_t *dev_clusters = 0;
	cudaError_t cudaStatus;

	// Choose which GPU to run on, change this on a multi-GPU system.
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		goto Error;
	}

	// Allocate GPU buffers for three vectors (two input, one output)    .
	cudaStatus = cudaMalloc((void**)&dev_points, numberOfPoints * sizeof(point_t));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_clusters, numberOfClusters * sizeof(point_t));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	// Copy input vectors from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(dev_points, points, numberOfPoints * sizeof(point_t), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

	cudaStatus = cudaMemcpy(dev_clusters, clusters, numberOfClusters * sizeof(point_t), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

	//int numberOfThread = THREAD_IN_BLOCK / ONE_THREAD_WORK;
	setCloseClusterKernel << <numberOfPoints / (THREAD_IN_BLOCK / ONE_THREAD_WORK) + 1, THREAD_IN_BLOCK >> > (dev_points, numberOfPoints, dev_clusters, numberOfClusters);

	// Check for any errors launching the kernel
	cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "setCloseClusterKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
		goto Error;
	}

	// cudaDeviceSynchronize waits for the kernel to finish, and returns
	// any errors encountered during the launch.
	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
		goto Error;
	}

	// Copy output vector from GPU buffer to host memory.
	cudaStatus = cudaMemcpy(points, dev_points, numberOfPoints * sizeof(point_t), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

Error:
	cudaFree(dev_points);
	cudaFree(dev_clusters);

	return cudaStatus;
}