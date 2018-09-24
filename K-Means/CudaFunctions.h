#pragma once

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>

bool cudaInicDT(float* points, int numberOfPoints, float dt, float** inicedPoints);
cudaError_t inicDTWithCuda(float* points, int numberOfPoints, int dim, float dt, float** inicedPoints);