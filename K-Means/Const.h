#pragma once

#include "cuda_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MASTER 0
#define DEBUG_MOOD

typedef struct
{
	double x;
	double y;
	double z;
	double vx;
	double vy;
	double vz;
	int cluster;
} point_velocity_t;

typedef struct
{
	double x;
	double y;
	double z;
	int cluster;
} point_t;

void checkAllocation(void* ptr);