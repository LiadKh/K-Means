#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>

point_t* incPoints(point_t* points, int numberOfPoints, float dt);
cudaError_t incPointsWithCuda(point_t* points, int numberOfPoints, float dt, point_t* inicedPoints);