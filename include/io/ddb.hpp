/**
 * @file include/ddb.hpp
 *
 * @brief Generic class to store a DDB and then can be used to compute mode frequencies
 * and eigen displacements
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of Agate.
 *
 * Agate is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Agate is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Agate.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef DDB_HPP
#define DDB_HPP

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "io/dtset.hpp"
#include <complex>
#include <vector>
#include <map>
#include "base/geometry.hpp"

/** 
 * Simple class to read a DDB file generated by Abinit
 */
class Ddb : virtual public Dtset {

  public :

    /** Define a nice type for complex */
    typedef std::complex<double> complex;

    /** 
     * Define a type for 2nd derivative information
     * array is composed of idir1 ipert1 idr2 ipert2
     */
    typedef std::pair<std::array<unsigned,4>,complex> d2der;

  protected :

    bool                       _haveMasses; ///< True if phi/sqrt(m1*m2)
    unsigned                   _nqpt;   ///< Number of qpt/block in the DDB file
    std::map< geometry::vec3d, std::vector< d2der > >     _blocks; ///< Second derivative for each qpt
    std::vector<unsigned>      _zion;    ///< Ionic Charge of each atom. Different than _znucl because of Pseudopotentials.

    /**
     * Transform all cartesian (Ha/bohr-^2) second derivatives for phonons only to
     * reduced system
     */
    void blocks2Reduced();

    /**
     * Append in the d2der variable the static dielectric tensor given in reduced coordinates
     * @param epsinf is the static dielectric tensor in conventional units
     */
    void setEpsInf(const geometry::mat3d &epsinf);

    /**
     * Append in the d2der variable the born effectives charges in reduced coordinates
     * @param iatom the atom to add the BEC
     * @param zeff the BEC of the atom iatom in conventional units
     */
    void setZeff(const unsigned iatom, const geometry::mat3d &zeff);

    /**
     * Get the ddb for a given qpt to be modified
     * @return the corresponding ddb
     */
    std::vector<Ddb::d2der>& getD2der(const geometry::vec3d qpt);

  public :

    const static int ERFOUND = -1000;  ///< Error number if qpt not found in _blocks.

    /**
     * Constructor.
     */
    Ddb();

    /**
     * Destructor.
     */
    virtual ~Ddb();

    /**
     * Fill a DDB with the content of a file.
     * @param filename The name of the input file to read.
     */
    virtual void readFromFile(const std::string& filename) = 0;

    /**
     * Check flag to know if the data we have is normalized by the masses or not/
     * If it is normalized, then we already have phi/sqrt(m1*m2 (phonopy case))
     * It it is not normalize, then we just have phi (abinit case);
     * @return true if the masses are already included, false otherwise.
     */
    bool isNormalized() const {
      return _haveMasses;
    }

    /**
     * Return some information about the DDB file read
     */
    std::string info() const;

    /**
     * Get the ddb for a given qpt
     * @return the corresponding ddb
     */
    const std::vector<Ddb::d2der>& getDdb(const geometry::vec3d qpt) const;

    /**
     * Get all qpt in the ddb
     * @return the qpt in the ddb
     */
    const std::vector<geometry::vec3d> getQpts() const;
    
    /**
     * Try to build an DDB with the file file
     * Can return a DdbAbinit or DdbPhonopy
     * @param file Name of the file to read
     * @return A pointer to the DDB or throw an Exception if failed.
     */
    static Ddb* getDdb(const std::string& file);

    /**
     * Build the base Dtset from an other Dtset to get some structural information if needed
     * By default it just copies everything
     * @param dtset is the Dtset for the reference structure
     */
    virtual void buildFrom(const Dtset &dtset) {
      *dynamic_cast<Dtset*>(this) = dtset;
    }

    /**
     * Get the zion 
     * @return _zion
     */
    inline const std::vector<unsigned>& zion() const { return _zion; }

    using Dtset::dump; //Clang warning: indicate we want both dump functions

    virtual void dump(const geometry::vec3d qpt, std::string filename="");
    
    /**
     * Return the Born effective charge matrice of the iatom atom
     * @param iatom the indice of the atom to query the BEC
     * @return a matrice with the BEC of the queried atom in cartesian coordinates
     */
    virtual geometry::mat3d getZeff(const unsigned iatom) const;

    /**
     * Convert DDb to the static dielectric tensor.
     * @return the static dielectric tensor Epsilon infiny in cartesian coordinates.
     */
    virtual geometry::mat3d getEpsInf() const;

};

#endif  // DDB_HPP
