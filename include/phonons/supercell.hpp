/**
 * @file include/supercell.hpp
 *
 * @brief Creat a supercell from a dtset. Can move atoms according to eigen displacements.
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


#ifndef SUPERCELL_HPP
#define SUPERCELL_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "io/dtset.hpp"
#include "base/geometry.hpp"
#include "phonons/dispdb.hpp"

/** 
 * Creat a supercell based on a Dtset for agiven q-point
 * and then displace atom w/r to an eigen displacement
 * TODO : Finish the implementation
 */
class Supercell : public Dtset{

  private :

    std::vector<unsigned>        _baseAtom;  ///< Store the indice of the originated atoms before supercell
    geometry::vec3d              _dim;       ///< Dimension of the super with respect to the original cell
    std::vector<geometry::vec3d> _cellCoord; ///< Coordinates of the cell with respect to the original cell
    std::vector<std::complex<double>> _fft;

  protected :

  public :

    enum Norming { NONE, NORMQ, NORMALL };

    /**
     * Constructor.
     */
    Supercell();

    /**
     * Constructor.
     */
    Supercell(const Dtset& dtset, const geometry::vec3d& qpt);
    
    /**
     * Constructor.
     */
    Supercell(const Dtset& dtset, const unsigned nx, const unsigned ny, const unsigned nz);

    /**
     * Constructor that builds a Supercell from a _HIST file taken the itime structure.
     * It builds the Dtset only.
     * @param hist HistData structure that contains a MD simulation or just one shot.
     * @param itime Time to use from the hist to build the dtset
     */
    Supercell(const HistData& hist, const unsigned itime = 0);

    /**
     * Destructor.
     */
    virtual ~Supercell();

    /**
     * Make a displacement in the supercell for the given qpt (which may not be the one used to construct the supercell)
     * Use the db displacement database to find the qpt and the mode
     * Then move atoms with eventually an amplitude and a phase.
     * @param qpt the qpt of the phonon
     * @param db The displacement database with the eigen displacements
     * @param imode the mode to condence.
     * @param amplitude A prefactor to increase the displacement
     * @param phase A phase factor to make alive the phonon
     */
    void makeDisplacement(const geometry::vec3d qpt, DispDB& db, unsigned imode, double amplitude, double phase);

    /**
     * Make a displacement in the supercell for the given qpt (which may not be the one used to construct the supercell).
     * Use the db displacement database to find the qpt and the mode
     * Then show the displacement in the spinat variable.
     * @param qpt the qpt of the phonon
     * @param db The displacement database with the eigen displacements
     * @param imode the mode to condence.
     * @param amplitude A prefactor to increase the displacement
     */
    void arrowDisplacement(const geometry::vec3d qpt, DispDB& db, unsigned imode, double amplitude);

    /**
     * Set the reference structure to this Dtset.
     * We need to find the dimensions of the supercell with respect to the dtset and then find the corresponding indices 
     * of each atom in each cell with respect to the supercell
     * @param dtset The supposed reference structure
     */
    void findReference(const Dtset& dtset);

    /**
     * Set the reference structure to the one of supercell
     * @param supercell The supercell that already has a reference structure.
     */
    void setReference(const Supercell& supercell);

    /**
     * Extract the displacements of all atoms with respect to the dtset 
     * which should be the same as the reference dtset
     * Internally, we build the supercell reference structure (remove strain and stress)
     * Then we transform the reduced positions of the supercell in cartesian coordinates
     * in the supercell reference structure.
     * @param dtset The reference dtset
     * @result A vector of displacements atom1.x atom1.y atom1.z atom2.x ,...
     * The result is has the length dimension (bohr) and is equal to
     * rprim_ref_supercell*(xred_supercell-xred_ref_supercell)
     */
    std::vector<double> getDisplacement(const Dtset &dtset);

    /**
     * Project the displacement of the supercell on the eigen displacements of the reference structure.
     * @param dtset The reference structure
     * @param db the displacement data base of the dtset reference structure
     * @param modes All the modes to project on.
     * @param normalized If set to true, the projection \alpha_i^2 is normalized \sum _alpha_i^2 =1, otherwise the projection is A^2*\alpha_i^2
     * @param modulus Boolean to controle weather the modulus of the complex projection is stored (true) or the real part only (false)
     * @result A vector of amplitude for each couple of qpt,mode in modes
     */
    std::vector<double> projectOnModes(const Dtset& dtset, DispDB& db, const DispDB::qptTree& modes, Norming normalized, bool modulus);

    /**
     * This routine is aimed at filtering a displacement to extract the only qpt of interest
     * If FFTW3 is not installed then we don't do FFT and thus there is no filtering
     * @param qpt is the Q-point we want to keep in the displacement
     * @param disp is the dispalcement of all atoms and all directions with respect to the reference structure
     * @return the q-space displacement for this qpt.
     */
    std::vector<std::complex<double>> filterDisp(const geometry::vec3d& qpt, const std::vector<double>& disp);

    /**
     * Compute the amplitude of each qpt in the supercell
     * @param dtset The reference dtset only used to get the displacement
     * @param displacement The displacement of the supercell with respect to the reference Dtset dtset. If not
     * provided, it will be calculated.
     * @result a vector for all apt with the 3 qpt components and the corresponding amplitude (squared norm)
     */
    std::vector<std::array<double,4>> amplitudes(const Dtset& dtset, const std::vector<double>& displacement=std::vector<double>());

    /**
     * Perfor the FFT of the input real space displacement.
     * Store the result in _fft with the slowest indices the qpt indices and the fastest the atoms and dimension
     * This allows to access quickly a given qpt or to process data for each qpt.
     * @param dispr The displacement in real space
     */
    void fft(const std::vector<double>& dispr);

    /**
     * Get the id of the reference atom and the cell coordinates
     * @param iatom id of the atom in the supercell
     * @param refAtom will be the id of the atom in the reference
     * @param x will be the x coodinate of the cell in which iatom is
     * @param y will be the y coodinate of the cell in which iatom is
     * @param z will be the z coodinate of the cell in which iatom is
     */
    void getRefCoord(int iatom, int &refAtom, int &x, int &y, int &z) const;

    /**
     * @brief Get the dimension of the supercell compared to the reference cell
     * @return the diagonal part of the multiplicity matrice
     */
    geometry::vec3d getDim() const;
};

#endif  // SUPERCELL_HPP
