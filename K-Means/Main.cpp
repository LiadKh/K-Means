#include "Functions.h"

void printIt(point_t* points, int size)
{
	for (int i = 0; i < size; i++)
	{
		printf("%3f ", points[i].x);
		printf("%3f ", points[i].y);
		printf("%3f ", points[i].z);
		printf("\n");
	}
}

int main(int argc, char* argv[])
{
	int rank, numberOfProcesses;
	int N, K, T, LIMIT, myNumberOfPoints;
	float dT, QM;
	point_t *allPoints, *myPoints, *clusters;

	initProcesses(&argc, &argv, &rank, &numberOfProcesses);
	if (rank == MASTER)
	{
		allPoints = readDataFile(INPUT_FILE, &N, &K, &T, &dT, &LIMIT, &QM);
		myNumberOfPoints = int(N / numberOfProcesses);
	}
	initProject(allPoints, &myPoints, &myNumberOfPoints);

	free(myPoints);
	if (rank == MASTER)
		free(allPoints);
	mpiFinish();
	return EXIT_SUCCESS;
}