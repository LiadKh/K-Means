#pragma once

#include "Const.h"
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

#define FINISH_WORK_TAG 999

static MPI_Datatype PointMPIType;

void mpiInit(int *argc, char** argv[], int *rank, int *numberOfProcesses);
void commitMpiPointType();
void scatterPoints(int rank, int numberOfProcesses, point_t* allPoints, int N, point_t** myPoints, int *numberOfPoints);
void broadcastIterationData(point_t **clusters, int k, float *dt);
point_t* gatherPoints(int rank, int numberOfProcesses, point_t* myPoints, int numberOfPointsToSend);
bool* gatherBool(int rank, int numberOfProcesses, bool check);
void collectPointsInClusters(int rank, int numberOfProcesses, int numberOfClusters, point_t** points, int *pointsInClusters);
float* sendArrayOfPointInCluster(int numberOfProcesses, point_t** pointsInClusters, int* numberOfPointInClusters, int numberOfClusters);
point_t* receiveArrayOfPointInCluster(int *workSize, int *clusterId, float distance, bool found);
