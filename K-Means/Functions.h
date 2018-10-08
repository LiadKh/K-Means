#pragma once

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "MpiFunctions.h"
#include "CudaFunctions.h"
#include "OmpFunctions.h"

point_t* readDataFile(char* fname, int *N, int* K, int* T, float* dT, int* LIMIT, float* QM);
void writeToFile(float t, float q, point_t *clusters, int k);
void initProcesses(int *argc, char** argv[], int *rank, int *numberOfProcesses);
void initWork(int rank, int numberOfProcesses, point_t *allPoints, int N, point_t **myPoints, int *myNumberOfPoints, point_t **clusters, int *k);
point_t* chooseFirstClusters(point_t* points, int numberOfClusters);
point_t* getNewClusters(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, int numberOfClusters);
bool checkMovedPoint(int rank, int numberOfProcesses, point_t* p1, point_t* p2, int numberOfPoints);
point_t* iteration(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int k, float *dt, point_t* oldPoints, point_t** incedPoints, float *q);
bool checkConditions(int iterations, int LIMIT, int T, float dt, bool movedPoint, float QM, float q);
void freeAllocations(int count, ...);
void printIt(point_t* points, int size);