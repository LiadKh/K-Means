#pragma once

#include "Const.h"
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double distance2Points(point_t p1, point_t p2);
void setClosestCluster(point_t* points, int numberOfPoints, point_t* cluster, int numberOfCluster);
point_t* averageCluster(point_t* points, int numberOfPoints, point_t* cluster, int numberOfCluster);