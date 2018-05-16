/**
 * @file hist2tdep.cpp
 *
 * @brief Construct infiles for TDEP from _HIST
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2014 Jordan Bieder
 *
 * This file is part of AbiOut
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#undef HAVE_CONFIG_H
#endif

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include "base/exception.hpp"
#include "io/parser.hpp"
#include "hist/histdatanc.hpp"
#include "base/phys.hpp"
#include "io/poscar.hpp"

/**
 * Main Program
 * @param argc Number of argument on the command line.
 * @param argv Array of strings for each argument.
 * @return 0 if succeeded, =/0 otherwise.
 */
int main(int argc, char** argv) {
  int rvalue = 0;     /// Value return by the main function.

  Parser parser(argc,argv);
  parser.setOption("input",'i',"","Input to read to construct the dataset");
  parser.setOption("version",'v',"Print the version number");
  parser.setOption("help",'h',"Print this message");

#ifdef _WIN32
  _set_output_format(_TWO_DIGIT_EXPONENT);
#endif

  try {
    parser.parse();
    if ( parser.getOption<bool>("version") ) throw EXCEPTION("",10);
    if ( parser.getOption<bool>("help") ) throw EXCEPTION("",0);

    if ( !parser.isSetOption("input") ) {
      throw EXCEPTION("You must specify an input file with -i filename or --input filename",ERRDIV);
    }

    HistDataNC hist;
    hist.readFromFile(parser.getOption<std::string>("input"));

    int natom = hist.natom();
    int ntime = hist.ntime();
    hist.waitTime(ntime);

    // Sort atoms once for all
    auto typat = hist.typat();
    std::vector<unsigned> atoms;
    for ( int it = 1 ; it <= (int)hist.znucl().size() ; ++it )
      for ( int t = 0 ; t < (int)typat.size() ; ++t ) 
        if ( typat[t] == it )
          atoms.push_back(t);
    Poscar ssposcar(*dynamic_cast<HistData*>(&hist));
    ssposcar.dump("SSPOSCAR");


    std::ofstream positions("infile.positions",std::ios::out);
    std::ofstream forces("infile.forces",std::ios::out);
    std::ofstream stat("infile.stat",std::ios::out);
    std::ofstream meta("infile.meta",std::ios::out);
    positions.setf(std::ios::scientific,std::ios::floatfield);
    positions.setf(std::ios::right,std::ios::adjustfield);
    forces.setf(std::ios::scientific,std::ios::floatfield);
    forces.setf(std::ios::right,std::ios::adjustfield);
    stat.setf(std::ios::fixed,std::ios::floatfield);
    stat.setf(std::ios::right,std::ios::adjustfield);
    stat.setf(std::ios::fixed,std::ios::floatfield);
    meta.setf(std::ios::left,std::ios::adjustfield);

    if ( !positions || !forces ) 
      throw EXCEPTION("Unable to open a file for writing",ERRDIV);

    for ( int itime = 0; itime < ntime; ++itime ) { 
      const double *xred = hist.getXred(itime);
      const double *fcart = hist.getFcart(itime);
      const double *stress = hist.getStress(itime);

      positions.precision(14);
      forces.precision(14);
      //stat.precision(14);

      for ( int iatom = 0; iatom < natom; ++iatom ) {
        positions << std::setw(23) << xred[atoms[iatom]*3] 
          << std::setw(23) << xred[atoms[iatom]*3+1]
          << std::setw(23) << xred[atoms[iatom]*3+2] << std::endl;
        forces    << std::setw(23) << fcart[atoms[iatom]*3]*phys::Ha2eV/phys::b2A
          << std::setw(23) << fcart[atoms[iatom]*3+1]*phys::Ha2eV/phys::b2A
          << std::setw(23) << fcart[atoms[iatom]*3+2]*phys::Ha2eV/phys::b2A << std::endl;
      }
      stat << std::setw(6) << itime
        << std::setprecision(5) << std::setw(12) << hist.getTime(itime) * phys::hbar/phys::Ha * 1e15
        << std::setprecision(6) << std::setw(15) << (hist.getEtotal(itime)+hist.getEkin(itime)) * phys::Ha2eV
        << std::setprecision(6) << std::setw(15) << hist.getEtotal(itime) * phys::Ha2eV
        << std::setprecision(6) << std::setw(12) << hist.getEkin(itime) * phys::Ha2eV
        << std::setprecision(2) << std::setw( 9) << hist.getTemperature(itime) 
        << std::setprecision(3) << std::setw( 9) << hist.getPressure(itime)
        << std::setprecision(3) << std::setw( 9) << stress[0] * phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21
        << std::setprecision(3) << std::setw( 9) << stress[1] * phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21
        << std::setprecision(3) << std::setw( 9) << stress[2] * phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21
        << std::setprecision(3) << std::setw( 9) << stress[5] * phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21
        << std::setprecision(3) << std::setw( 9) << stress[3] * phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21
        << std::setprecision(3) << std::setw( 9) << stress[4] * phys::Ha/(phys::b2A*phys::b2A*phys::b2A)*1e21
        << std::endl;
    }

    double temperature;
    std::stringstream thermo;
    hist.printThermo(0,ntime,thermo);
    std::string line;
    std::getline(thermo,line);
    std::getline(thermo,line);
    std::getline(thermo,line);
    std::getline(thermo,line);
    std::getline(thermo,line);
    thermo >> line >> line >> temperature;

    meta.precision(2);
    meta << std::setw( 9) << natom << "# natom" << std::endl;
    meta << std::setw( 9) << ntime << "# ntime" << std::endl;
    meta << std::setw( 9) << (hist.getTime(1)-hist.getTime(0))*phys::atu2fs << "# dtion [fs]" << std::endl;
    meta << std::setw( 9) << temperature << "# Temperature [K]" << std::endl;
    positions.close();
    forces.close();
    stat.close();
    meta.close();
  }
  catch ( Exception& e ) {
    rvalue = e.getReturnValue();
    if ( rvalue < 0 ) {
      std::cerr << e.fullWhat() << std::endl;
      if ( rvalue == Parser::ERARG || rvalue == Parser::EROPT ) {
        std::cerr << parser;
        rvalue = 0;
      }
    }
    else if ( rvalue == 10 ) { // ask for version number
      std::cout << PACKAGE_NAME << " version " << PACKAGE_VERSION << std::endl;
      rvalue = 0;
    }
    else {
      std::cout << parser;
    }
  }

  return rvalue;
}
