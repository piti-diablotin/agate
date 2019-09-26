/**
 * @file include/./electrondos.hpp
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


#ifndef ELECTRONDOS_HPP
#define ELECTRONDOS_HPP

#ifdef _WIN32
#include "base/win32.hpp"
#endif

#ifdef HAVE_CONFIG_H
#include "agate.h"
#undef HAVE_CONFIG_H
#endif

#include <string>
#include <fstream>
#include <vector>

/** 
 *
 */
class ElectronDos {

  private :
    unsigned _prtdos;
    unsigned _nsppol;
    unsigned _iatom;
    unsigned _nenergy;
    bool _prtdosm;
    bool _pawDecomposition;
    double _efermi;
    std::vector<double> _energies;
    std::vector<std::vector<double>> _dos;
    std::vector<std::vector<double>> _integrated;
    std::vector<std::vector<double>> _lm;

  protected :

  public :

    /**
     * Constructor.
     */
    ElectronDos();

    /**
     * Destructor.
     */
    virtual ~ElectronDos();

    void readFromFile(const std::string &filename);

    void readFromFile(std::istream &stream);

    bool isProjected() const;

    bool isMResolved() const;

    unsigned prtdos() const;

    unsigned atom() const;

    unsigned nsppol() const;

    double efermi() const;

    unsigned nenergy() const;

    enum PAWPart {PW=0,AE=1,PS=2};
    enum SOCProj {UU=0,UD=1,DU=2,DD=3,X=4,Y=5,Z=6};
    enum Angular {s=0,p=1,d=2,f=3,g=4};

    std::vector<double> dos(unsigned isppol,int tsmear=0) const; // prtdos1 prtdos2

    std::vector<double> dos(unsigned isppol, Angular angular) const; // prtdos 3

    std::vector<double> dos(unsigned isppol, Angular angular, int magnetic) const; // prtdos 3

    std::vector<double> dos(unsigned isppol, Angular angular, PAWPart part) const; // prtdos 3

    std::vector<double> dos(SOCProj proj) const; // prtdos 5

    std::vector<double> energies() const;
    bool pawDecomposition() const;
};

#endif  // ELECTRONDOS_HPP
