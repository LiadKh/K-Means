#pragma once

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "MpiFunctions.h"
#include "CudaFunctions.h"
#include "OmpFunctions.h"

void initProcesses(int *argc, char** argv[], int *rank, int *numberOfProcesses, char* input);
point_t* readDataFile(char* fname, int *N, int* K, int* T, float* dT, int* LIMIT, float* QM);
void initWork(int rank, int numberOfProcesses, point_t *allPoints, int N, point_t **myPoints, int *myNumberOfPoints, point_t **clusters, int *k);
point_t** setPointsInCluster(point_t* points, int numberOfPoints, int numberOfClusters, int *pointsInClusters);
float calucQ(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters, int *numberOfPointsInCluster);
void iteration(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t** clusters, int k, float dt, point_t* oldPoints, point_t** incedPoints, bool *isMovedPoint, float *q);
bool checkConditions(int iterations, int LIMIT, int T, float time, bool movedPoint, float QM, float q);
void writeToFile(float t, float q, point_t *clusters, int k);
void freeAllocations(int count, ...);