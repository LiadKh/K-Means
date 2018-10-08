# K-Means

k-means clustering is a method of vector quantization, which aims to partition n observations into k clusters in which each observation belongs to the cluster with the nearest mean, serving as a prototype of the cluster. This results in a partitioning of the data space into Voronoi cells. - Wikipedia

### Prerequisites

Nvidia GPU with cuda support, then install: 

Visual Studio 2015 

MPI must be installed in the machines - C:\Program Files\MPICH2 OR D:\Program Files\MPICH2

CUDA 8

## Getting Started

Input File format:
```
The first line of the file contains   N    K    T   dT   LIMIT   QM.
Next lines are Initial Positions and Velocities of the points (xi, yi, zi, vxi, vyi, vzi)
```
## Parallel and Distributed Computing

This project is a parallel implementation of K-Means to optimize the algorithm.
MPI sends data between the machines, OPM calculates by parallel work with CPU thread and CUDA calculates by parallel work with GPU.

# Building the program 
1. Build the project on Visual Studio 2015 with CUDA 8 support.
2. Set the .exe file on the same path in every machine.
1. In your main computer - open wmpiexec and enter computers ips and the build path with "input.txt" file. Don't forget to select the number of processes. Run the program with the number of processors = number of machines. 

# Algorithm
1. MASTER process reads the data from "input.txt" file.
### Init work
2. MASTER process broadcast k - **MPI**
3. MASTER process sends number of points to processes - **MPI**
## Iteration
4. MASTER process send dt - **MPI**
5. Each process finds the current position of each own points (dt) - **CUDA**
```
xi(t) = xi + t*vxi
yi(t) = yi + t*vyi
zi(t) = zi + t*vzi
```
6. Find the shortest cluster - **OMP** 
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
Each process check if point moved to another cluster
Sent to MASTER
MASTER check if there is or not the the answer from the processes
```
9. Calculate the quality of the result - **MPI & OMP**
```
Each process create K arrays - each array contained the point in the same cluster
MASTER get the point in the each cluster from all process
Each process ask MASTER to work
MASTER sent the point in the same cluster or that isn't work
Process find the largest distance between two points of this cluster
MASTER calculate the q
```
10. MASTER check all the termination condition and sent to other if there is more iteration
### Project result
11. Write the finish time, quality measure of the result and the clusters to file - "output.txt"

## Author

* **Liad Khamd** - [LKH](https://github.com/LiadKhamd)

####  For more details please visit [Project Wiki](https://github.com/LiadKhamd/K-Means/wiki)
