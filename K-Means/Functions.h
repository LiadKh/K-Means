#pragma once

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>
#include "MpiFunctions.h"
#include "CudaFunctions.h"
#include "OmpFunctions.h"

point_t* readDataFile(char* fname, int *N, int* K, int* T, float* dT, int* LIMIT, float* QM);