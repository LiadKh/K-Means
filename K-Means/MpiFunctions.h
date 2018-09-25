#pragma once

#include "Const.h"
#include <mpi.h>
#include <stdlib.h>

static MPI_Datatype PointMPIType;

void mpiInit(int *argc, char** argv[], int *rank, int *numprocs);
void commitMpiPointType();
void scatterPoints(point_t* allPoints, point_t** myPoints, int *numberOfPoints, float *dt);
void gatherPoints(point_t* allPoints, point_t* myPoints, int numberOfPoints);
void mpiFinish();