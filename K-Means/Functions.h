#pragma once

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MpiFunctions.h"
#include "CudaFunctions.h"
#include "OmpFunctions.h"

point_t* readDataFile(char* fname, int *N, int* K, int* T, float* dT, int* LIMIT, float* QM);
void initProcesses(int *argc, char** argv[], int *rank, int *numberOfProcesses);
void initProject(int rank, int numberOfPoints, point_t *allPoints, point_t **myPoints, int *myNumberOfPoints, point_t *clusters, int *k);
point_t* chooseK(point_t* points, int numberOfPoints, int k);
void interaction(point_t* points, int numberOfPoints, point_t* clusters, int rank, int numberOfProcess, int k, float *dt);