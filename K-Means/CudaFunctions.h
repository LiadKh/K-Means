#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>

point_t* incAndSetCloseCluster(point_t* points, int numberOfPoints, float dt, point_t* clusters, int numberOfClusters);
cudaError_t incPointsWithCuda(point_t* points, int numberOfPoints, float dT, point_t* incPoints, point_t* clusters, int numberOfClusters);