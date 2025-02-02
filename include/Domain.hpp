#pragma once
#include "Enums.hpp"
#include <mpi.h>

/**
 * @brief Data structure that holds geometrical information
 * necessary for decomposition.
 *
 */
struct Domain {
    /// Minimum x index including ghost cells
    int iminb{-1};
    /// Maximum x index including ghost cells
    int imaxb{-1};

    /// Minimum y index including ghost cells
    int jminb{-1};
    /// Maximum y index including ghost cells
    int jmaxb{-1};

    /// Cell length
    double dx{-1.0};
    /// Cell height
    double dy{-1.0};

    /// Number of cells in x direction
    int size_x{-1};
    /// Number of cells in y direction
    int size_y{-1};

    /// Iteration variables for fields. Same size as size_x and size_y,
    /// except when domain is decomposed
    int itermax_x{-1};
    int itermax_y{-1};

    /// Number of cells in x direction, not-decomposed
    int domain_imax{-1};
    /// Number of cells in y direction, not-decomposed
    int domain_jmax{-1};
};
