#pragma once

#include "Const.h"
#include <omp.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float distancePoints(point_t p1, point_t p2);
void setClosestCluster(point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters);
point_t* sumClusters(point_t* points, int numberOfPoints, int numberOfClusters);
point_t* combainPointsArrays(point_t* points, int numberOfArrays, int pointsInArray);
void setAverageToClusters(point_t* points, int numberOfPoints);
bool isMovedPoint(point_t* p1, point_t* p2, int size);
bool checkArray(bool *arr, int size);