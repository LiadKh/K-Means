#include "Functions.h"
#include <time.h>

int main(int argc, char* argv[])
{
	clock_t start = clock(), end;
	int rank, numberOfProcesses, N, K, T, LIMIT, myNumberOfPoints, iterationNumber = 0;
	float dT, time, QM, q;
	bool anotherIteration, isMovedPoint = NULL;
	point_t *allPoints, *myPoints, *clusters, *incPoints = NULL, *previousIncPoints = NULL;
	char input[INPUT_FILE_SIZE];

	initProcesses(&argc, &argv, &rank, &numberOfProcesses, input);
	if (rank == MASTER)
		allPoints = readDataFile(input, &N, &K, &T, &dT, &LIMIT, &QM);
	initWork(rank, numberOfProcesses, allPoints, N, &myPoints, &myNumberOfPoints, &clusters, &K);
	do
	{
		if (rank == MASTER)
			time = iterationNumber*dT;
		previousIncPoints = incPoints;
		iteration(rank, numberOfProcesses, myPoints, myNumberOfPoints, &clusters, K, time, previousIncPoints, &incPoints, &isMovedPoint, &q);
		if (rank == MASTER)
		{
			anotherIteration = checkConditions(iterationNumber, LIMIT, T, time, isMovedPoint, QM, q);//Check the termination condition
			iterationNumber++;
			printf("Iteration: %d\n", iterationNumber); fflush(stdout);
		}
		MPI_Bcast(&anotherIteration, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);//MASTER send if there is more iteration
		free(previousIncPoints);
	} while (anotherIteration);
	if (rank == MASTER)
	{//Write result to file
		end = clock();
		printf("Work time %f\n", (double)(end - start)); fflush(stdout);
		writeToFile(time, q, clusters, K);
		free(allPoints);
	}
	freeAllocations(2, myPoints, incPoints, clusters);
	MPI_Finalize();
	return EXIT_SUCCESS;
}