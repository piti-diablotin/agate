/**
 * @file src/ddbabinit.cpp
 *
 * @brief 
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


#include "io/ddbabinit.hpp"
#include <fstream>
#include <string>
#include <iomanip>
#include "base/utils.hpp"
#include "base/unitconverter.hpp"
#include "io/configparser.hpp"
#include "base/mendeleev.hpp"

using namespace Agate;

//
DdbAbinit::DdbAbinit() : Ddb() {
  _haveMasses = false;
}

//
DdbAbinit::~DdbAbinit() {
  ;
}

void DdbAbinit::readFromFile(const std::string& filename) {
  {
    // Start building structure information
    ConfigParser parser(filename);
    try {
      this->readConfig(parser);
      // znucl is given as double so readConfig fails to read znucl as unsigned.
      std::vector<double> znucl(parser.getToken<double>("znucl",_ntypat));
      _znucl.resize(_ntypat);
      for( unsigned z = 0 ; z < _ntypat ; ++z ) {
        _znucl[z] = static_cast<unsigned>(znucl[z]);
        if ( _znucl[z] > 118.0 || _znucl[z] == 0 ) { //max znucl allowed
          std::string err_str = "Found value " + utils::to_string(_znucl[z]) 
            + " for token znucl\nwhich should be smaller than 118 ";
          throw EXCEPTION(err_str,ERRDIV);
        }
      }
    }
    catch( Exception& e ) {
      e.ADD(std::string("Failed to read header from DDB file ")+filename,ERRDIV);
      throw e;
    }
   // zion is given as double so readConfig fails to read zion as unsigned.
    std::vector<double> zion(parser.getToken<double>("zion",_ntypat));
    _zion.clear();
    for( auto z : zion )
      _zion.push_back(static_cast<unsigned>(z));
 	
    try{
      std::vector<double> amu(parser.getToken<double>("amu",_ntypat));
      for ( unsigned ityp = 0 ; ityp < _ntypat ; ++ityp ) {
        MendeTable.mass[_znucl[ityp]] = amu[ityp];
      }
    }
    catch ( Exception& e ) {
      e.ADD("This is not a DDB file",ERRABT);
      std::clog << e.fullWhat() << std::endl;
    }
  }

  std::ifstream idd(filename,std::ios::in);
  if ( !idd ) 
    throw EXCEPTION(std::string("Failed to open DDB file ")+filename,ERRDIV);

  // Look for beginning of DDB information : Number of blocks
  unsigned nblock;
  {
    std::string tmp_line;
    do {
      std::getline(idd,tmp_line);
      if ( idd.fail() || idd.eof() )
        throw EXCEPTION(std::string("Bad DDB format in file ")+filename+". Could not find \"**** Database of total energy derivatives ****\"",ERRDIV);
   } while ( tmp_line.find(" **** Database of total energy derivatives ****") == std::string::npos );

    // Next line is the number of block
    std::getline(idd,tmp_line,'=');
    if ( idd.fail() || idd.eof() )
      throw EXCEPTION(std::string("Bad DDB format in file ")+filename,ERRDIV);
    idd >> nblock;
    std::getline(idd,tmp_line); // Empty line
    if ( idd.fail() || idd.eof() )
      throw EXCEPTION(std::string("Bad DDB format in file ")+filename,ERRDIV);
  }

  std::clog << "About to read a maximum of " << nblock << " data blocks." << std::endl;

  try {
    _nqpt = 0;
    for ( unsigned iqpt = 0 ; iqpt < nblock ; ++iqpt ) {
      if ( this->readBlock(idd) ) ++_nqpt;
    }
  }
  catch (Exception& e) {
    e.ADD(std::string("Unable to correctly read the DDB file")+filename,ERRDIV);
    throw e;
  }
}


//
bool DdbAbinit::readBlock(std::ifstream& idd) {
  // We are just at the beginning of a block
  // First read the number of elements
  std::string line;
  unsigned nelts = 0;
  std::getline(idd, line, ':');

  idd >> nelts;
  if ( line.find("2nd derivative") == std::string::npos ) {
    for ( unsigned ign = 0 ; ign <= nelts ; ++ign ) {
      std::getline(idd, line);
    }
    return false;
  }

  // read qpt
  double qx, qy, qz, weight;
  idd >> line >> qx >> qy >> qz >> weight; 
  if ( idd.fail() || idd.eof() || idd.bad() )
    throw EXCEPTION(std::string("Cannot identify the Qpt"),ERRDIV);

  std::vector<d2der> block;
  for ( unsigned ielt = 0 ; ielt < nelts && !idd.eof() ; ++ielt ) {
    // Need to read string since cpp does not understand x.xxDyy
    unsigned idir1, ipert1, idir2, ipert2;
    std::string real, img;
    idd >> idir1 >> ipert1 >> idir2 >> ipert2 >> real >> img;
    block.push_back(
       std::make_pair(
         std::array<unsigned,4>{{ idir1-1, ipert1-1, idir2-1, ipert2-1 }} ,
         complex(utils::stod(real),utils::stod(img))
         )
       );
  }

  // Vector size is twice the real size ... I don't know why
  if ( block.size() != nelts ) 
    throw EXCEPTION("Not enought derivates to construct block",ERRDIV);
  if ( qx*qx+qy*qy+qz*qz < 1e-7 ) {
    std::clog << "Reading Gamma point. Setting imaginary part to 0." << std::endl;
    for ( unsigned ielt = 0 ; ielt < nelts && !idd.eof() ; ++ielt ) {
      block[ielt].second.imag(0.e0);
    }
  }
  _blocks.insert(std::make_pair(
          geometry::vec3d({{ qx, qy, qz }}),
          block));

  std::getline(idd, line);
  return true;
}

void DdbAbinit::header(const Ddb &ddb, std::ostream &out) {
  using namespace std;
  std::vector<geometry::mat3d> rotations;
  std::vector<geometry::vec3d> tnons;
  UnitConverter dunit(UnitConverter::A);
  dunit = UnitConverter::bohr;
  ddb.getSymmetries(rotations,tnons,1e-3*dunit);
  out.setf(ios::right,ios::adjustfield);
  out << endl << " **** DERIVATIVE DATABASE ****    " << endl;
  out << "+DDB, Version number" << setw(10) << 100401 << endl << endl;
  out << " DDB written by Agate " << PACKAGE_VERSION << " with no warranty" << endl << endl;
  out << " " << setw(9) << "usepaw" << setw(10) << 1 << endl;
  out << " " << setw(9) << "natom" << setw(10) << ddb.natom() << endl;
  out << " " << setw(9) << "nkpt" << setw(10) << 1 << endl;
  out << " " << setw(9) << "nsppol" << setw(10) << (ddb.spinat().size() > 0? 2: 1) << endl;
  out << " " << setw(9) << "nsym" << setw(10) << rotations.size() << endl;
  out << " " << setw(9) << "ntypat" << setw(10) << ddb.ntypat() << endl;
  out << " " << setw(9) << "occopt" << setw(10) << 1 << endl;
  out << " " << setw(9) << "nband" << setw(10) << 1 << endl;
  out.setf(std::ios::scientific,std::ios::floatfield);
  out.precision(14);
  out << " " << setw(9) << "acell" 
    << setw(22) << ddb.acell()[0] 
    << setw(22) << ddb.acell()[1] 
    << setw(22) << ddb.acell()[2] << endl;
  out << " " << setw(9) << "amu";
  auto znucl = ddb.znucl();
  for ( unsigned z = 0 ; z < znucl.size() ; ++z ) {
    out << setw((z==0?22:(z%3==0?32:22))) << static_cast<double>(MendeTable.mass[znucl[z]]);
    if ( (z+1)%3 == 0 ) out << endl;
  }
  if ( znucl.size()%3 != 0 ) out << endl;
  out << " " << setw(9) << "dilatmx" << setw(22) << 1.0 << endl;
  out << " " << setw(9) << "ecut" << setw(22) << 40. << endl;
  out << " " << setw(9) << "pawecutdg" << setw(22) << 80. << endl;
  out << " " << setw(9) << "ecutsm" << setw(22) << 0. << endl;
  out << " " << setw(9) << "intxc" << setw(10) << 0 << endl;
  out << " " << setw(9) << "iscf" << setw(10) << 17 << endl;
  out << " " << setw(9) << "ixc" << setw(10) << 11 << endl;
  out << " " << setw(9) << "kpt" 
    << setw(22) << 0.
    << setw(22) << 0.
    << setw(22) << 0. << endl;
  out << " " << setw(9) << "kptnrm" << setw(22) << 1.0 << endl;
  out << " " << setw(9) << "ngfft" 
    << setw(10) << 2
    << setw(5) << 2
    << setw(5) << 2 << endl;
  out << " " << setw(9) << "nspden" << setw(10) << (ddb.spinat().size() > 0? 2: 1) << endl;
  out << " " << setw(9) << "nspinor" << setw(10) << 1 << endl;
  out << " " << setw(9) << "occ" << setw(22) << 2. << endl;
  out << " " << setw(9) << "rprim" 
    << setw(22) << ddb.rprim()[0] << setw(22) << ddb.rprim()[3] << setw(22) << ddb.rprim()[6] << endl
    << setw(32) << ddb.rprim()[1] << setw(22) << ddb.rprim()[4] << setw(22) << ddb.rprim()[7] << endl
    << setw(32) << ddb.rprim()[2] << setw(22) << ddb.rprim()[5] << setw(22) << ddb.rprim()[8] << endl;
  out << " " << setw(11) << "dfpt_sciss" << setw(22) << 0. << endl;
  out << " " << setw(9) << "spinat";
  auto spinat = ddb.spinat();
  if ( spinat.size() == 0 ) spinat.resize(ddb.natom(),{0,0,0});
  for ( unsigned iatom = 0 ; iatom < spinat.size() ; ++iatom )
    out << setw((iatom==0?22:32)) << spinat[iatom][0] << setw(22) << spinat[iatom][1] << setw(22) << spinat[iatom][2] << endl;
  out << " " << setw(9) << "symafm";
  for ( unsigned isym = 0 ; isym < rotations.size() ; ++isym) {
    out << setw((isym==0?10:(isym%12==0?20:5))) << 1;
    if ( (isym+1)%12 == 0 ) out << endl;
  }
  if ( rotations.size()%12 != 0 ) out << endl;
  out << " " << setw(9) << "symrel";
  for ( unsigned isym = 0 ; isym < rotations.size() ; ++isym ) {
    out << setw(isym==0?5:15) << " ";
    for ( auto i : rotations[isym] )
      out << setw(5) << lrint(i);
    out << endl;
  }
  out << " " << setw(9) << "tnons";
  for ( unsigned isym = 0 ; isym < tnons.size() ; ++isym)
    out << setw((isym==0?22:32)) << tnons[isym][0] << setw(22) << tnons[isym][1] << setw(22) << tnons[isym][2] << endl;
  out << " " << setw(9) << "tolwfr" << setw(22) << 1e-20 << endl;
  out << " " << setw(9) << "tphysel" << setw(22) << 0. << endl;
  out << " " << setw(9) << "tsmear" << setw(22) << 1e-3 << endl;
  out << " " << setw(9) << "typat";
  auto typat = ddb.typat();
  for ( unsigned iatom = 0 ; iatom < typat.size() ; ++iatom ) {
    out << setw((iatom==0?10:(iatom%12==0?20:5))) << typat[iatom];
    if ( (iatom+1)%12 == 0 ) out << endl;
  }
  if ( typat.size()%12 != 0 ) out << endl;
  out << " " << setw(9) << "wtk" << setw(22) << 1. << endl;
  out << " " << setw(9) << "xred";
  auto xred = ddb.xred();
  for ( unsigned iatom = 0 ; iatom < xred.size() ; ++iatom ) {
    out << setw((iatom==0?22:32)) << xred[iatom][0] << setw(22) << xred[iatom][1] << setw(22) << xred[iatom][2] << endl;
  }
  out << " " << setw(9) << "znucl";
  for ( unsigned z = 0 ; z < znucl.size() ; ++z ) {
    out << setw((z==0?22:(z%3==0?32:22))) << static_cast<double>(znucl[z]);
    if ( (z+1)%3 == 0 ) out << endl;
  }
  if ( znucl.size()%3 != 0 ) out << endl;
  out << " " << setw(9) << "zion";
  auto zion = ddb.zion();
  for ( unsigned z = 0 ; z < zion.size() ; ++z ) {
    out << setw((z==0?22:(z%3==0?32:22))) << static_cast<double>(zion[z]);
    if ( (z+1)%3 == 0 ) out << endl;
  }
  if ( zion.size()%3 != 0 ) out << endl;
  out << endl;
  out << " No information on the potentials yet " << endl;
}


void DdbAbinit::dump(const Ddb &ddb, std::string filename) {
  using namespace std;
  using namespace geometry;

  ofstream out (filename);
  if ( !out ) throw EXCEPTION(string("Unable to create file ")+filename,ERRDIV);

  DdbAbinit::header(ddb,out);
  out.setf(std::ios::right,std::ios::adjustfield);

  auto qpts = ddb.getQpts();

  out << endl;
  out << " **** Database of total energy derivatives ****" << endl;
  out << " Number of data blocks=" << setw(4) << qpts.size() << endl;
  out << endl;

  for ( unsigned iblock = 0 ; iblock < qpts.size() ; ++iblock ) {
    auto &qpt = qpts[iblock];
    auto &d2 = ddb.getDdb(qpt);
    out.unsetf(std::ios::floatfield);
    out << " 2nd derivatives (non-stat.)  - # elements :" << setw(8) << d2.size() << endl;
    out << " qpt";
    out.precision(8);
    out.setf(std::ios::scientific,std::ios::floatfield);
    out << setw(16) << qpt[0];
    out << setw(16) << qpt[1];
    out << setw(16) << qpt[2];
    // float
    out.precision(1);
    out.setf(std::ios::fixed,std::ios::floatfield);
    out << setw(6) << 1.0 << endl;
    //for ( auto &elt : d2 ) {
    //  auto &coord = elt.first;
    //  out.unsetf(std::ios::floatfield);
    //  out << setw(4) << 1+coord[0];
    //  out << setw(4) << 1+coord[1];
    //  out << setw(4) << 1+coord[2];
    //  out << setw(4) << 1+coord[3];
    //  out.setf(std::ios::scientific,std::ios::floatfield);
    //  out.precision(14);
    //  out << setw(22) << elt.second.real() << " " ;
    //  out << setw(22) << elt.second.imag() << " " ;
    //  out << endl;
    //}
    //*
    for ( unsigned iatom2 = 0 ; iatom2 < ddb.natom()+2 ; ++iatom2 ) {
      for ( unsigned idir2 = 0 ; idir2 < 3 ; ++idir2 ) {
        for ( unsigned iatom1 = 0 ; iatom1 < ddb.natom()+2 ; ++iatom1 ) {
          for ( unsigned idir1 = 0 ; idir1 < 3 ; ++idir1 ) {
            for ( auto &elt : d2 ) {
              auto &coord = elt.first;
              if ( 
                  coord[0] == idir1 &&
                  coord[1] == iatom1 &&
                  coord[2] == idir2 &&
                  coord[3] == iatom2
                 ) {
                out.unsetf(std::ios::floatfield);
                out << setw(4) << 1+idir1;
                out << setw(4) << 1+iatom1;
                out << setw(4) << 1+idir2;
                out << setw(4) << 1+iatom2;
                out.setf(std::ios::scientific,std::ios::floatfield);
                out.precision(14);
                out << setw(22) << elt.second.real() << " " ;
                out << setw(22) << elt.second.imag() << " " ;
                out << endl;
              }
            }
          }
        }
      }
    }
    //*/
    out << endl;
  }
  out << " List of bloks and their characteristics" << endl << endl ;
  for ( unsigned iblock = 0 ; iblock < qpts.size() ; ++iblock ) {
    auto &qpt = qpts[iblock];
    auto &d2 = ddb.getDdb(qpt);
    out.unsetf(std::ios::floatfield);
    out << " 2nd derivatives (non-stat.)  - # elements :" << setw(8) << d2.size() << endl;
    out << " qpt";
    out.precision(8);
    out.setf(std::ios::scientific,std::ios::floatfield);
    out << setw(16) << qpt[0];
    out << setw(16) << qpt[1];
    out << setw(16) << qpt[2];
    out.precision(1);
    out.setf(std::ios::fixed,std::ios::floatfield);
    out << setw(6) << 1.0 << endl;
    out << endl;
  }
  out.close();
}
