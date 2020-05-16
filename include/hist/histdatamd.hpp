/**
 * @file include/histdatamd.hpp
 *
 * @brief Abastract class for HIST with some new variables
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2017 Jordan Bieder
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


#ifndef HISTDATAMD_HPP
#define HISTDATAMD_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include "hist/histdata.hpp"
#include <ostream>

/** 
 * Handle _HIST file in NetCDF. 
 * Return an error when opening a NC file is not compiled with NetCDF support
 */
class HistDataMD : public HistData {

  private :

  protected :

    double _mdtemp[2];                ///< Temperature for MD simulation. 0 is for thermalization et 1 the real thermostat
    std::vector<double> _ekin;        ///< Kinetic energy 
    std::vector<double> _velocities;  ///< Atom velocities
    std::vector<double> _temperature; ///< Ionic temperature
    std::vector<double> _pressure;    ///< Pressure computed with velocities
    std::vector<double> _entropy;     ///< Electronic entropy

    /**
     * Compute velocities by finite differencies when the coordinates at itime have been read.
     * It computes the forward difference for time step 0, backwards difference for _ntime-1
     * and centered difference for any other time step
     * As a result velocities at time step itime-1 is computed when itime is provided (the new data read)
     * Then it calls computePressureTemperataure for the correct time
     * @param itime Time at which new data is available
     * @param dtion The time step
     */
    void computeVelocitiesPressureTemperature(unsigned itime, double dtion);

    /**
     * Compute Pressure and Temperature based on kinetic energy and stresses
     * @param itime Time at which to do the calculation
     */
    void computePressureTemperature(unsigned itime);

    /**
     * Compute the free energy, the internal energy, the heat capacity and the entropy using the PDOS.
     * This compute the function at a unique temperature computed as the average temperature between
     * tbegin and tend
     * @param tbegin first time to use
     * @param tend last time to use.
     * @param omegaMax The largest frequency. Default value -1 means all spectrum.
     * @return the values of F,E,Cv,S in eV eV kB et kB
     */
    std::array<double,4> computeThermoFunctionHA(unsigned tbegin, unsigned tend, const double omegaMax = -1) const;

    /**
     * Compute the free energy, the internal energy, the heat capacity and the entropy using the PDOS.
     * This compute the function at a unique temperature computed as the average temperature between
     * tbegin and tend
     * @param pdos The precomputed PDOS.
     * @param omegaMax The largest frequency. Default value -1 means all spectrum.
     * @return the values of F,E,Cv,S in eV eV kB et kB
     */
    std::array<double,4> computeThermoFunctionHA(std::vector<double> &pdos, const double T, const double omegaMax = -1) const ;

  public :

    /**
     * Constructor.
     */
    HistDataMD();

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataMD(const HistData& hist);

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataMD(HistData&& hist);

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistDataMD(const HistDataMD& hist);

    /**
     * Move constructor.
     * @param hist Supposed to be moved
     */
    HistDataMD(HistDataMD&& hist);

    /**
     * Destructor.
     */
    virtual ~HistDataMD();

    /**
     * Assignement operator
     * @param hist Supposed to be copied
     * @return The supposed new hist
     */
    HistDataMD& operator = (const HistData& hist);

    /**
     * Assignement operator
     * @param hist Supposed to be copied
     * @return The supposed new hist
     */
    HistDataMD& operator = (HistData&& hist);

    /**
     * Assignement operator
     * @param hist Supposed to be copied
     * @return The supposed new hist
     */
    HistDataMD& operator = (const HistDataMD& hist);

    /**
     * Move operator
     * @param hist Supposed to be moved
     * @return The supposed new hist
     */
    HistDataMD& operator = (HistDataMD&& hist);

    /**
     * Open a file and read it to fill the data
     * @param filename Name of the file
     */
    virtual void readFromFile(const std::string& filename) = 0;

    /**
     * Get the ionic kinetic energy at a given time
     * @param time The step of the MD simulation
     * @return Ionic kinetic energy
     */
    double getEkin(unsigned time) const;

    /**
     * Get the velocities for each atom at a given time
     * @param time The step of the MD simulation
     * @return electronic potential energy for a SCF cycle
     */
    const double* getVel(unsigned time) const;

    /**
     * Get the temperature at a given time
     * @param time The step of the MD simulation
     * @return temperature
     */
    double getTemperature(unsigned time) const;

    /**
     * Get the pressure at a given time
     * @param time The step of the MD simulation
     * @return pressure
     */
    double getPressure(unsigned time) const;

    /**
     * Add an histdata if data are compatible.
     * The time is not taken from the argument but rather recomputed to continue
     * @param hist The HistData to add after the curren HistData
     * @return the new full Histdata
     */
    virtual HistData& operator+=(HistData& hist);

    /**
     * Add an histdata if data are compatible.
     * The time is not taken from the argument but rather recomputed to continue
     * @param hist The HistData to add after the curren HistData
     * @return the new full Histdata
     */
    HistDataMD& operator+=(HistDataMD& hist) ;

    /**
     * Print basic mean information about a trajectory
     * Total energy
     * Volume
     * Temperature
     * Pressure
     * Stresses
     * @param tbegin First time to use to calculate mean values
     * @param tend Last time to use to calculate mean values
     * @param out The ostream to print data
     */
    virtual void printThermo(unsigned tbegin, unsigned tend, std::ostream &out = std::cout);

    /**
     * Driver to plot something 
     * @param tbegin First time to use to plot data
     * @param tend Last time to use to plot data
     * @param stream The stream containing the command to parse
     * @param gplot Pointer  to a plotter
     * @param save What to do with the calculated data : plot ? save to file ? save raw data?
     */
    virtual void plot(unsigned tbegin, unsigned tend, std::istream &stream, Graph *gplot, Graph::GraphSave save);

    /**
     * Dump the full history into the current format
     * @param filename Name or base name of the file to creat.
     * @param tbegin first time to start with
     * @param tend Last time (not included) in the dumping
     */
    virtual void dump(const std::string& filename, unsigned tbegin, unsigned tend, unsigned step = 1) const = 0;

    virtual bool hasThermo() const { return true; }

    virtual bool hasEtotal() const { return true; }

    virtual bool hasStress() const { return true; }

    /**
     * Compute the velocity autocorrelation function
     * @param tbegin First time to use to calculate mean values
     * @param tend Last time to use to calculate mean values
     */
    std::list<std::vector<double>> getVACF(unsigned tbegin, unsigned tend) const;

    /**
     * Compute the phonon density of state using the fourier transform of VACF
     * @param tbegin First time to use to calculate mean values
     * @param tend Last time to use to calculate mean values
     * @param smearing Smearing to fit the PDOS with gaussians
     */
    std::list<std::vector<double>> getPDOS(unsigned tbegin, unsigned tend, double smearing) const;

    /**
     * @brief Linear interpolation of the structures contained in the hist
     * @param ninter The number of structures to construct between to already present
     * structures in the HIST
     */
    virtual void interpolate(unsigned ninter, double amplitude);
};

#endif  // HISTDATAMD_HPP
