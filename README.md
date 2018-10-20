# K-Means
k-means clustering is a method of vector quantization, which aims to partition n observations into k clusters in which each observation belongs to the cluster with the nearest mean, serving as a prototype of the cluster. This results in a partitioning of the data space into Voronoi cells. - [Wikipedia](https://en.wikipedia.org/wiki/K-means_clustering)
### Prerequisites
Nvidia GPU with cuda support, then install: 

Visual Studio 2015 

MPI must be installed in the machines - C:\Program Files\MPICH2 OR D:\Program Files\MPICH2

CUDA 8

## Problem Definition
Given a set of points in 3-dimensional space. Initial position (xi, yi, zi) and velocity (vxi, vyi, vzi) are known for each point Pi. Its position at the given time t can be calculated as follows:

`xi(t) = xi + t*vxi`

`yi(t) = yi + t*vyi`

`zi(t) = zi + t*vzi`

Implement a simplified K-Means algorithm to find K clusters. Find the first occurrence during given time interval [0, T] when a system of K clusters has a Quality Measure q that is less than given value QM.


## Simplified K-Means algorithm
1.    Choose first K points as centers of clusters.
2.    Group points around the given cluster centers - for each point define a center that is most close to the point.
3.    Recalculate the cluster centers (average of all points in the cluster)
4.    Check the termination condition – no points move to other clusters or maximum iteration LIMIT was made.
5.    Repeat from 2 till the termination condition fulfills.
6.    Evaluate the Quality of the clusters found. Calculate the diameter of each cluster – maximum distance between any two points in this cluster. The Quality is equal to an average of ratio diameters of the cluster divided by distance to other clusters.

> For example, in case of k = 3 the quality is equal 
> `q = (d1/D12 + d1/D13 + d2/D21 + d2/D23 + d3/D31 + d3/D32) / 6`, 
> where di is a diameter of cluster i and Dij is a distance between centers of cluster i and cluster j.


## Input data and Output Result of the project
You will be supplied with the following data 
*     N - number of points
*     K - number of clusters to find
*     LIMIT – the maximum number of iterations for K-MEAN algorithm. 
*     QM – quality measure to stop
*     T – defines the end of time interval [0, T]
*     dT – defines moments t = n*dT, n = { 0, 1, 2, … , T/dT} for which calculate the clusters and the quality
*     Coordinates and Velocities of all points


## Input File format
The first line of the file contains   N    K    T   dT   LIMIT   QM.

Next lines are Initial Positions and Velocities of the points (xi, yi, zi, vxi, vyi, vzi)

```
For example:

50000    42    30    0.1    200.0       7.3

2.3      4.5      6.55     -2.3   13.3   1.1

76.2   -3.56    50.0        12    -0.7    22.3

 …

45.23   20      -167.1    98.0  99.2  -113.2 
```

## Output File format
The output file contains information on the found clusters at the moment when the Quality Measure QM is reached for the first time.
It must be in this format:
First occurrence t = value  with q = value
Centers of the clusters:

x1   y1   z1

x2   y2   z2

x3   y3   z3

x4   y`4   z4

```
For example, in case K = 4:

First occurrence t = 24.5  with q = 6.9

Centers of the clusters:

1.123     34        13.2

-5.3      17.01     90.4

33.56     -23       -1.3

14.1      98        14.9
```

## Parallel and Distributed Computing
This project is a parallel implementation of K-Means to optimize the algorithm.
MPI sends data between the machines, OPM calculates by parallel work with CPU thread and CUDA calculates by parallel work with GPU.

# Building the program 
1. Build the project on Visual Studio 2015 with CUDA 8 support.
2. Set the .exe file on the same path in every machine.
3. In your main computer - open wmpiexec.
4. Insert the path of the .exe file and the path of the folder which contain the input.txt
5. Enter computers ips
6. Select the number of processes. (Run the program with the number of processors = number of machines). 

# Algorithm
1. MASTER process reads the data from "input.txt" file.

### Init work
2. MASTER process broadcast k - **MPI**

3. MASTER process sends number of points to processes - **MPI**

## Iteration
4. MASTER process send dt - **MPI**

5. Each process finds the current position of each own points (dt) - **CUDA & OMP**

```
xi(t) = xi + t*vxi
yi(t) = yi + t*vyi
zi(t) = zi + t*vzi
```

6. Find the shortest cluster - **CUDA & OMP**

```
All the processes do:

  Parallel for i to number of points:
    Each thread do for i to k - Find the shortest distance between the k distance.
    The shortest point (k) set in the point.cluster (k id)

```

7. Recalculate the clusters centers - **MPI & OMP**

```
Each process do:

I. Parallel for i to number of points:
     Sum the x, y, z to thread array.
     Count the number of point by point.cluster as a counter to thread array.

II. Parallel for i to k:
     Each thread do for i to number of thread:
       Sum the x, y, z and point.cluster (as a counter of number in this cluster).

III. Send to MASTER process the sum of each cluster points array

MASTER process:

I. Parallel for i to k:
     Each thread do for i to number of processes:
       Sum the x, y, z and point.cluster

II. Parallel for i to k:
      Divided the points coordinates (x,y,z) by number of points (points.cluster) in the cluster - if no point in the                               
      cluster the old cluster will be taking
```

### Check condotion
8. Check if there is point that move to another cluster - **MPI & OMP**

```
Each process check if his point moved to another cluster
Sent to MASTER
MASTER check if there is or not the the answer from the processes
```

9. Calculate the quality of the result - **MPI & OMP**

```
Each process create K arrays - each array contained the point in the same cluster
Each process send the K arrays to process with rank -1
MASTER get the point in the each cluster from all process
Each process ask MASTER to work
MASTER sent the point in the same cluster or that isn't work
Process find and send the largest distance between two points of this cluster
MASTER calculate the q
```

10. MASTER check all the termination condition and sent to other if there is more iteration

### Project result
11. Write the finish time, quality measure of the result and the clusters to file - "output.txt"

## Author
* **Liad Khamd** - [LKH](https://github.com/LiadKhamd)

####  For more details please visit [Project Wiki](https://github.com/LiadKhamd/K-Means/wiki)
