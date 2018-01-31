/**
 * @file include/histdata.hpp
 *
 * @brief  Basic structure for a _HIST file
 * This is a pure virtual class. 
 * Not to be instanciated.
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


#ifndef HISTDATA_HPP
#define HISTDATA_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include <string>
#include <vector>
#include <list>
#ifdef HAVE_CPPTHREAD
#include <thread>
#include <mutex>
#include <atomic>
#endif
#include <iostream>
#include <memory>
#include <list>
#include <cmath>
#include "plot/graph.hpp"

/** 
 * Pure virtual class for HistData structure.
 */
class HistData {

  private :

  protected :

    unsigned _natom;    ///< Number of atom in the cell.
    unsigned _xyz;      ///< Number of dimension in space.
    unsigned _ntime;    ///< Number of time step.
    unsigned _nimage;
    bool     _isPeriodic; ///< Are the position all inside the cell or not

#ifdef HAVE_CPPTHREAD
    std::atomic<unsigned> _ntimeAvail;    ///< Number of time step.
#else
    unsigned _ntimeAvail;    ///< Number of time step.
#endif

    std::vector<double> _xcart;   ///< vectors (X) of atom positions in CARTesian coordinates
    std::vector<double> _xred;    ///< vectors (X) of atom positions in REDuced coordinates
    std::vector<double> _fcart;   ///< atom Forces in CARTesian coordinates Ha/bohr
    std::vector<double> _acell;   ///< CELL lattice vector scaling
    std::vector<double> _rprimd;  ///< Real space PRIMitive translations, Dimensional
    std::vector<double> _etotal;  ///< Total energy of the system
    std::vector<double> _time;    ///< Molecular Dynamics TIME hbar/Ha
    std::vector<double> _stress;  ///< 6 voigt value of the stress tensor for each time.
    std::vector<double> _spinat;  ///< spin of each atom. Dimension is (natom) (should only work for dtset

    std::vector<int> _typat; ///< Type of each atom.
    std::vector<int> _znucl; ///< znucl for each type of atom
#ifdef HAVE_SPGLIB
    double   _symprec; ///< Precision to find symmetries
#endif

#ifdef HAVE_CPPTHREAD
    mutable std::thread _thread;    ///< Thread for reading file and save user time
    std::atomic<bool>   _endThread; ///< Shall we abord the thread 
#endif

    std::string      _filename; ///< To display which histfile we are readind

    /**
     * Compute autocorrelation function of ntime*n data (n function(ntime))
     * using FFT
     * @param begin begin iterator of a vector of howmany functions arranged like r_01,r_02,r_0howmany,...,r_T1,r_T2,...r_Thowmany
     * @param end end iterator of a vector of howmany functions arranged like r_01,r_02,r_0howmany,...,r_T1,r_T2,...r_Thowmany
     * @param howmany number of functions.
     * @return The autocorrelation fuction acf_1t=0 acf_2t=0... acf_1t=1 acf_2t=1 ....acf_nt=T
     */
    static std::vector<double> acf(std::vector<double>::const_iterator begin, std::vector<double>::const_iterator end, const int howmany);

    void waitTime(unsigned t) const;

    /**
     * Check if the atomes in the hist are in the same order as in this one.
     * If not, it returns a vector of how to get the order of this in hist.
     * @param hist the new hist to reorder eventually.
     * @return A vector of indice to explore hist to get the same order as in this.
     */
    std::vector<unsigned> reorder(const HistData &hist) const ;

  public :


    /**
     * Constructor.
     */
    HistData();

    /**
     * Copy constructor.
     * @param hist Supposed to be copied
     */
    HistData(const HistData& hist);

    /**
     * Move constructor.
     * @param hist Supposed to be moved
     */
    HistData(HistData&& hist);

    /**
     * Destructor.
     */
    virtual ~HistData();

    /**
     * Assignement operator
     * @param hist Supposed to be copied
     * @return The supposed new hist
     */
    HistData& operator = (const HistData& hist);

    /**
     * Move operator
     * @param hist Supposed to be moved
     * @return The supposed new hist
     */
    HistData& operator = (HistData&& hist);

    /**
     * Open a file and read it to fill the data
     * @param filename Name of the file
     */
    virtual void readFromFile(const std::string& filename) = 0;

    /**
     * Get the number of atoms
     * @return the number of atoms
     */
    unsigned natom() const { return _natom; }

    /**
     * Get the total number of time step at the end of readFromFile
     * @return the number of time steps
     */
    unsigned ntime() const { return _ntime; }

    /**
     * Get the number of time steps that are available for processing during readFromFile
     * @return the number of time steps
     */
    unsigned ntimeAvail() const { return _ntimeAvail; }

    /**
     * Get the corresponding parameter at a given time.
     * @param time The step of the MD simulation
     * @param natom Number of atom read for the current time time
     * @return xcart
     */
    const double* getXcart(unsigned time, unsigned* natom = nullptr) const;

    /**
     * Get the corresponding parameter at a given time.
     * @param time The step of the MD simulation
     * @param natom Number of atom read for the current time time
     * @return xred
     */
    const double* getXred(unsigned time, unsigned* natom = nullptr) const;

    /**
     * Get the corresponding parameter at a given time.
     * @param time The step of the MD simulation
     * @param natom Number of atom read for the current time time
     * @return fcart 
     */
    const double* getFcart(unsigned time, unsigned* natom = nullptr) const;

    /**
     * Get the corresponding parameter at a given time.
     * @param time The step of the MD simulation
     * @return acell
     */
    const double* getAcell(unsigned time) const;

    /**
     * Get the corresponding parameter at a given time.
     * @param time The step of the MD simulation
     * @return rprimd
     */
    const double* getRprimd(unsigned time) const;

    /**
     * Get the corresponding parameter at a given time.
     * @param time The step of the MD simulation
     * @return the total energy of the system (electronic potential energy)
     */
    double getEtotal(unsigned time) const;

    /**
     * Get the 6 stress values (in Voigt notation)
     * @param time The step of the MD simulation
     * @return the pointer to the first value of stress tensor
     */
    const double* getStress(unsigned time) const;

    /**
     * Get the 3 composantes of spin for each atom
     * @param time The step of the MD simulation
     * @param natom Number of atom read
     * @return the pointer to the first atom spinat
     */
    const double*  getSpinat(unsigned time, unsigned* natom = nullptr) const;

    /**
     * Get the corresponding parameter at a given time.
     * @param time The step of the MD simulation
     * @return Time
     */
    double getTime(unsigned time) const;

    /**
     * Get the number of type of atoms.
     * @return number of different znucl;
     */
    unsigned getNtypat() const;

    /**
     * Compute distance between two atoms
     * IDs are in the hist object, not local
     * @param iatom1 First atom (starting point)
     * @param iatom2 Second atom (ending point)
     * @return the distance between iatom1 and iatom2
     */
    double getDistance(const unsigned iatom1, const unsigned iatom2, const unsigned itime) const;

    /**
     * Compute angle between 3 atoms
     * IDs are in the hist object, not local
     * The two vectors are iatom2-iatom1 and iatom3-iatom2
     * @param iatom1 First atom (starting point)
     * @param iatom2 Second atom (middle point)
     * @param iatom2 Third atom (ending point)
     * @return the angle
     */
    double getAngle(const unsigned iatom1, const unsigned iatom2, const unsigned iatom3, const unsigned itime) const;

    /**
     * Get the znucl of each type of atom
     * @return znucl value of each type of atom
     */
    const std::vector<int> znucl() const;

    /**
     * Get the type of each atom
     * @return type of each atom in the cell
     */
    const std::vector<int> typat() const;

    /**
     * Get the number of images.
     * @return number of images.
     */
    int nimage() const {return std::max((const unsigned)1,_nimage);}

    /**
     * Get the name of the hist file we are actually using
     * @return the path to the filename.
     */
    const std::string& filename() const {return _filename;}

    /**
     * Try to build an HistData with the file file
     * Can return a HistDataNC or other child class.
     * @param file Name of the file to read
     * @param wait Do you need to wait for the full file to be loaded in memory ? eg. for data analysis yes.
     * @return A pointer to the HistData or throw an Exception if failed.
     */
    static HistData* getHist(const std::string& file, bool wait = false);

    /**
     * Get the space group number
     * @param itime The step of the MD run
     * @param symprec Precision to find symetries (in bohr)
     * @param name The spacegroup name is set if variable is present.
     * @return The space group number
     */
    unsigned getSpgNum(const unsigned itime, double symprec, std::string *name = nullptr) const;

    /**
     * Get the space group name
     * @param itime The step of the MD run
     * @return A string with the name of the space group number
     */
    std::string getSpgHM(const unsigned itime) const;

    /**
     * Add an histdata if data are compatible.
     * The time is not taken from the argument but rather recomputed to continue
     * @param hist The HistData to add after the curren HistData
     * @return the new full Histdata
     */
    virtual HistData& operator+=(HistData& hist);

    /**
     * Calculates the radia pair distribution function between two type of atoms or all atoms.
     * @param znucl1 First type of atom to include in the calculation.
     * @param znucl2 Second type of atom to include in the calculation.
     * @param Rmax The maximum radius cutoff to compute the function.
     * @param dR The shell thickness to compute the function.
     * @param tbegin First time to use for calculation
     * @param tend last time to use for calculation
     * @return A pair of two vector. The first one is the radius (absisses) and the second is the pair distribution function corresponding to the radius (ordonnees).
     */
    virtual std::pair<std::vector<double>,std::vector<double>> getPDF(unsigned znucl1, unsigned znucl2, double Rmax, double dR, unsigned tbegin, unsigned tend) const;

    /**
     * Compute the Mean Square Displacement at time itime for a specific znucl.
     * @param tbegin first time to consider
     * @param tend last time not included
     * return <(x(t)-x0)^2>
     */
    virtual std::list<std::vector<double>> getMSD(unsigned tbegin,unsigned tend) const;

    /**
     * Compute the gyration radius tensor of each type of atoms
     * @param tbegin first time to consider
     * @param tend last time not included
     */
    virtual std::list<std::vector<double>> getGyration(unsigned tbegin,unsigned tend) const;

    /**
     * Compute the position autocorrelation function
     * @param tbegin First time to use to calculate mean values
     * @param tend Last time to use to calculate mean values
     */
    std::list<std::vector<double>> getPACF(unsigned tbegin, unsigned tend) const;

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
     * Recompute xred and xcart so that the periodic boundaries a imposed
     * or remove periodic boundaries
     * @param toPeriodic True to impose periodicity, False to remove periodicity
     */
    virtual void periodicBoundaries(unsigned itime, bool toPeriodic);

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
    virtual void dump(const std::string& filename, unsigned tbegin, unsigned tend, unsigned step = 1) const;

    /**
     * Compute the average of all quantities and build an HistDataDtset with it
     * Build is set as a dtset
     * the average is build between tbegin and tend
     * @param tbegin First time to consider
     * @param tend Last time (not included) to build the average
     * @return the equivalent Dtset of the average of the current hist file.
     */
    virtual HistData* average(unsigned tbegin, unsigned tend);

    /**
     * Compute the centroid of a hist if there are several images
     */
    virtual void centroid();

    /**
     * Change position of one atom at a given time step.
     * The new x, y, z are in reduce coordinated.
     * @param itime The time step 
     * @param iatom The atom indice
     * @param x New x position of the atom
     * @param y New y position of the atom
     * @param z Nez z position of the atom
     */
    void moveAtom(unsigned itime, unsigned iatom, double x, double y, double z);
    
    /**
     * Change the origin of the box
     * @param itime The time step 
     * @param x New x position of the origin
     * @param y New y position of the origin
     * @param z Nez z position of the origin
     */
    void shiftOrigin(unsigned itime, double x, double y, double z);

    /**
     * Check if tbegin and tend are compatible with the current set of data
     * 0<=tbegin<tend<=_ntime
     * @param tbegin first time 
     * @param tend last time
     */
    void checkTimes(unsigned tbegin, unsigned tend) const ;

    virtual bool hasThermo() const { return false; }

    virtual bool hasEtotal() const { return false; }

    virtual bool isPeriodic() const { return _isPeriodic; }

    /**
     * Try to build a HistData which integral of PACF is as small as possible
     * @param tbegin First time to use
     * @param tend Last time to use 
     * @param ntime The number of time step to include
     * @param T Initial temperature for the annealing
     * @param mu Factor for decreasing T. Should be 0 < mu < 1. The law is T=mu.T
     * @param step The number of steps to be done at fixed temperature
     *
     */
    void decorrelate(unsigned tbegin, unsigned tend, unsigned ntime, double T, double mu, unsigned step);
};

#endif  // HISTDATA_HPP
