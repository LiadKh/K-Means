#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>

void incPointsCUDA(point_velocity_t* points, int numberOfPoints, double dt, point_t *incPoints);
cudaError_t incPointsWithCuda(point_velocity_t* points, int numberOfPoints, double dT, point_t* incPoints);
void setClusterCUDA(point_t* points, int numberOfPoints, point_t* clusters, int numberOfCluster);
cudaError_t setCloseClusterWithCuda(point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters);