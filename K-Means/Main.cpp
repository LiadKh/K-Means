#include "Functions.h"
#include <stdlib.h>

void printIt(float* points, int size)
{
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < DIMENSION; j++)
			printf("%3f ", points[i*DIMENSION + j]);
		printf("\n");
	}
}
	

int main(int argc, char* argv[])
{
	int rank, numprocs;
	int N, K, T, myNumberOfPoints;
	float dT, LIMIT, QM;
	float* allPoints, *myPoints;
	mpiInit(&argc, &argv, &rank, &numprocs);
	if (rank == MASTER)
	{
		allPoints = readDataFile(INPUT_FILE, &N, &K, &T, &dT, &LIMIT, &QM);
		myNumberOfPoints = int(N / numprocs);
	}
	sendPoints(allPoints, &myPoints, &myNumberOfPoints);

	float* inicedMyPoints;
	cudaInicDT(myPoints, myNumberOfPoints, dT, &inicedMyPoints);

	getPoints(allPoints, inicedMyPoints, myNumberOfPoints);
	printIt(allPoints, N);
	free(myPoints);
	free(inicedMyPoints);
	if (rank == MASTER)
		free(allPoints);
	mpiFinish();
	return EXIT_SUCCESS;
}