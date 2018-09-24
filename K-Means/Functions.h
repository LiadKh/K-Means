#pragma once

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>
#include "MpiFunctions.h"
#include "CudaFunctions.h"
#include "OmpFunctions.h"

float* readDataFile(char* fname, int *N, int* K, int* T, float* dT, float* LIMIT, float* QM);
void mpiFinish();