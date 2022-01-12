# Computation on Distributed Systems using OpenMPI  

## About

https://curs.upb.ro/2021/pluginfile.php/439968/mod_resource/content/11/Tema%203%20-%20Enunt.pdf

Parallelize calculations using OpenMPI in a system with different clusters having a random number of workers.
This system is being administrated only by coordinator processes that guide the data through the clusters,
the workers can't communicate between them and can communicate only with their coordinator. This approach is more
efficient and modularized.

## How to run the tests

- run local bash script

## Structure & Flow

### For solving the tasks I used the following:

- openMPI library for communicating between processes and easing the use of parallelism.  

- tema3.cpp

    The implementation of this system is done in 3 parts and 2 phases:
    -   phases:
        -   all processes need to know the topology of the system
        -   workers are doing jobs received from the coordinators
    -   parts:
        -   coordinators
        -   process no. 0
        -   workers

    #### phase 1: all processes need to know the topology of the system
    
    -   coordinators:

    There are always 3 coordinators with the following ranks: 0, 1, 2. Each coordinators has a map called
    'topology' in which he is filling the system's topology dynamically by communicating with the others.
    Each coordinator reads from his workers, adds them to the map and then proceeds to send his entry to
    the other coordinators.
    Each coordinator receives the other ones entry and completes the topology so everyone can have a local
    topology completed.
    Now that the coordinators have completed their topologies, they proceed to send it to the workers so they
    will gain knowledge about the system.

    -   workers:

    Each one of them are receving the topology and completing it locally so it will be the same.

    **At this step in time, every process knows exactly the topology of the system.**

    #### phase 2: workers are doing jobs received from the coordinators

    - process no. 0:

    The first process is in charge of calculating the load balance for every worker and of sending it properly to their
    coordinators in order to distribute it further. So, the first process gets the N (number of jobs) and splits it equally
    according to the number of workers. If the split isn't done equally, then several dummy jobs are introduced and then
    dissmised in order to have the perfect balance for every worker. Then, calculates the load for every cluster, sends this
    load to every coordinator and also sends each ones jobs.
    He then proceeds to wait for the jobs to get finished so he can sort them out and print them to the terminal.

    - coordinators:

    The coordinators receive the load and the jobs from process no. 0. After that, they split the load in order to send them
    to their workers. Coordinators wait for the jobs to be finished from the workers side, then they send them back to
    process no. 0 to finalize them.

    - workers:

    The workers receive their load, they do their own jobs and send them back to their assigned coordinator.



## Project structure including tests
```bash
.
├── README.md
├── sol
│   ├── Makefile
│   ├── tema3
│   └── tema3.cpp
├── test.sh
└── tests
    ├── test1
    │   ├── allowed.txt
    │   ├── cluster0.txt
    │   ├── cluster1.txt
    │   ├── cluster2.txt
    │   ├── inputs.txt
    │   ├── output.txt
    │   └── topology.txt
    ├── test2
    │   ├── allowed.txt
    │   ├── cluster0.txt
    │   ├── cluster1.txt
    │   ├── cluster2.txt
    │   ├── inputs.txt
    │   ├── output.txt
    │   └── topology.txt
    ├── test3
    │   ├── allowed.txt
    │   ├── cluster0.txt
    │   ├── cluster1.txt
    │   ├── cluster2.txt
    │   ├── inputs.txt
    │   ├── output.txt
    │   └── topology.txt
    ├── test4
    │   ├── allowed.txt
    │   ├── cluster0.txt
    │   ├── cluster1.txt
    │   ├── cluster2.txt
    │   ├── inputs.txt
    │   ├── output.txt
    │   └── topology.txt
    └── testbonus
        ├── allowed.txt
        ├── cluster0.txt
        ├── cluster1.txt
        ├── cluster2.txt
        ├── inputs.txt
        ├── output.txt
        └── topology.txt
```
