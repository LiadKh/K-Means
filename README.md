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

### Installing

Open the program in Visual Studio 

Build the program in the same path on the machines 

In your main computer - open wmpiexec and enter computers ips and the build path with "input.txt" file. Don't forget to select the number of processes (Please set the number of computers you have inserted)

## Parallel and Distributed Computing

This project is a parallel implementation of K-Means to optimize the algorithm.
MPI sends data between the machines, OPM calculates by parallel work with CPU thread and CUDA calculates by parallel work with GPU.

## Author

* **Liad Khamd** - [LKH](https://github.com/LiadKhamd)

####  For more details please visit [Project Wiki](https://github.com/LiadKhamd/K-Means/wiki)
