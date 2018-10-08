#pragma once

#define MASTER 0
#define INPUT_FILE "D:\\Projects\\Visual studio\\Parallel and Distributed Computing - Final Project\\K-Means\\K-Means\\input.txt"
#define OUTPUT_FILE "D:\\Projects\\Visual studio\\Parallel and Distributed Computing - Final Project\\K-Means\\K-Means\\output.txt"

typedef struct
{
	float x;
	float y;
	float z;
	float vx;
	float vy;
	float vz;
	int cluster;
} point_t;