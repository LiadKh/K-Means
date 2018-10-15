#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#pragma warning (disable : 4996)

#define NUMBER_OF_POINTS 1000 //3000000
#define NUMBER_OF_CLUSTER 20
#define TIME_INTERVAL 30
#define dT 0.1
#define LIMIT_ITERATION 50
#define QM 1.0
#define OUTPUT "input.txt"

float randomIntNumber(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

float float_rand()
{
	return  rand() / (float)RAND_MAX; /* [0, 1.0] */
}

float getRandom(float Min, float Max)
{
	return randomIntNumber(Min, Max) + float_rand();
}

int main(int argc, const char* argv[])
{
	srand((unsigned int)time(NULL));
	float r;
	FILE *f = fopen(OUTPUT, "w");
	if (f == NULL)
	{
		printf("File problem");
		exit(EXIT_FAILURE);
	}
	fprintf(f, "%d %d %d %f %d %f\n", NUMBER_OF_POINTS, NUMBER_OF_CLUSTER, TIME_INTERVAL, dT, LIMIT_ITERATION, QM);
	for (int i = 0; i < NUMBER_OF_POINTS; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			r = getRandom(-i, i);
			fprintf(f, "%f ", r);
		}
		for (int j = 0; j < 3; j++)
		{
			r = getRandom(-1, 1);
			fprintf(f, "%f ", r);
		}
		fprintf(f, "\n");
	}
	fclose(f);
	system("pause");
}