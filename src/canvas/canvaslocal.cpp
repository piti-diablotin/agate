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
#include <iomanip>

//
CanvasLocal::CanvasLocal(bool drawing) : CanvasPos(drawing),
  _octacolor(),
  _cube(_opengl),
  _view(ANGLES),
  _orientations()
{
  _octacolor[0] = 1.f;
  _octacolor[1] = 0.f;
  _octacolor[2] = 0.f;
  _octacolor[3] = 0.f;
  _octacolor[4] = 0.f;
  _octacolor[5] = 1.f;
  ;
}

//
CanvasLocal::CanvasLocal(CanvasPos &&canvas) : CanvasPos(std::move(canvas)),
  _octacolor(),
  _cube(_opengl),
  _view(ANGLES),
  _orientations()
{
  _octacolor[0] = 1.f;
  _octacolor[1] = 0.f;
  _octacolor[2] = 0.f;
  _octacolor[3] = 0.f;
  _octacolor[4] = 0.f;
  _octacolor[5] = 1.f;

  const double *rprimd = _histdata->getRprimd(0);
  const double *xcart = _histdata->getXcart(0);
  Octahedra::u3f angles;

  if ( !_octahedra.empty() && typeid(_octahedra[0].get()) != typeid(OctaAngles*) ) {
    switch ( _view ) {
      case ANGLES :
        for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
          OctaAngles *tmpocta = new OctaAngles(std::move(*_octahedra[i].get()));
          tmpocta->buildCart(rprimd,xcart,angles);
          _octahedra[i].reset(tmpocta);
        }
        break;
      case LENGTHS :
        for ( unsigned i = 0 ; i < _octahedra.size() ; ++i ) {
          OctaAngles *tmpocta = new OctaAngles(std::move(*_octahedra[i].get()));
          tmpocta->buildCart(rprimd,xcart,angles);
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
  const float width = (float)mendeleev::radius[_octahedra_z];

  const float factor = ( _view == ANGLES ? 1.f/15.f : 1.f/0.10f ); CanvasPos::drawCell(); 

  this->buildBorders(_itime);

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
          glScalef(width,width,width);
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
  if ( z > 0 && (_natom+_onBorders.size()) > 6 && _hasTranslations) {
    _octahedra_z = z;
    _octahedra.clear();
    std::vector<double> xcartTotal((_natom+_onBorders.size())*3);
    const double *histXcart = _histdata->getXcart(0);
    const double *xred = _histdata->getXred(0);
    const double *rprimd = _histdata->getRprimd(0);

    this->buildBorders(0);
    std::copy(&histXcart[0],&histXcart[_natom*3],&xcartTotal[0]);
    std::copy(&_xcartBorders[0],&_xcartBorders[_onBorders.size()*3],&xcartTotal[_natom*3]);

    try {
      Octahedra::u3f angles;
      switch ( _view ) {
        case ANGLES :
          for ( unsigned iatom = 0 ; iatom < _natom+_onBorders.size() ; ++iatom ){
            if ( _znucl[_typat[iatom]] == _octahedra_z ) {
              OctaAngles *tmpocta = new OctaAngles(iatom,_natom,xred,&xcartTotal[0],rprimd,_opengl); //Construct octahedra based on xcart at time 0
              tmpocta->buildCart(rprimd,histXcart,angles);
              _octahedra.push_back(std::unique_ptr<Octahedra>(tmpocta));
            }
          }
          break;
        case LENGTHS :
          for ( unsigned iatom = 0 ; iatom < _natom+_onBorders.size() ; ++iatom ){
            if ( _znucl[_typat[iatom]] == _octahedra_z ) {
              OctaAngles *tmpocta = new OctaAngles(iatom,_natom,xred,&xcartTotal[0],rprimd,_opengl); //Construct octahedra based on xcart at time 0
              tmpocta->buildCart(rprimd,histXcart,angles);
              OctaLengths *pushocta = new OctaLengths(std::move(*tmpocta));
              _octahedra.push_back(std::unique_ptr<Octahedra>(pushocta));
            }
          }
          break;
      }

      _orientations.resize(_octahedra.size());
      for ( unsigned i = 0 ; i <_octahedra.size() ; ++i ) {
        _orientations[i] = angles[i].second;
      }
    }
    catch (Exception &e) {
      e.ADD("Abording construction of octahedra",ERRDIV);
      throw e;
    }
  }
  else if ( z == -1 || z == 0) {
    _octahedra.clear();
    _octahedra_z = -1;
  }
  else{
    throw EXCEPTION("Not enough data to build octahedra",ERRDIV);
  }
}

//
void CanvasLocal::my_alter(std::string token, std::istringstream &stream) {
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
  else if ( token == "rot" ) {
    std::string ext;
    try {
      stream >> ext;
      std::ofstream file(ext,std::ios::out);
      if ( !file )
        throw EXCEPTION("Unable to open file "+ext,ERRABT);
      // Write header
      file << "# " << std::setw(20) << "Time step";
      for ( unsigned i = 0 ; i < _octahedra.size() ; ++ i ) {
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center() << std::setw(6) << " alpha";
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center() << std::setw(6) << " beta";
        file << std::setw(9) << "atom " << std::setw(7) << _octahedra[i]->center() << std::setw(6) << " gamma";
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
          OctaAngles oa(*dynamic_cast<Octahedra*>(octa.get()));
          oa.build(rprimd,xcart,angles);
        }

#pragma omp ordered
        {
        file << std::setw(22) << itime;
        for ( unsigned i = 0 ; i <_octahedra.size() ; ++i ) {
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
      stream >> ext;
      std::ofstream file(ext,std::ios::out);
      if ( !file )
        throw EXCEPTION("Unable to open file "+ext,ERRABT);
      // Write header
      file << "# " << std::setw(20) << "Time step";
      for ( unsigned i = 0 ; i < _octahedra.size() ; ++ i ) {
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
        for ( unsigned i = 0 ; i <_octahedra.size() ; ++i ) {
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
    if ( value == "rot" || value == "rotations" ) 
      _view = ANGLES;
    else if ( value == "lengths" ) 
      _view = LENGTHS;
    else 
      throw EXCEPTION("Bad value. rotattions or lengths are allowed", ERRDIV);
    this->convertOctahedra();
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

//
void CanvasLocal::help(std::ostream &out) {
  using std::endl;
  using std::setw;
  out << endl << "-- Here are the commands related to local mode --" << endl;
  out <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  out << setw(40) << ":c or :color (plus|minus)" << setw(59) << "Set the color in RGB for plus or minus rotations." << endl;
  out << setw(40) << ":length filename" << setw(59) << "Dump for each octaheadra the a b and c lengths in filename." << endl;
  out << setw(40) << ":rot filename" << setw(59) << "Dump for each octaheadra the alpha, beta and gamma angles in filename." << endl;
  out << setw(49) << ":loc or :local (rotatations|lengths)" << setw(59) << "Select which proppertie of the octahedra is displayed" << endl;
  out << "Commands from positions mode are also available." << endl;
}
