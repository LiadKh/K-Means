#include "Functions.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
	int rank, numprocs;
	int N, K, T,myNumberOfPoints;
	float dT, LIMIT, QM;
	float* allPoints, *myPoints;
	mpiInit(&argc, &argv, &rank, &numprocs);
	if (rank == MASTER)
	{
		allPoints = readDataFile(INPUT_FILE, &N, &K, &T, &dT, &LIMIT, &QM);
		myNumberOfPoints = int(N / numprocs);
	}
	sendPoints(allPoints, &myPoints, &myNumberOfPoints);
	getPoints(allPoints, myPoints, myNumberOfPoints);

	free(myPoints);
	if (rank == MASTER)
		free(allPoints);
	mpiFinish();
	return EXIT_SUCCESS;
}