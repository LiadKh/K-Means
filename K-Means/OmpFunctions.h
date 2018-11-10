#pragma once

#include "Const.h"
#include <omp.h>
#include <stdlib.h>
#include <string.h>

void incPointsOMP(point_velocity_t* points, int numberOfPoints, double dt, point_t *incPoints);
double distancePoints(point_t *p1, point_t *p2);
void setCloseClusterOMP(point_t* points, int numberOfPoints, point_t* clusters, int numberOfClusters);
point_t* sumClusters(point_t* points, int numberOfPoints, int numberOfClusters, int *pointInCluster);
point_t* combainPointsArrays(point_t* points, int numberOfArrays, int pointsInArray, int *pointInCluster);
void setAverageToClusters(point_t* clusters, point_t* previousClusters, int numberOfPoints);
bool isMovedPoint(point_t* points, int* pointInCluster, int size);
bool checkArray(bool *arr, int size);
double biggestDistance(point_t* points, int numberOfPoints);
double findQ(double* maxDistance, point_t* clusters, int numberOfClusters);