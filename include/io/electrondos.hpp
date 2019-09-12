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
    int _prtdos;
    int _nsppol;
    int _iatom;
    int _nenergy;
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

    int prtdos() const;

    int iatom() const;

    int nsppol() const;

    double efermi() const;

    int nenergy() const;

    enum PAWPart {PW,AE,PS};
    enum SOCProj {UU,UD,DU,DD,X,Y,Z};

    const std::vector<double>& dos(unsigned isppol,int tsmear=0); // prtdos1 prtdos2

    const std::vector<double>& dos(unsigned isppol, unsigned  angular, int magnetic); // prtdos 3

    const std::vector<double>& dos(unsigned isppol, unsigned angular, PAWPart part); // prtdos 3

    const std::vector<double>& dos(unsigned isppol, SOCProj proj); // prtdos 5
};

#endif  // ELECTRONDOS_HPP
