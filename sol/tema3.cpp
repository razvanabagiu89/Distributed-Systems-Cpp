#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <climits>
#include <cmath>
using namespace std;

#define NO_CLUSTERS 3 

int main (int argc, char *argv[]) {
    int no_processes;
    int rank;
    /* only process no. 0 sets and uses this flag in order to
    maintain the jobs equally as described in readme */
    int flag_divide;
    int total_workers;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &no_processes);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    MPI_Status status;
    map<int, list <int>> topology;

    // Phase 1

    // if it's a coordinator
    if (rank == 0 || rank == 1 || rank == 2) {
        
        int rank1 = INT_MAX;
        int rank2 = INT_MAX;
        // send my rank to the other coordinators and also receive their ranks
        for(int i = 0; i < NO_CLUSTERS; i++) {
            if (i != rank) {
                MPI_Send(&rank, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << i << ")\n";
                if (rank1 != INT_MAX) {
                    MPI_Recv(&rank2, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
                }
                else {
                    MPI_Recv(&rank1, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
                }
            }
        }

        string file = "cluster" + to_string(rank) + ".txt";
        ifstream ClusterFile(file);
        string output;

        getline (ClusterFile, output);
        int no_workers = stoi(output);
        list<int> aux_workers;

        // for the other two coordinators
        list<int> workers1, workers2;
        int no_workers1 = INT_MAX;
        int no_workers2 = INT_MAX;

        // read & parse from file
        for(int i = 0; i < no_workers; i++) {
            getline (ClusterFile, output);
            aux_workers.push_back(stoi(output));            
        }
        // close file
        ClusterFile.close(); 

        int aux;
        // send no_workers and my cluster to the other coordinators and also receive their's
        for(int i = 0; i < NO_CLUSTERS; i++) {
            if(i != rank) {

                // send no_workers in order for the coordinator to know how many workers to receive
                MPI_Send(&no_workers, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << i << ")\n";
                // receive no_workers from the other coordinators
                if(no_workers1 != INT_MAX) {
                    MPI_Recv(&no_workers2, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
                }
                else {
                    MPI_Recv(&no_workers1, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
                }
                
                for(list <int> :: iterator it = aux_workers.begin(); it != aux_workers.end(); ++it) {
                    // send every worker
                    aux = *it;
                    MPI_Send(&aux, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                    cout << "M(" << rank << "," << i << ")\n";
                }
            }
        }

        // receive every worker
        for(int i = 0; i < NO_CLUSTERS; i++) {
            if (i != rank) {
                if(workers1.empty()) {
                    for(int j = 0; j < no_workers1; j++) {
                        MPI_Recv(&aux, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
                        workers1.push_back(aux);
                    }
                }
                else {
                    for(int j = 0; j < no_workers2; j++) {
                        MPI_Recv(&aux, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
                        workers2.push_back(aux);
                    }
                }
            }
        }
        

        // insert them in the map
        topology.insert(std::pair<int, list<int>>(rank, aux_workers));
        topology.insert(std::pair<int, list<int>>(rank1, workers1));
        topology.insert(std::pair<int, list<int>>(rank2, workers2));

        // output topology
        cout << rank << " -> ";
        for (map<int, list<int>>::iterator itr = topology.begin(); itr != topology.end(); ++itr) {
            cout << itr->first << ":";
            for(list <int> :: iterator it = itr->second.begin(); it != itr->second.end(); ++it) {
                if(next(it) == itr->second.end()) {
                    cout << *it << " ";
                }
                else {
                cout << *it << ",";
                }
            }
        }
        cout << '\n';

        // the coordinators have the role to send the workers the full topology after they have it completed
        auto list_workers = topology.find(rank)->second;
        int worker_id;
        int aux_worker_id;
        for(list <int> :: iterator it = list_workers.begin(); it != list_workers.end(); ++it) {
            worker_id = *it;

            MPI_Send(&rank, 1, MPI_INT, worker_id, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";
            MPI_Send(&rank1, 1, MPI_INT, worker_id, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";
            MPI_Send(&rank2, 1, MPI_INT, worker_id, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";

            MPI_Send(&no_workers, 1, MPI_INT, worker_id, 2, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";
            MPI_Send(&no_workers1, 1, MPI_INT, worker_id, 2, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";
            MPI_Send(&no_workers2, 1, MPI_INT, worker_id, 2, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";

            // iterate every coordinator's list and send
            // first coordinator
            for(list <int> :: iterator it1 = aux_workers.begin(); it1 != aux_workers.end(); ++it1) {
                aux_worker_id = *it1;
                MPI_Send(&aux_worker_id, 1, MPI_INT, worker_id, 3, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << worker_id << ")\n";
            }

            // second coordinator
            for(list <int> :: iterator it2 = workers1.begin(); it2 != workers1.end(); ++it2) {
                aux_worker_id = *it2;
                MPI_Send(&aux_worker_id, 1, MPI_INT, worker_id, 3, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << worker_id << ")\n";
            }

            // third coordinator
            for(list <int> :: iterator it3 = workers2.begin(); it3 != workers2.end(); ++it3) {
                aux_worker_id = *it3;
                MPI_Send(&aux_worker_id, 1, MPI_INT, worker_id, 3, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << worker_id << ")\n";
            }
        }

        // Phase 2
        int N;

        // if it's process no. 0
        if(rank == 0) {
            
            flag_divide = 0;
            N = stoi(argv[1]);
            total_workers = no_workers + no_workers1 + no_workers2;
            if(N % total_workers != 0) {
               int aux_divide = N;
               // maintain the jobs equally as described in readme
               N = floor(N/total_workers) * (total_workers) + total_workers;
               flag_divide = N - aux_divide;
            }
            int load = N/total_workers;

            // computes how many jobs to send per cluster
            int load1, load2, load3;
            load1 = aux_workers.size() * load;
            load2 = workers1.size() * load;
            load3 = workers2.size() * load;

            // send every load to the coordinators
            // first coordinator - no output here because of same process communication: M(0, 0)
            MPI_Send(&load1, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            // second coordinator
            MPI_Send(&load2, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << 1 << ")\n";   
            // third coordinator   
            MPI_Send(&load3, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << 2 << ")\n";

            // send jobs

            // first coordinator - no output here because of same process communication: M(0, 0)
            for(int i = 0; i < load1; i++) {
                MPI_Send(&i, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            }

            // second coordinator
            for(int i = load1; i < load1 + load2; i++) {
                MPI_Send(&i, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << 1 << ")\n";
            }

            // third coordinator
            for(int i = load1 + load2; i < N; i++) {
                MPI_Send(&i, 1, MPI_INT, 2, 1, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << 2 << ")\n";
            }
        }

        // receives load from process no. 0 and initializes an array
        int load;
        MPI_Recv(&load, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        int result[load];

        // receives all jobs from its cluster
        for(int i = 0; i < load; i++) {
            MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            result[i] = aux;
        }

        // send data for workers to process
        int count = 0;
        int limit = load/aux_workers.size();
        int load_per_worker = limit;
        for(list <int> :: iterator it = aux_workers.begin(); it != aux_workers.end(); ++it) {
            int worker_id = *it;
            // send coordinator's rank in order to know who to send back
            MPI_Send(&rank, 1, MPI_INT, worker_id, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";

            // send load of work
            MPI_Send(&load_per_worker, 1, MPI_INT, worker_id, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << worker_id << ")\n";

            // send jobs
            for(int i = count; i < load; i++) {
                if(count == limit) {
                    count = limit;
                    limit += load/aux_workers.size();
                    break;
                }
                aux = result[i];
                MPI_Send(&aux, 1, MPI_INT, worker_id, 1, MPI_COMM_WORLD);
                cout << "M(" << rank << "," << worker_id << ")\n";
                count++;
            }
        }

        // wait for all jobs and send them to process no. 0
        for(int i = 0; i < load; i++) {
            MPI_Recv(&aux, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            MPI_Send(&aux, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
            if (rank != 0) { // no output here for process no. 0: M(0, 0)
                cout << "M(" << rank << "," << 0 << ")\n";
            }
        }

        // process no. 0: final computation
        if (rank == 0) {
            list <int> final_vector;
            int final_elem;
            // receive all jobs
            for(int i = 0; i < N; i++) {
                MPI_Recv(&final_elem, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
                final_vector.push_back(final_elem);
            }
            // sort and dismiss dummy jobs
            final_vector.sort();
            for (int i = 0; i < flag_divide; i++) {
                final_vector.pop_back();
            }
            // final output
            cout << "Rezultat: ";
            for(list <int> :: iterator it = final_vector.begin(); it != final_vector.end(); ++it) {
                cout << *it << ' ';
            }
            cout << '\n';
        }
    }
    else { 
        // receive topology and print it
        int rank1, rank2, rank3;
        MPI_Recv(&rank1, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&rank2, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&rank3, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

        int no_workers1, no_workers2, no_workers3;
        MPI_Recv(&no_workers1, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(&no_workers2, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
        MPI_Recv(&no_workers3, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);

        list <int> workers1, workers2, workers3;
        int worker_aux;
        for(int i = 0; i < no_workers1; i++) {
            MPI_Recv(&worker_aux, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status);
            workers1.push_back(worker_aux);
        }
        for(int i = 0; i < no_workers2; i++) {
            MPI_Recv(&worker_aux, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status);
            workers2.push_back(worker_aux);
        }
        for(int i = 0; i < no_workers3; i++) {
            MPI_Recv(&worker_aux, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status);
            workers3.push_back(worker_aux);
        }

        // complete the topology
        topology.insert(std::pair<int, list<int>>(rank1, workers1));
        topology.insert(std::pair<int, list<int>>(rank2, workers2));
        topology.insert(std::pair<int, list<int>>(rank3, workers3));

        // output
        cout << rank << " -> ";
        for (map<int, list<int>>::iterator itr = topology.begin(); itr != topology.end(); ++itr) {
            cout << itr->first << ":";
            for(list <int> :: iterator it = itr->second.begin(); it != itr->second.end(); ++it) {
                if(next(it) == itr->second.end()) {
                    cout << *it << " ";
                }
                else {
                cout << *it << ",";
                }
            }
        }
        cout << '\n';

        // Phase 2

        // receive coordinator's rank
        int coordinator;
        MPI_Recv(&coordinator, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);

        // receive load of work
        int load;
        MPI_Recv(&load, 1, MPI_INT, coordinator, 1, MPI_COMM_WORLD, &status);

        // receive jobs, work on jobs and send them back
        int aux;
        for(int i = 0; i < load; i++) {
            MPI_Recv(&aux, 1, MPI_INT, coordinator, 1, MPI_COMM_WORLD, &status);
            aux *= 2;
            MPI_Send(&aux, 1, MPI_INT, coordinator, 1, MPI_COMM_WORLD);
            cout << "M(" << rank << "," << coordinator << ")\n";
            aux = 0;
        }
    }
    MPI_Finalize();
}
