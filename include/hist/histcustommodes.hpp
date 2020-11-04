/**
 * @file include/./histcustommodes.hpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@uliege.be>
 *
 * @copyright Copyright 2019 Jordan Bieder
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


#ifndef HISTCUSTOMMODES_HPP
#define HISTCUSTOMMODES_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "hist/histdatadtset.hpp"
#include "phonons/dispdb.hpp"
#include <functional>
#include <bitset>
#include <map>

/** 
 * This class is designe to build a Hist object with either
 * user definied phonon modes and amplitude or with populating
 * the phonons using a DispDB and thermal factor (see Zacharias 2016)
 */
class HistCustomModes : public HistDataDtset {

  public :
    /**
     * @brief The SeedType enums the seed to use for the random number generator
     */
    enum SeedType {None, Time, Random, User};
    /**
     * @brief The InstableModes enums how to treat the instable modes
     */
    enum InstableModes {Ignore, Absolute, Constant};
    /**
     * @brief Define what strain we know
     */
    enum StrainType {Iso=0,Tetra=1,Shear=2};
    /**
     * @brief The Strain enums which strain to apply
     */
    enum StrainDir {x, y, z, xy, xz, yz};

    /**
     * @brief Define the amplitude bounds for the strain;
     */
    enum StrainDistBound { IsoMin, IsoMax, TetraMin, TetraMax, ShearMin, ShearMax };


  private :
    Dtset &_reference;                             ///< Reference structure (contained in the DDB)
    DispDB &_db;                                   ///< All the eigendisplacements for all qpt available
    std::vector<DispDB::qptTree> _condensedModes;  ///< Vector of qpt and modes to be condensed in the builders (@see buildHist @see addNoise)
    SeedType _seedType;                            ///< Type of seed to use when initializing the RNG
    unsigned _seed;                                ///< The seed if _seedType==User
    double _instableAmplitude;                     ///< The amplitude of the instable modes if treated with InstableModes::Constant @see zachariasAmplitudes
    std::bitset<3>          _strainTypes;          ///< The type of strain
    std::vector<StrainDir>  _strainTetraDir;       ///< The direction(s) of the tetra strain(s)
    std::vector<StrainDir>  _strainShearDir;       ///< The direction(s) of the shear strain(s)
    std::vector<geometry::mat3d> _strainDist;      ///< The amplitude of the strains

  protected :

  public :

    /**
     * Constructor.
     * @param dtset will be the reference structure
     * @param db is the database for the displacements
     */
    HistCustomModes(Dtset& dtset, DispDB& db);

    /**
     * Destructor.
     */
    virtual ~HistCustomModes();

    /**
     * @brief buildHist Build an Hist using the reference structure and the phonons to be condensed contains in inputCondensedModes
     * @param inputCondensedModes are the modes that will be condensed. The size of the vector is the number of generated structures.
     */
    void buildHist(const std::vector<DispDB::qptTree>& inputCondensedModes=std::vector<DispDB::qptTree>(), const std::vector<geometry::mat3d>& inputStrainMatrix=std::vector<geometry::mat3d>());

    /**
     * @brief addNoiseToHist to an already existing hist
     * @warning xred and xcart are modified but not the forces, velocities or any other thermodynamical values.
     * @param hist The trajectory to add noise to.
     * @param temperature The temperature in K to use to populate the phonons.
     * @param instableModes How to treat the instable modes
     * @param callback A function that will be executed when the Hist is fully built.
     */
    void addNoiseToHist(const HistData &hist, double temperature, const std::map<StrainDistBound,double>& strainBounds, InstableModes instableModes,std::function<void()> callback);


    /**
     * @brief animateModes Will animate a few phonons modes with respect to the reference structure.
     * @param condensedModes The modes to be condensed.
     * @param ntime The number of time to animate the phonons. It will really animate the phonons.
     */
    void animateModes(DispDB::qptTree &condensedModes, unsigned ntime);

    /**
     * @brief zachariasAmplitudes build the amplitudes to condensed the phonons at a given temperature
     * using the Bose Einstein distribution
     * @see Phys. Rev. B 94, 075125
     * @param temperature the temperature in K
     * @param ntime Number of random amplitudes to generate
     * @param supercell Size of the supercell
     * @param instable The way to treat the instable modes @see InstableModes
     */
    void zachariasAmplitudes(double temperature, unsigned ntime,vec3d supercell, InstableModes instable = Absolute);

    /**
     * Build the strain amplitudes to apply at a hist time step
     * @param temperature the temperature in K
     * @param ntime Number of random amplitudes to generate
     * @param supercell Size of the supercell
     */
    void strainDist(const std::map<StrainDistBound,double>& distBounds, unsigned ntime);

    /**
     * @brief Construct the total strain matrix to apply to rprim
     * @param Amplitudes of the different strains strainTot the matrix to construct
     */

    geometry::mat3d getStrainMatrix(const std::array<double,3>& amplitudes);

    /**
     * Set de possible directions to apply tetra strain 
     * @param x allow tetra strain along x
     * @param y allow tetra strain along y
     * @param z allow tetra strain along z
     */
    void setStrainTetraDir(bool x, bool y, bool z);

    /**
     * Set de possible directions to apply shear strain 
     * @param xy allow shear strain along xy
     * @param xz allow shear strain along xz
     * @param yz allow shear strain along yz
     */
    void setStrainShearDir(bool xy, bool xz, bool yz);

    /**
     * Apply a random rotation matrix on strainMatrix
     * @param strainMatrix The strain matrix to be randomly rotated according to strainDir
     * @param type The type of strain to be rotated can only be Tetra or Shear
     * @see setStrainDir
     */

    void rotateStrain(geometry::mat3d &strainMatrix, const StrainType type);

    /**
     * @brief seedType getter
     * @return the seedType used
     */
    SeedType seedType() const;

    /**
     * @brief setSeedType Setter for the seedType
     * @param seedType The new type to used
     */
    void setSeedType(const SeedType& seedType);

    /**
     * @brief seed The currently used seed
     * @return the seed number
     */
    unsigned seed() const;

    /**
     * @brief setSeed Setter for the seed
     * @param seed The new seed to use
     */
    void setSeed(const unsigned& seed);

    /**
     * @brief reserve This function allocate all the needed memory to build an hist
     * of ntime.
     * @param ntime Number of time step to reserve
     * @param dtset Use to know the dimensions of the structure to be used (natom mainly)
     */
    void reserve(unsigned ntime, const Dtset& dtset);

    /**
     * @brief insert Insert the dtset in the Hist at the itime position to build the Hist
     * @param itime Time at with the dtset will be inserted
     * @param dtset The dtset to copy into the hist
     */
    void insert(unsigned itime, const Dtset& dtset);

    /**
     * @brief push Will push the dtset to the end of the current hist.
     * The memory must be available.
     * Basically it inserts the dtset at _ntimeAvail position over the _ntime dtset available
     * @param dtset The dtset to insert
     */
    void push(const Dtset& dtset);

    /**
     * @brief instableAmplitude Getter for the amplitude used for the instable modes
     * @return  the amplitude in A
     */
    double instableAmplitude() const;

    /**
     * @brief setInstableAmplitude Set the amplitude to be used if the instable modes ared condensed with a fixed amplitude.
     * @param instableAmplitude The amplitude in A.
     */
    void setInstableAmplitude(double instableAmplitude);
};

#endif  // HISTCUSTOMMODES_HPP
