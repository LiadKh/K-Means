#include "Functions.h"
#include <stdlib.h>

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
	int rank, numprocs;
	int N, K, T, LIMIT, myNumberOfPoints;
	float dT, QM;
	point_t* allPoints, *myPoints;

	mpiInit(&argc, &argv, &rank, &numprocs);
	if (rank == MASTER)
	{
		allPoints = readDataFile(INPUT_FILE, &N, &K, &T, &dT, &LIMIT, &QM);
		myNumberOfPoints = int(N / numprocs);
	}
	commitMpiPointType();
	scatterPoints(allPoints, &myPoints, &myNumberOfPoints);
	work(myPoints, myNumberOfPoints);
	
	incDTpoint(myPoints, myNumberOfPoints, dT, &inicedMyPoints);

	gatherPoints(allPoints, inicedMyPoints, myNumberOfPoints);
	if (rank == MASTER) {
		printIt(allPoints, N);
	}
	free(myPoints);
	free(inicedMyPoints);
	if (rank == MASTER)
		free(allPoints);
	mpiFinish();
	return EXIT_SUCCESS;
}