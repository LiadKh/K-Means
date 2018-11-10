#pragma once

#include "Const.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "MpiFunctions.h"
#include "CudaFunctions.h"
#include "OmpFunctions.h"

#define PATH_SIZE 50
#define INPUT_FILE "input.txt"
#define OUTPUT_FILE "output.txt"
#define CUDA_PERCENT_OF_WORK 0.2 //[0,1]

extern MPI_Datatype PointMPIType;

static char newLine[] = "------------------------------------------------------------";

char* createFileName(char* path, int pathSize, char* fileName, int nameSize);
void initProcesses(int *argc, char** argv[], int *rank, int *numberOfProcesses, char* path, int *pathSize);
point_velocity_t* readDataFile(char* path, int pathSize, int *N, int* K, double* T, double* dT, int* LIMIT, double* QM);
void initWork(int rank, int numberOfProcesses, point_velocity_t *allPoints, int N, point_velocity_t **myPoints, int *myNumberOfPoints, point_t **clusters, int *k);
void incPointsDT(point_t* points, int numberOfPoints, double dt, point_t *incPoints);
void setCluster(point_t *points, int numberOfPoints, point_t* clusters, int numberOfCluster);
point_t* getNewClusters(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* oldClusters, int numberOfClusters, int *numberOfPointInCluster);
bool checkMovedPoint(int rank, int numberOfProcesses, point_t* points, int* pointInCluster, int numberOfPoints);
point_t** setPointsInCluster(point_t* points, int numberOfPoints, int numberOfClusters, int *pointsInClusters);
bool checkTerminationCondition(int iterations, int LIMIT, bool movedPoint);
double kmeansIterations(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int k, int LIMIT);
double calucQ(int rank, int numberOfProcesses, point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters, int *numberOfPointsInCluster);
bool checkConditions(double T, double time, double QM, double q);
void work(int rank, int numberOfProcesses, double *time, double *q, double dt, point_velocity_t *points, int numberOfPoints, point_t* clusters, int k, int LIMIT, double T, double QM);
void writeToFile(char* path, int pathSize, double t, double q, point_t *clusters, int k);
void freeAllocations(int count, ...);