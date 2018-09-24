#pragma once

#include "Const.h"
#include <mpi.h>
#include <stdlib.h>

void mpiInit(int *argc, char** argv[], int *rank, int *numprocs);
void sendPoints(float* allPoints, float** myPoints, int *numberOfPoints);
void getPoints(float* allPoints, float* myPoints, int numberOfPoints);