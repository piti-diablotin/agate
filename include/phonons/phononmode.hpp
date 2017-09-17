/**
 * @file include/phononmode.hpp
 *
 * @brief Compute eigenvectors,eigendisplacements and energies of one qpt
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of AbiOut.
 *
 * AbiOut is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AbiOut is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AbiOut.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PHONONMODE_HPP
#define PHONONMODE_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#ifdef HAVE_EIGEN
#include "base/eigen.hpp"
#endif
#include <complex>
#include "base/geometry.hpp"
#include "io/ddb.hpp"

/** 
 * PhononMode calculate the modes for one qpt and takes as input the IFC in Q-space.
 */
class PhononMode {

  private :

    /** Define a nice type for complex */
    typedef std::complex<double> complex;

    unsigned            _natom;       ///< Number of atoms
    bool                _hasASR;      ///< Flag to know if we've computed the ASR correction _asr.
#ifdef HAVE_EIGEN
    Eigen::Vector3d     _qpt;         ///< The Qpt in reduced coord
    Eigen::MatrixXcd    _d2cart;      ///< Interatomic Force Constants for this Qpt
    Eigen::MatrixXcd    _eigenVec;    ///< Eigen vector \f$\gamma(q)\f$
    Eigen::MatrixXcd    _eigenDisp;   ///< Eigen displacement \f$\gamma(g)/\sqrt(M_\kappa\f$
    Eigen::VectorXd     _frequencies; ///< frequencies \f$\omega\f$, negative if imaginary.
    Eigen::Matrix3d     _gprim;       ///< Vector of reciprocal space
    Eigen::Matrix<complex,Eigen::Dynamic,3> _asr; ///< Acoustic Sum Rule Correction (lines=3*natom)
#endif
    std::vector<double> _mass;        ///< Masses of each atom

    /**
     * Initialize the dynamical matrix with the IFC from DDD
     * @param ddb The interatomic forces for this Q-pt
     */
    void computeForceCst(const std::vector<Ddb::d2der>& ddb);

    /**
     * Correct the dynamical matrix with the ASR
     */
    void applyASR();

  public :

    /**
     * Constructor.
     */
    PhononMode();

    /**
     * Constructor which allocate correct size
     */
    PhononMode(unsigned natom);

    /**
     * Destructor.
     */
    virtual ~PhononMode();

    /**
     * Set the attribute to the correct size
     */
    void resize(const unsigned natom);

    /**
     * Calculate the correction for the Acoustic Sum Rule which is the error breaking it:
     * \f$ \forall \kappa, \sum_{\alpha\kappa'\beta} C_{\kappa,\alpha\kappa'\beta}(q=0)\f$
     * @param ddb DDB contains with all needed information to extract gamma point information.
     */
    void computeASR(const Ddb& ddb);

    /**
     * Initialize the dynamical matrix with the IFC from DDD
     * @param qpt The Q-pt
     * @param ddb The full DDB
     */
    void computeForceCst(const geometry::vec3d& qpt, const Ddb& ddb);

    /**
     * Calculate the dynamical matrix for the current qpts and print the frequencies.
     */
    void computeEigen(double *freq = nullptr, complex *mode = nullptr);

    /**
     * Calculate the dynamical matrix for all qpts in the ddb and print the eigen freq and others eigen vectors.
     * @param ddb The full DDB
     */
    void computeAllEigen(const Ddb& ddb, double *freq = nullptr, complex *modes = nullptr);
};

#endif  // PHONONMODE_HPP
