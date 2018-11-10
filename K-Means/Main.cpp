#include "Functions.h"

int main(int argc, char* argv[])
{
#ifdef DEBUG_MOOD
	clock_t start = clock(), end;
#endif
	int rank, numberOfProcesses, N, K, LIMIT, myNumberOfPoints, pathSize;
	double DT, time, T, QM, q;
	point_t *clusters;
	point_velocity_t *allPoints, *myPoints;

	char path[PATH_SIZE];

	initProcesses(&argc, &argv, &rank, &numberOfProcesses, path, &pathSize);
	if (rank == MASTER)
		allPoints = readDataFile(path, pathSize, &N, &K, &T, &DT, &LIMIT, &QM);
	initWork(rank, numberOfProcesses, allPoints, N, &myPoints, &myNumberOfPoints, &clusters, &K);
	work(rank, numberOfProcesses, &time, &q, DT, myPoints, myNumberOfPoints, clusters, K, LIMIT, T, QM);
	if (rank == MASTER)
	{//Write result to file
#ifdef DEBUG_MOOD
		end = clock();
		printf("Total work time %f sec\n%s\n", (double)(end - start) / CLOCKS_PER_SEC, newLine); fflush(stdout);
#endif
		writeToFile(path, pathSize, time, q, clusters, K);
		free(allPoints);
	}
	freeAllocations(2, myPoints, clusters);
	MPI_Finalize();
	return EXIT_SUCCESS;
}