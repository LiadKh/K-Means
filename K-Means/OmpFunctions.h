#pragma once

#include "Const.h"
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double distance2Points(point_t p1, point_t p2);
void setClosestCluster(point_t* points, int numberOfPoints, point_t* clusters, int numberOfCluster);
point_t* averageClusters(point_t* points, int numberOfPoints, int numberOfCluster);
point_t* combainPointsArrays(point_t* points, int numberOfArrays, int pointsInArray);