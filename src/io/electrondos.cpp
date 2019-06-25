/**
 * @file src/./electrondos.cpp
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


#include "io/electrondos.hpp"
#include "base/exception.hpp"

//
ElectronDos::ElectronDos() :
  _nsppol(1),
  _natom(0),
  _efermi(0.e0),
  _energies(),
  _dosTotal(),
  _dosProjected()
{
  ;
}

//
ElectronDos::~ElectronDos() {
  ;
}

void ElectronDos::readFromFile(const std::string &filename)
{
  std::ofstream file(filename, std::ios::in);
  if (file)
    this->readFromFile(file);
  else
    throw EXCEPTION("Unable to openf file "+filename,ERRDIV);
  file.close();
}

void ElectronDos::readFromFile(std::istream &stream)
{
  std::string tmp;
  // Read 3 first dummy lines
  std::getline(stream,tmp);
  std::getline(stream,tmp);
  std::getline(stream,tmp);

  //Read nsppol
  std::getline(stream,tmp,'=');
  stream >> _nsppol;
  std::getline(stream,tmp);

  // Find fermi energy
  do {
    stream >> tmp;
  } while (tmp != "Fermi");
  stream >> tmp >> tmp; // energy :
  stream >> _efermi;

}

