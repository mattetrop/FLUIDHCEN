#include <cmath>

#include "Communication.hpp"
#include "ViscositySolver.hpp"


// Default constructor
// K_EPS_model::K_EPS_model() {}

void K_EPS_model::solve(Fields &field, Grid &grid) {
    
    double dt = field.dt();
    double nu = field.nu();

    for (auto currentCell : grid.fluid_cells()) {
        int i = currentCell->i();
        int j = currentCell->j();
        
        double k1 = Discretization::convection_KEPS(field.k_matrix(),field.u_matrix(),field.v_matrix(),i,j);
        double k2 = Discretization::laplacian_KEPS(field.k_matrix(), field.nuT_matrix(), nu, _sk, i ,j);
        double k3 = (nu + field.nuT(i,j)) * Discretization::strain_rate(field.u_matrix(), field.v_matrix(), i, j);
        double k4 = field.E(i, j);
        
        double e1 = Discretization::convection_KEPS(field.e_matrix(),field.u_matrix(),field.v_matrix(),i,j);
        double e2 = Discretization::laplacian_KEPS(field.e_matrix(), field.nuT_matrix(), nu, _se, i ,j);
        double e3 = _C1 * (field.E(i, j) * k3) / field.K(i,j); //field.damp1(i,j) * _C1 * (field.E(i, j) * k3) / field.K(i,j);
        double e4 = _C2 * std::pow(field.E(i, j),2) / field.K(i,j); //field.damp2(i,j) * _C2 * std::pow(field.E(i, j),2) / field.K(i,j);

        double k = field.K(i, j) + dt * (-k1 + k2 + k3 - k4);
        double e = field.E(i, j) + dt * (-e1 + e2 + e3 - e4);

        // bound
        k = std::max(k, 1e-4);
        e = std::max(e, 1e-4);

        field.K(i, j) = k;
        field.E(i, j) = e;

        double nuT = _C0 * (k*k)/e;
        // _nuT(i, j) = _dampmu(i,j) * C0 * (_K(i,j)*_K(i,j))/_E(i,j);
        field.nuT(i, j) = nuT;

        assert(nuT != nuT); // check for NaN
        assert(nuT > 0);
    }
}