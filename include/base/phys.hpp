/**
 * @file phys.hpp
 *
 * @brief Store some physical constantes
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

#ifndef PHYS_HPP
#define PHYS_HPP
#include <cmath>
#include <vector>
#include <string>

namespace phys {

  /** Length */
  const double b2A = 0.52917720859e0;              ///< 1 bohr in angstrom
  const double A2b = 1e0/b2A; 			 ///< 1 angstrom in bohr
  const double A2m = 1e-10;			 ///< 1 angstrom in m 

  /** Time */
  const double Hz = 6.62606957e-34;              ///< 1 Hertz in J
  const double THz2Ha = 1.519829846e-04;         ///< 1 THz in Ha
  const double h = 6.6260695729e-34;             ///< Plank constant in J.s
  const double hbar = 1.05457172647e-34;         ///< Plank constant in J.s

  /** Energy */
  const double eV = 1.602176565e-19;             ///< 1 electron volt in J
  const double Ha = 4.35974434e-18;              ///< 1 Hatree in J
  const double Ry = (1.360569253e+01*eV);        ///< 1 Ry in J

  /** Other */
  const double Na = 6.0221412927e23;             ///< Avogadro
  const double kB = 1.3806488e-23;               ///< Boltzman J/K
  const double c = 2.99792458e+8;                ///< Light velocity in  m/s
  const double pi = 3.1415926535897932384626433832795; ///< pi ...

  /** Mass */
  const double amu = 1.660538921e-27;                        ///< u in Kg 
  const double emass =   9.10938215e-31;                     ///< emass in Kg 
  const double amu_emass = amu/emass;   ///< Atomic mass unit 

  /** Dielectric Constant */
  const double Eps_0 = 0.0055263494;

  /** Conversion **/
  const double Ha2THz = 6.579683920e+03;         ///< 1 Ha in THz;
  const double Ha2eV = Ha / eV;                  ///< 1 Ha in eV
  const double  K2eV = kB / eV;                  ///< 1 Kelvin in eV
  const double Ry2eV = Ry / eV;                  ///< 1 Ry in  eV
  const double Hz2eV = Hz / eV;                  ///< 1 Hz in eV
  const double  m2eV = Hz * c / eV;              ///<  1 m in eV
  const double atu2fs = hbar/Ha*1e15;            ///< 1 atomic unit of time 2 fs
  const double Ohmcm = 2e0*pi*Ha2THz*10e0/9e0;   ///< Conductivity unit conversion abinit -> Ohm.cm

  /** Factor needed in linear response calc **/ 
  
  const double fac = (eV/(A2m*A2m*amu*4.*pi*pi*1e024));

    /**
     * Unit of the eigen energies.
     */

  double BoseEinstein(double energy, double temperature);
}

namespace Units {
  enum Energy { eV, Ha, THz, pcm };
  Energy getEnergyUnit(std::string unit);
  double getFactor(Energy from, Energy to);
  std::string toString(Energy);
}

#endif   // PHYS_HPP
