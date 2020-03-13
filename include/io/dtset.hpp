/**
 * @file dtset.hpp
 *
 * @brief Manage information for Abinit dtset
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

#ifndef DTSET_HPP
#define DTSET_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif


#include <vector>
#include <string>
#include <sstream>
#include "base/exception.hpp"
#include "base/geometry.hpp"
#include "io/configparser.hpp"
#include "hist/histdata.hpp"

/**
 * Equivalent of the dataset_type in Abinit
 * Add some automatic functions to construct a simple dtset from an input file 
 * and write a dtset as an input file.
 */
class Dtset {

  private :

  protected :
    
    unsigned                        _natom;   ///< Number of atoms in the system.
    unsigned                        _ntypat;  ///< Number of different type of atoms in the system.
    std::vector<int>                _typat;   ///< Type of each atom.
    std::vector<int>                _znucl;   ///< Atom number of each type of atom.
    geometry::vec3d                 _acell;   ///< Scaling parameters for _rpimd_a=_rpim_a*acell_a
    geometry::mat3d                 _rprim;   ///< Primitive vector of the unit cell. (Scaled by acell).
    geometry::mat3d                 _gprim;   ///< Reciprocal space vector
    std::vector<geometry::vec3d>    _xcart;   ///< Cartesienne coordinates of each atom.
    std::vector<geometry::vec3d>    _xred;    ///< Reduced coordinates (in rprimd)
    std::vector<geometry::vec3d>    _spinat;  ///< Spin for each atom
    std::vector<geometry::vec3d>    _velocities; ///< Velocities for each atom
    std::string                     _findsym; ///< FINDSYM result.

    /**
     * Build rprim angles between vectors each vector is of size 1.0
     * @param angdeg The angles alpha beta ang gamma
     */
    void buildRprim(const double angdeg[3]);

  public :

    /**
     * Constructor that builds an empty default Dtset.
     */
    Dtset();

    /**
     * Constructor that builds a dtset from a _HIST file taken the itime structure.
     * @param hist HistData structure that contains a MD simulation or just one shot.
     * @param itime Time to use from the hist to build the dtset
     */
    Dtset(const HistData& hist, const unsigned itime = 0);

    /**
     * Destructor.
     */
    virtual ~Dtset();

    /**
     * Fill a Dtset from an input file.
     * @param filename The name of the input file to read.
     */
    virtual void readFromFile(const std::string& filename);

    /**
     * Fill a Dtset from an input file.
     * @param parser The ConfigParser to use to get the parameters
     */
    virtual void readConfig(ConfigParser& parser, unsigned img = 0, unsigned jdtset = 0);

    /**
     * Creat an input file with the current Dtset.
     * Dump the input file to the stream
     * @param out The ostream stream to write into.
     */
    virtual void dump(std::ostream& out) const;

    /**
     * Creat an input file with the current Dtset.
     * @param filename The name of the file to be written.
     */
    virtual void dump(const std::string& filename) const;

    /**
     * Creat a CIF file using FINDSYM utility
     * @param out Output stream to write the CIF file
     * @param tolerance Tolerance factor for findsym (in bohr)
     */
    virtual void cif(std::ostream& out, const double tolerance=0.0001);

    /**
     * Creat a CIF file using FINDSYM utility
     * @param filename Name of the file to write the cif file
     * @param tolerance Tolerance factor for findsym (in bohr)
     */
    virtual void cif(const std::string& filename, const double tolerance=0.0001);

    /**
     * Creat a Dtset from a CIF file (supposed to be created by findsym
     * @param cifFile The CIF file containing the information
     */
    virtual void setCif(const std::string& cifFile);

    /**
     * Regenerate the structure using findsym analysis
     * @param tolerance Tolerance factor for findsym (in bohr)
     * @param prtcif Print the cif file in stdout
     */
    virtual void reBuildStructure(const double tolerance, const bool prtcif=false);
    
    /**
     * Get the number of atoms.
     * @return _natom.
     */
    virtual unsigned natom() const { return _natom; }

    /**
     * Get ntypat
     * @return _ntypat.
     */
    virtual unsigned ntypat() const { return _ntypat; }

    /**
     * Get type of each atom.
     * @return _typat.
     */
    virtual const std::vector<int>& typat() const { return _typat; }

    /**
     * Get the znucl 
     * @return _znucl
     */
    virtual const std::vector<int>& znucl() const { return _znucl; }

    /**
     * Get _acell
     * @return _acell.
     */
    virtual const geometry::vec3d& acell() const { return _acell; }

    /**
     * Get the primitive vectors.
     * @return _prim.
     */
    virtual const geometry::mat3d& rprim() const { return _rprim; }

    /**
     * Get the reciprocal primitive vectors.
     * @return _gprim.
     */
    virtual const geometry::mat3d& gprim() const { return _gprim; }

    /**
     * Get the cartesian coordinates.
     * @return _xcart.
     */
    virtual const std::vector<geometry::vec3d>& xcart() const { return _xcart; }

    /**
     * Get the reduced coordinates.
     * @return _xred.
     */
    virtual const std::vector<geometry::vec3d>& xred() const { return _xred; }

    /**
     * Get the spin of each atom 
     * @return _spinat
     */
    virtual const std::vector<geometry::vec3d>& spinat() const { return _spinat; }

    /**
     * Get the velocity of each atom 
     * @return _velocities_
     */
    virtual const std::vector<geometry::vec3d>& velocities() const { return _velocities; }

    /**
     * Clear spin -> set to 0
     */
    virtual void clearSpinat();

    /**
     * Get the findsym result
     * @return _findsym.
     */
    virtual const std::string& findsym() const { return _findsym; }

    /**
     * Try to build the primitive cell of the current dtset
     */
    virtual void standardizeCell(const bool primitive, const double tolerance);

    /**
     * Get the symmetry of the current dtset;
     * Note that rotations and translations have to be used together at the same time
     * rotations[i]+translations[i] to get the correct operation.
     * @param rotations rotations operation
     * @param translations translations operation
     * @param symprec Precision for the symmetry finder
     */
    void getSymmetries(std::vector<geometry::mat3d> &rotations, std::vector<geometry::vec3d> &translations, double symprec) const;

    bool operator==(const Dtset& dtset1) const;
};

#endif //DTSET_HPP

