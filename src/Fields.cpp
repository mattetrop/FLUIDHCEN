
#include <algorithm>
#include <iostream>

#include "Communication.hpp"
#include "Fields.hpp"

Fields::Fields(double nu, double dt, double tau, int imax, int jmax, double UI, double VI, double PI)
    : _nu(nu), _dt(dt), _tau(tau) {
    _U = Matrix<double>(imax + 2, jmax + 2, UI);
    _V = Matrix<double>(imax + 2, jmax + 2, VI);
    _P = Matrix<double>(imax + 2, jmax + 2, PI);

    _F = Matrix<double>(imax + 2, jmax + 2, 0.0);
    _G = Matrix<double>(imax + 2, jmax + 2, 0.0);
    _RS = Matrix<double>(imax + 2, jmax + 2, 0.0);
}

void Fields::calculate_fluxes(Grid &grid) {
    for (int i = 1; i <= grid.size_x() - 1; i++) {
        for (int j = 1; j <= grid.size_y(); j++) {
            _F(i, j) = _U(i, j) +
                       _dt * (_nu * (Discretization::laplacian(_U, i, j)) - Discretization::convection_u(_U, _V, i, j));
        }
    }

    for (int i = 1; i <= grid.size_x(); i++) {
        for (int j = 1; j <= grid.size_y() - 1; j++) {
            _G(i, j) = _V(i, j) +
                       _dt * (_nu * (Discretization::laplacian(_V, i, j)) - Discretization::convection_v(_U, _V, i, j));
        }
    }
}

void Fields::calculate_rs(Grid &grid) {
    for (int i = 1; i <= grid.size_x(); i++) {
        for (int j = 1; j <= grid.size_y(); j++) {
            rs(i, j) = 1 / _dt * ((_F(i, j) - _F(i - 1, j)) / grid.dx() + (_G(i, j) - _G(i, j - 1)) / grid.dy());
        }
    }
}

void Fields::calculate_velocities(Grid &grid) {
    for (int i = 1; i <= grid.size_x() - 1; i++) {
        for (int j = 1; j <= grid.size_y(); j++) {
            _U(i, j) = _F(i, j) - _dt / grid.dx() * (_P(i + 1, j) - _P(i, j));
        }
    }

    for (int i = 1; i <= grid.size_x(); i++) {
        for (int j = 1; j <= grid.size_y() - 1; j++) {
            _V(i, j) = _G(i, j) - _dt / grid.dy() * (_P(i, j + 1) - _P(i, j));
        }
    }
}

double Fields::calculate_dt(Grid &grid) {
    double dx_2 = grid.dx() ^ 2;
    double dy_2 = grid.dy() ^ 2;
    double coefficient = (dx_2 * dy_2) / (dx_2 + dy_2);
    _dt = coefficient / (2 * _nu);

    return _dt;
}

double &Fields::p(int i, int j) { return _P(i, j); }
double &Fields::u(int i, int j) { return _U(i, j); }
double &Fields::v(int i, int j) { return _V(i, j); }
double &Fields::f(int i, int j) { return _F(i, j); }
double &Fields::g(int i, int j) { return _G(i, j); }
double &Fields::rs(int i, int j) { return _RS(i, j); }

Matrix<double> &Fields::p_matrix() { return _P; }

double Fields::dt() const { return _dt; }
