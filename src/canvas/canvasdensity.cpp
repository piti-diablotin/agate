/**
 * @file src/./canvasdensity.cpp
 *
 * @brief 
 *
 * @author Jordan Bieder <jordan.bieder@cea.fr>
 *
 * @copyright Copyright 2018 Jordan Bieder
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


#include "canvas/canvasdensity.hpp"

//
CanvasDensity::CanvasDensity(bool drawing) : CanvasPos(drawing),
  _origin(0),
  _npoints(1),
  _ipoint(0),
  _ibegin(0),
  _iend(-1),
  _scaleValues(1.),
  _normal(AbiBin::gridDirection::A),
  _density(),
  _map(_opengl),
  _colors{ {0.,0.,1}, {1.,1.,1}, {1.,0.,0.} },
  _dispDen(AbiBin::getDen::SUM),
  _scaleFunction(sqrt)
{
  _nLoop = -2;
}

CanvasDensity::CanvasDensity(CanvasPos &&canvas) : CanvasPos(std::move(canvas)),
  _origin(0),
  _npoints(0),
  _ipoint(0),
  _ibegin(0),
  _iend(-1),
  _scaleValues(1.),
  _normal(AbiBin::gridDirection::A),
  _density(),
  _map(_opengl),
  _colors{ {0.,0.,1}, {1.,1.,1}, {1.,0.,0.} },
  _dispDen(AbiBin::getDen::SUM),
  _scaleFunction(sqrt)
{
  _nLoop = -2;
  if ( _histdata == nullptr ) return;
  try {
    _density.readFromFile(_histdata->filename());
    this->setData();
  }
  catch ( Exception &e ) {
    e.ADD("Need a density or potential file",ERRDIV);
    std::clog << e.fullWhat() << std::endl;
  }
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
  try {
    _density.readFromFile(_histdata->filename());
    this->setData();
  }
  catch ( Exception &e ) {
    e.ADD("Need a density or potential file",ERRDIV);
    std::clog << e.fullWhat() << std::endl;
  }
}

void CanvasDensity::refresh(const geometry::vec3d &cam, TextRender &render) {
  if ( _histdata == nullptr ) return;
  CanvasPos::refresh(cam,render);
  CanvasPos::drawCell();
  if ( _npoints == 0 ) return;

  std::vector<double> values;
  auto normal = _density.getVector(_normal);
  if ( _status != PAUSE ) {
    double renorm = _density.getData(_ipoint,_normal,_dispDen,values);
    double shift=0;
    if ( values.size() > 0 ) {
      //std::clog << _scaleFunction << std::endl;
      switch (_scaleFunction) {
        case linear : {
            for ( auto& v : values ) v *= _scaleValues*renorm;
            break;
          }
        case log: {
            shift = (_dispDen==AbiBin::DIFF?2:1);
            for ( auto& v : values ) {v = std::log(shift+_scaleValues*v*renorm)/std::log(3);}
            break;
          }
        case sqrt: {
            if (_dispDen==AbiBin::DIFF)
              for ( auto& v : values ) v= _scaleValues*(v<0.?-1.:1.)*std::sqrt(std::abs(v*renorm));
            else
              for ( auto& v : values ) v= _scaleValues*std::sqrt(std::abs(v*renorm));
            break;
          }
      }
      if (_dispDen==AbiBin::DIFF && _scaleFunction != log)
        for ( auto& v : values ) v = (v+1)*0.5;
    }
  }
#ifdef HAVE_GL
  const double *rprimd = _histdata->getRprimd(_itime);
  const GLfloat fx[] = {(GLfloat)rprimd[0], (GLfloat)rprimd[3], (GLfloat)rprimd[6]};
  const GLfloat fy[] = {(GLfloat)rprimd[1], (GLfloat)rprimd[4], (GLfloat)rprimd[7]};
  const GLfloat fz[] = {(GLfloat)rprimd[2], (GLfloat)rprimd[5], (GLfloat)rprimd[8]};
  for ( int i = 0 ; i < _translate[0] ; ++i) {
    const GLfloat fi = (GLfloat) i;
    const GLfloat xtrans[] = { fi*fx[0], fi*fx[1], fi*fx[2] };
    for ( int j = 0 ; j < _translate[1] ; ++j) {
      const GLfloat fj = (GLfloat) j;
      const GLfloat ytrans[] = { fj*fy[0]+xtrans[0], fj*fy[1]+xtrans[1], fj*fy[2]+xtrans[2] };
      for ( int k = 0 ; k < _translate[2] ; ++k ) {
        const GLfloat fk = (GLfloat) k;
        const GLfloat ztrans[] = { fk*fz[0]+ytrans[0], fk*fz[1]+ytrans[1], fk*fz[2]+ytrans[2] };
        glPushMatrix();
        glTranslatef(ztrans[0],ztrans[1],ztrans[2]);
        GLfloat scale = ((GLfloat)_ipoint/(GLfloat)_npoints);
        glTranslatef((GLfloat)normal[0]*scale,(GLfloat)normal[1]*scale,(GLfloat)normal[2]*scale);
        try {
          _map.draw(values,_colors[1],_colors[2],_colors[0],_status!=PAUSE);
        }
        catch ( Exception &e ) {
          std::cerr << e.fullWhat() << std::endl;
        }
        glPopMatrix();
      }
    }
  }
#endif
}

void CanvasDensity::setNTime(int ntime) {
  if ( _npoints == 0 ) {
    CanvasPos::setNTime(ntime);
    return;
  }
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

AbiBin::gridDirection CanvasDensity::normal() const
{
    return _normal;
}

AbiBin::getDen CanvasDensity::dispDen() const {
    return _dispDen;
}

CanvasDensity::scaleFunc CanvasDensity::scaleFunction() const {
    return _scaleFunction;
}

double CanvasDensity::scaleValues() const
{
    return _scaleValues;
}

const float (*CanvasDensity::colors() const )[3] {
  return _colors;
}

void CanvasDensity::my_alter(std::string token, std::istringstream &stream) {
  bool update = false;

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
    int tag = stream.tellg();
    std::string name;
    stream >> name;
    if ( name == "up" ) tomodify = _colors[2];
    else if ( name == "down" ) tomodify = _colors[0];
    else if ( name == "zero" ) tomodify = _colors[1];
    else {
      stream.seekg(tag);
      CanvasPos::my_alter(token,stream);
    }
    if ( tomodify != nullptr ) {
      stream >> c[0] >> c[1] >> c[2];
      if ( !stream.fail() && c[0] < 256 && c[1] < 256 && c[2] < 256 ) {
        tomodify[0] = (float)c[0]/255.f;
        tomodify[1] = (float)c[1]/255.f;
        tomodify[2] = (float)c[2]/255.f;
      }
      else throw EXCEPTION("Bad color numbers",ERRDIV);
      update = true;
    }
  }
  else if ( token == "n" || token == "normal" ){
    char c;
    stream >> c;
    if ( c == 'a' ) _normal = AbiBin::gridDirection::A;
    else if ( c == 'b' ) _normal = AbiBin::gridDirection::B;
    else if ( c == 'c' ) _normal = AbiBin::gridDirection::C;
    else throw EXCEPTION("Don't know what to do",ERRDIV);
    this->setData();
  }
  else if ( token == "density" ){
    std::string den;
    stream >> den;
    auto backup = _dispDen;
    if ( den == "up" ) _dispDen = AbiBin::getDen::UP;
    else if ( den == "down" ) _dispDen = AbiBin::getDen::DOWN;
    else if ( den == "sum" ) _dispDen = AbiBin::getDen::SUM;
    else if ( den == "diff" ) _dispDen = AbiBin::getDen::DIFF;
    else if ( den == "x" ) _dispDen = AbiBin::getDen::X;
    else if ( den == "y" ) _dispDen = AbiBin::getDen::Y;
    else if ( den == "z" ) _dispDen = AbiBin::getDen::Z;
    else throw EXCEPTION("density unknown",ERRDIV);
    switch(_density.getNspden()) {
      case 1: {
                if ( _dispDen!= AbiBin::getDen::SUM )  {
                  _dispDen = backup;
                  throw EXCEPTION("Only one (the total #sum) density to display",ERRDIV);
                }
                break;
              }
      case 2: {
                if ( _dispDen == AbiBin::getDen::X || _dispDen == AbiBin::getDen::Y || _dispDen == AbiBin::getDen::Z )  {
                  _dispDen = backup;
                  throw EXCEPTION("Only collinear densities (#sum #diff #up #down) to display",ERRDIV);
                }
                break;
              }
      case 4: {
                if ( _dispDen == AbiBin::getDen::DIFF || _dispDen == AbiBin::getDen::UP || _dispDen == AbiBin::getDen::DOWN ) {
                  _dispDen = backup;
                  throw EXCEPTION("Only total and projected densities (#sum #x #y #z) to display",ERRDIV);
                }
                break;
              }
    }
    update = true;
  }
  else if ( token == "scale" ){
    try {
      std::string func = parser.getToken<std::string>("function");
      if ( func == "linear" ) _scaleFunction = linear;
      else if ( func == "log" ) _scaleFunction = log;
      else if ( func == "sqrt" ) _scaleFunction = sqrt;
      else throw EXCEPTION("unknow function "+func,ERRDIV);
      update = true;
    }
    catch ( Exception &e ) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND )
        throw e;
    }
    try {
      _scaleValues = parser.getToken<double>("factor");
      update = true;
    }
    catch ( Exception &e ) {
      if ( e.getReturnValue() != ConfigParser::ERFOUND )
        throw e;
    }
  }
  else if ( token == "show" || token == "hide" ) {
    CanvasPos::my_alter(token,stream);
  }
  if ( update && _status == PAUSE ) _status = UPDATE;
}

void CanvasDensity::help(std::ostream &out) {
  using std::endl;
  using std::setw;
  out << endl << "-- Here are the commands related to density mode --" << endl;
  out <<         "   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   " << endl;
  out << setw(40) << ":c or :color (zero|up|down)" << setw(59) << "Set the color in RGB for zero plus(max) or minus(negative min) numbers." << endl;
  out << setw(40) << ":n or :normal (a|b|c)" << setw(59) << "Set the normal to the displayed density plan." << endl;
  out << setw(40) << ":density (up|down|sum|diff|x|y|z)" << setw(59) << "Display up or down density or alternatively the sum or difference or projected density (non collinera)" << endl;
  out << setw(40) << ":scale function (linear|log|sqrt) factor F" << setw(59) << "Use a function for the color scale and eventually scales the result by a factor F to improve contrast" << endl;
  out << "Commands from positions mode also available : show, hide, color" << endl;
}
