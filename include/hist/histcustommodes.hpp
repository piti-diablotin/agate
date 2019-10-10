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

/** 
 * This class is designe to build a Hist object with either
 * user definied phonon modes and amplitude or with populating
 * the phonons using a DispDB and thermal factor (see Zacharias 2016)
 */
class HistCustomModes : public HistDataDtset{

  public :
    enum SeedType {None, Time, Random, User};
    enum InstableModes {Ignore, Absolute, Constant};

  private :
    Dtset &_reference;
    DispDB &_db;
    std::vector<DispDB::qptTree> _condensedModes;
    double _temperature;
    SeedType _seedType;
    unsigned _seed;
    double _instableAmplitude;


  protected :

  public :

    /**
     * Constructor.
     */
    HistCustomModes(Dtset& _dtset, DispDB& _db);

    /**
     * Destructor.
     */
    virtual ~HistCustomModes();

    double temperature() const;
    void setTemperature(double temperature);

    void buildHist(std::vector<DispDB::qptTree> inputCondensedModes=std::vector<DispDB::qptTree>());
    void animateModes(DispDB::qptTree &condensedModes, unsigned ntime);
    void zachariasAmplitudes(double temperature, unsigned ntime,vec3d supercell, InstableModes instable = Absolute);
    SeedType seedType() const;
    void setSeedType(const SeedType& seedType);
    unsigned seed() const;
    void setSeed(const unsigned& seed);
    void reserve(unsigned ntime, const Dtset& dtset);
    void insert(unsigned itime, const Dtset& dtset);
    void push(const Dtset& dtset);
    double instableAmplitude() const;
    void setInstableAmplitude(double instableAmplitude);
};

#endif  // HISTCUSTOMMODES_HPP
