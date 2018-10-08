#include "Functions.h"

int main(int argc, char* argv[])
{
	int rank, numberOfProcesses, N, K, T, LIMIT, myNumberOfPoints;
	float dT, time, QM, q, n = 0;
	point_t *allPoints, *myPoints, *clusters, *incPoints = NULL, *previousIncPoints = NULL;
	bool anotherIteration;

	initProcesses(&argc, &argv, &rank, &numberOfProcesses);
	if (rank == MASTER)
		allPoints = readDataFile(INPUT_FILE, &N, &K, &T, &dT, &LIMIT, &QM);
	initWork(rank, numberOfProcesses, allPoints, N, &myPoints, &myNumberOfPoints, &clusters, &K);
	do
	{
		if (rank == MASTER)
			time = n*dT;
		previousIncPoints = incPoints;
		//newClusters = iteration()
		if (rank == MASTER)
		{
			anotherIteration = checkConditions(iterations, int LIMIT, int T, float dt, bool movedPoint, float QM, float q);
			iterations++;
		}
		free(previousIncPoints);
	} while (anotherIteration);
	if (rank == MASTER)
	{//Write result to file
		writeToFile(time, q, clusters, K);
		free(allPoints);
	}
	freeAllocations(2, myPoints, clusters, incPoints);
	mpiFinish();
	return EXIT_SUCCESS;
}