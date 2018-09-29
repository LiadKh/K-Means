#pragma once

#include "Const.h"
#include <mpi.h>
#include <stdlib.h>

static MPI_Datatype PointMPIType;

void mpiInit(int *argc, char** argv[], int *rank, int *numberOfProcesses);
void commitMpiPointType();
void broadcastClusters(int rank, point_t** clusters, int *k);
void scatterPoints(point_t* allPoints, point_t** myPoints, int *numberOfPoints);
void broadcastDT(float *dt);
point_t* gatherPoints(int rank, int numberOfProcesses, point_t* myPoints, int numberOfPointsToSend);
void mpiFinish();