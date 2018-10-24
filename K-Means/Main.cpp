#include "Functions.h"

int main(int argc, char* argv[])
{
	clock_t start, end;
	int rank, numberOfProcesses, N, K, LIMIT, myNumberOfPoints, iterationNumber = 0, pathSize;
	float dT, time, T, QM, q;
	bool anotherIteration, isMovedPoint = NULL;
	point_t *allPoints, *myPoints, *clusters, *incPoints = NULL, *previousIncPoints = NULL;
	char path[PATH_SIZE];

	initProcesses(&argc, &argv, &rank, &numberOfProcesses, path, &pathSize);
	if (rank == MASTER)
	{
		allPoints = readDataFile(path, pathSize, &N, &K, &T, &dT, &LIMIT, &QM);
		start = clock();
	}
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
			printf("Iteration: %d\t  q: %f\n%s\n", ++iterationNumber, q, newLine); fflush(stdout);
		}
		MPI_Bcast(&anotherIteration, 1, MPI_C_BOOL, MASTER, MPI_COMM_WORLD);//MASTER send if there is more iteration
		free(previousIncPoints);
	} while (anotherIteration);
	if (rank == MASTER)
	{//Write result to file
		end = clock();
		printf("Total work time %f sec\n%s\n", (double)(end - start) / CLOCKS_PER_SEC, newLine); fflush(stdout);
		writeToFile(path, pathSize, time, q, clusters, K);
		free(allPoints);
	}
	freeAllocations(2, myPoints, incPoints, clusters);
	MPI_Finalize();
	return EXIT_SUCCESS;
}