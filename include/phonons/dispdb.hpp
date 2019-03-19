/**
 * @file include/dispdb.hpp
 *
 * @brief 
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


#ifndef DISPDB_HPP
#define DISPDB_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <vector>
#include <string>
#include <complex>
#include "base/geometry.hpp"
#include "io/ddb.hpp"
#include "io/eigparserphonons.hpp"

using geometry::vec3d;
using geometry::norm;


/** 
 *
 */
class DispDB {

  public :

    typedef std::complex<double> cplx;

  private :

  protected :

    unsigned _natom;                    ///< Number of atoms in the cell (input)
    unsigned _nqpt;                     ///< Number of qpt int the DB
    unsigned _nmode;                    ///< Number of modes per qpt in the DB
    std::vector<vec3d>::iterator _iqpt; ///< Activated qpt to find modes 

    std::vector<vec3d>  _qpts;          ///< List all the qpts coordinates
    std::vector<cplx>   _modes;         ///< List all the eigen displacements
    std::vector<cplx>   _linResE;       ///< Linear Response E displacements
    std::vector<double> _energies;      ///< List all the energies of each mode

  public :

    /**
     * Define a type for storing one mode
     */
    typedef struct qMode { 
      unsigned imode;
      double amplitude;
      double energy;
      bool operator==(const qMode& m1) const { return imode == m1.imode ;}
    } qMode;

    /**
     * Define a type for storing qpt, modes and energy
     */
    typedef std::map<geometry::vec3d,std::vector<qMode>> qptTree;

    /**
     * Constructor.
     */
    DispDB();

    /**
     * Constructor.
     */
    DispDB(unsigned natom);

    /**
     * Destructor.
     */
    virtual ~DispDB();

    /**
     * clear all arrays
     */
    void clear();

    /**
     * Read a anaddb output file (or log file?) to build the DB
     * @param filename Name of the file to read.
     * @param natom Number of atom corresponding to the simulation. If -1 then use internal value.
     * If -1 and internal _natom value is also -1 then throw exception.
     */
    void readFromFile(std::string filename, unsigned natom = -1);

    /**
     * Use a DDB to build the displacement DB
     * @param ddb the ddb from abinit
     */
    void computeFromDDB(Ddb &ddb);

    /**
     * Load displacement from an EigparserPhonons
     * @param eigparser the EigparserPhonons containing eigen vectors;
     */
    void loadFromEigParserPhonon(EigParserPhonons& eigparser);

    /**
     * Compute linear response to electric field 
     * The direction is given by x y z and the amplitude by A
     * @param Edir Direction of the electric field
     * @param A Amplitude of the E field
     * @param ddb Derivative data base 
     */
    void linearResponseE(std::vector<double> &Edir, double A, Ddb &ddb);

    /**
     * Ask if a qpts is present in the DDB.
     * @param qpt  coordinate of the qpt
     */
    bool hasQpt(const vec3d qpt) const;

    /**
     * Set Qpt to then acces the mode of this qpt
     * @param qpt coordinate of the qpt
     */
    void setQpt(const vec3d qpt);

    /**
     * Get Qpt to then acces the mode of this qpt later
     * @param qpt coordinate of the qpt
     * @result the distance from the first qpt ie "myqpt"-_qpts.begin()
     */
    unsigned getQpt(const vec3d qpt);

    /**
     * Get the number of atoms
     * @return the number of atoms
     */
    inline double natom() const { return _natom; }

    /**
     * Get all the displacements for a given mode of the activated qpt (_iqpt)
     * @param imode The ith mode of the _iqpt qpt
     * @return the starting point to acces de desired mode
     */
    std::vector<cplx>::const_iterator getMode(unsigned imode);

    /**
     * Get all the displacements for a given mode of the activated qpt (_iqpt)
     * @param dq is the ith qpt to acces (or internally the distance to _qpts.begin()
     * @param imode The ith mode of the _iqpt qpt
     * @return the starting point to acces de desired mode
     */
    std::vector<DispDB::cplx>::const_iterator getMode(unsigned dq, unsigned imode);

    /**
     * Get energy of a given mode
     * @param imode The ith mode of the _iqpt qpt
     * @return the energy of the desired mode
     */
    double getEnergyMode(unsigned imode);

    /**
     * Get energy of a given mode
     * @param dq is the ith qpt to acces (or internally the distance to _qpts.begin()
     * @param imode The ith mode of the _iqpt qpt
     * @return the energy of the desired mode
     */
    double getEnergyMode(unsigned dq, unsigned imode);

    /**
     * Concatene two objects;
     * @param disp The disp to add to current object
     */
    DispDB& operator += ( const DispDB& disp );
};

#endif  // DISPDB_HPP
