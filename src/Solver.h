//
// Created by Charles Du on 11/10/22.
//

#ifndef ISO_TLC_SEA_SOLVER_H
#define ISO_TLC_SEA_SOLVER_H

#include <Eigen/Core>
#include "Energy_Formulation.h"

enum StopType {
    Unknown,
    Xtol_Reached,
    Ftol_Reached,
    Gtol_Reached,
    Max_Iter_Reached,
    Injectivity,
    Failure,
    Success
};

std::string get_stop_type_string(StopType t);

class Solver {
public:
    Solver() = default;

    virtual ~Solver() = default;

    // optimize energy f starting from x0
    virtual void optimize(Energy_Formulation* f, const Eigen::VectorXd& x0) = 0;

    void reset() {
        curr_iter = 0;
        stop_type = Unknown;
        curr_energy = std::numeric_limits<double>::infinity();
        curr_x.resize(0);
    }

    Eigen::VectorXd get_x() {
        return curr_x;
    }

    double get_energy() {
        return curr_energy;
    }

    StopType get_stop_type() {
        return stop_type;
    }

    size_t get_num_iter() {
        return curr_iter;
    }

    double xtol_abs = 1e-8;
    double xtol_rel = 1e-8;
    double ftol_abs = 1e-8;
    double ftol_rel = 1e-8;
    double gtol = 1e-8;
    size_t  maxIter = 10000;
    bool stop_at_injectivity = false;

protected:
    // check if the optimization is stagnant. If so, modify type according to the type of stagnation
    bool is_stagnant(double energy, double energy_next, double x_norm, double step_norm, StopType& type) const {
        //check ftol
        if (fabs(energy_next - energy) < ftol_abs) {
            type = Ftol_Reached;
            return true;
        }
        if (energy != 0 && fabs((energy_next - energy) / energy) < ftol_rel) {
            type = Ftol_Reached;
            return true;
        }
        // check xtol
        if (step_norm < xtol_abs) {
            type = Xtol_Reached;
            return true;
        }
        if (x_norm != 0 && step_norm / x_norm < xtol_rel) {
            type = Xtol_Reached;
            return true;
        }
        return false;
    }

    // current energy value
    double curr_energy = std::numeric_limits<double>::infinity();
    // current iteration number
    size_t  curr_iter = 0;
    // x at current iteration
    Eigen::VectorXd curr_x;
    // stop type for optimization
    StopType stop_type = Unknown;
};

#endif //ISO_TLC_SEA_SOLVER_H
