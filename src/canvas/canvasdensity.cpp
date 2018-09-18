/**
 * @file src/./canvasdensity.cpp
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


#include "canvas/canvasdensity.hpp"

//
CanvasDensity::CanvasDensity(bool drawing) : CanvasPos(drawing),
  _origin(0),
  _npoints(1),
  _ipoint(0),
  _ibegin(0),
  _iend(-1),
  _normal(AbiBin::gridDirection::A),
  _density(),
  _map(_opengl),
  _colors{ {0.,0.,1}, {1.,1.,1}, {1.,0.,0.} },
  _dispDen(AbiBin::getDen::SUM)
{
  _nLoop = -2;
}

CanvasDensity::CanvasDensity(CanvasPos &&canvas) : CanvasPos(std::move(canvas)),
  _origin(0),
  _npoints(1),
  _ipoint(0),
  _ibegin(0),
  _iend(-1),
  _normal(AbiBin::gridDirection::A),
  _density(),
  _map(_opengl),
  _colors{ {0.,0.,1}, {1.,1.,1}, {1.,0.,0.} },
  _dispDen(AbiBin::getDen::SUM)
{
  _nLoop = -2;
  if ( _histdata == nullptr ) return;
  _density.readFromFile(_histdata->filename());
  this->setData();
}

//
CanvasDensity::~CanvasDensity() {
  ;
}

void CanvasDensity::clear() {
  CanvasPos::clear();
  _origin = 0;
  _npoints = 0;
  _ipoint = 0;
}

void CanvasDensity::setData() {
  try {
    _origin = 0;
    _npoints = _density.getPoints(_normal); 

    AbiBin::gridDirection udir, vdir;
    switch(_normal) { 
      case AbiBin::gridDirection::A :{
                                       udir = AbiBin::gridDirection::B;
                                       vdir = AbiBin::gridDirection::C;
                                       break;
                                     }
      case AbiBin::gridDirection::B :{
                                       udir = AbiBin::gridDirection::C;
                                       vdir = AbiBin::gridDirection::A;
                                       break;
                                     }
      case AbiBin::gridDirection::C :{
                                       udir = AbiBin::gridDirection::A;
                                       vdir = AbiBin::gridDirection::B;
                                       break;
                                     }
    }
    auto uvec = _density.getVector(udir);
    auto vvec = _density.getVector(vdir);
    int upoints = _density.getPoints(udir);
    int vpoints = _density.getPoints(vdir);
    double orig[] = {0,0,0};
    _map.genUnit(orig,&uvec[0],&vvec[0],upoints,vpoints);
  }
  catch ( Exception &e ) {
    std::clog << e.fullWhat() << std::endl;
    this->clear();
  }
  this->setNTime(1);
  if ( _status == PAUSE ) _status = UPDATE;
}

void CanvasDensity::setHist(HistData& hist) {
  CanvasPos::setHist(hist);
  if ( _histdata == nullptr ) return;
  _density.readFromFile(_histdata->filename());
  this->setData();
}

void CanvasDensity::refresh(const geometry::vec3d &cam, TextRender &render) {
  CanvasPos::refresh(cam,render);
  CanvasPos::drawCell();

  std::vector<double> values;
  auto normal = _density.getVector(_normal);
  if ( _status != PAUSE ) {
    _density.getData(_ipoint,_normal,values);
    if ( values.size() > 0 ) {
      for ( auto& v : values ) v=(v<0.?-1.:1.)*std::sqrt(std::abs(v));
    }
  }
  GLfloat scale = ((GLfloat)_ipoint/(GLfloat)_npoints);
  glTranslatef((GLfloat)normal[0]*scale,(GLfloat)normal[1]*scale,(GLfloat)normal[2]*scale);
  try {
    _map.draw(values,_colors[1],_colors[0],_colors[2],_status!=PAUSE);
  }
  catch ( Exception &e ) {
    std::cerr << e.fullWhat() << std::endl;
  }
}

void CanvasDensity::setNTime(int ntime) {
  if ( ntime == 0 ) return;
  if ( ntime != 1 ) 
    throw EXCEPTION("ntime should be equal to 1 in density mode!",ERRDIV);
  if ( _ibegin >= _npoints )
    throw EXCEPTION("time_begin is to large : should be <"+utils::to_string(_npoints),ERRDIV);
  else if ( _ibegin < 0 )
    throw EXCEPTION("time_begin negative not allowed",ERRDIV);

  if ( _iend == -1 ) _iend = _npoints;
  else if ( _iend <= _ibegin )
    throw EXCEPTION("time_end < time_begin is not allowed",ERRDIV);
  else if ( _iend > _npoints ) {
    _iend = _npoints;
    Exception e = EXCEPTION("Setting time_end to last time:"+utils::to_string(ntime),ERRWAR);
    std::clog << e.fullWhat() << std::endl;
  }
  if ( _ipoint < _ibegin ) {
    _ipoint = _ibegin;
    if ( _status == PAUSE ) _status = UPDATE;
  }
}

void CanvasDensity::nextFrame(const int count) {
  if ( _status == START ) {
    _ipoint += _dir*count;
    if ( _ipoint >= _iend ) {
      if ( ++_iLoop < _nLoop || _nLoop == -2 || _nLoop == -1 ) {
        _ipoint = ( _nLoop == -2 ? _iend-2 : _ibegin );
        if ( _nLoop == -2 ) _dir = -1;
      }
      else {
        _status = PAUSE;
        _ipoint = _iend-1;
      }
    }
    else if ( _ipoint <= _ibegin ) {
      _ipoint = _ibegin;
      _dir = 1;
    }
  }
}

void CanvasDensity::nextStep() {
  if ( ++_ipoint >= _iend ) --_ipoint;
  _status = UPDATE;
}

void CanvasDensity::step(const int istep) {
  if ( (_ipoint=istep) >= _iend ) _ipoint=_iend-1;
  if ( _ipoint < _ibegin ) _ipoint=_ibegin;
  _status = UPDATE;
}

void CanvasDensity::previousStep() {
  if ( --_ipoint == -1 || _ipoint < _ibegin) _ipoint = _ibegin;
  _status = UPDATE;
}

int CanvasDensity::itime() const { return _ipoint; }

int CanvasDensity::ntime() const { return _npoints; }

int CanvasDensity::tbegin() const { return _ibegin; }

int CanvasDensity::tend() const { return _iend-1; }

void CanvasDensity::my_alter(std::string token, std::istringstream &stream) {
  if ( token == "c" || token == "color" ){
    unsigned c[3];
    float *tomodify = nullptr;
    std::string name;
    stream >> name;
    if ( name == "up" ) tomodify = _colors[2];
    else if ( name == "down" ) tomodify = _colors[0];
    else if ( name == "zero" ) tomodify = _colors[1];
    else throw EXCEPTION("Don't know what to do",ERRDIV);
    stream >> c[0] >> c[1] >> c[2];
    if ( !stream.fail() && c[0] < 256 && c[1] < 256 && c[2] < 256 ) {
      tomodify[0] = (float)c[0]/255.f;
      tomodify[1] = (float)c[1]/255.f;
      tomodify[2] = (float)c[2]/255.f;
    }
    else throw EXCEPTION("Bad color numbers",ERRDIV);
    if ( _status == PAUSE ) _status = UPDATE;
  }
  else if ( token == "n" || token == "normal" ){
    char c;
    stream >> c;
    if ( c == 'x' ) _normal = AbiBin::gridDirection::A;
    else if ( c == 'y' ) _normal = AbiBin::gridDirection::B;
    else if ( c == 'z' ) _normal = AbiBin::gridDirection::C;
    else throw EXCEPTION("Don't know what to do",ERRDIV);
    this->setData();
  }
}

void CanvasDensity::help(std::ostream &out) {
  using std::endl;
  using std::setw;
  out << endl << "-- Here are the commands related to local mode --" << endl;
  out <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  out << setw(40) << ":c or :color (plus|minus)" << setw(59) << "Set the color in RGB for plus or minus rotations." << endl;
  out << "Commands from positions mode are also available." << endl;
}
