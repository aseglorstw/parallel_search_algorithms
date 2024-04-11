# Parallel BFS and DFS with OpenMP in C++

This repository contains implementations of parallel Breadth-First Search (BFS) and Depth-First Search (DFS) algorithms using OpenMP in C++.

## BFS Implementation

The BFS algorithm is designed to traverse levels of the graph in parallel. Each thread is responsible for processing a portion of the current level, with each thread having its own local queue. Once a level is fully processed, each thread merges its local queue into the global queue and waits for other threads to finish.

## DFS Implementation

The DFS algorithm gradually increases the depth of exploration until the desired node is found. Initially, each child node triggers its own recursion. However, if the depth becomes significant, further parallelization is avoided.



