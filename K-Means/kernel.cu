
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include "Const.h"

#define THREAD_IN_BLOCK 1024

__global__ void incDTKernel(point_t *inicedPoints, const point_t *points, float dT, int numberOfPoints)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < numberOfPoints)
	{
		inicedPoints[index].x = points[index].x + dT*points[index].vx;
		inicedPoints[index].y = points[index].y + dT*points[index].vy;
		inicedPoints[index].z = points[index].z + dT*points[index].vz;
	}
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t incDTWithCuda(point_t* points, int numberOfPoints, float dT, point_t* inicedPoints)
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

	int numberOfBlock = numberOfPoints / THREAD_IN_BLOCK + 1;
	if (numberOfPoints % THREAD_IN_BLOCK != 0)
		numberOfBlock++;
	// Launch a kernel on the GPU with one thread for each element.
	incDTKernel << <numberOfBlock, THREAD_IN_BLOCK >> > (dev_iniced_points, dev_points, dT, numberOfPoints);

	// Check for any errors launching the kernel
	cudaStatus = cudaGetLastError();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
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
	cudaStatus = cudaMemcpy(inicedPoints, dev_iniced_points, numberOfPoints * sizeof(point_t), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

Error:
	cudaFree(dev_points);
	cudaFree(dev_iniced_points);

	return cudaStatus;
}
