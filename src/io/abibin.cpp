/**
 * @file src/./abibin.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#include "io/abibin.hpp"
#include "base/exception.hpp"
#include <fstream>
#include <algorithm>
#include <numeric>

const std::vector<int> AbiBin::densityFform({52,53,54,55,56,57,58,59,60,61,62,63,67,68,69,110,70,64,65,66,71});
const std::vector<int> AbiBin::potentialFform({102,103,104,105,106,107,108,109,111,112,113,114});

//
AbiBin::AbiBin() : Dtset(),
  _nspden(0),
  _ngfft{0,0,0},
  _fftData(),
  _header()
{
  ;
}

//
AbiBin::~AbiBin() {
  ;
}

void AbiBin::readFromFile(const std::string& filename) {

  try {
    _header.readFromFile(filename);
    static_cast<Dtset>(*this) = static_cast<Dtset>(_header);
    _rprim = _header.rprim();
  }
  catch( Exception& e ) {
    e.ADD("Header cannot be read properly",e.getReturnValue());
    throw e;
  }

  std::ifstream file;
  
  file.open(filename,std::ios::in|std::ios::binary);

  if ( !file )
    throw EXCEPTION(std::string("File ")+filename+" could not be opened",ERRABT);

  unsigned int marker;                  // Size of the next variable.

  std::vector<int> dsize;
  int dfullsize;
  std::vector<double> ddummy;

  auto checkMarker = [&](int level, int charsize=0){
    file.read((char*)(&marker),sizeof(int));
    if ( level > 0 ) {
      dfullsize = std::accumulate(dsize.begin(),dsize.end(),0);
    }
    if ( marker != (charsize*sizeof(char)+(dfullsize)*sizeof(double)) )
      throw EXCEPTION(std::string("Bad header file: <")+(level<0?"/H":"H")+utils::to_string(std::abs(level))+">",ERRABT);
    if ( level > 0 ) {
      ddummy.resize(*std::max_element(dsize.begin(),dsize.end()));
    }
  };

  _nspden = _header.nspden();
  std::copy(_header.ngfft(),_header.ngfft()+3,_ngfft);
  file.seekg(_header.endHeader());
  _fftData.resize(_ngfft[0]*_ngfft[1]*_ngfft[2]*_nspden);
  dsize = {_ngfft[0]*_ngfft[1]*_ngfft[2]};
  try {
    for ( int ispden = 0 ; ispden < _nspden ; ++ ispden ) {
      checkMarker(70+ispden);
      file.read((char*)&ddummy[0],marker);
      for ( int z = 0 ; z < _ngfft[2] ; ++z ) {
        for ( int y = 0 ; y < _ngfft[1] ; ++y ) {
          for ( int x = 0 ; x < _ngfft[0] ; ++x ) {
            _fftData[ispden*dsize[0]+(x*_ngfft[1]+y)*_ngfft[2]+z] = ddummy[(z*_ngfft[1]+y)*_ngfft[0]+x];
          }
        }
      }
      checkMarker(-(70+ispden));
    }
  }
  catch ( Exception &e ) {
    _nspden = 0;
    _ngfft[0] = 0; _ngfft[1] = 0; _ngfft[2] = 0;
    _fftData.clear();
    throw e;
  }
}

int AbiBin::getPoints(gridDirection dir) {
  switch (dir) {
    case A: return _ngfft[0]; break;
    case B: return _ngfft[1]; break;
    case C: return _ngfft[2]; break;
    default : throw EXCEPTION("This direction does not exist",ERRDIV);
  }
}

geometry::vec3d AbiBin::getVector(gridDirection dir) {
  switch (dir) {
    case A: return { _rprim[0], _rprim[3], _rprim[6] } ; break;
    case B: return { _rprim[1], _rprim[4], _rprim[7] } ; break;
    case C: return { _rprim[2], _rprim[5], _rprim[8] } ; break;
    default : throw EXCEPTION("This direction does not exist",ERRDIV);
  }
}

double AbiBin::getData(int origin, gridDirection dir, getDen function, std::vector<double> &data) {
  if ( origin < 0 )
    throw EXCEPTION("Origin cannot be negative",ERRDIV);
  if ( _fftData.size() == 0 ) {
    data.clear();
  }
  const int npoints = _ngfft[0]*_ngfft[1]*_ngfft[2];

  switch(_nspden) {
    case 1: {
              if ( function != SUM ) 
                throw EXCEPTION("Only one (the total #sum) density to display",ERRDIV);
              break;
            }
    case 2: {
              if ( function == X || function == Y || function == Z ) 
                throw EXCEPTION("Only collinear densities (#sum #diff #up #down) to display",ERRDIV);
              break;
            }
    case 4: {
              if ( function == DIFF || function == UP || function == DOWN ) 
                throw EXCEPTION("Only total and projected densities (#sum #x #y #z) to display",ERRDIV);
              break;
            }
  }

  //double inv_max = 1.;//(*std::max_element(_fftData.begin(),_fftData.begin()+npoints));

  int planSize;
  int dimV;
  int coord[3];
  int indU, indV, indNormal;

  switch(dir) {
    case A: {
              indNormal = 0;
              planSize = _ngfft[1]*_ngfft[2];
              dimV=_ngfft[2];
              indU = 1;
              indV = 2;
              break;
            }
    case B: {
              indNormal = 1;
              planSize = _ngfft[2]*_ngfft[0];
              dimV=_ngfft[0];
              indU = 2;
              indV = 0;
              break;
            }
    case C: {
              indNormal = 2;
              planSize = _ngfft[0]*_ngfft[1];
              dimV=_ngfft[1];
              indU = 0;
              indV = 1;
              break;
            }
    default: throw EXCEPTION("Unknown direction",ERRDIV);
  }
  if ( origin >= _ngfft[indNormal] )
    throw EXCEPTION("Origin is too large",ERRDIV);

  coord[indNormal]=origin;
  data.resize(planSize);
  int shiftOrigin = npoints;
  switch ( function ) {
    case SUM : 
    case DOWN:
    case DIFF:
      shiftOrigin *= 0;
      //if ( function != SUM ) inv_max*=-2;
      break;
    case UP : 
    case X :
      shiftOrigin *= 1;
      break;
    case Y :
      shiftOrigin *= 2;
      break;
    case Z :
      shiftOrigin *= 3;
      break;
  }
  for ( int u = 0 ; u < _ngfft[indU] ; ++u ) {
    for ( int v = 0 ; v < _ngfft[indV] ; ++v ) {
      coord[indU] = u;
      coord[indV] = v;
      data[u*dimV+v] = _fftData[shiftOrigin+((coord[0]*_ngfft[1]+coord[1])*_ngfft[2])+coord[2]]/**inv_max*/;
    }
  }
  if ( function == DOWN || function == DIFF) {
    int shiftOrigin = planSize*_ngfft[indNormal];
    //double inv_max2 = inv_max*( function == DOWN ? 1 : 2 );
    double operation = ( function == DOWN ? 1 : 2 );
    for ( int u = 0 ; u < _ngfft[indU] ; ++u ) {
      for ( int v = 0 ; v < _ngfft[indV] ; ++v ) {
        coord[indU] = u;
        coord[indV] = v;
        data[u*dimV+v] -= _fftData[shiftOrigin+((coord[0]*_ngfft[1]+coord[1])*_ngfft[2])+coord[2]]*operation/**inv_max2*/;
      }
    }
  }
  double max = *std::max_element(_fftData.begin(),_fftData.end());
  double min = *std::min_element(_fftData.begin(),_fftData.end());
  return 1./((function==SUM?1:0.5)*std::max(std::abs(min),std::abs(max)));
}
