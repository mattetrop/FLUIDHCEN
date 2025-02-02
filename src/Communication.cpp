#include <mpi.h>
#include <iostream>
#include "Communication.hpp"
#include <vector>
#include "Fields.hpp"
#include "Datastructures.hpp"

MPI_Comm MPI_COMMUNICATOR;

/*
TODO -->  each process should store here information about its rank and communicator,
as well as its neighbors. If a process does not have any neighbor in some direction, simply
use MPI_PROC_NULL as rank to skip the respective communication steps
*/

Communication::Communication(int argn, char **args){
    init_parallel(argn, args);
}

void Communication::init_parallel(int argn, char **args){
    MPI_Init(&argn, &args);


    // initialized to sequential execution
    int iproc{1};
    int jproc{1};

    if (argn > 2){
        iproc = *args[2] - 48;  //convert to ASCII
        jproc = *args[3] - 48;
    }

    int num_proc; // total number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    if (num_proc != iproc * jproc) {
        std::cerr << "Incompatible number of processors and domain decomposition!\n";
        MPI_Finalize();
        exit(1);
    }

    // Ask MPI to decompose our processes in a 2D cartesian grid for us
    int dims[2] = {iproc, jproc};
    MPI_Dims_create(num_proc, 2, dims);     // #processes - #dimensions of cartesian grid - #dimensions vector

    // both dimensions are not periodic
    int periods[2] = {false, false};

    // Let MPI assign arbitrary ranks if it deems it necessary
    int reorder = true;

    // Create a communicator given the 2D torus topology.
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &MPI_COMMUNICATOR);

    // My rank in the new communicator
    int my_rank;
    MPI_Comm_rank(MPI_COMMUNICATOR, &my_rank);
    my_rank_global = my_rank;

    // Get my coordinates in the new communicator
    int my_coords[2];
    MPI_Cart_coords(MPI_COMMUNICATOR, my_rank, 2, my_coords);
    my_coords_global[0] = my_coords[0];
    my_coords_global[1] = my_coords[1];

    MPI_Barrier(MPI_COMM_WORLD);

    if(my_rank_global == 0){
        std::cout << "\n(1/4) INITIALIZING PARALLEL COMMUNICATION...\n" << std::endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Print my location in the 2D torus.
    printf("[MPI process %d] I am located at (%d, %d).\n", my_rank, my_coords[0],my_coords[1]);

    MPI_Barrier(MPI_COMM_WORLD);
}

void Communication::finalize(){
    MPI_Finalize();
}

std::array<int, 4> Communication::get_neighbours() {
    std::array<int, 4> neighbours_ranks;

    // check if neighbours to communicate with
    MPI_Cart_shift(MPI_COMMUNICATOR, 0, 1, &neighbours_ranks[LEFT], &neighbours_ranks[RIGHT]);
    MPI_Cart_shift(MPI_COMMUNICATOR, 1, 1, &neighbours_ranks[DOWN], &neighbours_ranks[UP]);

    return neighbours_ranks;
}


void Communication::communicate(Matrix<double> &matrix){
    // Get my coordinates in the new communicator
    int my_coords[2];
    MPI_Cart_coords(MPI_COMMUNICATOR, my_rank_global, 2, my_coords);

    std::array<int,4> neighbours_ranks = get_neighbours();

    MPI_Status status;
    int inner_index_cols = matrix.num_cols() - 2;
    int inner_index_rows = matrix.num_rows() - 2;

    std::vector<double> send_x(matrix.num_rows(), 0);
    std::vector<double> rcv_x(matrix.num_rows(), 0);

    std::vector<double> send_y(matrix.num_cols(), 0);
    std::vector<double> rcv_y(matrix.num_cols(), 0);

    if(neighbours_ranks[RIGHT]!= MPI_PROC_NULL){

            for(int j=0; j< matrix.num_rows(); j++){
                send_x[j] = matrix(inner_index_cols,j);
            }

            MPI_Sendrecv(&send_x[0], send_x.size(), MPI_DOUBLE, neighbours_ranks[RIGHT], 0,
                         &rcv_x[0], rcv_x.size(), MPI_DOUBLE, neighbours_ranks[RIGHT], 0,  MPI_COMMUNICATOR, &status);

            for(int j=0; j< matrix.num_rows(); j++){
                matrix(inner_index_cols+1,j) = rcv_x[j];
            }
    }

    if(neighbours_ranks[LEFT]!= MPI_PROC_NULL){

            for(int j=0; j< matrix.num_rows(); j++){
                send_x[j] = matrix(1,j);
            }

            MPI_Sendrecv(&send_x[0], send_x.size(), MPI_DOUBLE, neighbours_ranks[LEFT], 0,
                         &rcv_x[0], rcv_x.size(), MPI_DOUBLE, neighbours_ranks[LEFT], 0,  MPI_COMMUNICATOR, &status);

            for(int j=0; j< matrix.num_rows(); j++){
                matrix(0,j) = rcv_x[j];
            }
    }

    if(neighbours_ranks[UP]!= MPI_PROC_NULL){

            for(int i=0; i< matrix.num_cols(); i++){
                send_y[i] = matrix(i,inner_index_rows);
            }

            MPI_Sendrecv(&send_y[0], send_y.size(), MPI_DOUBLE, neighbours_ranks[UP], 0,
                         &rcv_y[0], rcv_y.size(), MPI_DOUBLE, neighbours_ranks[UP], 0,  MPI_COMMUNICATOR, &status);

            for(int i=0; i< matrix.num_cols(); i++){
                matrix(i,inner_index_rows+1) = rcv_y[i];
            }
    }

    if(neighbours_ranks[DOWN]!= MPI_PROC_NULL){

            for(int i=0; i< matrix.num_cols(); i++){
                send_y[i] = matrix(i,1);
            }

            MPI_Sendrecv(&send_y[0], send_y.size(), MPI_DOUBLE, neighbours_ranks[DOWN], 0,
                         &rcv_y[0], rcv_y.size(), MPI_DOUBLE, neighbours_ranks[DOWN], 0,  MPI_COMMUNICATOR, &status);

            for(int i=0; i< matrix.num_cols(); i++){
                matrix(i,0) = rcv_y[i];
            }
    }
    
}


double Communication::reduce_min(double value){
    double global_min ;
    MPI_Allreduce(&value, &global_min, 1, MPI_DOUBLE, MPI_MIN, MPI_COMMUNICATOR);
    return global_min;
}


double Communication::reduce_sum(double residual){
    double globalsum ;
    MPI_Allreduce(&residual, &globalsum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMMUNICATOR);
    return globalsum;
}