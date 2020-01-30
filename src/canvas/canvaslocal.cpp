/**
 * @file src/canvasrot.cpp
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


#include "canvas/canvaslocal.hpp"
#include "base/mendeleev.hpp"
#include "shape/octaangles.hpp"
#include "shape/octalengths.hpp"
#include "plot/gnuplot.hpp"
#include <iomanip>

using Agate::Mendeleev;

//
CanvasLocal::CanvasLocal(bool drawing) : CanvasPos(drawing),
  _baseCart(false),
  _octacolor(),
  _cube(_opengl),
  _view(ANGLES),
  _orientations()
{
  _octacolor[0] = 0.f;
  _octacolor[1] = 0.f;
  _octacolor[2] = 1.f;
  _octacolor[3] = 1.f;
  _octacolor[4] = 0.f;
  _octacolor[5] = 0.f;
  ;
}

//
CanvasLocal::CanvasLocal(CanvasPos &&canvas) : CanvasPos(std::move(canvas)),
  _baseCart(false),
  _octacolor(),
  _cube(_opengl),
  _view(ANGLES),
  _orientations()
{
  _octacolor[0] = 0.f;
  _octacolor[1] = 0.f;
  _octacolor[2] = 1.f;
  _octacolor[3] = 1.f;
  _octacolor[4] = 0.f;
  _octacolor[5] = 0.f;

  if ( _histdata == nullptr ) return;
  if ( _histdata->ntime() == 1 ) _baseCart = true;

  const double *rprimd = _histdata->getRprimd(0);
  const double *xcart = _histdata->getXcart(0);
  Octahedra::u3f angles;

  if ( !_octahedra.empty() && typeid(_octahedra[0].get()) != typeid(OctaAngles*) ) {
    switch ( _view ) {
      case ANGLES :
        for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
          OctaAngles *tmpocta = new OctaAngles(std::move(*_octahedra[i].get()));
          tmpocta->buildCart(rprimd,xcart,angles,_baseCart);
          _octahedra[i].reset(tmpocta);
        }
        break;
      case LENGTHS :
        for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
          OctaAngles *tmpocta = new OctaAngles(std::move(*_octahedra[i].get()));
          tmpocta->buildCart(rprimd,xcart,angles,false);
          OctaLengths *pushocta = new OctaLengths(std::move(*tmpocta));
          delete tmpocta;
          _octahedra[i].reset(pushocta);
        }
        break;
    }
  }
  _orientations.resize(_octahedra.size());
  for ( unsigned i = 0 ; i <_octahedra.size() ; ++i ) {
    _orientations[i] = angles[i].second;
  }

}

//
CanvasLocal::~CanvasLocal() {
  ;
}

//
void CanvasLocal::refresh(const geometry::vec3d &cam, TextRender &render){
  (void)(cam);
  (void) render;
  const int totalnatom = _natom+_onBorders.size();
  if ( totalnatom < 1 ) return;
  this->updateHist();
  if ( !_opengl ) return;
#ifdef HAVE_GL
  const double *rprimd = _histdata->getRprimd(_itime);
  const double *xcart = _histdata->getXcart(_itime);
  const GLfloat fx[] = {(GLfloat)rprimd[0], (GLfloat)rprimd[3], (GLfloat)rprimd[6]};
  const GLfloat fy[] = {(GLfloat)rprimd[1], (GLfloat)rprimd[4], (GLfloat)rprimd[7]};
  const GLfloat fz[] = {(GLfloat)rprimd[2], (GLfloat)rprimd[5], (GLfloat)rprimd[8]};
  //const float width = (float)MendeTable.radius[_octahedra_z];

  const float factor = ( _view == ANGLES ? 1.f/15.f : 1.f/0.10f ); CanvasPos::drawCell(); 

  this->buildBorders(_itime,false);

  //glDisable(GL_LIGHTING);
  Octahedra::u3f angles;
  _cube.push();
  for ( int k = 0 ; k < _translate[2] ; ++k ) {
    const GLfloat fk = (GLfloat) k;
    const GLfloat ztrans[] = { (fk)*fz[0], (fk)*fz[1],(fk)*fz[2] };
    for ( int j = 0 ; j < _translate[1] ; ++j) {
      const GLfloat fj = (GLfloat) j;
      const GLfloat ytrans[] = { ztrans[0]+(fj)*fy[0], ztrans[1]+(fj)*fy[1],ztrans[2]+(fj)*fy[2] };
      for ( int i = 0 ; i < _translate[0] ; ++i) {
        const GLfloat fi = (GLfloat) i;
        const GLfloat xtrans[] = { ytrans[0]+(fi)*fx[0], ytrans[1]+(fi)*fx[1],ytrans[2]+(fi)*fx[2] };
        glPushMatrix();
        glTranslatef(xtrans[0],xtrans[1],xtrans[2]);

        if (  k == 0 && j == 0 && i == 0 ){
          for( auto& octa : _octahedra )
            octa->build(rprimd,xcart,angles);

          if ( _view == LENGTHS ) {
            double a = 0;
            double b = 0;
            double c = 0;
            for ( auto& angle : angles ) {
              a += angle.second[0];
              b += angle.second[1];
              c += angle.second[2];
            }
            a /= angles.size();
            b /= angles.size();
            c /= angles.size();
            for ( auto& angle : angles ) {
              angle.second[0] -= a;
              angle.second[1] -= b;
              angle.second[2] -= c;
            }
          }
        }

        for ( unsigned i = 0 ; i <_octahedra.size() ; ++i ) {
          int iatom = _octahedra[i]->center();
          if ( iatom >= _natom && !(_display & CanvasPos::DISP_BORDER ) ) continue;
          glPushMatrix();
          ( iatom < _natom ) 
            ? glTranslatef(xcart[3*iatom],xcart[3*iatom+1],xcart[3*iatom+2])
            : glTranslatef(_xcartBorders[3*(iatom-_natom)],_xcartBorders[3*(iatom-_natom)+1],_xcartBorders[3*(iatom-_natom)+2]);
          glRotatef(-_orientations[i][0],1.,0.,0.);
          glRotatef(-_orientations[i][1],0.,1.,0.);
          glRotatef(-_orientations[i][2],0.,0.,1.);
          glScalef(MendeTable.radius[_znucl[_typat[iatom]]],MendeTable.radius[_znucl[_typat[iatom]]],MendeTable.radius[_znucl[_typat[iatom]]]);
          _cube.draw(&_octacolor[0],&_octacolor[3],angles[i].second[0]*factor,angles[i].second[1]*factor,angles[i].second[2]*factor);
          glPopMatrix();
        }

        glPopMatrix();
      }
    }
    _cube.pop();
    glPopMatrix();

    glFlush();
  }
#endif
}


//
void CanvasLocal::updateOctahedra(int z) {
  int typat = -1;
  for ( unsigned t = 0 ; t < _znucl.size() ; ++t ) {
    if ( _znucl[t] == std::abs(z) ) {
      typat = t;
      break;
    }
  }
  if ( z > 0 && (_natom+_onBorders.size()) > 6 && _hasTranslations) {
    for ( auto it = _octahedra_z.begin() ; it != _octahedra_z.end() ; ++it ) {
      if ( *it == z ) {
        return;
      }
    }
    _octahedra_z.push_back(z);
    unsigned startOcta = _octahedra.size();
    std::vector<double> xcartTotal((_natom+_onBorders.size())*3);
    const double *histXcart = _histdata->getXcart(0);
    const double *xred = _histdata->getXred(0);
    const double *rprimd = _histdata->getRprimd(0);

    this->buildBorders(0,false);
    std::copy(&histXcart[0],&histXcart[_natom*3],&xcartTotal[0]);
    std::copy(&_xcartBorders[0],&_xcartBorders[_onBorders.size()*3],&xcartTotal[_natom*3]);

    try {
      Octahedra::u3f angles;
      switch ( _view ) {
        case ANGLES :
          for ( unsigned iatom = 0 ; iatom < _natom+_onBorders.size() ; ++iatom ){
            if ( _typat[iatom] == typat ) {
              OctaAngles *tmpocta = new OctaAngles(iatom,_natom,xred,&xcartTotal[0],rprimd,_opengl); //Construct octahedra based on xcart at time 0
              tmpocta->buildCart(rprimd,histXcart,angles,_baseCart);
              _octahedra.push_back(std::unique_ptr<Octahedra>(tmpocta));
            }
          }
          break;
        case LENGTHS :
          for ( unsigned iatom = 0 ; iatom < _natom+_onBorders.size() ; ++iatom ){
            if ( _typat[iatom] == typat ) {
              OctaAngles *tmpocta = new OctaAngles(iatom,_natom,xred,&xcartTotal[0],rprimd,_opengl); //Construct octahedra based on xcart at time 0
              tmpocta->buildCart(rprimd,histXcart,angles,false);
              OctaLengths *pushocta = new OctaLengths(std::move(*tmpocta));
              _octahedra.push_back(std::unique_ptr<Octahedra>(pushocta));
            }
          }
          break;
      }

      _orientations.resize(_octahedra.size());
      for ( unsigned i = startOcta ; i <_octahedra.size() ; ++i ) {
        _orientations[i] = angles[i-startOcta].second;
      }
    }
    catch (Exception &e) {
      e.ADD("Abording construction of octahedra",ERRDIV);
      throw e;
    }
  }
  else if ( z < 0) {
    for ( auto it = _octahedra_z.begin() ; it != _octahedra_z.end() ; ++it ) {
      if ( *it == -z ) {
        _octahedra_z.erase(it);
        break;
      }
    }
    auto octa = _octahedra.begin();
    auto iocta = 0;
    do {
      if ( _typat[octa->get()->center()] == typat ) {
        octa = _octahedra.erase(octa);
        _orientations.erase(_orientations.begin()+iocta);
      }
      else {
        ++octa;
        ++iocta;
      }
    } while ( octa != _octahedra.end() );
  }
  else if ( z == 0) {
    _octahedra.clear();
    _octahedra_z.clear();
    _orientations.clear();
  }
  else{
    throw EXCEPTION("Not enough data to build octahedra",ERRDIV);
  }
}

//
void CanvasLocal::my_alter(std::string token, std::istringstream &stream) {

  std::string line;
  size_t pos = stream.tellg();
  std::getline(stream,line);
  stream.clear();
  stream.seekg(pos);
  ConfigParser parser;
  parser.setSensitive(true);
  parser.setContent(line);

  if ( token == "c" || token == "color" ){
    unsigned c[3];
    float *tomodify = nullptr;
    std::string name;
    stream >> name;
    if ( name == "plus" ) tomodify = &_octacolor[3];
    else if ( name == "minus" ) tomodify = &_octacolor[0];
    else throw EXCEPTION("Don't know what to do",ERRDIV);
    stream >> c[0] >> c[1] >> c[2];
    if ( !stream.fail() && c[0] < 256 && c[1] < 256 && c[2] < 256 ) {
      tomodify[0] = (float)c[0]/255.f;
      tomodify[1] = (float)c[1]/255.f;
      tomodify[2] = (float)c[2]/255.f;
    }
    else throw EXCEPTION("Bad color numbers",ERRDIV);
  }
  else if ( token == "div" || token == "division" ){
    unsigned int div;
    stream >> div;
    if ( !stream.fail() ) { 
      _cube.division(div);
      _cube.genUnit();
    }
  }
  else if ( token == "basis" ) {
    std::string base;
    stream >> base;
    if ( !stream.fail() && _histdata != nullptr ) { 
      if ( base == "cart" ) {
        _baseCart = true;
        this->resetBase();
      }
      else if ( base == "rel" ) {
        _baseCart = false;
        this->resetBase();
      }
      else
        throw EXCEPTION("Unknown basis "+base,ERRABT);
    }
    else
      throw EXCEPTION("Stream error: specify either cart or rel",ERRABT);
  } 
  else if ( token == "rot" ) {
    std::string ext;
    try {
      ext = utils::readString(stream);
      std::ofstream file(ext,std::ios::out);
      if ( !file )
        throw EXCEPTION("Unable to open file "+ext,ERRABT);
      // Write header
      file << "# " << std::setw(20) << "Time step";
      for ( unsigned i = 0 ; i < _octahedra.size() ; ++ i ) {
        if ( _octahedra[i]->center() >= _natom ) continue;
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center()+1 << std::setw(6) << " alpha";
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center()+1 << std::setw(6) << " beta";
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center()+1 << std::setw(6) << " gamma";
      }
      file << std::endl;
      // Write data
      file.precision(14);
      file.setf(std::ios::scientific,std::ios::floatfield);
      file.setf(std::ios::right,std::ios::adjustfield);
      const double *rprimd0 = _histdata->getRprimd(0);
      const double *xcart0 = _histdata->getXcart(0);
#pragma omp for ordered, schedule(static)
      for ( int itime = _tbegin ; itime < _tend ; ++itime ) {
        Octahedra::u3f angles;
        Octahedra::u3f angles0;
        const double *rprimd = _histdata->getRprimd(itime);
        const double *xcart = _histdata->getXcart(itime);
        for( auto& octa : _octahedra ){
          OctaAngles oa(*dynamic_cast<Octahedra*>(octa.get()));
          oa.buildCart(rprimd0,xcart0,angles0,_baseCart);
          oa.build(rprimd,xcart,angles);
        }

#pragma omp ordered
        {
        file << std::setw(22) << itime;
        for ( unsigned i = 0 ; i < angles.size() ; ++i ) {
          file << std::setw(22) << angles[i].second[0];
          file << std::setw(22) << angles[i].second[1];
          file << std::setw(22) << angles[i].second[2];
        }
        file << std::endl;
        }
      }
      file.close();
    }
    catch ( Exception &e ) {
      e.ADD("Unable to dump data "+ext,ERRDIV);
      throw e;
    }
    std::ostringstream out;
    out << "Dumping to file " << ext << " finished.";
    throw EXCEPTION(out.str(), ERRCOM);
  }
  else if ( token == "length" ) {
    std::string ext;
    try {
      ext = utils::readString(stream);
      std::ofstream file(ext,std::ios::out);
      if ( !file )
        throw EXCEPTION("Unable to open file "+ext,ERRABT);
      // Write header
      file << "# " << std::setw(20) << "Time step";
      for ( unsigned i = 0 ; i < _octahedra.size() ; ++ i ) {
        if ( _octahedra[i]->center() >= _natom ) continue;
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center() << std::setw(6) << " a";
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center() << std::setw(6) << " b";
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center() << std::setw(6) << " c";
      }
      file << std::endl;
      // Write data
      file.precision(14);
      file.setf(std::ios::scientific,std::ios::floatfield);
      file.setf(std::ios::right,std::ios::adjustfield);
#pragma omp for ordered, schedule(static)
      for ( int itime = _tbegin ; itime < _tend ; ++itime ) {
        Octahedra::u3f angles;
        const double *rprimd = _histdata->getRprimd(itime);
        const double *xcart = _histdata->getXcart(itime);
        for( auto& octa : _octahedra ){
          OctaLengths oa(*dynamic_cast<Octahedra*>(octa.get()));
          oa.build(rprimd,xcart,angles);
        }
        double a = 0;
        double b = 0;
        double c = 0;
        for ( auto& angle : angles ) {
          a += angle.second[0];
          b += angle.second[1];
          c += angle.second[2];
        }
        a /= angles.size();
        b /= angles.size();
        c /= angles.size();
        for ( auto& angle : angles ) {
          angle.second[0] -= a;
          angle.second[1] -= b;
          angle.second[2] -= c;
        }

#pragma omp ordered
        {
        file << std::setw(22) << itime;
        for ( unsigned i = 0 ; i < angles.size() ; ++i ) {
          file << std::setw(22) << angles[i].second[0];
          file << std::setw(22) << angles[i].second[1];
          file << std::setw(22) << angles[i].second[2];
        }
        file << std::endl;
        }
      }
      file.close();
    }
    catch ( Exception &e ) {
      e.ADD("Unable to dump data "+ext,ERRDIV);
      throw e;
    }
    std::ostringstream out;
    out << "Dumping to file " << ext << " finished.";
    throw EXCEPTION(out.str(), ERRCOM);
  }
  else if ( token == "local" || token == "loc" ) {
    std::string value;
    stream >> value;
    if ( stream.fail() )
      throw EXCEPTION("You need to specify rotations or lengths",ERRDIV);
    if ( value == "rot" || value == "rotation" ) 
      _view = ANGLES;
    else if ( value == "length" ) 
      _view = LENGTHS;
    else 
      throw EXCEPTION("Bad value. rotattions or lengths are allowed", ERRDIV);
    this->convertOctahedra();
  }
  else if ( token == "plot" || token == "print" || token == "data" ) {
    Graph::GraphSave save = Graph::GraphSave::NONE;
    if ( token == "print" )
      save = Graph::GraphSave::PRINT;
    else if ( token == "data" )
      save = Graph::GraphSave::DATA;

    try {
      if ( _gplot == nullptr ) 
        _gplot.reset(new Gnuplot);

      _gplot->setWinTitle(_info);
    }
    catch ( Exception &e ) {
      e.ADD("Unable to plot with gnuplot.\nInstead, writing data.", ERRWAR);
      std::cerr << e.fullWhat() << std::endl;
      _gplot.reset(nullptr);
    }

    this->plot(_tbegin, _tend, stream,save);
  }
  else 
    CanvasPos::my_alter(token, stream);
}

void CanvasLocal::convertOctahedra() {
  if ( _octahedra.empty() ) return;
  switch ( _view ) {
    case ANGLES :
      for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
        OctaAngles *tmpocta = new OctaAngles(std::move(*_octahedra[i].get()));
        _octahedra[i].reset(tmpocta);
      }
      break;
    case LENGTHS :
      for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
        OctaLengths *tmpocta = new OctaLengths(std::move(*_octahedra[i].get()));
        _octahedra[i].reset(tmpocta);
      }
      break;
  }
}

std::array<double,3> CanvasLocal::getAverageRotations(unsigned itime) {
  if ( _histdata == nullptr ) 
    throw EXCEPTION("Load a file first",ERRDIV);
  else if ( _octahedra.size() == 0 )
    throw EXCEPTION("First select the octahedra",ERRDIV);

  std::array<double,3> average = {0};
  std::array<double,3> absAverage = {0};
  const double *rprimd0 = _histdata->getRprimd(0);
  const double *xcart0 = _histdata->getXcart(0);
  Octahedra::u3f angles;
  Octahedra::u3f angles0;
  const double *rprimd = _histdata->getRprimd(itime);
  const double *xcart = _histdata->getXcart(itime);
  //std::clog << "size is " << _octahedra.size() << std::endl;
  for( auto& octa : _octahedra ){
    if ( (unsigned) octa->center() >= _histdata->natom() ) continue;
    OctaAngles oa(*dynamic_cast<Octahedra*>(octa.get()));
    oa.buildCart(rprimd0,xcart0,angles0,_baseCart);
    oa.build(rprimd,xcart,angles);
  }
  std::sort(angles.begin(),angles.end(),[xcart](const std::pair< unsigned, std::array<float,3> >& e1, const std::pair< unsigned, std::array<float,3> >& e2){
      const unsigned iatom1 = e1.first;
      const unsigned iatom2 = e2.first;
      if ( std::abs(xcart[3*iatom1+2] - xcart[3*iatom2+2]) < 4 /*bohr*/) {
        if ( std::abs(xcart[3*iatom1+1] - xcart[3*iatom2+1]) < 4 /*bohr*/) {
          return xcart[3*iatom1+0] < xcart[3*iatom2+0];
        }
        else {
          return xcart[3*iatom1+1] < xcart[3*iatom2+1];
        }
      }
      else {
        return xcart[3*iatom1+2] < xcart[3*iatom2+2];
      }
    }
  );
  /*
  for ( auto octa:angles ) 
    std::clog << octa.first << " -> " << xcart[3*octa.first+0] << "  " << xcart[3*octa.first+1] << "  " << xcart[3*octa.first+2] << std::endl;
    //*/
  unsigned nxy = 1;
  unsigned nx = 1;
  bool findX = false;
  for ( unsigned iscan = 0 ; iscan < angles.size()-1 ; ++iscan ) {
    if ( !findX && std::abs(xcart[3*angles[iscan].first+1]-xcart[3*angles[iscan+1].first+1]) < 4 /*bohr*/ ) {
      nx++;
    }
    else findX = true;
    if ( std::abs(xcart[3*angles[iscan].first+2]-xcart[3*angles[iscan+1].first+2]) < 4 /*bohr*/ ) {
      nxy++;
    }
    else break;
  }
  unsigned ny = nxy/nx;
  unsigned nz = angles.size()/nxy;
  //std::clog << nx << " " << ny << " " << nz << std::endl;

  double evenAv[3][2] = {0};
  for ( unsigned iz = 0 ; iz < nz ; ++iz ) {
    for ( unsigned iy = 0 ; iy < ny ; ++iy ) {
      for ( unsigned ix = 0 ; ix < nx ; ++ix ) {
        double tmpAngle[3];
        for ( unsigned a = 0 ; a < 3 ; ++a ) {
          const double tmp = angles[ix+(iy+iz*ny)*nx].second[a];
          absAverage[a] += std::abs(tmp);
          tmpAngle[a] = tmp;
        }
        if ( (iy+iz)%2 == 0 ) evenAv[0][ix%2==0?0:1] += tmpAngle[0];
        else evenAv[0][ix%2==0?1:0] += tmpAngle[0];
        if ( (iz+ix)%2 == 0 ) evenAv[1][iy%2==0?0:1] += tmpAngle[1];
        else evenAv[0][iy%2==0?1:0] += tmpAngle[1];
        if ( (ix+iy)%2 == 0 ) evenAv[2][iz%2==0?0:1] += tmpAngle[2];
        else evenAv[0][iz%2==0?1:0] += tmpAngle[2];
      }
    }
  }
  
  for ( unsigned a = 0 ; a < 3 ; ++a )
    average[a] = (evenAv[a][0]*evenAv[a][1] > 0) ? // +rotation
      absAverage[a]/(nx*ny*nz)
      :-absAverage[a]/(nx*ny*nz);

  return average;
}


bool CanvasLocal::baseCart() const {
  return _baseCart;
}

const float* CanvasLocal::octacolor() const {
  return _octacolor;
}

CanvasLocal::LocalView CanvasLocal::view() const {
  return _view;
}

void CanvasLocal::resetBase() {
  if ( _histdata == nullptr ) return;
  const double *rprimd = _histdata->getRprimd(0);
  const double *xcart = _histdata->getXcart(0);
  Octahedra::u3f angles;

  if ( !_octahedra.empty() && typeid(_octahedra[0].get()) != typeid(OctaAngles*) ) {
    switch ( _view ) {
      case ANGLES :
        for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
          OctaAngles *tmpocta = dynamic_cast<OctaAngles*>(_octahedra[i].get());
          tmpocta->buildCart(rprimd,xcart,angles,_baseCart);
        }
        _orientations.resize(_octahedra.size());
        for ( unsigned i = 0 ; i <_octahedra.size() ; ++i ) {
          _orientations[i] = angles[i].second;
        }
        break;
      case LENGTHS :
        //for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
        //  OctaAngles *tmpocta = new OctaAngles(std::move(*_octahedra[i].get()));
        //  tmpocta->buildCart(rprimd,xcart,angles,false);
        //  OctaLengths *pushocta = new OctaLengths(std::move(*tmpocta));
        //  delete tmpocta;
        //  _octahedra[i].reset(pushocta);
        //}
        break;
    }
  }
}

void CanvasLocal::plot(unsigned tbegin, unsigned tend, std::istream &stream, Graph::GraphSave save) {
  std::string function;
  Graph::Config config;

  stream >> function;
  unsigned ntime = tend-tbegin;
  std::vector<double> &x = config.x;
  std::list<std::vector<double>> &y = config.y;
  //std::list<std::pair<std::vector<double>,std::vector<double>>> &xy = config.xy;
  std::list<std::string> &labels = config.labels;
  //std::vector<short> &colors = config.colors;
  std::string &filename = config.filename;
  std::string &xlabel = config.xlabel;
  std::string &ylabel = config.ylabel;
  std::string &title = config.title;
  bool &doSumUp = config.doSumUp;

  std::string line;
  size_t pos = stream.tellg();
  std::getline(stream,line);
  stream.clear();
  stream.seekg(pos);
  ConfigParser parser;
  parser.setSensitive(true);
  parser.setContent(line);

  try {
    std::string tunit = parser.getToken<std::string>("tunit");
    if ( tunit == "fs" ) {
      xlabel = "Time [fs]";
      x.resize(ntime);
      for ( unsigned i = tbegin ; i < tend ; ++i ) x[i-tbegin]=_histdata->getTime(i)*phys::atu2fs;
    }
    else if ( tunit == "step" ) {
      xlabel = "Time [step]";
      x.resize(ntime);
      for ( unsigned i = tbegin ; i < tend ; ++i ) x[i-tbegin]=i;
    }
    else {
      throw EXCEPTION("Unknow time unit, allowed values fs and step",ERRDIV);
    }
  }
  catch (Exception &e) {
    xlabel = "Time [step]";
    x.resize(ntime);
    for ( unsigned i = tbegin ; i < tend ; ++i ) x[i-tbegin]=i;
  }

  // rotations
  if ( function == "rotations" || function == "rot" ) {
    filename = "Rotations";
    ylabel = "Rotations [degree]";
    title = "Rotations";
    std::clog << std::endl << " -- Average rotations --" << std::endl;

    std::vector<double> alpha(ntime);
    std::vector<double> beta(ntime);
    std::vector<double> gamma(ntime);
    Exception e;

#pragma omp parallel for schedule(static), shared(e)
    for ( unsigned itime = tbegin ; itime < tend ; ++itime ) {
      try {
        std::array<double,3> angles = this->getAverageRotations(itime);
        alpha[itime-tbegin] = angles[0];
        beta[itime-tbegin] = angles[1];
        gamma[itime-tbegin] = angles[2];
      }
      catch( Exception &ee ) {
#pragma omp critical(getAverage)
        e += ee;
      }
    }
    if ( e.getReturnValue() != 0 ) {
      e.ADD("Unable to plot average rotations",ERRABT);
      throw e;
    }
    y.push_back(std::move(alpha));
    y.push_back(std::move(beta));
    y.push_back(std::move(gamma));
    labels.push_back("alpha");
    labels.push_back("beta");
    labels.push_back("gamma");
    doSumUp = true;
  }
  else {
    CanvasPos::plot(tbegin,tend,stream,save);
    return;
  }
  config.save = save;
  try {
    filename = parser.getToken<std::string>("output");
  }
  catch (Exception &e) {
    filename = utils::noSuffix(this->info())+std::string("_")+filename;
  }
  Graph::plot(config,_gplot.get());
  if ( _gplot != nullptr )
    _gplot->clearCustom();
  stream.clear();
}

//
void CanvasLocal::help(std::ostream &out) {
  using std::endl;
  using std::setw;
  out << endl << "-- Here are the commands related to local mode --" << endl;
  out <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  out << setw(40) << ":basis (cart|rel)" << setw(59) << "Set the basis for angle calculation. cart for cartesian and rel for relative to first time step." << endl;
  out << setw(40) << ":c or :color (plus|minus)" << setw(59) << "Set the color in RGB for plus or minus rotations." << endl;
  out << setw(40) << ":length filename" << setw(59) << "Dump for each octaheadra the a b and c lengths in filename." << endl;
  out << setw(40) << ":rot filename" << setw(59) << "Dump for each octaheadra the alpha, beta and gamma angles in filename." << endl;
  out << setw(49) << ":loc or :local (rotatation|length)" << setw(59) << "Select which proppertie of the octahedra is displayed" << endl;
  out << setw(49) << ":plot (rotatations|rot)" << setw(59) << "Plot average rotations of the cell taking care of the sign" << endl;
  out << "Commands from positions mode are also available." << endl;
}
