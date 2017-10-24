/**
 * @file src/supercell.cpp
 *
 * @brief 
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


#include "phonons/supercell.hpp"
#include "base/mendeleev.hpp"
#include <cmath>
#include <cstring>
#ifdef HAVE_FFTW3
#include "fftw3.h"
#endif

//
Supercell::Supercell() : Dtset(),
  _baseAtom(),
  _dim(),
  _cellCoord(),
  _fft()
{
  ;
}

//
Supercell::Supercell(const Dtset& dtset, const geometry::vec3d& qpt) : Dtset(),
  _baseAtom(),
  _dim(),
  _cellCoord(),
  _fft()
{
  const double Rx = (qpt[0] > 1e-6 ) ? std::floor(1.0e0/qpt[0]) : 1.0;
  const double Ry = (qpt[1] > 1e-6 ) ? std::floor(1.0e0/qpt[1]) : 1.0;
  const double Rz = (qpt[2] > 1e-6 ) ? std::floor(1.0e0/qpt[2]) : 1.0;
  if ( std::abs(Rx-1.0e0/qpt[0]) > 1e-10 ) 
    throw EXCEPTION("Unable to find supercell multiple for x direction",ERRDIV);
  if ( std::abs(Ry-1.0e0/qpt[1]) > 1e-10 ) 
    throw EXCEPTION("Unable to find supercell multiple for y direction",ERRDIV);
  if ( std::abs(Rz-1.0e0/qpt[2]) > 1e-10 ) 
    throw EXCEPTION("Unable to find supercell multiple for z direction",ERRDIV);

  *this = Supercell(dtset,(unsigned) Rx, (unsigned) Ry, (unsigned) Rz);
}


//
Supercell::Supercell(const Dtset& dtset, const unsigned nx, const unsigned ny, const unsigned nz) : Dtset(),
  _baseAtom(),
  _dim(),
  _cellCoord(),
  _fft()
{
  const double Rx = (double) nx;
  const double Ry = (double) ny;
  const double Rz = (double) nz;

  _dim[0] = Rx;
  _dim[1] = Ry;
  _dim[2] = Rz;

  _natom = dtset.natom()*static_cast<unsigned>(Rx*Ry*Rz);
  _ntypat = dtset.ntypat();
  _znucl = dtset.znucl();
  _acell = dtset.acell();
  geometry::mat3d ucrprim = dtset.rprim();
  _rprim = dtset.rprim();
  _rprim[0] *= Rx;
  _rprim[1] *= Ry;
  _rprim[2] *= Rz;
  _rprim[3] *= Rx;
  _rprim[4] *= Ry;
  _rprim[5] *= Rz;
  _rprim[6] *= Rx;
  _rprim[7] *= Ry;
  _rprim[8] *= Rz;
  _gprim = geometry::invertTranspose(_rprim);
  bool hasSpin = (!dtset.spinat().empty());

  //Build _xcart
  for ( int i = 0 ; i < _dim[0] ; ++i) {
    using namespace geometry;
    const double fi = (double) i;
    const vec3d xtrans = {{ fi*ucrprim[0], fi*ucrprim[3], fi*ucrprim[6] }};
    for ( int j = 0 ; j < _dim[1] ; ++j) {
      const double fj = (double) j;
      const vec3d ytrans = {{ fj*ucrprim[1]+xtrans[0], fj*ucrprim[4]+xtrans[1], fj*ucrprim[7]+xtrans[2] }};
      for ( int k = 0 ; k < _dim[2] ; ++k ) {
        const double fk = (double) k;
        const vec3d ztrans = {{ fk*ucrprim[2]+ytrans[0], fk*ucrprim[5]+ytrans[1], fk*ucrprim[8]+ytrans[2] }};
        const vec3d coord = {{ fi, fj, fk }};
        for ( unsigned iatom = 0 ; iatom < dtset.natom() ; ++iatom ) {
          _baseAtom.push_back(iatom);
          _cellCoord.push_back(coord);
          _typat.push_back(dtset.typat()[iatom]);
          _xcart.push_back(dtset.xcart()[iatom]+ztrans);
          if ( hasSpin )
            _spinat.push_back(dtset.spinat()[iatom]);
        }
      }
    }
  }
  _xred = geometry::changeBasis(_rprim, _xcart);
}

//
Supercell::Supercell(const HistData& hist, const unsigned itime) : Dtset(hist,itime),
  _baseAtom(),
  _dim(),
  _cellCoord(),
  _fft()
{
  ;
}

//
Supercell::~Supercell() {
  ;
}

//
void Supercell::makeDisplacement(const geometry::vec3d qpt, DispDB& db, unsigned imode, double amplitude, double phase) {
  using namespace geometry;
  unsigned natom = db.natom();
  if ( natom*static_cast<unsigned>(_dim[0]*_dim[1]*_dim[2]) != _natom )
    throw EXCEPTION("Hmm supercell and DispDB are incoherent",ERRDIV);
  if ( _cellCoord.size() == 0 or _baseAtom.size() == 0 )
    throw EXCEPTION("Hmm supercell is not built ahead of a reference structure",ERRDIV);
  unsigned q = db.getQpt(qpt);
  auto mymode = db.getMode(q,imode);

  for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
    const vec3d R = _cellCoord[iatom];
    const unsigned iatomUC = _baseAtom[iatom];
    const double qR_theta = 2.*phys::pi*dot(qpt,R)+phase;
    vec3d Re = {{ mymode[iatomUC*3].real(),mymode[iatomUC*3+1].real(),mymode[iatomUC*3+2].real() }};
    vec3d Im = {{ mymode[iatomUC*3].imag(),mymode[iatomUC*3+1].imag(),mymode[iatomUC*3+2].imag() }};
    //std::cerr << iatom << " " << mymode[iatomUC*3] << " " << mymode[iatomUC*3+1] << " " << mymode[iatomUC*3+2] << std::endl;
    _xcart[iatom] += (Re*std::cos(qR_theta)-Im*std::sin(qR_theta))*amplitude;
  }
  _xred = geometry::changeBasis(_rprim, _xcart);
}

//
void Supercell::findReference(const Dtset& dtset) {
  using namespace geometry;
  using std::max;
  using std::min;
  // First find the dimension
  mat3d ref_rprim = dtset.rprim();
  vec3d ref_vec[3];
  vec3d super_vec[3];

  //double super_volume = det(_rprim);
  //double ref_volume = det(ref_rprim);
  unsigned multiplicity = static_cast<unsigned>(std::rint(_natom/dtset.natom())); 
  for ( int i = 1 ; i < 4 ; ++i ) {
    super_vec[i-1] = {{ _rprim[mat3dind(i,1)], _rprim[mat3dind(i,2)], _rprim[mat3dind(i,3)] }};
    ref_vec[i-1] = {{ ref_rprim[mat3dind(i,1)], ref_rprim[mat3dind(i,2)], ref_rprim[mat3dind(i,3)] }};
    _dim[i-1] = std::round(norm(super_vec[i-1])/norm(ref_vec[i-1]));
  }
  if ( static_cast<unsigned>(_dim[0]*_dim[1]*_dim[2]) != multiplicity 
      || _natom != multiplicity*dtset.natom() )
    throw EXCEPTION("Volume multiplicity does not match cell multiplicity", ERRDIV);

  auto ref_typat = dtset.typat();
  auto ref_znucl = dtset.znucl();
  auto ref_xcart = dtset.xcart();

  //Check we have the same znucl
  if ( ref_znucl.size() != _znucl.size() )
    throw EXCEPTION("znucl have not the same size !",ERRDIV);

  std::vector<int> ref2super(_znucl.size()+1);
  for ( unsigned z = 0 ; z < ref_znucl.size() ; ++z ) {
    for ( unsigned find = 0 ; find < _znucl.size() ; ++ find ) {
      if ( _znucl[find] == ref_znucl[z] ) {
        ref2super[z+1] = find+1;
        break;
      }
    }

    if ( _znucl[ref2super[z+1]-1] != ref_znucl[z] )
      throw EXCEPTION("znucl are not the same !",ERRDIV);
  }

  _baseAtom.resize(_natom);
  _cellCoord.resize(_natom);

  // Now find reference atom in the reference supercell
  // This might be very slow but is the easiest and should always work
#pragma omp parallel for schedule(static)
  for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) { // For each supercell atom
    vec3d pos = _xcart[iatom];
    unsigned guess[3] = {0};
    // Reduce search area
    for ( unsigned i = 0 ; i < 3 ; ++ i ) {
      const double normvec = norm(ref_vec[i]);
      guess[i] = (unsigned) max(0.e0,dot(pos,ref_vec[i])/(normvec*normvec)-1);
    }
    //std::cerr << "Guess " << guess[0] << " " << guess[1] << " " << guess[2] << std::endl;

    unsigned match = (unsigned)(-1);
    vec3d R = {{0.e0,0.e0,0.e0}};
    double closest = 1e12;
    for ( unsigned ref_iatom = 0 ; ref_iatom < dtset.natom() ; ++ref_iatom ) { // Scan each reference atom
      if ( ref2super[ref_typat[ref_iatom]] != _typat[iatom] ) continue;

      for ( unsigned i = guess[0] ; i < min((unsigned)_dim[0],guess[0]+3) ; ++i) {
        const double fi = (double) i;
        const vec3d xtrans = {{ fi*ref_rprim[0], fi*ref_rprim[3], fi*ref_rprim[6] }};
        for ( unsigned j = guess[1] ; j < min((unsigned)_dim[1],guess[1]+3) ; ++j) {
          const double fj = (double) j;
          const vec3d ytrans = {{ fj*ref_rprim[1]+xtrans[0], fj*ref_rprim[4]+xtrans[1], fj*ref_rprim[7]+xtrans[2] }};
          for ( unsigned k = guess[2] ; k < min((unsigned)_dim[2],guess[2]+3) ; ++k ) {
            const double fk = (double) k;
            const vec3d ztrans = {{ fk*ref_rprim[2]+ytrans[0], fk*ref_rprim[5]+ytrans[1], fk*ref_rprim[8]+ytrans[2] }};
            vec3d cell = {{ fi, fj, fk }};
            double distance = norm(ref_xcart[ref_iatom]+ztrans-pos);
            if ( distance < closest ) {
              match = ref_iatom;
              R = cell;
              closest = distance;
            }
          }
        }
      }
    }
    if ( match == (unsigned) -1 )
      throw EXCEPTION("Error finding reference structure",ERRDIV);
    _baseAtom[iatom] = match;
    _cellCoord[iatom] = R;
  }
  //*
  std::vector<unsigned> check(dtset.natom(),0);
  for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
    ++check[_baseAtom[iatom]];
    //std::cerr << "Atom " << iatom << ":\t" << _baseAtom[iatom] << "\t";
    //print(_cellCoord[iatom]);
  }
  for ( auto k : check ) {
    //std::cerr << k << " " << (double)k/(_dim[0]*_dim[1]*_dim[2]) << std::endl;
    if ( k != (unsigned)(_dim[0]*_dim[1]*_dim[2]) ) 
      throw EXCEPTION("Mapping on reference structure failed",ERRDIV);
  }
  //*/
}

//
void Supercell::setReference(const Supercell& supercell) {
  if ( _natom != supercell._natom )
    throw EXCEPTION("Number of atoms mismatches",ERRDIV);

  if ( supercell._znucl.size() != _znucl.size() )
    throw EXCEPTION("znucl have not the same size !",ERRDIV);

  for ( unsigned z = 0 ; z < _znucl.size() ; ++z ) {
    if ( supercell._znucl[z] != _znucl[z] )
      throw EXCEPTION("znucl are not the same !",ERRDIV);
  }

  _dim = supercell._dim;
  _baseAtom = supercell._baseAtom;
  _cellCoord = supercell._cellCoord;
}

//
std::vector<double> Supercell::getDisplacement(const Dtset &dtset) {
  using namespace geometry;
  if ( _baseAtom.size() != _natom 
      || _cellCoord.size() != _natom 
      || _znucl.size() != dtset.znucl().size() )
    throw EXCEPTION("Construct reference first",ERRDIV);

  for ( unsigned z = 0 ; z < dtset.znucl().size() ; ++z ) {
    bool found = false;
    for ( unsigned find = 0 ; find < _znucl.size() ; ++find ) {
      if ( _znucl[find] == dtset.znucl()[z] ) {
        found = true;
        break;
      }
    }
    if ( !found )
      throw EXCEPTION("znucl are not the same !",ERRDIV);
  }

  unsigned natom = dtset.natom();
  if ( natom*static_cast<unsigned>(_dim[0]*_dim[1]*_dim[2]) != _natom )
    throw EXCEPTION("Hmm supercell and dtset are incoherent",ERRDIV);

  auto ref_rprim = dtset.rprim();
  auto ref_rprim_supercell = ref_rprim;
  ref_rprim_supercell[0] *= _dim[0];
  ref_rprim_supercell[3] *= _dim[0];
  ref_rprim_supercell[6] *= _dim[0];
  ref_rprim_supercell[1] *= _dim[1];
  ref_rprim_supercell[4] *= _dim[1];
  ref_rprim_supercell[7] *= _dim[1];
  ref_rprim_supercell[2] *= _dim[2];
  ref_rprim_supercell[5] *= _dim[2];
  ref_rprim_supercell[8] *= _dim[2];

  std::vector<double> displacements(3*_natom,0);
  std::vector<vec3d> xcart_supercell(_natom);
  auto ref_xcart = dtset.xcart();
  for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
    const double fi = _cellCoord[iatom][0];
    const vec3d xtrans = {{ fi*ref_rprim[0], fi*ref_rprim[3], fi*ref_rprim[6] }};
    const double fj = _cellCoord[iatom][1];
    const vec3d ytrans = {{ fj*ref_rprim[1]+xtrans[0], fj*ref_rprim[4]+xtrans[1], fj*ref_rprim[7]+xtrans[2] }};
    const double fk = _cellCoord[iatom][2];
    const vec3d ztrans = {{ fk*ref_rprim[2]+ytrans[0], fk*ref_rprim[5]+ytrans[1], fk*ref_rprim[8]+ytrans[2] }};

    vec3d position = ref_xcart[_baseAtom[iatom]]+ztrans;
    vec3d super_xcart_in_ref = ref_rprim_supercell * _xred[iatom];
    for ( unsigned d = 0 ; d < 3 ; ++ d )
      displacements[iatom*3+d] = super_xcart_in_ref[d] - position[d];
  }
  _fft.clear();
  return displacements;
}

std::vector<double> Supercell::projectOnModes(const Dtset& dtset, DispDB& db, const DispDB::qptTree& modes,bool normalized) {
  using namespace geometry;
  if ( _baseAtom.size() != _natom 
      || _cellCoord.size() != _natom 
      || _znucl.size() != dtset.znucl().size() )
    this->findReference(dtset);

  std::vector<double> displacements;
  try {
    displacements = this->getDisplacement(dtset);
  }
  catch (Exception &e) {
    e.ADD("Cannot compute displacements",ERRDIV);
    throw e;
  }
  std::vector<double> mass(dtset.natom(),0.);
  for ( unsigned iatom = 0 ; iatom < dtset.natom() ; ++iatom ) {
    mass[iatom] = mendeleev::mass[dtset.znucl()[dtset.typat()[iatom]-1]]*phys::amu_emass; // type starts at 1
  }

  double norm2_disp = 0.;

  std::vector<double> results;

  // For all qpt;
  for ( auto qpt = modes.begin() ; qpt != modes.end() ; ++qpt ) {
    auto& qpoint = qpt->first;
    unsigned dq = db.getQpt(qpoint);

    auto filtered = this->filterDisp(qpoint,displacements);
    norm2_disp = 0.;
    for ( unsigned iatomUC = 0 ; iatomUC < 3*_natom/(_dim[0]*_dim[1]*_dim[2]) ; ++iatomUC ) {
      norm2_disp += mass[iatomUC/3]*std::norm(filtered[iatomUC]);
    }
    // Renormalize
    for ( unsigned iatomUC = 0 ; iatomUC < 3*_natom/(_dim[0]*_dim[1]*_dim[2]) ; ++iatomUC ) {
      filtered[iatomUC] /= std::sqrt(norm2_disp);
    }

    for ( auto& vib : qpt->second ) {
      auto mymode = db.getMode(dq,vib.imode);

      std::complex<double> projection(0,0);
      for ( unsigned iall = 0 ; iall < filtered.size() ; ++iall ) {
        projection += mass[iall/3]*filtered[iall]*mymode[iall];
      }
      results.push_back(normalized ? std::norm(projection) : std::norm(projection)*norm2_disp);
    }
  }
  return results;
}

std::vector<std::complex<double>> Supercell::filterDisp(const geometry::vec3d& qpt, const std::vector<double>& disp){
  const int nx = (int) _dim[0];
  const int ny = (int) _dim[1];
  const int nz = (int) _dim[2];
  const int nzh = nz/2+1;
  std::vector<std::complex<double>> dispq;

  int selectx = (int) ( qpt[0]*nx );
  int selecty = (int) ( qpt[1]*ny );
  int selectz = (int) ( qpt[2]*nz );

  if ( qpt[0] < 0. )
    selectx = nx + selectx;
  if ( qpt[1] < 0. )
    selecty = ny + selecty;
  if ( qpt[2] < 0. )
    selectz *= -1.;


  this->fft(disp);
  int natomUC = _natom/(nx*ny*nz);
  auto begin = _fft.begin();
  std::advance(begin,(selectx*ny*nzh+selecty*nzh+selectz)*natomUC*3);
  auto end = begin;
  std::advance(end,3*natomUC);
  dispq.resize(3*natomUC);
  std::copy(begin,end,dispq.begin());

  return dispq;
}

std::vector<std::array<double,4>> Supercell::amplitudes(const Dtset& dtset) {
  const int nx = (int) _dim[0];
  const int ny = (int) _dim[1];
  const int nz = (int) _dim[2];
  const int nzh = nz/2+1;
  const int natomUC = _natom/(nx*ny*nz);

  std::vector<std::array<double,4>> amplitudes(nx*ny*nzh);

  std::vector<std::complex<double>> dispq(3*natomUC);

  if ( _baseAtom.size() != _natom 
      || _cellCoord.size() != _natom 
      || _znucl.size() != dtset.znucl().size() )
    this->findReference(dtset);

  std::vector<double> displacements;
  try {
    displacements = this->getDisplacement(dtset);
  }
  catch (Exception &e) {
    e.ADD("Cannot compute displacements",ERRDIV);
    throw e;
  }
  std::vector<double> mass(dtset.natom(),0.);
  for ( unsigned iatom = 0 ; iatom < dtset.natom() ; ++iatom ) {
    mass[iatom] = mendeleev::mass[dtset.znucl()[dtset.typat()[iatom]-1]]*phys::amu_emass; // type starts at 1
  }
  this->fft(displacements);

  for ( int qx = 0 ; qx < nx ; ++qx ) {
    for ( int qy = 0 ; qy < ny ; ++qy ) {
      for ( int qz = 0 ; qz < nzh ; ++qz ) {
        auto dispq = _fft.begin();
        std::advance(dispq,(qx*ny*nzh+qy*nzh+qz)*natomUC*3);
        double norm2_disp = 0.;
        for ( int iatomUC = 0 ; iatomUC < 3*natomUC ; ++iatomUC ) {
          norm2_disp += mass[iatomUC/3]*std::norm(dispq[iatomUC]);
        }
        if ( norm2_disp < 1e-10 ) norm2_disp = 0;
        amplitudes[qx*ny*nzh+qy*nzh+qz] = {{(double)qx/(double)nx,(double)qy/(double)ny,(double)qz/(double)nz,norm2_disp}};
      }
    }
  }
  return amplitudes;
}

void Supercell::fft(const std::vector<double>& dispr) {
#ifdef HAVE_FFTW3
  const int nx = (int) _dim[0];
  const int ny = (int) _dim[1];
  const int nz = (int) _dim[2];
  const int nzh = nz/2+1;
  const int natomUC = _natom/(nx*ny*nz);
  const int howmany = 3*natomUC;
  const double inv_vol = 1./(double)(nx*ny*nz);
  const int idist = nx*ny*nz; const int odist = nx*ny*nzh;
  const int *inembed = NULL; const int *onembed = NULL;
  const int istride = 1; const int ostride = 1;
  const int n[3]={nx,ny,nz};

  double *fft_in;
  fftw_complex *fft_out;
  fftw_plan plan_forward;

  if ( dispr.size() != _natom*3 ) throw EXCEPTION("Vector has not the expected size",ERRDIV);
  if ( _fft.size() > 0 ) return; // Consider the FFT is already done

  fft_in = (double*) fftw_malloc( sizeof ( double ) * nx * ny * nz * howmany);
  fft_out = (fftw_complex*) fftw_malloc( sizeof ( fftw_complex ) * nx * ny * nzh * howmany );

  for ( unsigned iatom = 0 ; iatom < _natom ; ++iatom ) {
    const int ref_atom = _baseAtom[iatom];
    auto& R = _cellCoord[iatom];
    for ( unsigned dim = 0 ; dim < 3 ; ++dim ) {
      int start_fft = (ref_atom*3+dim)*idist;
      fft_in[start_fft+((unsigned)R[0]*ny+(unsigned)R[1])*nz+(unsigned)R[2]] = dispr[iatom*3+dim];
    }
  }

#ifdef HAVE_FFTW3_THREADS
  fftw_plan_with_nthreads(1);
#endif

#pragma omp critical (supercell_fft)
  {
    plan_forward = fftw_plan_many_dft_r2c(3, n, howmany, 
        fft_in, inembed, istride, idist, 
        fft_out, onembed, ostride, odist, FFTW_ESTIMATE);
  }

  fftw_execute(plan_forward);
  _fft.resize(nx*ny*nzh*natomUC*3);

#define outIndice(dim,iatom,qx,qy,qz) ((iatom*3+dim)*odist+(qx*ny+qy)*nzh+qz)
#define dispqIndice(dim,iatom,qx,qy,qz) ( ((((qx*ny+qy)*nzh+qz)*natomUC+iatom)*3)+dim )
  for ( int qx = 0 ; qx < nx ; ++qx ) {
    for ( int qy = 0 ; qy < ny ; ++qy ) {
      for ( int qz = 0 ; qz < nzh ; ++qz ) {
        for ( int iatom = 0 ; iatom < natomUC ; ++iatom ) {
          for ( int dim = 0 ; dim < 3 ; ++dim ) {
            _fft[dispqIndice(dim,iatom,qx,qy,qz)].real(fft_out[outIndice(dim,iatom,qx,qy,qz)][0]*inv_vol);
            _fft[dispqIndice(dim,iatom,qx,qy,qz)].imag(fft_out[outIndice(dim,iatom,qx,qy,qz)][1]*inv_vol);
          }
        }
      }
    }
  }
#undef outIndice
#undef dispqIndice

#pragma omp critical (supercell_fft)
  {
    fftw_destroy_plan(plan_forward);
  }
  fftw_free(fft_in);
  fftw_free(fft_out);
#else
  (void) dispr;
  throw EXCEPTION("FFTW3 is required for this operation.",ERRDIV);
#endif
}
