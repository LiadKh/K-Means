#pragma once

#include "Const.h"
#include <mpi.h>
#include <stdio.h>

void MpiInit(int *argc, char** argv[], int *rank, int *numprocs);