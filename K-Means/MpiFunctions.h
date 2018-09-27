#pragma once

#include "Const.h"
#include <mpi.h>
#include <stdlib.h>

static MPI_Datatype PointMPIType;

void mpiInit(int *argc, char** argv[], int *rank, int *numprocs);
void commitMpiPointType();
void broadcastDataDt(float *dt);
void scatterPoints(point_t* allPoints, point_t** myPoints, int *numberOfPoints);
point_t* gatherPoints(int rank, point_t* myPoints, int numberOfProcesses, int numberOfPointsToSend);
void mpiFinish();