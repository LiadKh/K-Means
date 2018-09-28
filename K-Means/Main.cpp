#include "Functions.h"

void printIt(point_t* points, int size)
{
	printf("%d", size); fflush(stdout);
	for (int i = 0; i < size; i++)
	{
		printf("%3f ", points[i].x);
		printf("%3f ", points[i].y);
		printf("%3f ", points[i].z);
		printf("%d ", points[i].cluster);
		printf("\n"); fflush(stdout);
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

	initProject(rank, allPoints, &myPoints, &myNumberOfPoints, &clusters, &K);
	setClosestCluster(myPoints, myNumberOfPoints, clusters, K);
	if (rank == MASTER)
		free(allPoints);
	allPoints = gatherPoints(rank, myPoints, numberOfProcesses, myNumberOfPoints);
	if (rank == MASTER)
		printIt(allPoints, N);
	free(clusters);
	free(myPoints);
	if (rank == MASTER)
		free(allPoints);
	mpiFinish();
	return EXIT_SUCCESS;
}