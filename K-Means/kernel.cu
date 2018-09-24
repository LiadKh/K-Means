
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

#define THREAD_IN_BLOCK 1024

__global__ void inicDTKernel(float *inicedPoints, const float *points, float dt, int dim, int numberOfPoints)
{
	int index = blockIdx.x * blockDim.x + threadIdx.x;
	if (index < numberOfPoints)
	{
		for (int i = 0; i < dim; i++) {
			inicedPoints[index * dim + i] = points[index * dim * 2 + i] + dt * points[index * dim * 2 + i + dim];
		
		}
	}
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t inicDTWithCuda(float* points, int numberOfPoints, int dim, float dt, float** inicedPoints)
{
	float *dev_points = 0;
	float *dev_iniced_points = 0;
	cudaError_t cudaStatus;

	// Choose which GPU to run on, change this on a multi-GPU system.
	cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		goto Error;
	}

	// Allocate GPU buffers for three vectors (two input, one output)    .
	cudaStatus = cudaMalloc((void**)&dev_points, numberOfPoints *dim * 2 * sizeof(float));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_iniced_points, numberOfPoints *dim * sizeof(float));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	// Copy input vectors from host memory to GPU buffers.
	cudaStatus = cudaMemcpy(dev_points, points, numberOfPoints *dim * 2 * sizeof(float), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

	int numberOfBlock = numberOfPoints / THREAD_IN_BLOCK + 1;
	if (numberOfPoints % THREAD_IN_BLOCK != 0)
		numberOfBlock++;
	// Launch a kernel on the GPU with one thread for each element.
	inicDTKernel << <numberOfBlock, THREAD_IN_BLOCK >> > (dev_iniced_points, dev_points, dt, dim, numberOfPoints);

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
	cudaStatus = cudaMemcpy(*inicedPoints, dev_iniced_points, numberOfPoints *dim * sizeof(float), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

Error:
	cudaFree(dev_points);
	cudaFree(dev_iniced_points);

	return cudaStatus;
}
