#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>

bool incDTpoint(point_t* points, int numberOfPoints, float dt, point_t** inicedPoints);
cudaError_t incDTWithCuda(point_t* points, int numberOfPoints, float dt, point_t* inicedPoints);